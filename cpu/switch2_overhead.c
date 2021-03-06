#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <numeric>
#include <algorithm>
using namespace std;
#include <sys/time.h>
#include <time.h>

#define get_start_time(t1,t2,elap) 	struct timeval t1, t2; \
					double elap; \
					gettimeofday(&t1, NULL);
#define get_end_time(t1,t2,elap)	gettimeofday(&t2, NULL); \
					elap = (t2.tv_usec - t1.tv_usec); \
					printf("Elapsed time = %fus\n",elap);

#define ITERATIONS 10
#define READ_OH 32	// Obtained from previous measurement

int compare(const void *, const void *);
void get_elapsed_time(double *);

int main( int argc, const char* argv[] ){

    int i, num_args;
    double timings[ITERATIONS];

    get_elapsed_time(timings);

    double sum = accumulate(timings, timings+ITERATIONS, 0);
    printf("Sum:\t%f\n",sum);

    double max = (double)*max_element(timings, timings+ITERATIONS);
    double min = (double)*min_element(timings, timings+ITERATIONS);
    double range = max-min;

    double avg = sum/ITERATIONS;

    qsort (timings, ITERATIONS, sizeof(long), compare);
    for (i = 0; i < ITERATIONS; i++) {
        printf("Iteration %d: process creation overhead = %f us\n", i, timings[i]);
    }

    printf("Max:\t%f\nMin:\t%f\nRange:\t%f\n",max,min,range);
    printf("avg: %f cycles\n", avg);
    printf("med: %f cycles\n", (double)timings[ITERATIONS/2]);
    
    return 0;
}

int compare (const void * a, const void * b) 
{
    return ( *(int*)a - *(int*)b );
}

void get_elapsed_time(double *timings) {
    unsigned long start, end;
    unsigned int i;
    register unsigned cycles_low0, cycles_high0, cycles_low1, cycles_high1;
    pid_t pid;
    int pipefd [2];
    char buff;
    struct timeval t1, t2; 
    double elap;

    for (i = 0; i < ITERATIONS; i++) {

	    /* code to measure */
	    if ((pid = fork()) == -1) {
		    perror("[!] failed fork");
		    exit(1);
	    } else if (pid > 0) {
			
		    //asm volatile ("cpuid\n\t" "rdtsc\n\t" "mov %%edx, %0\n\t" "mov %%eax, %1\n\t" : "=r" (cycles_high0), "=r" (cycles_low0) :: "%rax", "%rbx", "%rcx", "%rdx");
		    gettimeofday(&t1, NULL);
		    wait(NULL);
		    read(pipefd[0], &buff, 1);

	    } else {
		    //asm volatile ("cpuid\n\t" "rdtsc\n\t" "mov %%edx, %0\n\t" "mov %%eax, %1\n\t" : "=r" (cycles_high1), "=r" (cycles_low1) :: "%rax", "%rbx", "%rcx", "%rdx");
		    gettimeofday(&t2, NULL);
		    write(pipefd[1], "a", 1);

		    exit(0);
	    }

	    elap = (t2.tv_usec - t1.tv_usec);
	    //start = cycles_high0;
	    //end = cycles_high1;
            //start = ( (start << 32) | cycles_low0);
            //end = ( (end << 32) | cycles_low1);
	    if (elap>0)
        	timings[i] = elap;
	    else {
		printf("elap<0\n");
		timings[i] = -elap;
	    }
    }
    
}


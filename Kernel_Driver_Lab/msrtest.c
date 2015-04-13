#include "msrdrv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


const unsigned long totalPoints = 64*1048576;

double generateRandomPoint(void)
{
	double generatedRand;
	generatedRand = ((double)rand() / (double)RAND_MAX)*2.0 - 1.0;
	return generatedRand;
}

double calculatePi(int num_in_circ, int total)
{
	return (4.0f * ((double)num_in_circ/(double)total));
}

double getDistanceFromOrigin(double x, double y)
{
	//printf("dist : %lf\n", (sqrt(x*x + y*y)));
	return (sqrt(x*x + y*y));

}

double calc_pi(void)
{
	
	/* Seed for Random */
	srand((unsigned)time(NULL));
	/* Variables for Profiling */
	clock_t start_time, end_time;
	printf("%s\n", "Estimating Pi Using Monte-Carlo Method\n");
	/* Profiling Start */
	start_time = clock();
	/* Point Counter */
	unsigned long inCircle = 0;
	/* Iterator for Monte-Carlo-Pi Estimation */
	unsigned long iterationCounter;
	for (iterationCounter = 0; iterationCounter < totalPoints; iterationCounter++)
	{
		/* Generate two random points
		(posx, posy) = ([-1.0,+1,0],[-1.0,+1.0] */
		double positionX = generateRandomPoint();
		double positionY = generateRandomPoint();
		/* Print every generated point */
		//fprintf(stdout, "Generated Point: (x,y) = (%f,%f)\n", positionX, positionY);
		/* Get the distance */
		double distanceFromOrigin = getDistanceFromOrigin(positionX, positionY);
		/* Accumuate point counter */
		if (distanceFromOrigin < 1.0)
			inCircle++;
	}
	/* Caculate Pi */
	double estimatedPi = calculatePi(inCircle, totalPoints);
	/* Print the result */
	printf("Pi is estimated as: %f\n", estimatedPi);
	/* End of profiling */
	end_time = clock();
	/* Print profiling result */
	printf("Time : %f\n", ((double)(end_time - start_time)) / CLOCKS_PER_SEC);
	/* Wait for user to key the hit */
	printf("Calculation has been finished.\n\n");
	//getch();
	/* Non-anomalous termination */
	return 0;
}




static int loadDriver()
{
    int fd;
    fd = open("/dev/" DEV_NAME, O_RDWR);
    if (fd == -1) {
        perror("Failed to open /dev/" DEV_NAME);
    }
    return fd;
}

static void closeDriver(int fd)
{
    int e;
    e = close(fd);
    if (e == -1) {
        perror("Failed to close fd");
    }
}

/*
 * Reference:
 * Intel Software Developer's Manual Vol 3B "253669.pdf" August 2012
 * Intel Software Developer's Manual Vol 3C "326019.pdf" August 2012
 */
int main(void)
{
    int fd;
    struct MsrInOut msr_start[] = {
        { MSR_WRITE, 0x38f, 0x00, 0x00 },       // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        { MSR_WRITE, 0xc1, 0x00, 0x00 },        // ia32_pmc0: zero value (35-5)
        { MSR_WRITE, 0xc2, 0x00, 0x00 },        // ia32_pmc1: zero value (35-5)
        { MSR_WRITE, 0xc3, 0x00, 0x00 },        // ia32_pmc2: zero value (35-5)
        { MSR_WRITE, 0xc4, 0x00, 0x00 },        // ia32_pmc3: zero value (35-5)
		{ MSR_WRITE, 0x309, 0x00, 0x00 },       // ia32_fixed_ctr0: zero value (35-17) - Instruction Retired
        { MSR_WRITE, 0x30a, 0x00, 0x00 },       // ia32_fixed_ctr1: zero value (35-17) - CPU Clock UnHalted.Core
        { MSR_WRITE, 0x30b, 0x00, 0x00 },       // ia32_fixed_ctr2: zero value (35-17) - CPU Clock UnHalted.Ref
        { MSR_WRITE, 0x186, 0x00414f2e, 0x00 }, // ia32_perfevtsel0, Longest Latency Cache Reference.
        { MSR_WRITE, 0x187, 0x0041412e, 0x00 }, // ia32_perfevtsel1, Longest Latency Cache Miss.
        { MSR_WRITE, 0x188, 0x0041003c, 0x00 }, // ia32_perfevtsel2, UnHalted Core Cycles.
        { MSR_WRITE, 0x189, 0x004100c5, 0x00 }, // ia32_perfevtsel3, Branch Misses Retired.
        { MSR_WRITE, 0x38d, 0x333, 0x00 },      // ia32_perf_fixed_ctr_ctrl: ensure 3 FFCs enabled
        { MSR_WRITE, 0x38f, 0x0f, 0x07 },       // ia32_perf_global_ctrl: enable 4 PMCs & 3 FFCs
        { MSR_STOP, 0x00, 0x00 }
    };

    struct MsrInOut msr_stop[] = {
        { MSR_WRITE, 0x38f, 0x00, 0x00 },       // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        { MSR_WRITE, 0x38d, 0x00, 0x00 },       // ia32_perf_fixed_ctr_ctrl: clean up FFC ctrls
        { MSR_READ, 0xc1, 0x00 },               // ia32_pmc0: read value (35-5)
        { MSR_READ, 0xc2, 0x00 },               // ia32_pmc1: read value (35-5)
        { MSR_READ, 0xc3, 0x00 },               // ia32_pmc2: read value (35-5)
        { MSR_READ, 0xc4, 0x00 },               // ia32_pmc3: read value (35-5)
        { MSR_READ, 0x309, 0x00 },              // ia32_fixed_ctr0: read value (35-17)
        { MSR_READ, 0x30a, 0x00 },              // ia32_fixed_ctr1: read value (35-17)
        { MSR_READ, 0x30b, 0x00 },              // ia32_fixed_ctr2: read value (35-17)
        { MSR_STOP, 0x00, 0x00 }
    };

	struct MsrInOut msr_read_eax[] = {
		{ MSR_READ_EAX }
	};

	struct MsrInOut msr_read_ecx[] = {
		{ MSR_READ_ECX }
	};

	struct MsrInOut msr_read_edx[] = {
		{ MSR_READ_EDX }
	};

	struct MsrInOut read_tsc[] = {
		{ MSR_RDTSC }
	};

	printf("\n=============================================\n");
    fd = loadDriver();
	printf("\nDriver has been loaded!\n\n");
	printf("Before starting instructions :\n");
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_eax);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_ecx);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_edx);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)read_tsc);
	printf("eax : %016llx, ecx : %016llx, edx : %016llx\n", msr_read_eax[0].value, msr_read_ecx[0].value, msr_read_edx[0].value);
	printf("time stamp : %016lld\n\n", read_tsc[0].value);
	long long ts_start = read_tsc[0].value;
    ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_start);
	printf("=============================================\n");
	printf("Performance Monitoring Unit has been reset and started.\n\n");
    printf("Now calculating PI value by Monte-Carlo Method with n=2^26\n\n");
	double pi_val = calc_pi();
    ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_stop);
	printf("Performance Monitoring Unit has been stopped.\n\n");
	printf("=============================================\n");
	printf("After executed instructions :\n");
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_eax);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_ecx);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_read_edx);
	ioctl(fd, IOCTL_MSR_CMDS, (long long)read_tsc);
	long long ts_end = read_tsc[0].value;
	printf("eax : %016llx, ecx : %016llx, edx : %016llx\n", msr_read_eax[0].value, msr_read_ecx[0].value, msr_read_edx[0].value);
	printf("time stamp : %016lld\n\n", read_tsc[0].value);
	printf("time stamp difference : %016lld\n\n", (ts_end - ts_start));
	printf("=============================================\n");
	printf("Results :\n");
    printf("Longest Latency Cache Reference:    %7lld\n", msr_stop[2].value);	// Original : uops_retired
    printf("Longest Latency Cache Miss:         %7lld\n", msr_stop[3].value);	// Original : uops_issued
    printf("UnHalted Core Cycles:               %7lld\n", msr_stop[4].value);	// Original : stalled cycles
    printf("Branch Misses Retired:              %7lld\n", msr_stop[5].value);	// Original : resource stalls
    printf("Instruction Retired:                %7lld\n", msr_stop[6].value);	
    printf("CPU Clock UnHalted - Core:          %7lld\n", msr_stop[7].value);
    printf("CPU Clock UnHalted - Ref:           %7lld\n\n", msr_stop[8].value);
	printf("=============================================\n");
    closeDriver(fd);
    return 0;
}

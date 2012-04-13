//PPU code 

#include <sched.h>
#include <libspe2.h>
#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#include <math.h>
#include <time.h>
#include <altivec.h>
#include <ppu_intrinsics.h>
#include <libspe2.h>
#include <stdlib.h>

#include "common.h"
#include <sys/time.h>


//pointer to spe code
extern spe_program_handle_t spe_code;

// handle which will be returned bu spe_context_create
spe_context_ptr_t speid;

// SPU entry point address which is initially set to default
unsigned int entry = SPE_DEFAULT_ENTRY;

//used to return data regaring abnormal return from SPE
spe_stop_info_t stop_info;



// one iteration, used for calculations
particle_Data particle_Array_PPU[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));

// used for saving and printing in one shot later
typedef struct 
{
	// no need to be alligned, this is no being sent to SPUs
	particle_Data particleArray[PARTICLES_MAXCOUNT];

}
iterationData;

// MEGA ARRAY OF ALL DATA, entire simulation
iterationData fullSimilationData[ITERATION_COUNT];

int fullDataCounter = 0;



/// subdivisions of arrays for each spe
particle_Data spe1_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe2_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe3_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe4_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe5_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));
particle_Data spe6_Data[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));


particle_Data tempParticleArray[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));


//__vector double zeroVector = {0,0};
__vector unsigned int oneVector = {1,1,1,1};
__vector unsigned int axisBitShiftMask = {0,1,2,0};
__vector unsigned short resetOctantCount = {0,0,0,0,0,0,0};
__vector unsigned short increment = {1,1,1,1,1,1,1,1};
__vector unsigned short octantCount;


int speNumber = 0;
//particle_Data* speData;
int i;
// based off http://www.ibm.com/developerworks/library/pa-libspe2/
/* NOTE -- the prototype is based on the standard pthread thread signature */
void *spe_code_launch_1(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe1_Data, 1, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_2(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe2_Data, 2, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_3(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe3_Data, 3, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_4(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe4_Data, 4, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_5(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe5_Data, 5, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

void *spe_code_launch_6(void *data) 
{
//	printf("inside of thread function\n");
	int retval;
	unsigned int entry_point = SPE_DEFAULT_ENTRY; /* Required for continuing 
      execution, SPE_DEFAULT_ENTRY is the standard starting offset. */
	spe_context_ptr_t my_context;
//	printf("before creating context\n");
	/* Create the SPE Context */
	my_context = spe_context_create(SPE_EVENTS_ENABLE|SPE_MAP_PS, NULL);
//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe6_Data, 6, NULL);
	} 
	while (retval > 0); /* Run until exit or error */
	spe_context_destroy(my_context);	
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	


// setup, assign particles initla positions and masses
// this is done in scalar fashion, NOT SIMD
// insignificant to performance since it's only done once

	//time_t startTime = time(NULL);

	printf("Sun Mass: %f\n",sunMass );
	printf("Earth Mass: %f\n",earthMass );

	double mult = sunMass * earthMass * GRAVITATIONALCONSTANT;
	
	printf("Result: %f\n", mult );



	//seed random generator
	srand( time(NULL) );

	printf("\n\n\n~~~~~~~~Printing out particles and their randomly assigned positions: \n\n");

	int pC = 0;
	for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	{
		int grideSize = GRID_SIZE;

	//	printf("\n grideSize/2: %d", grideSize/2);

		double xPos = (double)( rand() % grideSize  - grideSize/2);
		double yPos = (double)( rand() % grideSize  - grideSize/2);
		double zPos = (double)( rand() % grideSize  - grideSize/2);

		particle_Array_PPU[pC].positionXY[0] = xPos;
		particle_Array_PPU[pC].positionXY[1] = yPos;
		particle_Array_PPU[pC].positionZ[0] = zPos;

		particle_Array_PPU[pC].positionZ[2] = PARTICLES_DEFAULTMASS;

		if(pC == 0)
		{
			// center, high mass

			particle_Array_PPU[pC].positionZ[1] = sunMass;


		}
		if(pC == 1)
		{


		}

		//particle_Array_PPU[pC].position = vec_splat(particle_Array_PPU[pC].position, 1);
		//particle_Array_PPU[pC].position = vec_splats((double)GRAVITATIONALCONSTANT); --> use splats, seems faster
		
		printf("Particle %d:   ", pC );
		printf("x= %f, y=%f, z=%f , mass:%f", particle_Array_PPU[pC].positionXY[0], particle_Array_PPU[pC].positionXY[1], particle_Array_PPU[pC].positionZ[0], particle_Array_PPU[pC].velocityZ[1]);
		printf("\n");
		
	}


	// copy arrays into spe ones
	pC = 0;
	for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	{

		spe1_Data[pC] = particle_Array_PPU[pC];	
		spe2_Data[pC] = particle_Array_PPU[pC];	
		spe3_Data[pC] = particle_Array_PPU[pC];	
		spe4_Data[pC] = particle_Array_PPU[pC];	
		spe5_Data[pC] = particle_Array_PPU[pC];	
		spe6_Data[pC] = particle_Array_PPU[pC];		
	}


	i=0;


	int speCount = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES,-1);


	struct timeval start;
	gettimeofday(&start,NULL);


	int iterCount = 0;
	for (iterCount = 0; iterCount< ITERATION_COUNT; iterCount++)
	{

		//printf("++++++++++++++ START of ITERATION # %d of %d +++++++++++++++\n", i, ITERATION_COUNT );

		int retval;
		pthread_t spe1_Thread;
		pthread_t spe2_Thread;
		pthread_t spe3_Thread;
		pthread_t spe4_Thread;
		pthread_t spe5_Thread;
		pthread_t spe6_Thread;


		//speData = spe1_Data;
		speNumber = 0;
		/* Create Thread */
	//	printf("spe1_Data value: %d\n", (int)spe1_Data );
		retval = pthread_create(&spe1_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_1, // Thread function
								NULL // Thread argument
								);

	//	printf("spe2_Data value: %d\n", (int)spe2_Data );
		
		retval = pthread_create(&spe2_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_2, // Thread function
								NULL // Thread argument
								);
		
		
		retval = pthread_create(&spe3_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_3, // Thread function
								NULL // Thread argument
								);

		
		retval = pthread_create(&spe4_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_4, // Thread function
								NULL // Thread argument
								);

		retval = pthread_create(&spe5_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_5, // Thread function
								NULL // Thread argument
								);

		retval = pthread_create(&spe6_Thread, // Thread object
								NULL, // Thread attributes
								spe_code_launch_6, // Thread function
								NULL // Thread argument
								);
		


		//Wait for Thread Completion
		retval = pthread_join(spe1_Thread, NULL);


		retval = pthread_join(spe2_Thread, NULL);

		
		retval = pthread_join(spe3_Thread, NULL);

		retval = pthread_join(spe4_Thread, NULL);
		
		retval = pthread_join(spe5_Thread, NULL);
		
		retval = pthread_join(spe6_Thread, NULL);
		

		
		speNumber = 1;
		
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
		{
			particle_Array_PPU[i] = spe1_Data[i];
		}

		speNumber = 2;
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
		{
			particle_Array_PPU[i] = spe2_Data[i];
		}

		speNumber = 3;
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
		{
			particle_Array_PPU[i] = spe3_Data[i];
		}

		speNumber = 4;
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
		{
			particle_Array_PPU[i] = spe4_Data[i];
		}

		speNumber = 5;
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<speNumber*PARTICLES_MAXCOUNT/SPU_COUNT; ++i)
		{
			particle_Array_PPU[i] = spe5_Data[i];
		}

		speNumber = 6;
		for(i=(speNumber-1)*PARTICLES_MAXCOUNT/SPU_COUNT; i<PARTICLES_MAXCOUNT; ++i)
		{
			particle_Array_PPU[i] = spe6_Data[i];
		}

		// reset spe counter
		speNumber = 0;
		


		// copy arrays into spe ones
		pC = 0;
		for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
		{

			spe1_Data[pC] = particle_Array_PPU[pC];	
			spe2_Data[pC] = particle_Array_PPU[pC];	
			spe3_Data[pC] = particle_Array_PPU[pC];	
			spe4_Data[pC] = particle_Array_PPU[pC];	
			spe5_Data[pC] = particle_Array_PPU[pC];	
			spe6_Data[pC] = particle_Array_PPU[pC];	


			
			/*		
			printf("Particle %d positions:   ", pC );
			printf("x= %f, y=%f, z=%f , mass:%f", particle_Array_PPU[pC].position[0], particle_Array_PPU[pC].position[1], particle_Array_PPU[pC].position[2], particle_Array_PPU[pC].velocity[3]);
			printf("\n");
			*/


			fullSimilationData[iterCount].particleArray[pC]= particle_Array_PPU[pC];
		}

		

	//	printf("++++++++++++++ END of ITERATION # %d of %d +++++++++++++++\n", iterCount, ITERATION_COUNT );


	}

	struct timeval end;
	gettimeofday(&end,NULL);
	double deltaTime = ((end.tv_sec - start.tv_sec)*1000.0f + (end.tv_usec -start.tv_usec)/1000.0f);


	printf("print out values from post spe calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f , mass:%f", particle_Array_PPU[i].positionXY[0], particle_Array_PPU[i].positionXY[1], particle_Array_PPU[i].positionZ[0], particle_Array_PPU[i].velocityZ[1]);
		printf("\n");
	
	}-
	
	printf("Calculations Execution time:    %f\n",deltaTime);


	FILE *filePointer;
	filePointer = fopen("fileLog1.txt","w");	

	iterCount = 0;
	for (iterCount = 0; iterCount< ITERATION_COUNT; iterCount++)
	{

		fprintf(filePointer,"\n");

		pC = 0;
	    for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	    {
	
			fprintf(filePointer,"%f,",fullSimilationData[iterCount].particleArray[pC].positionXY[0]);
			fprintf(filePointer,"%f,",fullSimilationData[iterCount].particleArray[pC].positionXY[1]);
			fprintf(filePointer,"%f",fullSimilationData[iterCount].particleArray[pC].positionZ[0]);

			fprintf(filePointer,"|");
	

		}
	}

	fclose(filePointer);


	return 0;
}



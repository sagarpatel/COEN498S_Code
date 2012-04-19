/*
Sagar Patel
9356037


 Copyright (C) 2012  Sagar Patel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/



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
iterationData fullSimilationData[fullDataCount];

/*
// data for individual spu
particle_Data spe1_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
particle_Data spe2_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
particle_Data spe3_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
particle_Data spe4_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
particle_Data spe5_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
particle_Data spe6_Data[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));
*/
typedef struct 
{
	particle_Data positionArray[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));

}
singleSPEData;

singleSPEData fullSPEData[6];


__vector float zeroVector = {0,0,0,0};
__vector unsigned int oneVector = {1,1,1,1};
__vector unsigned int axisBitShiftMask = {0,1,2,0};
__vector unsigned short resetOctantCount = {0,0,0,0,0,0,0};
__vector unsigned short increment = {1,1,1,1,1,1,1,1};
__vector unsigned short octantCount;

__vector float initPositionVector = {5,0,0,0};
__vector float initialVelocityVector_X = {0.005f, 0, 0, PARTICLES_DEFAULTMASS};
__vector float initialVelocityVector_Y = {0, 0.05f, 0, PARTICLES_DEFAULTMASS};
__vector float initialVelocityVector_Y_minus = {0, -0.05f, 0, PARTICLES_DEFAULTMASS};

// mass scale down is ^-5



int speNumber = 0;



spe_context_ptr_t contextPointerSPE1;
spe_context_ptr_t contextPointerSPE2;
spe_context_ptr_t contextPointerSPE3;
spe_context_ptr_t contextPointerSPE4;
spe_context_ptr_t contextPointerSPE5;
spe_context_ptr_t contextPointerSPE6;


int contextReadySPE1 = 0;
int contextReadySPE2 = 0;
int contextReadySPE3 = 0;
int contextReadySPE4 = 0;
int contextReadySPE5 = 0;
int contextReadySPE6 = 0;


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
	contextPointerSPE1 = my_context;
	contextReadySPE1 = 1;

	//printf("FROM LAUNCHER my_contextSPE1: %x\n", my_context );

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[0].positionArray, 1, NULL);
		contextReadySPE1 = 1;
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
	contextPointerSPE2 = my_context;
	contextReadySPE2 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[1].positionArray, 2, NULL);
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
	contextPointerSPE3 = my_context;
	contextReadySPE3 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[2].positionArray, 3, NULL);
		contextReadySPE3 = 1;
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
	contextPointerSPE4 = my_context;
	contextReadySPE4 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[3].positionArray, 4, NULL);
		contextReadySPE4 = 1;
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
	contextPointerSPE5 = my_context;
	contextReadySPE5 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[4].positionArray, 5, NULL);
		contextReadySPE5 = 1;
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
	contextPointerSPE6 = my_context;
	contextReadySPE6 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, fullSPEData[5].positionArray, 6, NULL);
		contextReadySPE6 = 1;
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

	printf("DMA Count %d\n", DMA_COUNT );

	time_t startTime = time(NULL);





	i=0;



	int speCount = spe_cpu_info_get(SPE_COUNT_PHYSICAL_SPES,-1);
/*
	printf("\n");
	printf("%d", speCount);

	printf("\n");
	printf("\n");
	printf("--------------\n");
	printf("Starting spe1 part\n");
*/
/*
	// wait for user input, gives time to start graphics
	printf("Press Enter to continue\n");

	getchar();
*/

	struct timeval start;
	gettimeofday(&start,NULL);


	int iterCount = 0;


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
	



	// wait for all SPEs to be started
	while(contextReadySPE1 == 0 || contextReadySPE2 == 0 || contextReadySPE3 == 0 || contextReadySPE4 == 0 || contextReadySPE5 == 0 || contextReadySPE6 == 0 )
	{
		printf("Waiting for all start\n");
	}



	unsigned int speMsgArray[6];
	int speCounter = 0;	

	while( (speMsgArray[0] != KILLOPCODE) && (speMsgArray[1] != KILLOPCODE) && (speMsgArray[2] != KILLOPCODE) && (speMsgArray[3] != KILLOPCODE) && (speMsgArray[4] != KILLOPCODE) && (speMsgArray[5] != KILLOPCODE) )
	{

		printf("In main while\n");
		
		// from http://www.ibm.com/developerworks/power/library/pa-tacklecell2/index.html
		spe_out_mbox_read(contextPointerSPE1, &speMsgArray[0], 1);
		spe_out_mbox_read(contextPointerSPE2, &speMsgArray[1], 1);
		spe_out_mbox_read(contextPointerSPE3, &speMsgArray[2], 1);
		spe_out_mbox_read(contextPointerSPE4, &speMsgArray[3], 1);
		spe_out_mbox_read(contextPointerSPE5, &speMsgArray[4], 1);
		spe_out_mbox_read(contextPointerSPE6, &speMsgArray[5], 1);

		printf("After all reads\n");

		for(speCounter = 0; speCounter < SPU_COUNT ; speCounter++ )
		{
			int opCode = (int)speMsgArray[speCounter];

			printf("OPCODE %d\n", opCode );

			if( (opCode != 0) && (opCode != KILLOPCODE) )
			{

				int arrayCounter = 0;
				int startIndex = (int)opCode * PARTICLE_DMA_MAX;
				int stopIndex = ((int)opCode * PARTICLE_DMA_MAX) + PARTICLE_DMA_MAX;

				printf("opCode: %d\t startIndex: %d\n", opCode, startIndex);

				printf("Before entering transcription loop\n");
				for(arrayCounter = startIndex; arrayCounter < stopIndex; arrayCounter ++)
				{
					// array counter should represent iteration index
					// speCounter should be partcle id
					printf("Inside transcription loop: %d\n", arrayCounter);
					fullSimilationData[arrayCounter].particleArray[speCounter] = fullSPEData[speCounter].positionArray[arrayCounter]; // copy iterations of single body


				}
				printf("After transcription loop\n");

			}
			

		}



	}
	//test = 0;




	// reset spe counter
	speNumber = 0;
	


/*
	time_t endTime = time(NULL);
	int deltaTime = endTime - startTime;
*/

	// need to look into http://www.xmlsoft.org/


	struct timeval end;
	gettimeofday(&end,NULL);
	float deltaTime = ((end.tv_sec - start.tv_sec)*1000.0f + (end.tv_usec -start.tv_usec)/1000.0f);


	printf("Execution time:    %f\n",deltaTime);


	FILE *filePointer;
	filePointer = fopen("fileLog1.txt","w");
	//fprintf(filePointer, "<SimulationData>\n");
	

	iterCount = 0;
	int pC = 0;
	for (iterCount = 0; iterCount< ITERATION_COUNT; iterCount++)
	{
		//printf("Iteration: %d\n", iterCount);
		//fprintf(filePointer,"<Iter>\n");
		fprintf(filePointer,"\n");

		pC = 0;
	    for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	    {
		
			//printf("Particle %d positions:   ", pC );
		//	fprintf(filePointer,"<Obj>\n");
	    	

			//printf("x= %f, y=%f, z=%f", fullSimilationData[iterCount].particleArray[pC].position[0], fullSimilationData[iterCount].particleArray[pC].position[1], fullSimilationData[iterCount].particleArray[pC].position[2]);
			//printf("\n");
			
	    	/*
			fprintf(filePointer,"<PX>%f</PX>\n",fullSimilationData[iterCount].particleArray[pC].position[0]);
			fprintf(filePointer,"<PY>%f</PY>\n",fullSimilationData[iterCount].particleArray[pC].position[1]);
			fprintf(filePointer,"<PZ>%f</PZ>\n",fullSimilationData[iterCount].particleArray[pC].position[2]);
			*/

			fprintf(filePointer,"%f,",fullSimilationData[iterCount].particleArray[pC].position[0]);
			fprintf(filePointer,"%f,",fullSimilationData[iterCount].particleArray[pC].position[1]);
			fprintf(filePointer,"%f",fullSimilationData[iterCount].particleArray[pC].position[2]);

			fprintf(filePointer,"|");
			//fprintf(filePointer,"</Obj>\n");			
			//fullSimilationData[fullDataCounter].particleArray[pC]= particle_Array_PPU[pC];
			
		}

		//fprintf(filePointer,"</Iter>\n");


	}


	//fprintf(filePointer, "</SimulationData>\n");


	fclose(filePointer);


	return 0;
}



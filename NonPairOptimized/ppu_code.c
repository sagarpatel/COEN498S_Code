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
		retval = spe_context_run(my_context, &entry_point, 0, spe1_Data, 1, NULL);
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
	contextPointerSPE3 = my_context;
	contextReadySPE3 = 1;

//	printf("context created\n");
	/* Load the embedded code into this context */
	spe_program_load(my_context, &spe_code);
//	printf("program loaded\n");
	/* Run the SPE program until completion */
	do 
	{	
		retval = spe_context_run(my_context, &entry_point, 0, spe3_Data, 3, NULL);
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
		retval = spe_context_run(my_context, &entry_point, 0, spe4_Data, 4, NULL);
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
		retval = spe_context_run(my_context, &entry_point, 0, spe5_Data, 5, NULL);
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
		retval = spe_context_run(my_context, &entry_point, 0, spe6_Data, 6, NULL);
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

	time_t startTime = time(NULL);

	//seed random generator
	srand( time(NULL) );

	printf("\n\n\n~~~~~~~~Printing out particles and their randomly assigned positions: \n\n");

	int pC = 0;
	for(pC = 0; pC < PARTICLES_MAXCOUNT; ++pC)
	{
		int grideSize = GRID_SIZE;

	//	printf("\n grideSize/2: %d", grideSize/2);

		float xPos = (float)( rand() % grideSize  - grideSize/2);
		float yPos = (float)( rand() % grideSize  - grideSize/2);
		float zPos = (float)( rand() % grideSize  - grideSize/2);

		particle_Array_PPU[pC].position[0] = xPos;
		particle_Array_PPU[pC].position[1] = yPos;
		particle_Array_PPU[pC].position[2] = zPos;

		particle_Array_PPU[pC].velocity[3] = PARTICLES_DEFAULTMASS;

		if(pC == 0)
		{
			// center, high mass
			particle_Array_PPU[pC].position = zeroVector;
			particle_Array_PPU[pC].velocity = zeroVector; //initialVelocityVector_Y_minus;

			printf("Earth mass: %f\n", earthMass );
			particle_Array_PPU[pC].velocity[3] = earthMass; // PARTICLES_DEFAULTMASS * 500.0f;
		}

		else
		{



		}

		//particle_Array_PPU[pC].position = vec_splat(particle_Array_PPU[pC].position, 1);
		//particle_Array_PPU[pC].position = vec_splats((float)GRAVITATIONALCONSTANT); --> use splats, seems faster
		
		printf("Particle %d:   ", pC );
		printf("x= %f, y=%f, z=%f , mass:%f", particle_Array_PPU[pC].position[0], particle_Array_PPU[pC].position[1], particle_Array_PPU[pC].position[2], particle_Array_PPU[pC].velocity[3]);
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

	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{
     /////// INSERT QUADRANT CODE HERE , actually octant --> 8 equal sub cubes 
		
		// compare with zero vector to get on which side of each axis the particle is
		// 0 is negative, 1 is positive side of the axis
		__vector bool int axisDirection = vec_cmpgt(particle_Array_PPU[i].position, zeroVector);



		// need to manually set, can't cast due to size difference error
		__vector unsigned int shiftedAxis = { (unsigned int)axisDirection[0],
											  (unsigned int)axisDirection[1],
											  (unsigned int)axisDirection[2],
												0};
		// need to do this to revert 1s into NON 2s complement form --> vec_cmgt doc LIES
		shiftedAxis = vec_andc(oneVector, shiftedAxis);

		/*
		printf("Particle %d axis sign:   ", i );
		printf("x= %x, y=%x, z=%x", shiftedAxis[0], shiftedAxis[1], shiftedAxis[2]);
		printf("\n");
		*/

		// shift 3 axies simultaneously (actually only 2, 1 stays in origina positon
		//, with intent to OR them later
		shiftedAxis = vec_sl(shiftedAxis, axisBitShiftMask); // will also use as x vector

		__vector unsigned int axis_Y = vec_splats(shiftedAxis[1]);
		__vector unsigned int axis_Z = vec_splats(shiftedAxis[2]);
		// merge shhifted x y z values by OR-ing
		// this gives the octant id, range from 0-7 (000 to 111 in binary)
		shiftedAxis = vec_or(shiftedAxis, axis_Y);
		shiftedAxis = vec_or(shiftedAxis, axis_Z);
		// insert octant value into last slot of position vector of particle
		particle_Array_PPU[i].position[3] = (float)shiftedAxis[0];

		//printf("Oct ID: %d \n", shiftedAxis[0]);

		/////// Update octant vector by incrementing octant that the particle is in
		// The only possible non SIMD line in the entire program, 
		//irreleant since quadrant counting should occur on PPU anyways
		octantCount[shiftedAxis[0]] ++ ;
		
	}
	i=0;

	printf("\n");

		printf("Particle disttribution across the octants: \n");
		printf("O0: %d    O1: %d    O2: %d    O3: %d    O4: %d    O5: %d    O6: %d    O7: %d\n",
				octantCount[0], octantCount[1], octantCount[2], octantCount[3], 
				octantCount[4],	octantCount[5], octantCount[6], octantCount[7]);
		printf("\n");


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



	unsigned int test  = 0;
	printf("&test: %x\n", &test );
	unsigned int *testPtr = &test;

	while(test == 0)
	{
		
		// from http://www.ibm.com/developerworks/power/library/pa-tacklecell2/index.html
		printf("contexttextPointerSPE1: %x\n", contextPointerSPE1 );
		printf("contexttextPointerSPE2: %x\n", contextPointerSPE2 );
		printf("contexttextPointerSPE3: %x\n", contextPointerSPE3 );
		printf("contexttextPointerSPE4: %x\n", contextPointerSPE4 );
		printf("contexttextPointerSPE5: %x\n", contextPointerSPE5 );
		printf("contexttextPointerSPE6: %x\n", contextPointerSPE6 );


		printf("&test: %x\n", &test );
		printf("testPtr: %x\n", testPtr );
		spe_out_mbox_read(contextPointerSPE1, testPtr, 1);//, SPE_MBOX_ALL_BLOCKING );
		printf("After 1\n");
/*
		spe_out_mbox_read(contextPointerSPE2, &test, 1);
		printf("After 2\n");

		spe_out_mbox_read(contextPointerSPE3, &test, 1);
		printf("After 3\n");

		spe_out_mbox_read(contextPointerSPE4, &test, 1);
		printf("After 4\n");

		spe_out_mbox_read(contextPointerSPE5, &test, 1);
		printf("After 5\n");

		spe_out_mbox_read(contextPointerSPE6, &test, 1);
		printf("After 6\n");
		
*/

		printf("Test: %d\n", test );


	}
	//test = 0;


	printf("Out of while msg\n");

/*
	//Wait for Thread Completion
	retval = pthread_join(spe1_Thread, NULL);


	retval = pthread_join(spe2_Thread, NULL);

	
	retval = pthread_join(spe3_Thread, NULL);

	retval = pthread_join(spe4_Thread, NULL);
	
	retval = pthread_join(spe5_Thread, NULL);
	
	retval = pthread_join(spe6_Thread, NULL);
	*/

	printf("After thread join\n");

		
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


		//fullSimilationData[iterCount].particleArray[pC]= particle_Array_PPU[pC];
	}

	


	struct timeval end;
	gettimeofday(&end,NULL);
	float deltaTime = ((end.tv_sec - start.tv_sec)*1000.0f + (end.tv_usec -start.tv_usec)/1000.0f);


	printf("print out values from post spe calculations\n");
	i = 0;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{

		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f , mass:%f", particle_Array_PPU[i].position[0], particle_Array_PPU[i].position[1], particle_Array_PPU[i].position[2], particle_Array_PPU[i].velocity[3]);
		printf("\n");
	
	}
	//cleaining the array
	octantCount = resetOctantCount;
	for(i = 0; i<PARTICLES_MAXCOUNT; ++i)
	{
     /////// INSERT QUADRANT CODE HERE , actually octant --> 8 equal sub cubes 
		
		// compare with zero vector to get on which side of each axis the particle is
		// 0 is negative, 1 is positive side of the axis
		__vector bool int axisDirection = vec_cmpgt(particle_Array_PPU[i].position, zeroVector);



		// need to manually set, can't cast due to size difference error
		__vector unsigned int shiftedAxis = { (unsigned int)axisDirection[0],
											  (unsigned int)axisDirection[1],
											  (unsigned int)axisDirection[2],
												0};
		// need to do this to revert 1s into NON 2s complement form --> vec_cmgt doc LIES
		shiftedAxis = vec_andc(oneVector, shiftedAxis);

		/*
		printf("Particle %d axis sign:   ", i );
		printf("x= %x, y=%x, z=%x", shiftedAxis[0], shiftedAxis[1], shiftedAxis[2]);
		printf("\n");
		*/

		// shift 3 axies simultaneously (actually only 2, 1 stays in origina positon
		//, with intent to OR them later
		shiftedAxis = vec_sl(shiftedAxis, axisBitShiftMask); // will also use as x vector

		__vector unsigned int axis_Y = vec_splats(shiftedAxis[1]);
		__vector unsigned int axis_Z = vec_splats(shiftedAxis[2]);
		// merge shhifted x y z values by OR-ing
		// this gives the octant id, range from 0-7 (000 to 111 in binary)
		shiftedAxis = vec_or(shiftedAxis, axis_Y);
		shiftedAxis = vec_or(shiftedAxis, axis_Z);
		// insert octant value into last slot of position vector of particle
		particle_Array_PPU[i].position[3] = (float)shiftedAxis[0];

		//printf("Oct ID: %d \n", shiftedAxis[0]);

		/////// Update octant vector by incrementing octant that the particle is in
		// The only possible non SIMD line in the entire program, 
		//irreleant since quadrant counting should occur on PPU anyways
		octantCount[shiftedAxis[0]] ++ ;
		
	}
	i=0;

	printf("\n");

		printf("Particle disttribution across the octants: \n");
		printf("O0: %d    O1: %d    O2: %d    O3: %d    O4: %d    O5: %d    O6: %d    O7: %d\n",
				octantCount[0], octantCount[1], octantCount[2], octantCount[3], 
				octantCount[4],	octantCount[5], octantCount[6], octantCount[7]);
		printf("\n");



/*
	time_t endTime = time(NULL);
	int deltaTime = endTime - startTime;
*/

	// need to look into http://www.xmlsoft.org/


	printf("Execution time:    %f\n",deltaTime);


	FILE *filePointer;
	filePointer = fopen("fileLog1.txt","w");
	//fprintf(filePointer, "<SimulationData>\n");
	

	iterCount = 0;
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



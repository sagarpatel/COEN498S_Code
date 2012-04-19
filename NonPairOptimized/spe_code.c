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



// From http://www.ibm.com/developerworks/library/pa-libspe2/
// slightly modified

#include <stdio.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>

#include "common.h"


typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;


volatile particle_Data particle_Array_SPU[PARTICLES_MAXCOUNT] __attribute__((aligned(sizeof(particle_Data)*PARTICLES_MAXCOUNT)));


int main(unsigned long long spe_id, unsigned long long pdata, unsigned long long envp)
{
 
/*
  printf("Hello, World! (From SPU:%llx)\n",spe_id);

  printf("envp value: %d \n", (int)envp);
  printf("pdata value: %d\n", (int)pdata );
*/
///main loop

   unsigned int tag_id;

  /* Reserve a tag for application usage */
  if ((tag_id = mfc_tag_reserve()) == MFC_TAG_INVALID) 
  {
    printf("ERROR: unable to reserve a tag\n");
    return 1;
  }

  	/*
  	printf("particle_Array_SPU value %d \n", particle_Array_SPU);
  	printf("&particle_Array_SPU value %d \n", &particle_Array_SPU);
	*/

  	mfc_get(&particle_Array_SPU, pdata, sizeof(particle_Array_SPU),tag_id, 0, 0);
  	

  	//printf("after mfc_get\n");

  	mfc_write_tag_mask(1<<tag_id);

  	//wait for DMA and gurantee completion
  	mfc_read_tag_status_all();



  //	printf("%d\n", &particle_Array_SPU );
	
	//printf("after array address\n");

	// temp particle Datas used for calculations, not pointers, purposefully passed by value
	particle_Data pDi;
	particle_Data pDj;


	//temp vectors used for calculations in loop
	__vector float tempAcceleration = {0,0,0,0};
	__vector float tempVelocity = {0,0,0,0};
	__vector float tempDistance = {0,0,0,0}; //--> use 4th element to store radius
	__vector float tempDistanceRL1 = {0,0,0,0};
	__vector float tempDistanceRL2 = {0,0,0,0};

	__vector float tempNumerator = {0,0,0,0};
	__vector float tempMassSplat = {0,0,0,0};
	__vector float tempGConstant = {GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT };
	__vector float tempDELATTIME = {DELTA_TIME, DELTA_TIME, DELTA_TIME, DELTA_TIME};
	__vector float tempEPS= {EPS, EPS, EPS, EPS};

	__vector float zeroVector = {0,0,0,0};
	__vector unsigned int oneVector = {1,1,1,1};

	__vector unsigned int axisBitShiftMask = {0,1,2,0};


	__vector unsigned char yzxwMask = { 4,5,6,7, 8,9,10,11, 0,1,2,3,  12,13,14,15};
	__vector unsigned char zxywMask = { 8,9,10,11, 0,1,2,3, 4,5,6,7,  12,13,14,15};


	__vector float tempUnitVector = {0,0,0,0};
	__vector float distanceVector = {0,0,0,0};



	//stupid C99, need to declare indicies before for loops
	int i = 0;
	int j = 0;
	int it_counter = 0;

	float massSave;

//	printf("\n^^^^^^^   Now starting main loop of spe : %d \n\n\n", (int)envp);


	int startPoint = ((int)envp - 1) * PARTICLES_MAXCOUNT/SPU_COUNT;
	int endPoint = envp*PARTICLES_MAXCOUNT/SPU_COUNT;
	// to avoid out of bounds array
	if (endPoint > PARTICLES_MAXCOUNT)
		endPoint = PARTICLES_MAXCOUNT;

/*
	printf("startPoint: %d\n", startPoint );
	printf("endPoint: %d\n", endPoint );
	*/

	// this first loop is to calculate the forces/accelerations
	// NOTE ---> NO FORCES ARE APPLIED IN THIS LOOP, NO POSITIONS WILL BE CHANGED.
	// The calculated accelerations will be used to increment the particles velocity vector, NOT POSITION
	for(i = startPoint ; i< endPoint ; ++i)
	{

		//cache the particle data struct to the temp declared outside the loops
		pDi = particle_Array_SPU[i];
		massSave = pDi.velocity[3];

		for(j = 0; j<PARTICLES_MAXCOUNT; ++j)
		{

			//for every particle i, calculate for all j's
			// get resultant total velocity, don't apply it in these loops,
			// apply velocities for all bodies at the same time, in seperate loop at the end.

			//cache the particle data struct to the temp declared outside the loops
			pDj = particle_Array_SPU[j];
			/*
			if(pDj.velocity[3] < 1000.0f)
			{
				printf("Mass of particle: %d is %f \n", j, pDj.velocity[3] );
			}
			*/

			tempDistance = spu_sub(pDj.position,pDi.position); //actual distance vector between objects i and j
			
			// save value for unit vector calculation later
			distanceVector = tempDistance;


			/*
			 //Print distances between particles
			printf("Particle %d:   ", i );
			printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
			printf("\n");
			*/

			//use the distance vector  right now for numerator, before we overwrite is later in the code
			// use mass of subject mass

			//printf("Mass of particle: %d: %f\n",j, pDj.velocity[3] );

			tempMassSplat = spu_splats((float)pDj.velocity[3]); //mass is stored in the last element (3) of velocity vector
			tempNumerator = spu_madd(tempMassSplat, tempGConstant, zeroVector);
			

			/*
			//Print numerator
			printf("Numerator %d:   ", i );
			printf("x= %f, y=%f, z=%f", tempNumerator[0], tempNumerator[1], tempNumerator[2]);
			printf("\n");
			*/
			 
			 //Assembly for vector rotate
			//__asm__("addi    4,4,1;");

		// denominator part
			// sqaure each component, x,y,z beforehand
			tempDistance = spu_madd(tempDistance, tempDistance, zeroVector);

			//using perm instead of rotate, bleurg
			//tempDistanceRL1 = spu_perm(tempDistance, zeroVector, yzxwMask); // imitates lxfloat left rotate
			//tempDistanceRL2 = spu_perm(tempDistance, zeroVector, zxywMask); // imitates 2xfloat left rotate

			//nvm found spu function to rotate floats, using quadwords
			// rotate by multiple of 32 bits (sizeof float)
			// need to test
			//tempDistanceRL1 = spu_rlqw(tempDistance,1);
			//tempDistanceRL2 = spu_rlqw(tempDistance,2);
			
			///\\\\ doing it manually
			tempDistanceRL1[0] = tempDistance[1];
			tempDistanceRL1[1] = tempDistance[2];
			tempDistanceRL1[2] = tempDistance[0];

			tempDistanceRL2[0] = tempDistance[2];
			tempDistanceRL2[1] = tempDistance[0];
			tempDistanceRL2[2] = tempDistance[1];

			/*
			printf("tempDistance: %f %f %f \n", tempDistance[0], tempDistance[1], tempDistance[2] );
			printf("tempDistanceRL1: %f %f %f \n", tempDistanceRL1[0], tempDistanceRL1[1], tempDistanceRL1[2] );
			printf("tempDistanceRL2: %f %f %f \n\n", tempDistanceRL2[0], tempDistanceRL2[1], tempDistanceRL2[2] );
			*/
			
			//add both
			tempDistanceRL1 = spu_add(tempDistanceRL1, tempDistanceRL2);
			//add to original to get total ---> x+y+z
			tempDistance = spu_add(tempDistance, tempDistanceRL1); //tempDistance is now total distance squared
			
			// add EPS to avoid singularity
			tempDistance =  spu_add(tempDistance, tempEPS); //this is now the denominator value

			//save inverse magnitude for unit vector later
			tempUnitVector = spu_rsqrte(tempDistance);


			// invert vector to avoid division later
			tempDistance = spu_re(tempDistance); // this is final denominator (already inverted), only need to multiply
			// tempDistance is now eqivalent to 1/r^2 


			/*
			//Print denominator
			printf("Denominator %d:   ", i );
			printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
			printf("\n");
			*/

			//total acceleration applied to particle i, by particle j
			tempAcceleration = spu_madd(tempDistance, tempNumerator, zeroVector);
			
			// create unit vector
			tempUnitVector = spu_madd(distanceVector, tempUnitVector, zeroVector);
			
			// apply unit vector to acceleration
			tempAcceleration = spu_madd(tempUnitVector, tempAcceleration, zeroVector);

			//Print  accell
			/*
			if(pDi.velocity[3] != pDj.velocity[3])
			{
				printf("Acceleration applied on particle: %d : x= %f, y=%f, z=%f", i, tempAcceleration[0], tempAcceleration[1], tempAcceleration[2]);
				printf("\n");
			}
			*/

			//increment velocity value of particle with a*dt
			// need to explicitly call the array, since pDi is only a temp pass by value, doesn't change the particle
			particle_Array_SPU[i].velocity = spu_madd(tempAcceleration, tempDELATTIME, particle_Array_SPU[i].velocity);
			//restore mass in right position, in case
			particle_Array_SPU[i].velocity[3] = massSave;

			/*
			//Print velocity
			printf("Velocity %d:   ", i );
			printf("x= %f, y=%f, z=%f", particle_Array_SPU[i].velocity[0], particle_Array_SPU[i].velocity[1], particle_Array_SPU[i].velocity[2]);
			printf("\n");
			*/

			/*

			printf("Particle %d:   ", i );
			printf("x= %f, y=%f, z=%f", pDi.velocity[0], pDi.velocity[1], pDi.velocity[2]);
			printf("\n");

			*/
			
			//end of this loop
		}
		//printf("\n");
	}

	//now that all the accelerations for all particles are calculated,
	//apply them and update velocity 
	for(i = startPoint; i<endPoint; ++i)
	{
		//incrementing position with v*dt
		// spu_madd is awesome, it all gets done in one line! emulated the += operator, kinda, but more flexible
		particle_Array_SPU[i].position = spu_madd(particle_Array_SPU[i].velocity, tempDELATTIME, particle_Array_SPU[i].position);

		/*
		printf("Particle %d positions:   ", i );
		printf("x= %f, y=%f, z=%f", particle_Array_SPU[i].position[0], particle_Array_SPU[i].position[1], particle_Array_SPU[i].position[2]);
		printf("\n");
		*/
	

	}



/*
	printf("\n");
	printf("End of SPU #%d\n", (int)envp);
	printf("///////////////////\n\n");

*/
	//send back data
	mfc_put (&particle_Array_SPU, pdata, sizeof(particle_Array_SPU),tag_id, 0, 0);

    // wait for the DMA put to complete 
    mfc_write_tag_mask (1 << tag_id);
    mfc_read_tag_status_all ();




  	spu_write_out_mbox(123);
  	printf("Sent out message from: %d\n", (int)envp );

  	while(1);


  return (0);
}


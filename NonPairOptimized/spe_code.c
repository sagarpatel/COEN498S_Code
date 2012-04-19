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




volatile __vector float position_Array_SPU[PARTICLE_DMA_MAX] __attribute__((aligned(sizeof(particle_Data)*PARTICLE_DMA_MAX)));


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
  	printf("position_Array_SPU value %d \n", position_Array_SPU);
  	printf("&position_Array_SPU value %d \n", &position_Array_SPU);
	*/
/*
  	mfc_get(&position_Array_SPU, pdata, sizeof(position_Array_SPU),tag_id, 0, 0);
  	

  	//printf("after mfc_get\n");

  	mfc_write_tag_mask(1<<tag_id);

  	//wait for DMA and gurantee completion
  	mfc_read_tag_status_all();
*/


  //	printf("%d\n", &position_Array_SPU );
	
	//printf("after array address\n");



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

	
	particle_Data satData;
	particle_Data immuneData;

	
	immuneData.position = immuneBodyPosition;
	immuneData.velocity = immuneBodyVelocity;	


	int spuID = (int)envp;

	switch(spuID)
	{
		case 1:
			satData.position = sat1Position;
			satData.velocity = sat1Velocity;
			break;


		case 2:
			satData.position = sat2Position;
			satData.velocity = sat2Velocity;
			break;
			

		case 3:
			satData.position = sat3Position;
			satData.velocity = sat3Velocity;
			break;
			

		case 4:
			satData.position = sat4Position;
			satData.velocity = sat4Velocity;
			break;
			

		case 5:
			satData.position = sat5Position;
			satData.velocity = sat5Velocity;
			break;
			

		case 6:
			satData.position = sat6Position;
			satData.velocity = sat6Velocity;
			break;

	}
    

	tempMassSplat = spu_splats((float)immuneData.velocity[3]);

	int dmaCounter = 0;

	for(dmaCounter = 0; dmaCounter < DMA_COUNT; dmaCounter++)
	{
		int i = 0;

		for(i = 0; i < PARTICLE_DMA_MAX; i++)
		{


			tempDistance = spu_sub(immuneData.position,satData.position); //actual distance vector between objects i and j
			
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

			//printf("Mass of particle: %d: %f\n",j, immuneData.velocity[3] );

		//	tempMassSplat = spu_splats((float)immuneData.velocity[3]); //mass is stored in the last element (3) of velocity vector
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
			/*
			tempDistanceRL1[0] = tempDistance[1];
			tempDistanceRL1[1] = tempDistance[2];
			tempDistanceRL1[2] = tempDistance[0];

			tempDistanceRL2[0] = tempDistance[2];
			tempDistanceRL2[1] = tempDistance[0];
			tempDistanceRL2[2] = tempDistance[1];
			*/

			tempDistanceRL1 = spu_shuffle(tempDistance, zeroVector, yzxwMask); // imitates lxfloat left rotate
			tempDistanceRL2 = spu_shuffle(tempDistance, zeroVector, zxywMask); // imitates 2xfloat left rotate

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
			tempAcceleration = spu_mul(tempDistance, tempNumerator);
			
			// create unit vector
			tempUnitVector = spu_mul(distanceVector, tempUnitVector);
			
			// apply unit vector to acceleration
			tempAcceleration = spu_mul(tempUnitVector, tempAcceleration);

			//Print  accell
			/*
			if(satData.velocity[3] != immuneData.velocity[3])
			{
				printf("Acceleration applied on particle: %d : x= %f, y=%f, z=%f", i, tempAcceleration[0], tempAcceleration[1], tempAcceleration[2]);
				printf("\n");
			}
			*/

			/*
			printf("Acceleration applied on particle: %d : x= %f, y=%f, z=%f", i, tempAcceleration[0], tempAcceleration[1], tempAcceleration[2]);
			printf("\n");
			*/
			//increment velocity value of particle with a*dt
			// need to explicitly call the array, since satData is only a temp pass by value, doesn't change the particle
			//position_Array_SPU[i].velocity = spu_madd(tempAcceleration, tempDELATTIME, position_Array_SPU[i].velocity);
			//restore mass in right position, in case
			//position_Array_SPU[i].velocity[3] = massSave;


			satData.velocity = spu_madd(tempAcceleration, tempDELATTIME, satData.velocity);



			/*
			//Print velocity
			printf("Velocity %d:   ", i );
			printf("x= %f, y=%f, z=%f", position_Array_SPU[i].velocity[0], position_Array_SPU[i].velocity[1], position_Array_SPU[i].velocity[2]);
			printf("\n");
			*/

			/*

			printf("Particle %d:   ", i );
			printf("x= %f, y=%f, z=%f", satData.velocity[0], satData.velocity[1], satData.velocity[2]);
			printf("\n");

			*/

			//incrementing position with v*dt
			// spu_madd is awesome, it all gets done in one line! emulated the += operator, kinda, but more flexible
			//position_Array_SPU[i].position = spu_madd(position_Array_SPU[i].velocity, tempDELATTIME, position_Array_SPU[i].position);
			satData.position = spu_madd(satData.velocity, tempDELATTIME, satData.position);

			
			position_Array_SPU[i] = satData.position;

			/*
			printf("Particle %d positions:   ", i );
			printf("x= %f, y=%f, z=%f", position_Array_SPU[i].position[0], position_Array_SPU[i].position[1], position_Array_SPU[i].position[2]);
			printf("\n");
			*/

		}



				//send back data
		mfc_put (&position_Array_SPU, pdata, sizeof(position_Array_SPU),tag_id, 0, 0);

	    // wait for the DMA put to complete 
	    mfc_write_tag_mask (1 << tag_id);
	    mfc_read_tag_status_all ();




	  	spu_write_out_mbox(dmaCounter);
	  //	printf("Sent out message from: %d\n", (int)envp );

	 // 	unsigned int waitSig = spu_read_in_mbox(); // wait for okay from PPU

  }


	printf("\n");
	printf("End of SPU #%d\n", (int)envp);
	printf("///////////////////\n\n");



  spu_write_out_mbox(KILLOPCODE);

  unsigned int killSig = spu_read_in_mbox();
  printf("killSig: %d\n", killSig );

  return (0);
}


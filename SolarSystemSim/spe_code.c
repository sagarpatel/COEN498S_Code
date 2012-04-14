// From http://www.ibm.com/developerworks/library/pa-libspe2/
// slightly modified

#include <stdio.h>
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
//#include <simdmath.h>
//#include <recipd2.h>

#include "common.h"

#include <math.h>

// hack for double  resqrt since YDL SDK install is incompetent






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
	__vector double tempAccelerationXY = {0,0};
	__vector double tempAccelerationZ = {0,0};

	__vector double tempVelocityXY = {0,0};
	__vector double tempVelocityZ = {0,0};

	__vector double tempDistanceXY = {0,0}; 
	__vector double tempDistanceZ = {0,0}; 
	__vector double tempDistanceYX = {0,0};

	__vector double tempDistanceXYZ = {0,0};

	__vector double tempDistanceRL1 = {0,0};
	__vector double tempDistanceRL2 = {0,0};

	__vector double tempNumeratorXY = {0,0};
	__vector double tempNumeratorZ = {0,0};

	__vector double tempMassSplat = {0,0};
	__vector double tempGConstant = {GRAVITATIONALCONSTANT,GRAVITATIONALCONSTANT };
	__vector double tempDELATTIME = {(double)DELTA_TIME, (double)DELTA_TIME};
	__vector double tempEPS= {EPS, EPS};

	__vector double zeroVector = {0,0};
	__vector unsigned int oneVector = {1,1,1,1};

	__vector unsigned int axisBitShiftMask = {0,1,2,0};


	__vector unsigned char yzxwMask = { 4,5,6,7, 8,9,10,11, 0,1,2,3,  12,13,14,15};
	__vector unsigned char zxywMask = { 8,9,10,11, 0,1,2,3, 4,5,6,7,  12,13,14,15};


	__vector double tempUnitVectorXY = {0,0};
	__vector double tempUnitVectorZ = {0,0};

	__vector double distanceVectorXY = {0,0};
	__vector double distanceVectorZ = {0,0};

	__vector double tempTestXY = {0,0};
	__vector double tempTestZ = {0,0};
	__vector double temp_tempf= {0,0};

	__vector double tempPositionXY = {0,0};
	__vector double tempPositionZ = {0,0};

	//stupid C99, need to declare indicies before for loops
	int i = 0;
	int j = 0;
	int it_counter = 0;

	double massSave;

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
		massSave = pDi.positionZ[1];

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

			
			/*
			tempDistanceXY = spu_sub(pDj.positionXY,pDi.positionXY); //actual distance vector between objects i and j
			tempDistanceZ = spu_sub(pDj.positionZ,pDi.positionZ);
			*/
			
			// need to do this manually since compiler can't handle spu_sub with double vectors
			tempDistanceXY[0] = pDj.positionXY[0] - pDi.positionXY[0];
			tempDistanceXY[1] = pDj.positionXY[1] - pDi.positionXY[1];
			tempDistanceZ[0] = pDj.positionZ[0] - pDi.positionZ[0];

			

			// save value for unit vector calculation later
			distanceVectorXY = tempDistanceXY;
			distanceVectorZ = tempDistanceZ;

			/*
			 //Print distances between particles
			printf("Particle %d:   ", i );
			printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
			printf("\n");
			*/

			//use the distance vector  right now for numerator, before we overwrite is later in the code
			// use mass of subject mass

			//printf("Mass of particle: %d: %f\n",j, pDj.velocity[3] );

			tempMassSplat = spu_splats((double)pDj.positionZ[1]); //mass is stored in the last element (3) of velocity vector
			tempNumeratorXY = spu_mul(tempMassSplat, tempGConstant);
			tempNumeratorZ = spu_mul(tempMassSplat, tempGConstant);
			

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
			tempDistanceXY = spu_mul(tempDistanceXY, tempDistanceXY);
			tempDistanceZ = spu_mul(tempDistanceZ, tempDistanceZ);


			tempDistanceYX = spu_rlqwbyte(tempDistanceXY,64);

			/*
			printf("tempDistance: %f %f %f \n", tempDistance[0], tempDistance[1], tempDistance[2] );
			printf("tempDistanceRL1: %f %f %f \n", tempDistanceRL1[0], tempDistanceRL1[1], tempDistanceRL1[2] );
			printf("tempDistanceRL2: %f %f %f \n\n", tempDistanceRL2[0], tempDistanceRL2[1], tempDistanceRL2[2] );
			*/
			
			//add both
			tempDistanceXYZ = spu_add(tempDistanceXY, tempDistanceYX);
			//add to original to get total ---> x+y+z
			tempDistanceXYZ = spu_add(tempDistanceXYZ, tempDistanceZ); //tempDistance is now total distance squared
			
			// add EPS to avoid singularity
			tempDistanceXYZ =  spu_add(tempDistanceXYZ, tempEPS); //this is now the denominator value

			//save inverse magnitude for unit vector later

	
	/// from http://www.azillionmonkeys.com/qed/sqroot.html

			tempTestXY[1] = tempDistanceXYZ[0];
			printf("Before square root: %f\n",tempTestXY[1] );

	        //double x, z, tempf;
			double tempf;
		    unsigned long *tfptr = ((unsigned long *)&tempf) + 1;

			tempf = tempTestXY[1];
			*tfptr = (0xbfcdd90a00000000 - *tfptr)>>1; // estimate of 1/sqrt(tempTestXY[1]) 
			tempTestXY[0] =  tempf;
			tempTestZ[0] =  tempTestXY[1] * 0.5;                        // hoist out the /2    
			tempTestXY[0] = (1.5 * tempTestXY[0]) - (tempTestXY[0] * tempTestXY[0]) * (tempTestXY[0] * tempTestZ[0]);         // iteration formula     
			tempTestXY[0] = (1.5 * tempTestXY[0]) - (tempTestXY[0] * tempTestXY[0]) * (tempTestXY[0] * tempTestZ[0]);
			tempTestXY[0] = (1.5 * tempTestXY[0]) - (tempTestXY[0] * tempTestXY[0]) * (tempTestXY[0] * tempTestZ[0]);
			tempTestXY[0] = (1.5 * tempTestXY[0]) - (tempTestXY[0] * tempTestXY[0]) * (tempTestXY[0] * tempTestZ[0]);
			tempTestXY[0] = (1.5 * tempTestXY[0]) - (tempTestXY[0] * tempTestXY[0]) * (tempTestXY[0] * tempTestZ[0]);

		    tempTestXY[1] = tempTestXY[0] * tempTestXY[1];

		    printf("Result: %f\n", tempTestXY[1]);


/*			
			
			tempUnitVectorXY = spu_rsqrte(tempDistanceXY);
			tempUnitVectorZ = spu_rsqrte(tempDistanceZ);
			



			// invert vector to avoid division later
			tempDistanceXYZ = spu_re(tempDistanceXYZ); // this is final denominator (already inverted), only need to multiply
			// tempDistance is now eqivalent to 1/r^2 
*/

			/*
			//Print denominator
			printf("Denominator %d:   ", i );
			printf("x= %f, y=%f, z=%f", tempDistance[0], tempDistance[1], tempDistance[2]);
			printf("\n");
			*/

			//total acceleration applied to particle i, by particle j
			tempAccelerationXY = spu_mul(tempDistanceXY, tempNumeratorXY);
			tempAccelerationZ = spu_mul(tempDistanceZ, tempNumeratorZ);
			
			// create unit vector
			tempUnitVectorXY = spu_mul(distanceVectorXY, tempUnitVectorXY);
			tempUnitVectorZ = spu_mul(distanceVectorZ, tempUnitVectorZ);
			
			// apply unit vector to acceleration
			tempAccelerationXY = spu_mul(tempUnitVectorXY, tempAccelerationXY);
			tempAccelerationZ = spu_mul(tempUnitVectorZ, tempAccelerationZ);


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
			tempAccelerationXY = spu_mul(tempAccelerationXY, tempDELATTIME);
			
			tempVelocityXY[0] = particle_Array_SPU[i].velocityXY[0];
			tempVelocityXY[1] = particle_Array_SPU[i].velocityXY[1];

			particle_Array_SPU[i].velocityXY[0] = spu_add(tempAccelerationXY, tempVelocityXY)[0];
			particle_Array_SPU[i].velocityXY[1] = spu_add(tempAccelerationXY, tempVelocityXY)[1];




			tempAccelerationZ = spu_mul(tempAccelerationZ, tempDELATTIME);

			tempVelocityZ[0] = particle_Array_SPU[i].velocityZ[0];
			tempVelocityZ[1] = particle_Array_SPU[i].velocityZ[1];

			particle_Array_SPU[i].velocityZ[0] = spu_add(tempAccelerationZ, tempVelocityZ)[0];
			particle_Array_SPU[i].velocityZ[1] = spu_add(tempAccelerationZ, tempVelocityZ)[1];


			//restore mass in right position, in case
			particle_Array_SPU[i].positionZ[1] = massSave;

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
		
		tempVelocityXY[0] = particle_Array_SPU[i].velocityXY[0];
		tempVelocityXY[1] = particle_Array_SPU[i].velocityXY[1];
		

		tempPositionXY = spu_mul(tempVelocityXY, tempDELATTIME);


		tempTestXY[0] = particle_Array_SPU[i].positionXY[0];
		tempTestXY[1] = particle_Array_SPU[i].positionXY[1];

		particle_Array_SPU[i].positionXY[0] = spu_add(tempPositionXY, tempTestXY)[0];
		particle_Array_SPU[i].positionXY[1] = spu_add(tempPositionXY, tempTestXY)[1];






		tempVelocityZ[0] = particle_Array_SPU[i].velocityZ[0];
		tempVelocityZ[1] = particle_Array_SPU[i].velocityZ[1];

		tempPositionZ = spu_mul(tempVelocityZ, tempDELATTIME);

		tempTestZ[0] = particle_Array_SPU[i].positionZ[0];
		tempTestZ[1] = particle_Array_SPU[i].positionZ[1];


		particle_Array_SPU[i].positionZ[0] = spu_add(tempPositionZ, tempTestZ)[0];
		particle_Array_SPU[i].positionZ[1] = spu_add(tempPositionZ, tempTestZ)[1];


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




  return (0);
}


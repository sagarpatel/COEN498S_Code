// comon defines

#ifndef __common_h__
#define __common_h__

#define PARTICLES_MAXCOUNT 8 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 1.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT   0.000006673 //0.00000000006673 real value is 6.673 * 10^-11
#define DELTA_TIME 1
#define GRID_SIZE 10 // grid is a +- GRID_SIZE/2 cube
#define EPS 0.00001 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 1000

#define SPU_COUNT 6 // numbers of spus that will be used

typedef struct 
{
	 float position[4];	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	 float velocity[4];	// || --> 4th element will be used for mass value of the particle
} 
particle_Data_Shared;

// full array
extern particle_Data_Shared particle_Array_Shared[PARTICLES_MAXCOUNT]; 




#endif 

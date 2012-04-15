// comon defines

#ifndef __common_h__
#define __common_h__

#include <math.h>

#define PARTICLES_MAXCOUNT 2 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 10.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT   0.00000000006673 //0.00000000006673 real value is 6.673 * 10^-11
#define DELTA_TIME 60
#define GRID_SIZE 100 // grid is a +- GRID_SIZE/2 cube
#define EPS 0.001 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 10

#define MINFLOATVALUE 1E-37

#define MASSSCALEFACTOR 1E20

#define SPU_COUNT 6 // numbers of spus that will be used

typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;


// all masses will be scaled down by 10^10 each to match G scale up



//float GRAVITATIONALCONSTANT = 6.673 * pow(10, -5);
float sunMass = 1.9891 * pow(10,20); // original is 1.9891 * 10^30 kg
float earthMass = 5.9736 * pow(10,14); // original is 5.9736 * 10^24kg
float jupiterMass = 1.898 * pow(10,17); //^27 original
//float satMass = 1.0 * pow(10,-6);



#endif 

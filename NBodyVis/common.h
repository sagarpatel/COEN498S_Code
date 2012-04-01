// comon defines

#ifndef __common_h__
#define __common_h__

#define PARTICLES_MAXCOUNT 512 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 1000.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT  0.00000000006673 // real value is 6.673 * 10^-11
#define DELTA_TIME 60.0
#define GRID_SIZE 10 // grid is a +- GRID_SIZE/2 cube
#define EPS 1.0 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 1000

#define SPU_COUNT 6 // numbers of spus that will be used

#endif 

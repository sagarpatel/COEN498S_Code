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



// comon defines

#ifndef __common_h__
#define __common_h__

#define PARTICLES_MAXCOUNT 8 //must be power of 2 in orderfor array data align to work later on
#define PARTICLES_DEFAULTMASS 10.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT   0.000006673 //0.00000000006673 real value is 6.673 * 10^-11
#define DELTA_TIME 300
#define GRID_SIZE 100 // grid is a +- GRID_SIZE/2 cube
#define EPS 0.001 // EPS^2 constant to avoid singularities
#define ITERATION_COUNT 10000//8640

// 86400 seconds in 24 hours

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

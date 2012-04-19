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
#define CENTRALBODIES_COUNT 1
#define PARTICLES_DEFAULTMASS 10.0 // 1.0 is 1 kg
#define GRAVITATIONALCONSTANT   0.000006673 //0.00000000006673 real value is 6.673 * 10^-11
#define DELTA_TIME 300
#define GRID_SIZE 100 // grid is a +- GRID_SIZE/2 cube
#define EPS 0.001 // EPS^2 constant to avoid singularities

#define PARTICLE_DMA_MAX 512
#define ITERATION_COUNT 512//8640  //has to be >= PARTICLE_DMA_MAX

#define DMA_COUNT ITERATION_COUNT/PARTICLE_DMA_MAX
// 86400 seconds in 24 hours

#define SPU_COUNT 6 // numbers of spus that will be used



#define earthMass 59736000000000000000.0f // 5.9736 * pow(10,19);  // scaled for scaled G value  // original = 5.9736 * 10^24
#define moonMass 734900000000000000.0f // original mass 7.349 * 10^22 
#define satMass 11110.0f

typedef struct 
{
	__vector float position;	// includes x,y,z --> 4th vector element will be used to store quadrant id of the particle
	__vector float velocity;	// || --> 4th element will be used for mass value of the particle
} 
particle_Data;


//float earthMass = 59736000000000000000.0f; // 5.9736 * pow(10,19);  // scaled for scaled G value  // original = 5.9736 * 10^24


__vector float issPosition = {6721000,0,0,0};  //350000 m is high, need to add radius of earth 6371000 // addition is 6761000
__vector float issVelocity = {0,7707,0,0};
//float issMass = 4.5; // original = 4.5 * 10^5

__vector float hubblePosition = {6930000,0, 0, 0}; //559000m high, need to add earth radius
__vector float hubbleVelocity = {0, 7500, 0,0};
//float hubbleMass = 0.11110f ; // original is 11110 kg


/* Moon data, from: http://ssd.jpl.nasa.gov/horizons.cgi#results
 X = 1.142141171615604E+05 Y =-3.591656526588580E+05 Z = 2.279788111025651E+04
 VX= 1.002213369349858E+00 VY= 2.572033783875429E-01 VZ= 7.321967853480284E-02
 */

__vector float moonPosition = {114214117, -359165652, 22797881, 0};
__vector float moonVelocity = {1002, 257, 73};
//float moonMass = 734900000000000000.0f; // original mass 7.349 * 10^22 




//float satMass = 11110f ; // original is 11110 kg

__vector float sat1Position = {6930000,0, 0, 0}; //559000m high, need to add earth radius
__vector float sat1Velocity = {0, 7500, 0,satMass};


__vector float sat2Position = {-6930000,0, 0, 0}; //559000m high, need to add earth radius
__vector float sat2Velocity = {0, -7500, 0,satMass};


__vector float sat3Position = {0,6930000, 0, 0}; //559000m high, need to add earth radius
__vector float sat3Velocity = {-7500, 0, 0,satMass};


__vector float sat4Position = {0,-6930000, 0, 0}; //559000m high, need to add earth radius
__vector float sat4Velocity = {7500, 0, 0,satMass};


__vector float sat5Position = {0, 0, -6930000, 0}; //559000m high, need to add earth radius
__vector float sat5Velocity = {7500, 0, 0,satMass};


__vector float sat6Position = {0, 0, 6930000, 0}; //559000m high, need to add earth radius
__vector float sat6Velocity = {0, 7500, 0,satMass};


__vector float immuneBodyPosition = {0,0,0,0};
__vector float immuneBodyVelocity = {0,0,0, earthMass};

#endif 

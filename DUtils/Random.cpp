/*	
 * File: Random.cpp
 * Project: DUtils library
 * Author: Dorian Galvez
 * Date: April 2010
 * Description: manages pseudo-random numbers
 *
 */

#include "Random.h"
#include "Timestamp.h"
#include <cstdlib>
using namespace std;

using namespace DUtils;

void Random::SeedRand(){
	Timestamp time;
	time.setToCurrentTime();
	srand((unsigned)time.getFloatTime()); 
}

void Random::SeedRand(int seed)
{
	srand(seed); 
}

int Random::RandomInt(int min, int max){
	int d = max - min + 1;
	return int(((double)rand()/((double)RAND_MAX + 1.0)) * d) + min;
}

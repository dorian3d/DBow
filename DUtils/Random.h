/*	
 * File: Random.h
 * Project: DUtils library
 * Author: Dorian Galvez
 * Date: April 2010
 * Description: manages pseudo-random numbers
 *
 */

#pragma once
#ifndef __D_RANDOM__
#define __D_RANDOM__

#include <cstdlib>

namespace DUtils {

class Random
{
public:
	/**
	 * Sets the random number seed to the current time
	 */
	static void SeedRand();

	/** 
	 * Sets the given random number seed
	 * @param seed
	 */
	static void SeedRand(int seed);

	/**
	 * Returns a random number in the range [0..1]
	 * @return random T number in [0..1]
	 */
	template <class T>
	static T RandomValue(){
		return (T)rand()/(T)RAND_MAX;
	}

	/**
	 * Returns a random number in the range [min..max]
	 * @param min
	 * @param max
	 * @return random T number in [min..max]
	 */
	template <class T>
	static T RandomValue(T min, T max){
		return Random::RandomValue<T>() * (max - min) + min;
	}

	/**
	 * Returns a random int in the range [min..max]
	 * @param min
	 * @param max
	 * @return random int in [min..max]
	 */
	static int RandomInt(int min, int max);
};

}

#endif


/*	
 * File: Math.h
 * Project: DUtils library
 * Author: Dorian Galvez
 * Date: April 2010
 * Description: some math functions
 *
 */

#pragma once
#ifndef __D_MATH__
#define __D_MATH__

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace DUtils {

	class Math {

	public:

		/* Returns the mean of a population
		 * @param v
		 */
		template <class T>
		static double Mean(const std::vector<T> &v)
		{
			if(v.empty())
				return 0;
			else{
				double sum = 0;
				typename std::vector<T>::const_iterator it;
				for(it = v.begin(); it != v.end(); it++){
					sum += *it;
				}
				return sum/double(v.size());
			}
		}

		/* Returns the standard deviation of a population
		 * @param v
		 * @param mean (optional): the mean of the population
		 */
		template <class T>
		static double Stdev(const std::vector<T> &v)
		{
			return Math::Stdev<T>(v, Math::Mean<T>(v));
		}

		template <class T>
		static double Stdev(const std::vector<T> &v, double mean)
		{
			if(v.size() <= 1)
				return 0;
			else{
				// stdev = sqrt( Sum{ (x_i - mean)^2 } / (N-1) )
				double sum = 0;
				typename std::vector<T>::const_iterator it;
				for(it = v.begin(); it != v.end(); it++){
					sum += pow(*it - mean, 2);
				}
				return sqrt(sum/double(v.size()-1));
			}
		}

	};

}

#endif


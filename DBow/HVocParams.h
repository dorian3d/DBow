/**
 * File: HVocParams.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: parameters to create a hierarchical vocabulary
 */

#pragma once
#ifndef __H_VOC_PARAMS__
#define __H_VOC_PARAMS__

#include "VocParams.h"

#include <string>
using namespace std;

namespace DBow {

	class HVocParams :
		public VocParams
	{
	public:
		int k;
		int L;

	public:
		/**
		 * Constructor
		 * @param k branching factor
		 * @param L max depth levels
		 * @param desc_length (default: 64): descriptor length
		 * @param weighting (default: TF_IDF): weighting method
		 * @param scoring (default: L1_NORM): scoring method
		 * @param scale_score (default: true): scale scores
		 */
		HVocParams(int k, int L, int desc_length = 64,
			WeightingType weighting = TF_IDF,
			ScoringType scoring = L1_NORM,
			bool scale_score = true);

		/**
		 * Destructor
		 */
		~HVocParams(void);

		/**
		 * Returns a string with information about the parameters
		 * @return information string
		 */
		string toString() const;
	};

}

#endif


/**
 * File: BowVector.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: bag-of-words vector for representing image
 * Defines: WordId, WordValue, BowEntryVector, BowVector
 *
 * Note: this vector is implemented with a stl vector.
 *   The stl vector interface is public so that Vocabulary 
 *   subclasses can deal easily with BowVectors.
 *   BowVector entries must be in strict ascending order of ids for 
 *   easy scoring between isolated BowVectors with Vocabulary::Score (this 
 *   condition is not necessary if vectors are only going to be
 *   used in Database::Query and Database::AddEntry).
 *   Vocabulary subclasses can make use of BowVector::PutInOrder
 *   if they do not produce bow vectors in order.
 */

#pragma once
#ifndef __BOW_VECTOR__
#define __BOW_VECTOR__

#include "VocParams.h"
#include <cfloat>
#include <vector>
#include <cmath>
using namespace std;

namespace DBow {

	typedef unsigned int WordId;
	typedef double WordValue;

	// If you change the type of WordValue, make sure you change also the
	// epsilon value (this is needed by the KL method)
	const WordValue EPSILON = DBL_EPSILON; // FLT_EPSILON
	const WordValue LOG_EPS = log(EPSILON);

	/** Type of entries in the vector
	 */
	struct BowVectorEntry
	{
		WordId id;
		WordValue value;

		/**
		 * Constructor
		 */
		BowVectorEntry(){}
		BowVectorEntry(WordId _id, WordValue _value){
			id = _id;
			value = _value;
		}

		/** 
		 * Returns if the word id of the current word is lower than
		 *  w.id (this is used for arranging the vector entries)
		 */
		bool operator< (const BowVectorEntry &w) const;

		/** 
		 * Returns if the word id of the current word is the same as
		 *  w.id (this is used for arranging the vector entries)
		 */
		bool operator== (const BowVectorEntry &w) const;

		/**
		 * Compares the bow vector with a given word id
		 * @param id
		 * @return true iif this bow vector's id and the given one are the same
		 */
		bool operator== (WordId id) const;

		/** 
		 * Returns the sum of the word values of two entries
		 */
		WordValue operator+ (const BowVectorEntry &w) const;

		/**
		 * Returns the sum of a value and a vector value
		 */
		friend WordValue operator+ (WordValue w, const BowVectorEntry &v);
	};

	class BowVector: 
		public vector<BowVectorEntry>
	{
	public:

		/** Constructor
		 */
		BowVector(void);

		/** Destructor
		 */
		~BowVector(void);

		/**
		 * Normalizes the values in the vector
		 * @param norm_type norm used
		 */
		void Normalize(VocParams::ScoringType norm_type);

		/** 
		 * Puts the vector entries in ascending order of word ids
		 */
		void PutInOrder();

		/**
		 * Returns if the vector is already in order. Used for debugging purposes
		 * @return true iif in order
		 */
		bool isInOrder() const;
	};

}

#endif


/**
 * File: VocParams.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: parameters to create a new vocabulary.
 *   This class must be inherited
 */

#pragma once
#ifndef __VOC_PARAMS__
#define __VOC_PARAMS__

#include <string>
using namespace std;

namespace DBow {

	class Vocabulary;
	class Database;

	class VocParams
	{
	public:

		enum VocType 
		{
			HIERARCHICAL_VOC = 0	// HVocParams instance
		};

		enum WeightingType
		{
			TF_IDF,
			TF,
			IDF,
			BINARY
		};

		enum ScoringType
		{
			L1_NORM,
			L2_NORM,
			CHI_SQUARE,
			KL,
			BHATTACHARYYA,
			DOT_PRODUCT
		};

	public:

		// Voc, weighting and scoring types
		VocType Type;
		WeightingType Weighting;
		ScoringType Scoring;

		// Scale scores to 0..1 ?
		bool ScaleScore;

		// Descriptor length. It is usually 128 for SIFT and 64 or 128 for SURF
		int DescriptorLength;

	public:
		
		/**
		 * Returns a string with information about the parameters
		 * @return information string
		 */
		virtual string toString() const;

		/**
		 * Destructor
		 */
		virtual ~VocParams(void);

		/**
		 * Says if a bow vector should be normalized according to the scoring
		 * method chosen.
		 * @param scoring scoring used
		 * @param norm (out) norm to use
		 */
		static bool MustNormalize(ScoringType scoring, ScoringType& norm);

		/**
		 * Says if a bow vector should be normalized according to the scoring
		 * method chosen.
		 * @param norm (out) norm to use
		 */
		bool MustNormalize(ScoringType& norm) const;

	protected:

		/**
		 * Constructor available to subclasses only
		 * @param type type of voc
		 * @param desc_length descriptor length
		 * @param weighting weighting method
		 * @param scoring scoring method
		 */
		VocParams(VocType type, int desc_length, 
			WeightingType weighting, ScoringType scoring, bool scale_score);

	};

}

#endif


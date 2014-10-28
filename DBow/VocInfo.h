/**
 * File: VocInfo.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: information structure retrieved from vocabularies
 */

#pragma once
#ifndef __VOC_INFO__
#define __VOC_INFO__

#include "VocParams.h"

#include <string>
using namespace std;

namespace DBow {

	class VocInfo
	{
	public:

		// Type of vocabulary (determines subclass of vocParams)
		VocParams::VocType VocType;

		// Words in the vocabulary
		long WordCount;

		// Number of stopped words because of their high frequency
		long StoppedFrequentWords;

		// Number of stopped words because of their low frequency
		long StoppedInfrequentWords;

		// Other vocabulary parameters
		// Should be casted to the proper VocParams subclass pointer
		VocParams *Parameters;

	public:
		
		/**
		 * Empty constructor
		 */
		VocInfo(void);

		/**
		 * Copy constructor. Replicates data
		 */
		VocInfo(const DBow::VocInfo &v);

		/**
		 * Constructor which copies the given vocabulary params
		 * @param params vocabulary parameters
		 */
		VocInfo(const DBow::VocParams &params);

		/**
		 * Destructor
		 */
		virtual ~VocInfo(void);

		/**
		 * Copy operator. Replicates data
		 * @param v source
		 */
		VocInfo& operator=(const DBow::VocInfo &v);

		/**
		 * Returns a string with the vocabulary information
		 * @return information string
		 */
		string toString() const;
	};

}

#endif


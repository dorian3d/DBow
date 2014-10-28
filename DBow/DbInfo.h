/**
 * File: DbInfo.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: class to encapsulate db info
 */

#pragma once
#ifndef __D_DB_INFO__
#define __D_DB_INFO__

#include "VocInfo.h"
#include <string>
using namespace std;

namespace DBow {

	class DbInfo: 
		public VocInfo
	{
	public:

		// Number of entries in the database
		int EntryCount;

	public:
		
		/**
		 * Empty constructor
		 */
		DbInfo(void);

		/**
		 * Constructor from a VocInfo object
		 */
		DbInfo(const VocInfo &v);

		/**
		 * Copy constructor. Replicates data
		 */
		DbInfo(const DbInfo &v);

		/**
		 * Destructor
		 */
		~DbInfo(void);

		/**
		 * Copy operator. Replicates data
		 * @param v source
		 */
		DbInfo& operator=(const DBow::DbInfo &v);

		/**
		 * Returns a string with the database information
		 * @return information string
		 */
		string toString() const;

	};

}

#endif


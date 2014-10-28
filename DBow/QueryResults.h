/**
 * File: QueryResults.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: results obtained by querying a database
 */


#pragma once
#ifndef __D_QUERY_RESULTS__
#define __D_QUERY_RESULTS__

#include "DatabaseTypes.h"
#include <vector>
using namespace std;

namespace DBow {

	struct Result 
	{
		EntryId Id;
		double Score;

		/**
		 * Constructors
		 */
		inline Result(){}
		inline Result(EntryId _id, double _score):Id(_id), Score(_score){}

		/**
		 * Compares the scores of two results
		 * @return true iif a.Score > b.Score
		 */
		inline static bool GreaterThan(const Result &a, const Result &b){
			return a.Score > b.Score;
		}

		/**
		 * Compares the scores of two results
		 * @return true iif this.score < r.score
		 */
		inline bool operator<(const Result &r) const {
			return this->Score < r.Score;
		}

		/**
		 * Compares the scores of two results
		 * @return true iif this.score > r.score
		 */
		inline bool operator>(const Result &r) const
		{
			return this->Score > r.Score;
		}

		/**
		 * Compares the entry id of the result
		 * @return true iif this.id == id
		 */
		inline bool operator==(EntryId id) const
		{
			return this->Id == id;
		}
	};

	class QueryResults: 
		public vector<Result>
	{
	public:

		/**
		 * Empty constructor
		 */
		QueryResults(void);

		/**
		 * Destructor
		 */
		~QueryResults(void);

	};

}

#endif


/**
 * File: Database.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: an image database 
 */

#pragma once
#ifndef __D_DATABASE__
#define __D_DATABASE__

#include "BowVector.h"
#include "Vocabulary.h"
#include "DbInfo.h"
#include "DatabaseTypes.h"
#include "QueryResults.h"
#include <vector>
#include <list>
using namespace std;

namespace DBow {

class Database
{
public:

	/**
	 * Creates a database from the given vocabulary.
	 * @param voc vocabulary
	 */
	Database(const Vocabulary &voc);

	/**
	 * Creates a database from a file
	 * @param filename
	 */
	Database(const char *filename);

	/**
	 * Destructor
	 */
	virtual ~Database(void);

	/**
	 * Retrieves infoa bout the database
	 * @return db info
	 */
	DbInfo RetrieveInfo() const;

	/**
	 * Adds an entry to the database
	 * @param features features of the image, in the opencv format
	 * @return id of the new entry
	 */
	EntryId AddEntry(const vector<float> &features);

	/**
	 * Adds an entry to the database
	 * @param v bow vector to add
	 * @return id of the new entry
	 */
	EntryId AddEntry(const BowVector &v);

	/**
	 * Empties the database
	 */
	inline void Clear();

	/** 
	 * Returns the number of entries in the database
	 * @return number of entries
	 */
	inline unsigned int NumberOfEntries() const { return m_nentries; }

	/**
	 * Queries the database with some features
	 * @param ret (out) query results
	 * @param features query features
	 * @param max_results number of results to return
	 */
	void Query(QueryResults &ret, const vector<float> &features, 
		int max_results = 1) const;

	/**
	 * Queries the database with a bow vector
	 * @param ret (out) query results
	 * @param v vector to query with
	 * @param max_results number of results to return
	 */
	void Query(QueryResults &ret, const BowVector &v, 
		int max_results = 1) const;

	/**
	 * Saves the database along with the vocabulary in the given file
	 * @param filename file
	 * @param binary (default: true) store in binary format
	 */
	void Save(const char *filename, bool binary = true) const;

	/**
	 * Loads the database from a file
	 * @param filename
	 */
	void Load(const char *filename);

	/**
	 * Saves the vocabulary in a file
	 * @param filename file to store the vocabulary in
	 * @param binary (default: true) store in binary format
	 */
	inline void ExportVocabulary(const char *filename, bool binary = true) const
	{
		m_voc->Save(filename, binary);
	}

	/**
	 * Returns the vocabulary instance used by this database
	 * @return ref to the vocabulary
	 */
	inline const Vocabulary& Voc() const {
		return *m_voc;
	}

protected:

	/**
	 * Saves the database in binary format
	 * @param filename
	 */
	void SaveBinary(const char *filename) const;

	/**
	 * Saves the database in text format
	 * @param filename
	 */
	void SaveText(const char *filename) const;

	/**
	 * Loads the database from a binary file
	 * @param filename
	 */
	void LoadBinary(const char *filename);

	/**
	 * Loads the database from a text file
	 * @param filename
	 */
	void LoadText(const char *filename);

	/**
	 * Does the internal work to add an entry to the database
	 * @param v vector to add (it is modified)
	 * @return added entry id
	 */
	EntryId _AddEntry(BowVector &v);

	/**
	 * Does the internal work to query the database
	 * @param ret (out) query results
	 * @param v bow vector (it is modified)
	 * @param max_results returns only this number of results
	 */
	void _Query(QueryResults &ret, BowVector &v, int max_results) const;
	
protected:

	/**
	 * InvertedFile types
	 */

	struct IFEntry{
		EntryId id;
		WordValue value;

		IFEntry(EntryId _id, WordValue _value){
			id = _id;
			value = _value;
		}

		/** 
		 * Returns if the current IFEntry has the same id as the given one
		 * @param e entry to compare with
		 * @return true iif ids are the same
		 */
		inline bool operator==(const IFEntry &e) const { return id == e.id; }
	};

	typedef list<IFEntry> IFRow;

	// InvertedFile[wordid] = [ <docid,value>, ... ]
	class InvertedFile: public vector<IFRow>
	{
	public:
		InvertedFile(){}
		~InvertedFile(){}
	};

protected:

	// Vocabulary associated to this database
	Vocabulary *m_voc;

	// Inverted file 
	InvertedFile m_index;

	// Number of entries in the db
	unsigned int m_nentries;

private:

	/**
	 * Creates an instance of a vocabulary object
	 * depending on the header of the given filename
	 * @param type type of vocabulary
	 * @param copy (default: NULL) if given, the vocabulary
	 *   is initiated as a copy of this vocabulary
	 */
	void initVoc(VocParams::VocType type, const Vocabulary *copy = NULL);

	/**
	 * Loads the database from a filename
	 * The vocabulary has already been read
	 * @param f file stream
	 */
	template<class T> void _load(T& f);

	/**
	 * Performs several kinds of queries
	 * @param v bow vector to query (already normalized if necessary)
	 * @param ret allocated and empty vector to store the results in
	 * @param max_results maximum number of results in ret
	 * @param scale_score says if score must be scaled in the end (if applicable)
	 */
	void doQueryL1(const BowVector &v, QueryResults &ret, 
		const int max_results, const bool scale_score) const;
	void doQueryL2(const BowVector &v, QueryResults &ret, 
			const int max_results, const bool scale_score) const;
	void doQueryChiSquare(const BowVector &v, QueryResults &ret, 
		const int max_results, const bool scale_score) const;
	void doQueryKL(const BowVector &v, QueryResults &ret, 
		const int max_results, const bool scale_score) const;
	void doQueryBhattacharyya(const BowVector &v, QueryResults &ret, 
		const int max_results, const bool scale_score) const;
	void doQueryDotProduct(const BowVector &v, QueryResults &ret, 
		const int max_results, const bool scale_score) const;

};

}

// -- Inline functions

inline DBow::EntryId DBow::Database::AddEntry(const DBow::BowVector &v)
{
	DBow::BowVector w = v;
	return _AddEntry(w);
}

inline DBow::EntryId DBow::Database::AddEntry(const vector<float>& features)
{
	DBow::BowVector v;
	m_voc->Transform(features, v, false);
	return _AddEntry(v);
}

inline void
DBow::Database::Query(DBow::QueryResults &ret, const vector<float> &features, 
				int max_results) const
{
	DBow::BowVector v;
	m_voc->Transform(features, v, false);
	_Query(ret, v, max_results);
}

inline void
DBow::Database::Query(DBow::QueryResults &ret, const DBow::BowVector &v, 
				int max_results) const
{
	DBow::BowVector w = v;
	_Query(ret, w, max_results);
}


#endif


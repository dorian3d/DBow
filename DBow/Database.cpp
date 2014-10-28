/**
 * File: Database.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: an image database 
 */

#include "Database.h"
#include "Vocabulary.h"
#include "HVocabulary.h"
#include "QueryResults.h"
#include "DUtils.h"

#include <vector>
#include <algorithm>
#include <fstream>
using namespace std;

using namespace DBow;

Database::Database(const Vocabulary &voc) :
	m_voc(NULL), m_nentries(0)
{
	initVoc(voc.RetrieveInfo().VocType, &voc);
	m_index.resize(0);
	m_index.resize(voc.NumberOfWords());
}

Database::Database(const char *filename) :
	m_voc(NULL), m_nentries(0)
{
	Load(filename);
}

Database::~Database(void)
{
	delete m_voc;
}

DbInfo Database::RetrieveInfo() const
{
	DbInfo ret(m_voc->RetrieveInfo());
	ret.EntryCount = m_nentries;
	return ret;
}

EntryId Database::_AddEntry(BowVector &v)
{
	VocParams::ScoringType norm;
	if(VocParams::MustNormalize(m_voc->Scoring(), norm)){
		// vectors are stored normalized if needed
		v.Normalize(norm);
	}

	EntryId eid = m_nentries;

	// update inverted file
	BowVector::const_iterator it;
	for(it = v.begin(); it != v.end(); it++){
		// eids are in ascending order in the index
		m_index[it->id].push_back(IFEntry(eid, it->value));
	}

	m_nentries++;

	return eid;
}


void Database::Clear()
{
	m_index.resize(0);
	m_index.resize(m_voc->NumberOfWords());
	m_nentries = 0;
}

void 
Database::_Query(QueryResults &ret, BowVector &v, int max_results) const
{
	// This implementation is independent from that in Vocabulary::Score

	// check if the vector must be normalized
	VocInfo info = m_voc->RetrieveInfo();
	VocParams::ScoringType norm;
	if(VocParams::MustNormalize(info.Parameters->Scoring, norm)){
		v.Normalize(norm);
	}

	// ret is not in order until the end
	ret.resize(0);
	ret.reserve(100);

	switch(info.Parameters->Scoring){
		
		case VocParams::L1_NORM:
			doQueryL1(v, ret, max_results, info.Parameters->ScaleScore);
			break;

		case VocParams::L2_NORM:
			doQueryL2(v, ret, max_results, info.Parameters->ScaleScore);
			break;

		case VocParams::CHI_SQUARE:
			doQueryChiSquare(v, ret, max_results, info.Parameters->ScaleScore);
			break;

		case VocParams::KL:
			doQueryKL(v, ret, max_results, info.Parameters->ScaleScore);
			break;

		case VocParams::BHATTACHARYYA:
			doQueryBhattacharyya(v, ret, max_results, info.Parameters->ScaleScore);
			break;

		case VocParams::DOT_PRODUCT:
			doQueryDotProduct(v, ret, max_results, info.Parameters->ScaleScore);
			break;
	}
}

void Database::doQueryL1(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;
	
	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue qvalue = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue dvalue = rit->value;

			// scoring-dependent value
			double value = fabs(qvalue - dvalue) - fabs(qvalue) - fabs(dvalue);

			// check if this entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value; 
			}
		} // for each inverted row 
	} // for each word in features	

	// resulting "scores" are now in [-2 best .. 0 worst]
	
	// sort vector in ascending order
	// (scores are inverted now --the lower the better--)
	sort(ret.begin(), ret.end());

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	// complete score
	// ||v - w||_{L1} = 2 + Sum(|v_i - w_i| - |v_i| - |w_i|) 
	//		for all i | v_i != 0 and w_i != 0 
	// (Nister, 2006)
	if(scale_score){
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = -qit->Score/2.0;
	}else{
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = 2.0 + qit->Score;
	}
}

void Database::doQueryL2(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;

	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue qvalue = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue dvalue = rit->value;

			// scoring-dependent value
			double value = qvalue * dvalue;
			value = -value; // trick for smart sorting

			// check if this entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value;
			}
		} // for each inverted row 
	} // for each word in features	

	// resulting "scores" are now in [ -1 best .. 0 worst ]

	// sort vector in ascending order
	// (scores are inverted now --the lower the better--)
	sort(ret.begin(), ret.end());

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	if(scale_score){
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = 1.0 - sqrt(1.0 + qit->Score);
	}else {
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = sqrt(2 + 2 * qit->Score);
	}
}

void Database::doQueryChiSquare(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;
	
	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue vi = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue wi = rit->value;

			// scoring-dependent value
			double value = (vi - wi)*(vi - wi)/(vi + wi) - vi - wi;

			// check if this db entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value; 
			}
		} // for each inverted row 
	} // for each word in features	

	// resulting "scores" are now in [-2 best .. 0 worst]
	
	// sort vector in ascending order
	// (scores are inverted now --the lower the better--)
	sort(ret.begin(), ret.end());

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	// complete score
	// score = Sum (vi - wi)^2 / (vi + wi) ==
	//   Sum vi + Sum wi - Sum{i, wi != 0} vi - Sum{i, vi != 0} wi +
	//   + Sum_{i, vi != 0 && wi != 0} (vi - wi)^2 / (vi + wi)
	//
	// if there are no negative items, Sum vi = Sum wi = 1, since they
	// are normalized
	//
	// NOTE: this implementation assumes there are no negative items in
	// the vectors (there should not be if tf, idf or tf-idf are used)
	//
	if(scale_score){
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = -qit->Score/2.0;
	}else{
		for(qit = ret.begin(); qit != ret.end(); qit++) 
			qit->Score = 2.0 + qit->Score;
	}
}

void Database::doQueryKL(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;

	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue vi = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue wi = rit->value;

			// scoring-dependent value
			double value = vi * log(vi/wi);

			// check if this db entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value; 
			}
		} // for each inverted row 
	} // for each word in features	
	
	// resulting "scores" are now in [-X worst .. 0 best .. X worst]
	// but we cannot make sure which ones are better without calculating
	// the complete score

	// complete scores
	for(qit = ret.begin(); qit != ret.end(); ++qit){
		EntryId eid = qit->Id;
		double value = 0.0;
		
		for(it = v.begin(); it != v.end(); it++){
			const WordValue vi = it->value;
			const IFRow& row = m_index[it->id];

			if(row.end() == find(row.begin(), row.end(), IFEntry(eid, 0) )){
				value += vi * (log(vi) - LOG_EPS);
			}
		}

		qit->Score += value;
	}

	// real scores are now in [0 best .. X worst]
	
	// sort vector in ascending order
	// (scores are inverted now --the lower the better--)
	sort(ret.begin(), ret.end());

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	// this score cannot be scaled
}

void Database::doQueryBhattacharyya(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;
	
	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue vi = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue wi = rit->value;

			// scoring-dependent value
			double value = sqrt(vi * wi);

			// check if this db entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value; 
			}
		} // for each inverted row 
	} // for each word in features	

	// resulting "scores" are now in [1 best .. 0 worst]
	
	// sort vector in descending order
	sort(ret.begin(), ret.end(), Result::GreaterThan);

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	// this score is already scaled
}

void Database::doQueryDotProduct(const BowVector &v, QueryResults &ret, 
						 const int max_results, const bool scale_score) const
{
	BowVector::const_iterator it;
	IFRow::const_iterator rit;
	QueryResults::iterator qit;
	
	for(it = v.begin(); it != v.end(); it++){
		WordId wid = it->id;
		WordValue vi = it->value;
		
		const IFRow& row = m_index[wid];

		for(rit = row.begin(); rit != row.end(); rit++){
			EntryId eid = rit->id;
			WordValue wi = rit->value;

			// scoring-dependent value
			double value = vi * wi;

			// check if this db entry is already in the returning vector
			qit = find(ret.begin(), ret.end(), eid);

			if(qit == ret.end()){
				// insert
				ret.push_back(Result(eid, value));
			}else{
				// update
				qit->Score += value; 
			}
		} // for each inverted row 
	} // for each word in features	
	
	// resulting "scores" are now in [0 worst .. X best]

	// sort vector in descending order
	sort(ret.begin(), ret.end(), Result::GreaterThan);

	// cut vector
	if((int)ret.size() > max_results) ret.resize(max_results);

	// this score cannot be scaled
}

void Database::Save(const char *filename, bool binary) const
{
	if(binary){
		SaveBinary(filename);
	}else{
		SaveText(filename);
	}
}

void Database::Load(const char *filename)
{
	fstream f(filename, ios::in | ios::binary);

	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	char c;
	f.get(c);

	f.close();

	// if c is >= 32, it is text
	if(c >= 32)
		LoadText(filename);
	else
		LoadBinary(filename);
}

void Database::SaveBinary(const char *filename) const
{
	// Format:
	// [Vocabulary] (with magic word)
	// N W'
	// WordId_0 K_0 EntryId_0_0 Value_0_0 ... EntryId_0_(K_0) Value_0_(K_0)
	// ...
	// WordId_(W'-1) K_(W'-1) EntryId_(W'-1)_0 Value_(W'-1)_0 ... EntryId_(W'-1)_{K_(W'-1)} Value_(W'-1)_{K_(W'-1)}
	//
	// Where:
	// [Vocabulary]: whole vocabulary in binary format
	// N (int32): number of entries in the database
	// W' (int32): number of words with some row in the inverted file
	// WordId_i (int32): word id in the inverted file
	// K_i (int32): number of entries in the row of the WordId_i
	// EntryId_i_k (int32): doc (entry) id where the word WordId_i is present
	// Value_i_k (double64): value of word WordId_i in entry EntryId_i_k
	//

	m_voc->Save(filename, true);

	DUtils::BinaryFile f(filename, DUtils::FILE_MODES(DUtils::WRITE | DUtils::APPEND));

	int N = m_nentries;
	int W = 0;

	InvertedFile::const_iterator it;
	for(it = m_index.begin(); it != m_index.end(); it++){
		if(!it->empty()) W++;
	}

	f << N << W;

	IFRow::const_iterator rit;
	for(it = m_index.begin(); it != m_index.end(); it++){
		if(!it->empty()){
			int wordid = it - m_index.begin();
			int k = (int)it->size();

			f << wordid << k;
			
			for(rit = it->begin(); rit != it->end(); rit++){
				f << (int)rit->id << (double)rit->value;
			}
		}
	}

	f.Close();
}

	
void Database::SaveText(const char *filename) const
{
	// Format:
	// [Vocabulary]
	// N W'
	// WordId_0 K_0 EntryId_0_0 Value_0_0 ... EntryId_0_(K_0) Value_0_(K_0)
	// ...
	// WordId_(W'-1) K_(W'-1) EntryId_(W'-1)_0 Value_(W'-1)_0 ... EntryId_(W'-1)_{K_(W'-1)} Value_(W'-1)_{K_(W'-1)}
	//
	// Where:
	// [Vocabulary]: whole vocabulary in text format
	// N (int32): number of entries in the database
	// W' (int32): number of words with some row in the inverted file
	// WordId_i (int32): word id in the inverted file
	// K_i (int32): number of entries in the row of the WordId_i
	// EntryId_i_k (int32): doc (entry) id where the word WordId_i is present
	// Value_i_k (double64): value of word WordId_i in entry EntryId_i_k
	//

	m_voc->Save(filename, false);

	fstream f(filename, ios::out | ios::app);
	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	int N = m_nentries;
	int W = 0;

	InvertedFile::const_iterator it;
	for(it = m_index.begin(); it != m_index.end(); it++){
		if(!it->empty()) W++;
	}

	f << N << " " << W << endl;

	IFRow::const_iterator rit;
	for(it = m_index.begin(); it != m_index.end(); it++){
		if(!it->empty()){
			int wordid = it - m_index.begin();
			int k = (int)it->size();

			f << wordid << " " << k << " ";
			
			for(rit = it->begin(); rit != it->end(); rit++){
				f << (int)rit->id << " " 
					<< (double)rit->value << " ";
			}
			f << endl;
		}
	}

	f.close();
}


void Database::LoadBinary(const char *filename)
{
	// read type of voc (@see Vocabulary::SaveBinaryHeader)
	DUtils::BinaryFile f(filename, DUtils::READ);
	f.DiscardNextByte(); // magic word
	int voctype;
	f >> voctype;
	f.Close();

	initVoc((VocParams::VocType)voctype);

	unsigned int pos = m_voc->Load(filename);
	
	f.OpenForReading(filename);
	f.DiscardBytes(pos); // vocabulary read
	
	_load<DUtils::BinaryFile>(f);
	
	f.Close();
}

void Database::LoadText(const char *filename) 
{
	// read type of voc (@see Vocabulary::SaveTextHeader)
	fstream f(filename, ios::in);
	int voctype;
	f >> voctype;
	f.close();

	initVoc((VocParams::VocType)voctype);

	unsigned int pos = m_voc->Load(filename);

	f.open(filename, ios::in);
	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	f.seekg(pos); // vocabulary read

	_load<fstream>(f);

	f.close();
}

template<class T> 
void Database::_load(T& f)
{
	// Format:
	// N W'
	// WordId_0 K_0 EntryId_0_0 Value_0_0 ... EntryId_0_(K_0) Value_0_(K_0)
	// ...
	// WordId_(W'-1) K_(W'-1) EntryId_(W'-1)_0 Value_(W'-1)_0 ... EntryId_(W'-1)_{K_(W'-1)} Value_(W'-1)_{K_(W'-1)}
	//
	// Where:
	// [Vocabulary]: whole vocabulary in binary format
	// N (int32): number of entries in the database
	// W' (int32): number of words with some row in the inverted file
	// WordId_i (int32): word id in the inverted file
	// K_i (int32): number of entries in the row of the WordId_i
	// EntryId_i_k (int32): doc (entry) id where the word WordId_i is present
	// Value_i_k (double64): value of word WordId_i in entry EntryId_i_k
	//

	int N, W;
	f >> N >> W;

	m_index.resize(0);
	m_index.resize(m_voc->NumberOfWords());
	m_nentries = N;

	for(int i = 0; i < W; i++){
		int wordid, k;
		f >> wordid >> k;

		for(int j = 0; j < k; j++){
			int eid;
			double value;

			f >> eid >> value;

			m_index[wordid].push_back(IFEntry(eid, value));
		}
	}

}

void Database::initVoc(VocParams::VocType type, const Vocabulary *copy)
{
	delete m_voc;
	
	switch(type){
		case VocParams::HIERARCHICAL_VOC:

			if(copy)
				m_voc = new HVocabulary(
					*(static_cast<const HVocabulary*>(copy)));
			else
				m_voc = new HVocabulary(HVocParams(2,1));

			break;
	}

}


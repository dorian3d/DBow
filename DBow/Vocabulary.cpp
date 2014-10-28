/**
 * File: Vocabulary.h
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: generic vocabulary that must be inherited
 */

#include "Vocabulary.h"
#include "VocParams.h"
#include "DUtils.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <list>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <string>
using namespace std;

using namespace DBow;

Vocabulary::Vocabulary(const VocParams &params):
	m_created(false)
{
	m_params = new VocParams(params);
}

Vocabulary::Vocabulary(const char *filename):
	m_params(NULL)
{
	Load(filename);
}

Vocabulary::Vocabulary(const Vocabulary &voc)
{
	m_created = voc.m_created;
	m_frequent_words_stopped = voc.m_frequent_words_stopped;
	m_infrequent_words_stopped = voc.m_infrequent_words_stopped;
	m_word_frequency = voc.m_word_frequency;

	m_word_stopped = voc.m_word_stopped;
	m_stop_list = voc.m_stop_list;
	m_words_in_order = voc.m_words_in_order;

	m_params = new VocParams(*voc.m_params);
}

Vocabulary::~Vocabulary(void)
{
	delete m_params;
}

void Vocabulary::Save(const char *filename, bool binary) const
{
	if(binary)
		SaveBinary(filename);
	else
		SaveText(filename);	
}

unsigned int Vocabulary::Load(const char *filename)
{
	fstream f(filename, ios::in | ios::binary);

	if(!f.is_open()) throw DUtils::DException("Cannot open file");

	char c;
	f.get(c);

	f.close();

	// if c is >= 32, it is text
	if(c >= 32)
		return LoadText(filename);
	else
		return LoadBinary(filename);
	
}


VocInfo Vocabulary::RetrieveInfo() const
{
	VocInfo ret(*m_params);
	
	ret.VocType = m_params->Type;

	if(m_created){
		ret.WordCount = this->NumberOfWords();
		ret.StoppedFrequentWords = m_frequent_words_stopped;
		ret.StoppedInfrequentWords = m_infrequent_words_stopped;
	}else{
		ret.WordCount = 0;
		ret.StoppedFrequentWords = 0;
		ret.StoppedInfrequentWords = 0;
	}

	return ret;
}

double Vocabulary::Score(const BowVector &v, const BowVector &w) const
{
	// Note: this implementation is independent from the scoring
	// implementation in the class Database

	assert(v.isInOrder());
	assert(w.isInOrder());
	
	BowVector const *a, *b;
	BowVector v2 = v;
	BowVector w2 = w;

	VocParams::ScoringType norm;
	bool must_normalize = m_params->MustNormalize(norm);
	bool do_not_change_order = (m_params->Scoring == VocParams::KL);

	if(must_normalize){
		// normalize vectors
		v2.Normalize(norm);
		w2.Normalize(norm);

		if(do_not_change_order || (v2.size() < w2.size())){
			a = &v2;
			b = &w2;
		}else{
			a = &w2;
			b = &v2;
		}

	}else{
		
		if(do_not_change_order || (v.size() < w.size())){
			a = &v;
			b = &w;
		}else{
			a = &w;
			b = &v;
		}
	}

	// final score
	double score = 0.0;

	// a contains the shortest vector if the method is not KL
	unsigned int first_index = 0;
	
	BowVector::const_iterator ita;
	for(ita = a->begin(); ita != a->end(); ita++){		
		
		// binary search for ita->id
		int lo = first_index;
		int hi = b->size()-1;
		int mid;
		int pos = -1;
		
		while (lo <= hi)
		{
			mid = (lo + hi) / 2;
			if (ita->id == (*b)[mid].id){
				pos = mid;
				break;
			}else if (ita->id < (*b)[mid].id)
				hi = mid - 1;
			else
				lo = mid + 1;
		}
		
		if(pos >= 0){
			const WordValue vi = ita->value;
			const WordValue wi = (*b)[pos].value;
			
			// common non-zero entry found
			first_index = pos + 1;

			switch(m_params->Scoring){
				case VocParams::L1_NORM:
					score += fabs(vi - wi) - fabs(vi) - fabs(wi);
					break;

				case VocParams::L2_NORM:
					score += vi * wi;
					break;

				case VocParams::CHI_SQUARE:
					score += (vi - wi)*(vi - wi)/(vi + wi) - vi - wi;
					break;

				case VocParams::KL:
					score += vi * log(vi/wi);
					break;

				case VocParams::BHATTACHARYYA:
					score += sqrt(vi * wi);
					break;

				case VocParams::DOT_PRODUCT:
					score += vi * wi;
					break;
			}
		}else{
			const WordValue vi = ita->value;
			
			// in the case of KL, we must make some operations too
			if(m_params->Scoring == VocParams::KL){
				score += vi * (log(vi) - LOG_EPS);
			}
		}

	}

	switch(m_params->Scoring){
		case VocParams::L1_NORM:
			// ||v - w||_{L1} = 2 + Sum(|v_i - w_i| - |v_i| - |w_i|) 
			//		for all i | v_i != 0 and w_i != 0 
			// (Nister, 2006)
			if(m_params->ScaleScore) 
				score = -score/2.0;
			else
				score = 2.0 + score;
			break;

		case VocParams::L2_NORM:
			// ||v - w||_{L2} = sqrt( 2 - 2 * Sum(v_i * w_i) 
			//		for all i | v_i != 0 and w_i != 0 )
			// (Nister, 2006)
			if(m_params->ScaleScore) 
				score = 1.0 - sqrt(1.0 - score);
			else 
				score = sqrt(2 - 2 * score);
			break;

		case VocParams::CHI_SQUARE:
			// score = Sum (vi - wi)^2 / (vi + wi) ==
			//   Sum vi + Sum wi - Sum{i, wi != 0} vi - Sum{i, vi != 0} wi +
			//   + Sum_{i, vi != 0 && wi != 0} (vi - wi)^2 / (vi + wi)
			//
			score += accumulate(a->begin(), a->end(), 0.0)
				+ accumulate(b->begin(), b->end(), 0.0);
			
			if(m_params->ScaleScore) score = 1.0 - score/2.0;
			break;
			
		default: break;

		/*
		case VocParams::KL:
			// score = Sum (vi * log(vi/wi))
			// cannot scale
			break;

		case VocParams::BHATTACHARYYA:
			// score = Sum sqrt(vi * wi)
			// it is already scaled
			break;

		case VocParams::DOT_PRODUCT:
			// score = Sum (vi * wi)
			// cannot scale
			break;
		*/
	}

	return score;
}

void Vocabulary::StopWords(float frequent_words, float infrequent_words)
{
	if(!m_created) return;
	int nwords = m_words_in_order.size();
	StopWords((int)(frequent_words * nwords), (int)(infrequent_words * nwords));
}

void Vocabulary::StopWords(int frequent_words, int infrequent_words)
{
	if(!m_created) return;

	int nwords = m_words_in_order.size();
	int nfrequent = frequent_words;
	int ninfrequent = infrequent_words;

	if(nfrequent > nwords) nfrequent = nwords;
	if(ninfrequent > nwords) ninfrequent = nwords;

	// remove flags from already stopped words
	vector<WordId>::const_iterator it;
	for(it = m_stop_list.begin(); it != m_stop_list.end(); it++){
		m_word_stopped[*it] = false;
	}
	m_stop_list.resize(0);

	// get new stopped words
	for(int i = 0; i < ninfrequent; i++){
		WordId id = m_words_in_order[i];
		m_stop_list.push_back(id);
		m_word_stopped[id] = true;
	}

	for(int i = 0; i < nfrequent; i++){
		WordId id = m_words_in_order[ nwords - i - 1 ];
		m_stop_list.push_back(id);
		m_word_stopped[id] = true;
	}

	// update the number of stopped words
	m_frequent_words_stopped = nfrequent;
	m_infrequent_words_stopped = ninfrequent;	
}

void Vocabulary::Transform(const vector<float>& features, BowVector &v, bool arrange) const
{
	// words in v must be in ascending order

	assert(features.size() % m_params->DescriptorLength == 0);

	v.resize(0);
	v.reserve(features.size() / m_params->DescriptorLength);

	vector<WordId> stopped;
	stopped.reserve(v.capacity());

	// 3 implementations have been tried:
	// 1) unordered vector + sort
	// 2) ordered vector
	// 3) ordered list + conversion to vector
	// Number 1) worked better

	vector<float>::const_iterator it;

	int nd = 0;

	switch(m_params->Weighting){
		
		case VocParams::TF:
		case VocParams::IDF:
		case VocParams::TF_IDF:
			// Note: GetWordWeight returns at this moment the IDF value for TF_IDF and IDF, or 
			// 1 in the TF case. So that by multiplying by the tf- part, we get the final score.
			// We must multiply by n_i_d/n_d,
			// where n_i_d is the number of occurrences of word i in the document,
			// and n_d, the total number of words in the document

			// implementation 1) unordered vector + sort
			for(it = features.begin(); it < features.end(); it += m_params->DescriptorLength)
			{
				WordId id = Transform(it);
				
				if(isWordStopped(id)){
					vector<WordId>::iterator fit = find(stopped.begin(), stopped.end(), id);
					if(fit == stopped.end()){
						stopped.push_back(id);
						nd++;
					}
					
				}else{

					BowVector::iterator fit = find(v.begin(), v.end(), id);
					if(fit == v.end()){
						v.push_back(BowVectorEntry(id, GetWordWeight(id)));
						nd++;
					}else if(m_params->Weighting != VocParams::IDF){
						fit->value += GetWordWeight(id); // n_i_d is implicit in this operation
					}
				} // if word is stopped
			} // for feature

			// tf or tf-idf
			if(nd > 0 && m_params->Weighting != VocParams::IDF){
				for(BowVector::iterator fit = v.begin(); fit != v.end(); fit++) 
					fit->value /= (double)nd;
			}

			break;

		// - - 

		case VocParams::BINARY:
			// Weights are not used. Just put 1 in active words
			for(it = features.begin(); it < features.end(); it += m_params->DescriptorLength)
			{
				WordId id = Transform(it);
				
				if(!isWordStopped(id)){
					BowVector::iterator fit = find(v.begin(), v.end(), id);
					if(fit == v.end()){
						v.push_back(BowVectorEntry(id, 1));
					}
				} // if word is stopped
			} // for feature

			break;
	}

	if(arrange) v.PutInOrder();
	
}

void Vocabulary::GetWordWeightsAndCreateStopList(
	const vector<vector<float> >& training_features,
	vector<WordValue> &weights)
{
	const int NWords = GetNumberOfWords();
	const int NDocs = training_features.size();

	assert(NWords > 0 && NDocs > 0);

	weights.clear();
	weights.insert(weights.end(), NWords, 0);

	vector<vector<float> >::const_iterator mit;
	vector<float>::const_iterator fit;

	m_word_frequency.resize(0);
	m_word_frequency.resize(NWords, 0);

	switch(m_params->Weighting){
		case VocParams::IDF:
		case VocParams::TF_IDF:
			{
				// Note:
				// This is not actually a tf-idf score, but a idf score.
				// The complete tf-idf score is calculated in Vocabulary::Transform

				// calculate Ni: number of images in the voc data with
				// at least one descriptor vector path through node i.
				// calculate word frequency too
				vector<unsigned int> Ni(NWords, 0);
				vector<bool> counted(NWords, false);

				for(mit = training_features.begin(); mit != training_features.end(); mit++){
					fill(counted.begin(), counted.end(), false);
					
					for(fit = mit->begin(); fit < mit->end(); fit += m_params->DescriptorLength){
						WordId id = Transform(fit);
						
						m_word_frequency[id] += 1.f;
						if(!counted[id]){
							Ni[id]++;
							counted[id] = true;
						}
					}
				}

				// set ln(N/Ni)
				for(int i = 0; i < NWords; i++){
					if(Ni[i] > 0){
						weights[i] = log((double)NDocs / (double)Ni[i]);
					}// else // This cannot occur if using kmeans++
				}
			}

			break;

		// - -

		case VocParams::TF:
			fill(weights.begin(), weights.end(), 1);
			
		case VocParams::BINARY:
			// Note:
			// The tf score is calculated in Vocabulary::Transform
			// Here, we only create the frequency vector,
			// and fill weights with 1's.
			// In the binary case, weights are not necessary, so that their value
			// do not matter
			for(mit = training_features.begin(); mit != training_features.end(); mit++){
				for(fit = mit->begin(); fit < mit->end(); fit += m_params->DescriptorLength){
					WordId id = Transform(fit);
					m_word_frequency[id] += 1.f;
				}
			}
			
			break;
	}

	// set frequencies
	float total = accumulate(m_word_frequency.begin(), m_word_frequency.end(), 0.f);
	if(total > 0){
		transform(m_word_frequency.begin(), m_word_frequency.end(), 
			m_word_frequency.begin(), bind2nd(divides<float>(), total));
	}

	// create stop list
	CreateStopList();

}

float Vocabulary::GetWordFrequency(WordId id) const
{
	if(!m_created) return 0.f;

	assert(0 <= id && id < m_word_frequency.size());

	return m_word_frequency[id];
}

void Vocabulary::CreateStopList()
{
	// assert: m_word_frequency is set

	assert(m_word_frequency.size() > 0);

	m_stop_list.resize(0);

	m_word_stopped.resize(m_word_frequency.size());
	fill(m_word_stopped.begin(), m_word_stopped.end(), false);

	m_frequent_words_stopped = 0;
	m_infrequent_words_stopped = 0;

	// arrange words by ascending order
	m_words_in_order.resize(0);
	m_words_in_order.reserve(m_word_frequency.size());

	// better implementation with an ordered list + copy?
	vector<pair<float, WordId> > pairs; // <frequency, word id>
	pairs.reserve(m_word_frequency.size());

	vector<float>::const_iterator it;
	for(it = m_word_frequency.begin(); it != m_word_frequency.end(); it++){
		WordId id = it - m_word_frequency.begin();
		float fr = *it;
		
		pairs.push_back(make_pair(fr, id));
	}

	sort(pairs.begin(), pairs.end());

	vector<pair<float, WordId> >::const_iterator pit;
	for(pit = pairs.begin(); pit != pairs.end(); pit++){
		m_words_in_order.push_back(pit->second);
	}
}

void Vocabulary::SaveBinaryHeader(DUtils::BinaryFile &f) const
{
	// Binary header format:
	// XX Vt Wt St Ss D W SfW SiW 
	// 
	// Where:
	// XX (byte): magic word (byte with value 0) to identify the binary file
	// Vt (int32): vocabulary type
	// Wt (int32): weighting type
	// St (int32): scoring type
	// Ss (int32): scale score
	// D (int32): descriptor length
	// W (int32): actual number of words
	// SfW (int32): frequent nodes stopped
	// SiW (int32): infrequent nodes stopped

	f << '\0' // magic word
		<< (int)m_params->Type
		<< (int)m_params->Weighting 
		<< (int)m_params->Scoring
		<< (int)( m_params->ScaleScore ? 1 : 0 )
		<< m_params->DescriptorLength
		<< NumberOfWords()
		<< m_frequent_words_stopped 
		<< m_infrequent_words_stopped;
}

void Vocabulary::SaveTextHeader(fstream &f) const
{
	// Text header format:
	// Vt Wt St Ss D W SfW SiW 
	// 
	// Where:
	// Vt: vocabulary type
	// Wt: weighting type
	// St: scoring type
	// Ss: scale score
	// D: descriptor length
	// W: actual number of words
	// SfW: frequent nodes stopped
	// SiW: infrequent nodes stopped

	f  << m_params->Type << " "
		<< m_params->Weighting << " "
		<< m_params->Scoring << " "
		<< ( m_params->ScaleScore ? 1 : 0 ) << " "
		<< m_params->DescriptorLength << " "
		<< NumberOfWords() << " "
		<< m_frequent_words_stopped << " "
		<< m_infrequent_words_stopped
		<< endl;
}

int Vocabulary::LoadBinaryHeader(DUtils::BinaryFile &f)
{
	f.DiscardNextByte(); // magic word

	int voctype, weighting, scoring, scalescore, nwords, ndesc;

	f >> voctype >> weighting >> scoring >> scalescore >> ndesc >> nwords 
		>> m_frequent_words_stopped 
		>> m_infrequent_words_stopped;

	m_params->Type = (VocParams::VocType)voctype;
	m_params->Weighting = (VocParams::WeightingType)weighting;
	m_params->Scoring = (VocParams::ScoringType)scoring;
	m_params->ScaleScore = (scalescore != 0);
	m_params->DescriptorLength = ndesc;
	
	return nwords;
}

int Vocabulary::LoadTextHeader(fstream &f)
{
	int voctype, weighting, scoring, scalescore, nwords, ndesc;

	f >> voctype >> weighting >> scoring >> scalescore >> ndesc >> nwords 
		>> m_frequent_words_stopped 
		>> m_infrequent_words_stopped;

	m_params->Type = (VocParams::VocType)voctype;
	m_params->Weighting = (VocParams::WeightingType)weighting;
	m_params->Scoring = (VocParams::ScoringType)scoring;
	m_params->ScaleScore = (scalescore != 0);
	m_params->DescriptorLength = ndesc;
	
	return nwords;
}


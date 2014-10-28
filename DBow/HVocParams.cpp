/**
 * File: HVocParams.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: parameters to create a hierarchical vocabulary
 */

#include "HVocParams.h"

#include <sstream>
#include <string>
using namespace std;

using namespace DBow;

HVocParams::HVocParams(int k, int L, int desc_length,
					   WeightingType weighting, ScoringType scoring,
					   bool scale_score):
	VocParams(HIERARCHICAL_VOC, desc_length, weighting, scoring, scale_score)
{
	this->k = k;
	this->L = L;
}

HVocParams::~HVocParams(void)
{
}

string HVocParams::toString() const
{
	stringstream ss;
	ss << VocParams::toString();
	ss << "k: " << k << ", L: " << L << endl;
	return ss.str();
}


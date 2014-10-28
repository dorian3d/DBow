/**
 * File: VocParams.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: parameters to create a new vocabulary.
 *   This class must be inherited
 */

#include "VocParams.h"

#include <string>
#include <sstream>
using namespace std;

using namespace DBow;

VocParams::VocParams(VocType type, int desc_length, 
		WeightingType weighting, ScoringType scoring, bool scale_score)
{
	this->Type = type;
	this->DescriptorLength = desc_length;
	this->Weighting = weighting;
	this->Scoring = scoring;
	this->ScaleScore = scale_score;
}

VocParams::~VocParams(void)
{
}

string VocParams::toString() const
{
	stringstream ss;
	ss << "Vocabulary type: ";
	switch(Type){
		case HIERARCHICAL_VOC:
			ss << "hierarchical";
			break;
	}

	ss << endl << "WeightingType: ";
	switch(Weighting){
		case TF_IDF:
			ss << "tf-idf";
			break;
		case TF:
			ss << "tf";
			break;
		case IDF:
			ss << "idf";
			break;
		case BINARY:
			ss << "binary";
			break;
	}

	ss << endl << "ScoringType: ";
	switch(Scoring){
		case L1_NORM:
			ss << "L1 norm";
			break;
		case L2_NORM:
			ss << "L2 norm";
			break;
		case CHI_SQUARE:
			ss << "chi-square";
			break;
		case KL:
			ss << "KL";
			break;
		case BHATTACHARYYA:
			ss << "Bhattacharyya";
			break;
		case DOT_PRODUCT:
			ss << "dot product";
			break;
	}
	if(ScaleScore)
		ss << " scaling to 0..1";
	else
		ss << " without scaling";
	
	ss << endl;

	return ss.str();
}

bool VocParams::MustNormalize(ScoringType& norm) const
{
	return MustNormalize(Scoring, norm);
}

bool VocParams::MustNormalize(ScoringType scoring, ScoringType& norm)
{
	bool must_normalize;
	
	switch(scoring){
		case VocParams::L2_NORM:
			must_normalize = true;
			norm = VocParams::L2_NORM;
			break;

		case VocParams::L1_NORM:
		case VocParams::CHI_SQUARE:
		case VocParams::KL:
		case VocParams::BHATTACHARYYA:
			must_normalize = true;
			norm = VocParams::L1_NORM;
			break;
		
		default:
			must_normalize = false;
			break;
	}
	
	return must_normalize;
}

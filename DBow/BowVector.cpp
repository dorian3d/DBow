/**
 * File: BowVector.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: bag-of-words vector for representing images
 * Notes: see BowVector.h
 */

#include "BowVector.h"
#include <cmath>
#include <cassert>
#include <algorithm>
using namespace std;

using namespace DBow;

BowVector::BowVector(void)
{
}

BowVector::~BowVector(void)
{
}

void BowVector::Normalize(VocParams::ScoringType norm_type)
{
	assert(norm_type == VocParams::L1_NORM || norm_type == VocParams::L2_NORM);

	BowVector::iterator it;

	double norm = 0.0;
	switch(norm_type){
		
		case VocParams::L1_NORM:
			for(it = begin(); it != end(); it++)
				norm += fabs(it->value);

			break;
			
		case VocParams::L2_NORM:
			for(it = begin(); it != end(); it++)
				norm += it->value * it->value;
			norm = sqrt(norm);

			break;
			
		default:
			break;
	}

	if(norm > 0.0){
		for(it = begin(); it != end(); it++)
			it->value /= norm;
	}	
}

void BowVector::PutInOrder()
{
	sort(this->begin(), this->end());
}

bool BowVector::isInOrder() const
{
	unsigned int n = size();
	for(unsigned int i = 0; i < n-1; i++)
		if( (*this)[i].id >= (*this)[i+1].id ) return false;
	return true;
}

bool BowVectorEntry::operator< (const BowVectorEntry &w) const
{
	return(this->id < w.id);
}

bool BowVectorEntry::operator== (const BowVectorEntry &w) const
{
	return(this->id == w.id);
}

bool BowVectorEntry::operator== (WordId id) const
{
	return(this->id == id);
}

WordValue BowVectorEntry::operator+ (const BowVectorEntry &w) const
{
	return this->value + w.value;
}

namespace DBow
{
	WordValue operator+ (WordValue w, const BowVectorEntry &v)
	{
		return v.value + w;
	}
}


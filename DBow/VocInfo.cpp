/**
 * File: VocInfo.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: information structure retrieved from vocabularies
 */


#include "VocInfo.h"
#include "VocParams.h"

#include <string>
#include <sstream>
using namespace std;

using namespace DBow;

VocInfo::VocInfo(void): Parameters(NULL)
{
}

VocInfo::VocInfo(const VocInfo &v)
{
	*this = v;
}

VocInfo::VocInfo(const VocParams &params)
{
	this->Parameters = new VocParams(params);
	this->VocType = params.Type;
}

VocInfo::~VocInfo(void)
{
	delete this->Parameters;
}

VocInfo& VocInfo::operator=(const DBow::VocInfo &v)
{
	this->StoppedFrequentWords = v.StoppedFrequentWords;
	this->StoppedInfrequentWords = v.StoppedInfrequentWords;
	this->VocType = v.VocType;
	this->WordCount = v.WordCount;
	this->Parameters = new VocParams(*v.Parameters);

	return *this;
}

string VocInfo::toString() const
{
	stringstream ss;

	if(this->Parameters)
		ss << this->Parameters->toString() << endl;

	ss << "Number of words: " << WordCount << endl
		<< "Frequent words stopped: " << StoppedFrequentWords << endl
		<< "Infrequent words stopped: " << StoppedInfrequentWords << endl;

	return ss.str();	
}


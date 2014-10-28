/*
 * File: Timestamp.cpp
 * Author: Dorian Galvez
 * Date: March 2009
 * Description: timestamping functions
 * 
 * Note: in windows, this class has a 1ms resolution
 *
 */

#include <cstdio>
#include <cstdlib>

#ifdef WIN32
#include <sys/timeb.h>
#include <ctime>
#define sprintf sprintf_s
#else
#include <sys/time.h>
#endif

#include "Timestamp.h"

#include <cmath>
#include <sstream>
#include <iomanip>
using namespace std;

using namespace DUtils;

Timestamp::Timestamp(void)
{
}

Timestamp::~Timestamp(void)
{
}

void Timestamp::setToCurrentTime(){
	
#ifdef WIN32
	struct __timeb32 timebuffer;
	_ftime32_s( &timebuffer ); // C4996
	// Note: _ftime is deprecated; consider using _ftime_s instead
	m_secs = timebuffer.time;
	m_usecs = timebuffer.millitm * 1000;
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	m_secs = now.tv_sec;
	m_usecs = now.tv_usec;
#endif

}

void Timestamp::setTime(const string &stime){
	string::size_type p = stime.find('.');
	if(p == string::npos){
		m_secs = atol(stime.c_str());
	}else{
		m_secs = atol(stime.substr(0, p).c_str());
		
		string s_usecs = stime.substr(p+1, 6);
		m_usecs = atol(stime.substr(p+1).c_str());
		m_usecs *= (unsigned long)pow(10.0, double(6 - s_usecs.length()));
	}
}

double Timestamp::getFloatTime() const {
	return double(m_secs) + double(m_usecs)/1000000.0;
}

string Timestamp::getStringTime() const {
	char buf[32];
	sprintf(buf, "%.6lf", this->getFloatTime());
	return string(buf);
}

double Timestamp::operator- (const Timestamp &t) const {
	return this->getFloatTime() - t.getFloatTime();
}

Timestamp Timestamp::operator+ (double s) const
{
	unsigned long secs = (long)floor(s);
	unsigned long usecs = (long)((s - (double)secs) * 1e6);

	Timestamp t;

	const unsigned long max = 1000000ul;

	if(m_usecs + usecs >= max)
		t.setTime(m_secs + secs + 1, m_usecs + usecs - max);
	else
		t.setTime(m_secs + secs, m_usecs + usecs);
	
	return t;
}

Timestamp Timestamp::operator- (double s) const
{
	unsigned long secs = (long)floor(s);
	unsigned long usecs = (long)((s - (double)secs) * 1e6);

	Timestamp t;

	const unsigned long max = 1000000ul;

	if(m_usecs < usecs)
		t.setTime(m_secs - secs - 1, max - (usecs - m_usecs));
	else
		t.setTime(m_secs - secs, m_usecs - usecs);
	
	return t;
}

bool Timestamp::operator> (const Timestamp &t) const
{
	if(m_secs > t.m_secs) return true;
	else if(m_secs == t.m_secs) return m_usecs > t.m_usecs;
	else return false;
}

bool Timestamp::operator>= (const Timestamp &t) const
{
	if(m_secs > t.m_secs) return true;
	else if(m_secs == t.m_secs) return m_usecs >= t.m_usecs;
	else return false;
}

bool Timestamp::operator< (const Timestamp &t) const
{
	if(m_secs < t.m_secs) return true;
	else if(m_secs == t.m_secs) return m_usecs < t.m_usecs;
	else return false;
}

bool Timestamp::operator<= (const Timestamp &t) const
{
	if(m_secs < t.m_secs) return true;
	else if(m_secs == t.m_secs) return m_usecs <= t.m_usecs;
	else return false;
}

bool Timestamp::operator== (const Timestamp &t) const
{
	return(m_secs == t.m_secs && m_usecs == t.m_usecs);
}


string Timestamp::Format() const {
	return Timestamp::Format(getFloatTime());
}

string Timestamp::Format(double s) {
	int days = int(s / (24. * 3600.0));
	s -= days * (24. * 3600.0);
	int hours = int(s / 3600.0);
	s -= hours * 3600;
	int minutes = int(s / 60.0);
	s -= minutes * 60;
	int seconds = int(s);
	int ms = int((s - seconds)*1e6);

	stringstream ss;
	ss.fill('0');
	bool b;
	if(b = (days > 0)) ss << days << "d ";
	if(b = (b || hours > 0)) ss << setw(2) << hours << ":";
	if(b = (b || minutes > 0)) ss << setw(2) << minutes << ":";
	if(b) ss << setw(2);
	ss << seconds;
	if(!b) ss << "." << setw(6) << ms;

	return ss.str();
}



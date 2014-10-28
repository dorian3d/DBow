/*
 * File: Timestamp.h
 * Author: Dorian Galvez
 * Date: March 2009
 * Description: timestamping functions
 */

#ifndef __D_TIMESTAMP__
#define __D_TIMESTAMP__

#include <iostream>
using namespace std;

namespace DUtils {

class Timestamp
{
public:
	Timestamp(void);
	~Timestamp(void);

	/* Sets this instance to the current time
	 */
	void setToCurrentTime();

	/* Sets the timestamp from seconds and microseconds
	 * @param secs: seconds
	 * @param usecs: microseconds
	 */
	inline void setTime(unsigned long secs, unsigned long usecs){
		m_secs = secs;
		m_usecs = usecs;
	}

	/* Sets the timestamp from a string with the time in seconds
	 * @param stime: string such as "1235603336.036609"
	 */
	void setTime(const string &stime);

	/* Returns this timestamp as the number of seconds in (long) float format
	 */
	double getFloatTime() const;

	/* Returns this timestamp as the number of seconds in fixed length string format
	 */
	string getStringTime() const;

	/* Returns the difference in seconds between this timestamp (greater) and t (smaller)
	 * If the order is swapped, a negative number is returned
	 * @param t: timestamp to subtract from this timestamp
	 * @return difference in seconds
	 */
	double operator- (const Timestamp &t) const;

	/* Returns a copy of this timestamp + s seconds
	 * @param s: seconds
	 */
	Timestamp operator+ (double s) const;

	/* Returns a copy of this timestamp - s seconds
	 * @param s: seconds
	 */
	Timestamp operator- (double s) const;

	/* Returns whether this timestamp is at the future of t
	 * @param t
	 */
	bool operator> (const Timestamp &t) const;

	/* Returns whether this timestamp is at the future of (or is the same as) t
	 * @param t
	 */
	bool operator>= (const Timestamp &t) const;

	/* Returns whether this timestamp and t represent the same instant
	 * @param t
	 */
	bool operator== (const Timestamp &t) const;

	/* Returns whether this timestamp is at the past of t
	 * @param t
	 */
	bool operator< (const Timestamp &t) const;

	/* Returns whether this timestamp is at the past of (or is the same as) t
	 * @param t
	 */
	bool operator<= (const Timestamp &t) const;

	/* Returns a string version of the timestamp, with the format
	 * xd hh:mm:ss, hh:mm:ss, mm:ss or s.us
	 * @param s: timestamp in long format (given by getFloatTime) to format
	 */
	string Format() const;
	static string Format(double s);
	

protected:
	unsigned long m_secs;	// seconds
	unsigned long m_usecs;	// microseconds
};

}

#endif


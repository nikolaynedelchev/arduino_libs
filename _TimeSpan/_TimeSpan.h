/* 
* TimeDuration.h
*
* Created: 13.9.2014 г. 20:12:54
* Author: Nikolay
*/


#ifndef __TIMESPAN_H__
#define __TIMESPAN_H__

#include "Arduino.h"

class TimeSpan
{
//variables
public:
protected:
private:
	uint32_t	m_MS;
//functions
public:
	TimeSpan(uint32_t durationInMs) : m_MS(durationInMs){};
	TimeSpan() : m_MS(0){};
	~TimeSpan(){};
	
	int16_t		MSeconds()		const	{return (int16_t)	(InMSeconds()	- (InSeconds()	* 1000UL)); };
	int8_t		Seconds()		const	{return (int8_t)	(InSeconds()	- (InMinutes()	* 60UL));};
	int8_t		Minutes()		const	{return (int8_t)	(InMinutes()	- (InHours()	* 60UL));};
	int8_t		Hours()			const	{return (int8_t)	(InHours()		- (InDays()		* 24UL));};
	int8_t		Days()			const	{return (int8_t)	(InDays());};
	
	uint32_t	InMSeconds()	const	{return m_MS;};
	uint32_t	InSeconds()		const	{return m_MS / (1000UL);};
	uint32_t	InMinutes()		const	{return m_MS / (1000UL * 60UL);};
	uint32_t	InHours()		const	{return m_MS / (1000UL * 60UL * 60UL);};
	uint32_t	InDays()		const	{return m_MS / (1000UL * 60UL * 60UL * 24UL);};
		
	static TimeSpan		FromSeconds	(uint32_t seconds)	{return TimeSpan(seconds	* 1000UL);};
	static TimeSpan		FromMinutes	(uint32_t minutes)	{return TimeSpan(minutes	* 1000UL * 60UL);};
	static TimeSpan		FromHours	(uint32_t hours)	{return TimeSpan(hours		* 1000UL * 60UL * 60UL);};
	static TimeSpan		FromDays	(uint32_t days)		{return TimeSpan(days		* 1000UL * 60UL * 60UL * 24UL);};
	static TimeSpan		TillNow		(uint32_t from)		{return TimeSpan(millis() - from);};

	static TimeSpan		Infinity()						{return TimeSpan(~(0UL));};

    bool operator>	(TimeSpan &tS)	{return m_MS >  tS.m_MS;};
    bool operator>	(uint32_t ms)	{return m_MS >  ms;};
	bool operator>	(int ms)		{ return m_MS >  (uint32_t)ms; };
		
    bool operator<=	(TimeSpan &tS)	{return m_MS <= tS.m_MS;};
    bool operator<=	(uint32_t ms)	{return m_MS <= ms;};
	bool operator<=	(int ms)		{ return m_MS <= (uint32_t)ms; };

    bool operator<	(TimeSpan &tS)	{return m_MS <  tS.m_MS;};
    bool operator<	(uint32_t ms)	{return m_MS <  ms;};
	bool operator<	(int ms)		{ return m_MS <  (uint32_t)ms; };
		
    bool operator>=	(TimeSpan &tS)	{return m_MS >= tS.m_MS;};
    bool operator>=	(uint32_t ms)	{return m_MS >= ms;};
	bool operator>=	(int ms)		{ return m_MS >= (uint32_t)ms; };
		
    bool operator==	(TimeSpan &tS)	{return m_MS == tS.m_MS;};
    bool operator==	(uint32_t ms)	{return m_MS == ms;};
	bool operator==	(int ms)		{ return m_MS == (uint32_t)ms; };
		
    bool operator!=	(TimeSpan &tS)	{return m_MS != tS.m_MS;};
    bool operator!=	(uint32_t ms)	{return m_MS != ms;};
	bool operator!=	(int ms)		{ return m_MS != (uint32_t)ms; };

	String ToShortString()
	{
		String str = "";
		uint32_t inDays = InDays();
		uint32_t inHours = InHours();
		uint32_t inMinutes = InMinutes();
		uint32_t inSeconds = InSeconds();

		if (inDays > 0)
		{
			str += String(inDays) + "d";
		}
		else if (inHours > 0)
		{
			str += String(inHours) + "h";
		}
		else if (inMinutes > 0)
		{
			str += String(inMinutes) + "m";
		}
		else if (inSeconds > 0)
		{
			str += String(inSeconds) + "s";
		}
        else
        {
            str += String(m_MS) + "ms";
        }
		return str;
	};

	String ToString()
	{
		String str = "";
		uint32_t days = Days();
		uint32_t hours = Hours();
		uint32_t minutes = Minutes();
		uint32_t seconds = Seconds();
		if (days > 0)
		{
			str += String(days) + (days > 1) ? "days" : "day";
			if (hours > 0)
			{
				str += String(", ") + String(hours) + String("h");
			}
		}
		else if (hours > 0)
		{
			str += String(hours) + "h";
			if (minutes > 0)
			{
				str += String(", ") + String(minutes) + String("min");
			}
		}
		else if (minutes > 0)
		{
			str += String(minutes) + "min";
			if (seconds > 0)
			{
				str += String(", ") + String(seconds) + String("sec");
			}
		}
		else
		{
			str += String(seconds) + "sec";
		}
		return str;
	};

	TimeSpan	operator+	(const TimeSpan &tS)	{return TimeSpan(m_MS + tS.m_MS);};
	TimeSpan	operator+	(const uint32_t ms)		{return TimeSpan(m_MS + ms);};
	TimeSpan	operator+	(const int ms)			{return TimeSpan(m_MS + ms);};
		
	TimeSpan	operator-	(const TimeSpan &tS)	{return TimeSpan(m_MS - tS.m_MS);};
	TimeSpan	operator-	(const uint32_t ms)		{return TimeSpan(m_MS - ms);};
	TimeSpan	operator-	(const int ms)			{return TimeSpan(m_MS - ms);};
		
	TimeSpan&	operator+=	(const TimeSpan &tS)	{m_MS += tS.m_MS; return *this;};
	TimeSpan&	operator+=	(const uint32_t ms)		{m_MS += ms; return *this;};
	TimeSpan&	operator+=	(const int ms)			{m_MS += ms; return *this;};
		
	TimeSpan&	operator-=	(const TimeSpan &tS)	{m_MS -= tS.m_MS; return *this;};
	TimeSpan&	operator-=	(const uint32_t ms)		{m_MS -= ms; return *this;};
	TimeSpan&	operator-=	(const int ms)			{m_MS -= ms; return *this;};
	
	operator uint32_t() { return m_MS; };
protected:
private:
}; //TimeSpan

#endif //__TIMESPAN_H__

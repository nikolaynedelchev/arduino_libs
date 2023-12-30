#pragma once
#include "Arduino.h"
#include "_TimeSpan.h"


//class enum Months {
//	
//};

enum Months
{
	M_JAN = 0, M_FEB, M_MAR, M_APR, M_MAY, M_JUN, M_JUL, M_AUG, M_SEP, M_OCT, M_NOV, M_DEC
};

enum WeekDay {
	Monday,     //    assigned 0
	Tuesday,    //    assigned 1
	Wednesday,  //    assigned 2
	Thursday,   //    assigned 3
	Friday,     //    assigned 4
	Saturday,   //    assigned 5
	Sunday      //    assigned 6
};

struct DateTimeStruct
{
	int16_t		Year;
	int8_t		Month;
	int8_t		Day;
	int8_t		Hour;
	int8_t		Minute;
	int8_t		Second;
	int16_t		MSecond;
};

class DateTime
{
public:
	DateTime();
	~DateTime();
	const DateTimeStruct& Now();
	void SetDateTime(const DateTimeStruct& dateTime);
	int GetWeekDay();
	bool NeedDSTPlusOne();
	bool NeedDSTMinusOne();
	void MarkPlusOne() { DST_PlusOne = true; DST_MinusOne = false; };
	void MarkMinusOne() { DST_PlusOne = false; DST_MinusOne = true; };

	void AddTime(const TimeSpan& timeSpan);
	void ReduceTime(const TimeSpan& timeSpan);

// Operators
	DateTime	operator+	(const TimeSpan &tS)	{ DateTime dt; dt = *this; dt.AddTime(tS); return dt; };
	DateTime	operator+	(const uint32_t ms)		{ return *this + TimeSpan(ms); };
	DateTime	operator+	(const int ms)			{ return *this + TimeSpan((uint32_t)ms); };

	DateTime	operator-	(const TimeSpan &tS)	{ DateTime dt; dt = *this; dt.ReduceTime(tS); return dt; };
	DateTime	operator-	(const uint32_t ms)		{ return *this - TimeSpan(ms); };
	DateTime	operator-	(const int ms)			{ return *this - TimeSpan((uint32_t)ms); };

	DateTime&	operator+=	(const TimeSpan &tS)	{ this->AddTime(tS); return *this; };
	DateTime&	operator+=	(const uint32_t ms)		{ this->AddTime(TimeSpan(ms)); return *this; };
	DateTime&	operator+=	(const int ms)			{ this->AddTime(TimeSpan((uint32_t)ms)); return *this; };

	DateTime&	operator-=	(const TimeSpan &tS)	{ this->ReduceTime(tS); return *this; };
	DateTime&	operator-=	(const uint32_t ms)		{ this->ReduceTime(TimeSpan(ms)); return *this; };
	DateTime&	operator-=	(const int ms)			{ this->ReduceTime(TimeSpan((uint32_t)ms)); return *this; };

	bool operator>	(DateTime &tS);
	bool operator>=	(DateTime &tS);

	bool operator<	(DateTime &tS) 
	{ 
		auto res = (tS >= *this);
		return res; 
	};
	bool operator<=	(DateTime &tS) 
	{ 
		auto res = (tS > *this);
		return res; 
	};

private:
	int	 GetMonthDays();
	void Update();

	DateTimeStruct	m_now;
	uint32_t	m_lastUpdate;
	bool		DST_PlusOne;
	bool		DST_MinusOne;
};


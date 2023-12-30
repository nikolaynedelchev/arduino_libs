#include "_DateTime.h"
#include "_TimeSpan.h"

static char s_Daytab[12] =
{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/////////////////////////////////////////////////////////////
DateTime::DateTime()
	: DST_PlusOne(false)
	, DST_MinusOne(false)
{
	memset(&(this->m_now), 0, sizeof(DateTimeStruct));
	m_now.Year = 2014;
	m_lastUpdate = millis();
}

/////////////////////////////////////////////////////////////
DateTime::~DateTime()
{
}

/////////////////////////////////////////////////////////////
const DateTimeStruct& DateTime::Now()
{
	Update();
	return m_now;
}

/////////////////////////////////////////////////////////////
void DateTime::SetDateTime(const DateTimeStruct& dateTime)
{
	m_lastUpdate = millis();
	memcpy((void*)(&(m_now)), (void*)(&(dateTime)), sizeof(DateTimeStruct));
}

/////////////////////////////////////////////////////////////
int DateTime::GetWeekDay()
{
	Update();
	int d = m_now.Day + 1;
	int m = m_now.Month + 1;
	int y = m_now.Year;

	return ((d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) + 6) % 7;
}

/////////////////////////////////////////////////////////////
void DateTime::Update()
{
	uint32_t nowSys = millis();
	TimeSpan timeSpan(nowSys - m_lastUpdate);

	if (timeSpan < 1000)
	{
		return;
	}

	m_lastUpdate = nowSys;
	AddTime(timeSpan);
}

/////////////////////////////////////////////////////////////
int DateTime::GetMonthDays()
{
	if (1 != m_now.Month)
	{
		return s_Daytab[m_now.Month] + 0;
	}
	uint16_t y = m_now.Year;

	// if (year is not exactly divisible by 4) then (it is a common year)
	if (y % 4)
	{
		return s_Daytab[m_now.Month] + 0;
	}

	// else
	//if (year is not exactly divisible by 100) then(it is a leap year)
	if (y % 100)
	{
		return s_Daytab[m_now.Month] + 1;
	}

	// else
	// if (year is not exactly divisible by 400) then(it is a common year)
	if (y % 400)
	{
		return s_Daytab[m_now.Month] + 0;
	}

	//else (it is a leap year)
	return s_Daytab[m_now.Month] + 1;
}

/////////////////////////////////////////////////////////////
bool DateTime::NeedDSTPlusOne()
{
	if (2 != m_now.Month)
		return false;
	if (m_now.Day < (s_Daytab[2] - 7))
		return false;
	if (m_now.Hour < 2)
		return false;
	if (GetWeekDay() != 6)
		return false;

	if (DST_PlusOne)
		return false;
	DST_PlusOne = true;
	DST_MinusOne = false;
	return true;
}

/////////////////////////////////////////////////////////////
bool DateTime::NeedDSTMinusOne()
{
	if (9 != m_now.Month)
		return false;
	if (m_now.Day < (s_Daytab[9] - 7))
		return false;
	if (m_now.Hour < 2)
		return false;
	if (GetWeekDay() != 6)
		return false;

	if (DST_MinusOne)
		return false;
	DST_MinusOne = true;
	DST_PlusOne = false;
	return true;
}

/////////////////////////////////////////////////////////////
void DateTime::AddTime(const TimeSpan& timeSpan)
{
	m_now.MSecond += timeSpan.MSeconds();
	if (m_now.MSecond >= 1000)
	{
		m_now.Second++;
		m_now.MSecond -= 1000;
	}

	m_now.Second += timeSpan.Seconds();
	if (m_now.Second >= 60)
	{
		m_now.Minute++;
		m_now.Second -= 60;
	}

	m_now.Minute += timeSpan.Minutes();
	if (m_now.Minute >= 60)
	{
		m_now.Hour++;
		m_now.Minute -= 60;
	}

	m_now.Hour += timeSpan.Hours();
	if (m_now.Hour >= 24)
	{
		m_now.Day++;
		m_now.Hour -= 24;
	}

	int maxD = GetMonthDays();

	m_now.Day += timeSpan.Days();
	while (m_now.Day >= maxD)
	{
		m_now.Month++;
		m_now.Day -= maxD;
		if (m_now.Month >= 12)
		{
			m_now.Year++;
			m_now.Month -= 12;
		}
		maxD = GetMonthDays();
	}
}

/////////////////////////////////////////////////////////////
void DateTime::ReduceTime(const TimeSpan& timeSpan)
{
	m_now.MSecond -= timeSpan.MSeconds();
	if (m_now.MSecond < 0)
	{
		m_now.Second--;
		m_now.MSecond += 1000;
	}

	m_now.Second -= timeSpan.Seconds();
	if (m_now.Second < 0)
	{
		m_now.Minute--;
		m_now.Second += 60;
	}

	m_now.Minute -= timeSpan.Minutes();
	if (m_now.Minute < 0)
	{
		m_now.Hour--;
		m_now.Minute += 60;
	}

	m_now.Hour -= timeSpan.Hours();
	if (m_now.Hour < 0)
	{
		m_now.Day--;
		m_now.Hour += 24;
	}

	m_now.Day -= timeSpan.Days();
	while (m_now.Day < 0)
	{
		m_now.Month--;
		if (m_now.Month < 0)
		{
			m_now.Year--;
			m_now.Month += 12;
		}
		
		m_now.Day += GetMonthDays();
	}
}

/////////////////////////////////////////////////////////////
bool DateTime::operator>(DateTime &tS)
{
	// Year
	if (m_now.Year > tS.m_now.Year)
		return true;
	if (m_now.Year < tS.m_now.Year)
		return false;

	// Month
	if (m_now.Month> tS.m_now.Month)
		return true;
	if (m_now.Month < tS.m_now.Month)
		return false;

	// Day
	if (m_now.Day > tS.m_now.Day)
		return true;
	if (m_now.Day < tS.m_now.Day)
		return false;

	// Hour
	if (m_now.Hour > tS.m_now.Hour)
		return true;
	if (m_now.Hour < tS.m_now.Hour)
		return false;

	// Minute
	if (m_now.Minute > tS.m_now.Minute)
		return true;
	if (m_now.Minute < tS.m_now.Minute)
		return false;

	// Second
	if (m_now.Second > tS.m_now.Second)
		return true;
	if (m_now.Second < tS.m_now.Second)
		return false;

	// MSecond
	if (m_now.MSecond > tS.m_now.MSecond)
		return true;
	//if (m_now.MSecond < tS.m_now.MSecond)
	//	return false;
	return false;
}

/////////////////////////////////////////////////////////////
bool DateTime::operator>=(DateTime &tS)
{
	// Year
	if (m_now.Year > tS.m_now.Year)
		return true;
	if (m_now.Year < tS.m_now.Year)
		return false;

	// Month
	if (m_now.Month> tS.m_now.Month)
		return true;
	if (m_now.Month < tS.m_now.Month)
		return false;

	// Day
	if (m_now.Day > tS.m_now.Day)
		return true;
	if (m_now.Day < tS.m_now.Day)
		return false;

	// Hour
	if (m_now.Hour > tS.m_now.Hour)
		return true;
	if (m_now.Hour < tS.m_now.Hour)
		return false;

	// Minute
	if (m_now.Minute > tS.m_now.Minute)
		return true;
	if (m_now.Minute < tS.m_now.Minute)
		return false;

	// Second
	if (m_now.Second > tS.m_now.Second)
		return true;
	if (m_now.Second < tS.m_now.Second)
		return false;

	// MSecond
	if (m_now.MSecond > tS.m_now.MSecond)
		return true;
	if (m_now.MSecond < tS.m_now.MSecond)
		return false;

	return true;
}

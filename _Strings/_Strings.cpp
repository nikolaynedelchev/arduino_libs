#include "_Strings.h"
#include <Arduino.h>
#include <Stream.h>

String CreateStringFromPGM(const char * pgmPtr)
{
	String str("");
	CreateStringFromPGM(pgmPtr, str);
	return str;
}

void CreateStringFromPGM(const char * pgmPtr, String& outString)
{
	outString = "";
	CopyToStringFromPGM(pgmPtr, outString);
}

void AddPGMString(String& outString, const char * pgmPtr)
{
	String tempStr = "";
	CreateStringFromPGM(pgmPtr, tempStr);
	outString += tempStr;
}

void CopyToStringFromPGM(const char * pgmPtr, String& outString)
{
	if (nullptr == pgmPtr)
	{
		return;
	}
	char c;
	while (0 != (c = (char)pgm_read_byte(pgmPtr)))
	{
		outString += c;
		pgmPtr++;
	}
}

uint16_t LengthStringFromPGM(const char * pgmPtr)
{
	if (nullptr == pgmPtr)
	{
		return 0;
	}
	uint16_t res = 0;
	while (0 != ((char)pgm_read_byte(pgmPtr)))
	{
		res++;
		pgmPtr++;
	}
	return res;

}

String RemoveWord(String& str)
{
	while ((str.length() > 0) && (str.charAt(0) == ' '))
		str.remove(0, 1);
	String res;
	while ((str.length() > 0) && (str.charAt(0) != ' '))
	{
		res += str.charAt(0);
		str.remove(0, 1);
	}

	return res;
}

int RemoveToDigit(String& str)
{
	while ((str.length() > 0) && (false == isDigit(str.charAt(0))))
	{
		str.remove(0, 1);
	}
	return (int)str.length();
}

int RemoveAllDigit(String& str)
{
	while ((str.length() > 0) && (true == isDigit(str.charAt(0)))) str.remove(0, 1);
	return (int)str.length();
}

////////////////////////////////////////////////
// Extract first quoted string, remove it from input string and return it as result
// inputString : 'aaaa"bbb"cc'
// outut : 'bbb'
// inputString : 'cc'
String RemoveFirstQuotesString(String& str)
{
	String result("");
	while ((str.length() > 0) && ('"' != str.charAt(0))) str.remove(0, 1);
	if (0 == str.length())
		return result;
	str.remove(0, 1);
	while ((str.length() > 0) && ('"' != str.charAt(0)))
	{
		result += str.charAt(0);
		str.remove(0, 1);
	}
	if (0 == str.length())
		return result;
	str.remove(0, 1);

	return result;
}
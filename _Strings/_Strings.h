#pragma once
#include <Arduino.h>
#include <Stream.h>
#include <avr/pgmspace.h>


#ifdef __INTELLISENSE__
// help Intellisense
#define FLASH_MEM
#define FMEM
#undef F
#define F

#else
#define FLASH_MEM PROGMEM
#define FMEM PSTR

#endif
typedef const char* FlashMemPtr;

#define REG_STR(name, str) const char name[] FLASH_MEM = { str };

#define STRING(str) (CreateStringFromPGM(FMEM(str)))
#define S(name) (CreateStringFromPGM(&((name)[0])))
#define SL(name) (LengthStringFromPGM(&((name)[0])))


String CreateStringFromPGM(const char * pgmPtr);
void CreateStringFromPGM(const char * pgmPtr, String& outString);
void CopyToStringFromPGM(const char * pgmPtr, String& outString);
void AddPGMString(String& outString, const char * pgmPtr);

uint16_t LengthStringFromPGM(const char * pgmPtr);

int RemoveToDigit(String& str);
int RemoveAllDigit(String& str);
String RemoveFirstQuotesString(String& str);
int RemoveToDigit(String& str);
String RemoveWord(String& str);

// FlashMemPtr fPtr = FMEM("Some message");
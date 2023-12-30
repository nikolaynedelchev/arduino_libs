#pragma once
#include <Arduino.h>
#include "SmsMsg.h"

class MsgConstructor
{
public:
	void	AddMsg(SmsMsg&& msg);
	bool	GetMsg(SmsMsg& msg);

private:
	constexpr static uint16_t s_cap = 5;
	bool IsEmpty() const;
	static void inc(uint8_t& idx);
	SmsMsg m_messages[s_cap];
	uint8_t m_front = 0;
	uint8_t m_back = 0;
	uint8_t m_size = 0;
};



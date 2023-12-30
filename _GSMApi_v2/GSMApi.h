#pragma once
#include "MsgConstructor.h"
#include "GSMDriver.h"
#include <_DateTime.h>

class GSMApi
{
public:
	GSMApi();
	~GSMApi();
	void SetGsmDriver(GSMDriver* gsmPtr);
	void Update();

	void SendMsg(SmsMsg&& msg);
	bool RcvMsg(String& number, String& msg);
	bool RcvCall(String& number);

private:
	uint32_t				m_lastGsmOperation;
	uint16_t				m_delay;

	GSMDriver*				m_gsm;
	bool					m_incommingCall;
	String					m_callNumber;

	MsgConstructor			m_outgoing;
	MsgConstructor			m_incomming;
};


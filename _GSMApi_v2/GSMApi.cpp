#include "GSMApi.h"
#include "_Debug.h"
#include <_TimeSpan.h>
#include <_a_std.h>

#define CLOCK_GET_TIME TimeSpan::FromSeconds(30)
//#define SEND_FAKE_SMS

//////////////////////////////////////////////////////////////////////////
GSMApi::GSMApi()
	: m_lastGsmOperation(0)
	, m_delay(0)
	, m_gsm(nullptr)
	, m_incommingCall(false)
	, m_callNumber(String())
{
}

//////////////////////////////////////////////////////////////////////////
GSMApi::~GSMApi()
{
}

//////////////////////////////////////////////////////////////////////////
void GSMApi::SetGsmDriver(GSMDriver* gsmPtr)
{
	m_gsm = gsmPtr;
}

//////////////////////////////////////////////////////////////////////////
void GSMApi::Update()
{
	if (nullptr == m_gsm)
	{
		return;
	}

	// loop gsm driver
	m_gsm->Loop();

	// check for incomming calls
	if (m_gsm->HasNewCall(m_callNumber))
	{
		m_incommingCall = true;
		m_lastGsmOperation = millis();
		m_delay = 2000;
	}

	SmsMsg sms;
	// check for incomming messages
	if (m_gsm->HasNewSMS(sms.number, sms.msg))
	{
		m_incomming.AddMsg(astd::move(sms));
		m_lastGsmOperation = millis();
		m_delay = 1000;
	}

	// protect gsm modem from flooding
	if (millis() - m_lastGsmOperation < m_delay)
	{
		return;
	}

	if ((GSMDriverStatus::READY != m_gsm->Status()) || (false == m_gsm->IsSendSMSReady()))
	{
		return;
	}

	// check for ready outgoing messages and send them one by one

	if (m_outgoing.GetMsg(sms))
	{
		auto& body = sms.msg;
		if (body.length() > 0)
		{
			m_gsm->SendSMS(sms.number, body);
			m_lastGsmOperation = millis();
			m_delay = 4000;
		}
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
void GSMApi::SendMsg(SmsMsg&& msg)
{
#ifdef SEND_FAKE_SMS
	msg.AssembleBody();
	msg.StartNumbersIteration();
	String fakeNumber;
	while (msg.GetNextNumber(fakeNumber))
	{
		DBG_F_PRINT("SENDING FAKE SMS to : ");
		DBG_PRINT(fakeNumber);
		DBG_F_PRINTLN(", MSG : ");
		DBG_PRINTLN(msg.GetBody());
		delay(50);
		return;
	}
	//DBG_F_PRINT()
#else
	DBG_F_PRINTLN("= ADD MSG TO RING BUFFER =");
	m_outgoing.AddMsg(astd::move(msg));
#endif
}

//////////////////////////////////////////////////////////////////////////
bool GSMApi::RcvMsg(String& number, String& msg)
{
	SmsMsg sms;
	if (m_incomming.GetMsg(sms))
	{
		DBG_F_PRINTLN("= RCV MSG =");

		number = sms.number;
		msg = sms.msg;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool GSMApi::RcvCall(String& number)
{
	if (m_incommingCall)
	{
		DBG_F_PRINTLN("= RCV CALL =");

		m_incommingCall = false;
		number = astd::move(m_callNumber);
		return true;
	}
	return false;
}

#include "GSMDriver.h"
#include <Arduino.h>
#include <Stream.h>
#include "GSMDriverPrivates.h"
#include <_Debug.h>
#include <_Strings.h>
#include <_a_std.h>

/////////////////////////////////////////////////////////////////
void GSMDriver::SendCommand(String& comm)
{
	//DBG_PRINTLN(STRING("Command : ") + comm);

	m_GSM->print(comm);
	m_GSM->print((char)(GSMDriverConsts::CarriageReturn));
	String dummy = astd::move(comm);
}

/////////////////////////////////////////////////////////////////
void GSMDriver::ResponsiveDelay(uint32_t duration)
{
	uint32_t start = millis();

	if (m_delayStartFunc)
		m_delayStartFunc();

	while (millis() - start < duration)
		if (m_delaySysFunc)
			m_delaySysFunc();

	if (m_delayEndFunc)
		m_delayEndFunc();
}

/////////////////////////////////////////////////////////////////
String GSMDriver::ReadString(uint32_t timeout, bool& good, bool readMsg)
{
	uint32_t startTime = millis();
	String workString;
	DELAY_START_FUNC();
	good = true;

	while (millis() - startTime < timeout)
	{
		if (!m_GSM->available())
		{
			DELAY_FUNC();
			continue;
		}

		int inChar = m_GSM->read();

		if (inChar < 0)
		{
			DELAY_END_FUNC();
			good = false;
			return astd::move(workString);
		}

		if (
			((!readMsg) && ((int)GSMDriverConsts::CarriageReturn == inChar)) || 
			((int)GSMDriverConsts::LineFeed == inChar))
		{
			if (workString.length() > 0)
			{
				DELAY_END_FUNC();
				good = true;
				return astd::move(workString);
			}
			continue;
		}
		workString += (char)inChar;
	}
	good = false;
	return astd::move(workString);
}


/////////////////////////////////////////////////////////////////
GSMDriver::GSMDriver(	Stream *stream,
						SystemFunction powerONFunc,
						SystemFunction resetFunc,
						SystemFunction delaySysFunc,
						SystemFunction delayStartFunc,
						SystemFunction delayEndFunc)
	: m_activityReady(true)
	, m_lastActivityReady(false)
	, m_status(GSMDriverStatus::NOT_INITED)
	, m_GSM(stream)
	, m_powerOnFunc(powerONFunc)
	, m_resetFunc(resetFunc)
	, m_delaySysFunc(delaySysFunc)
	, m_delayStartFunc(delayStartFunc)
	, m_delayEndFunc(delayEndFunc)
	, m_inCall(false)
	, m_newSms(false)
	, m_outSms(false)
	, m_sendAttempts(0)
	, m_lastSendTime(0)
{
	if (nullptr != m_powerOnFunc)
	{
		//m_powerOnFunc();
	}
}

/////////////////////////////////////////////////////////////////
GSMDriver::~GSMDriver()
{
}

/////////////////////////////////////////////////////////////////
bool GSMDriver::Init()
{
	PowerONInit();
	return true;
}

/////////////////////////////////////////////////////////////////
void GSMDriver::PowerONInit()
{
	m_status = GSMDriverStatus::INITIALIZING;

	DBG_F_PRINTLN("GSM Init...");

	bool readStatus = false;
	int connectingTries = 0;
	String command;
	while (1)
	{
		connectingTries++;
		if (5 == connectingTries)
		{
			ResponsiveDelay(20000UL);
			if (nullptr != m_resetFunc)
			{
				m_resetFunc();
			}
		}
		else if (10 == connectingTries)
		{
			ResponsiveDelay(60000UL);
			if (nullptr != m_resetFunc)
			{
				m_resetFunc();
			}
		}
		else if (15 == connectingTries)
		{
			ResponsiveDelay(120000UL);
			connectingTries = 1;
		}

		m_powerOnFunc();

		bool powerReady = false;
		DBG_F_PRINTLN("Read until 'Call ready'");
		while (1)
		{
			String responseString = ReadString(16000, readStatus);
			if (false == readStatus)
			{
				// Gsm is not responsive, repeat power operation
				DBG_F_PRINTLN("-----");
				DBG_F_PRINTLN("Read failed");
				DBG_F_PRINT("Response string : ");
				DBG_PRINTLN(responseString);
				DBG_F_PRINTLN("-----");

				ResponsiveDelay(5000);
				break;
			}
			if (responseString == S(RESP_POWER_DOWN))
			{
				// Gsm Modem is shutting down, repeat power operation
				DBG_F_PRINTLN("Power down.");
				ResponsiveDelay(1000);
				break;
			}

			if (responseString.startsWith(S(RESP_CALL_READY)))
			{
				// Gsm is ready
				DBG_F_PRINTLN("Power ready !");
				ResponsiveDelay(1000);
				powerReady = true;
				break;
			}
		}

		if (false == powerReady)
		{
			// Repeate power operation
			continue;
		}

		ResponsiveDelay(500);

		// Turn OFF echo
		command = S(COMM_ECHO_OFF);
		SendCommand(command);
		ReadString(800, readStatus);
		if (false == readStatus)
		{
			// Set echo off failed
			DBG_F_PRINTLN("ECHO OFF failed");
			ResponsiveDelay(1000);
			continue;
		}
		ResponsiveDelay(150);

		// Set response mode to text
		command = S(COMM_RESPONSE_MODE);
		SendCommand(command);
		ReadString(800, readStatus);
		if (false == readStatus)
		{
			// Set response mode failed
			DBG_F_PRINTLN("Set response mode failed");
			ResponsiveDelay(1000);
			continue;
		}
		ResponsiveDelay(200);

		// Proceed with initializing
		DBG_F_PRINTLN("SEND AT Check");
		command = S(COMM_AT);
		SendCommand(command);
		String resp = ReadString(700, readStatus);
		if (false == readStatus)
		{
			// Gsm not responding to AT command
			DBG_F_PRINTLN("AT response timeout");
			ResponsiveDelay(1000);
			continue;
		}
		if (resp != S(RESP_OK))
		{
			// Gsm not responding OK
			DBG_F_PRINTLN("AT response ERROR, try again");
			ResponsiveDelay(1200);
			// Try again
			command = S(COMM_AT);
			SendCommand(command);
			resp = ReadString(600, readStatus);
			if ((false == readStatus) || (resp != S(RESP_OK)))
			{
				DBG_F_PRINTLN("AT response ERROR, turn GSM OFF");
				ResponsiveDelay(1000);
				continue;
			}
		}
		ResponsiveDelay(150);

		// Response to AT is OK, preceed
		DBG_F_PRINTLN("Check network ...");
		uint32_t networkWaitStart = millis();
		bool networkGood = false;
		while (millis() - networkWaitStart < 20000)
		{
			command = S(COMM_CHECK_NETWORK);
			SendCommand(command);
			resp = ReadString(1000, readStatus);

			if ((readStatus) && (resp == S(RESP_CHECK_NETWORK)))
			{
				networkGood = true;
				DBG_F_PRINTLN("Network OK");
				break;
			}
			DBG_F_PRINTLN("-----");
			DBG_F_PRINT("Network check response : ");
			DBG_PRINTLN(resp);
			DBG_F_PRINTLN("-----");
			ResponsiveDelay(300);
		}

		if (true == networkGood)
		{
			// GSM is READY
			break;
		}
		// No network
		ResponsiveDelay(2000);
		continue;
	}

	DBG_F_PRINTLN("GSM Ready !");
	m_status = GSMDriverStatus::READY;

	ResponsiveDelay(300);
	command = S(COMM_DELETE_SMS);
  	SendCommand(command);
 
	String resp = ReadString(1000, readStatus);
	if ((false == readStatus) || (resp != S(RESP_OK)))
	{
		DBG_F_PRINTLN("Fail initial deleting messages, reboot...");
	}
}

GSMDriver::ActivityStatus GSMDriver::GetActivity()
{
	String command = S(COMM_GSM_ACTIVITY);
	SendCommand(command);
	bool readStatus = false;
	String resp = ReadString(500, readStatus);

	delay(5);

	if (false == resp.startsWith(S(RESP_GSM_ACTIVITY)))
	{
		return ActivityStatus::ERROR;
	}

	resp.remove(0, SL(RESP_GSM_ACTIVITY));
	if (0 == RemoveToDigit(resp))
	{
		return ActivityStatus::ERROR;
	}

	auto st = resp.toInt();

	resp = ReadString(200, readStatus);
	if ((false == readStatus) || (resp != S(RESP_OK)))
	{
		return ActivityStatus::ERROR;
	}

	switch (st)
	{
	case 0:case 2:case 3:case 4:
		return (ActivityStatus(st));
	default:
		return ActivityStatus::ERROR;
	}
}

/////////////////////////////////////////////////////////////////
void GSMDriver::SafeModeLoop(String& gsmInput)
{
	if ((gsmInput.length() > 0) && (gsmInput != S(RESP_ERROR)))
	{
		StringParser(gsmInput);
		return;
	}

	bool readStatus = false;
	gsmInput = ReadString(500, readStatus);
	if ((readStatus) && (gsmInput.length() > 0))
	{
		StringParser(gsmInput);
	}
}

/////////////////////////////////////////////////////////////////
void GSMDriver::StringParser(String& gsmInput)
{
	if (gsmInput == S(RESP_OK))
	{
		return;
	}

	if (gsmInput == S(RESP_POWER_DOWN))
	{
		PowerONInit();
		return;
	}

	if (gsmInput.startsWith(S(RESP_NEW_SMS)))
	{
		ReadNewSMS(gsmInput);
		return;
	}

	if ((gsmInput == S(RESP_RING)) || (gsmInput == S(RESP_CLIP)))
	{
		ProcessIncomingCall(gsmInput);
		return;
	}

	if (gsmInput == S(RESP_ERROR))
	{
		PowerONInit();
		return;
	}

	if (gsmInput.startsWith(S(RESP_READ_SMS)))
	{
		ParseNewSms(gsmInput);
		return;
	}
}

/////////////////////////////////////////////////////////////////
void GSMDriver::ReadNewSMS(String& gsmInput)
{
	DBG_F_PRINTLN("New msg recieved(1)");
	gsmInput.remove(0, SL(RESP_NEW_SMS));
	int smsPos = gsmInput.toInt();
	//DBG_PRINTLN(STRING("sms pos : ") + String(smsPos));

	String readSmsCommand = S(COMM_READ_SMS) + String(smsPos);
	SendCommand(readSmsCommand);

	bool readStatus = false;
	gsmInput = ReadString(1000, readStatus);
	//DBG_PRINTLN(STRING("Response : ") + gsmInput);

	if ((false == readStatus) || (false == gsmInput.startsWith(S(RESP_READ_SMS))))
	{
		DBG_F_PRINT("Error in reading new sms response : ");
		DBG_PRINTLN(gsmInput);
		return;
	}
	ParseNewSms(gsmInput);
}
/////////////////////////////////////////////////////////////////
void GSMDriver::ParseNewSms(String& gsmInput)
{
	//DBG_F_PRINTLN("New msg recieved");

	bool readStatus = false;
	gsmInput.remove(0, SL(RESP_READ_SMS));

	String msgStatus = RemoveFirstQuotesString(gsmInput);
	String msgSender = RemoveFirstQuotesString(gsmInput);
	String msgBody = ReadString(500, readStatus, true);

	gsmInput = ReadString(500, readStatus);
	if (gsmInput == S(RESP_OK))
	{
		m_newSms = true;
		m_senderNumber = astd::move(msgSender);
		m_msgBody = astd::move(msgBody);
	}
	else
	{
		DBG_F_PRINTLN("Fail parsing sms");
	}

	ResponsiveDelay(100);
	String command = S(COMM_DELETE_SMS);
	SendCommand(command);

	gsmInput = ReadString(1000, readStatus);
	if ((false == readStatus) || (gsmInput != S(RESP_OK)))
	{
		DBG_F_PRINTLN("Fail deleting messages, reboot...");
		PowerONInit();
		return;
	}
}

/////////////////////////////////////////////////////////////////
void GSMDriver::ProcessIncomingCall(String& gsmInput)
{
	DBG_F_PRINTLN("Processing incoming call...");
	uint32_t startWait = millis();
	bool readStatus = false;
	while (millis() - startWait < 4000)
	{
		if (false == gsmInput.startsWith(S(RESP_CLIP)))
		{
			gsmInput = ReadString(2000, readStatus);
			continue;
		}
		break;
	}
	ResponsiveDelay(100);
	// Hangup call
	String command = S(COMM_HANGUP);
	SendCommand(command);

	// Try to get caller number
	String gsmNumber = RemoveFirstQuotesString(gsmInput);
	if (('+' == gsmNumber.charAt(0)) && (RemoveToDigit(gsmInput) > 0))
	{
		int num = gsmInput.toInt();
		if (num == (int)GSMDriverConsts::InternationalNumberType)
		{
			m_callerNumber = astd::move(gsmNumber);
			m_inCall = true;
		}
	}

	// Wait for response
	startWait = millis();
	gsmInput = ReadString(3000, readStatus);
	if (gsmInput == S(RESP_OK))
	{
		return;
	}

	PowerONInit();
	return;
}

/////////////////////////////////////////////////////////////////
void GSMDriver::SendSMS()
{
	if (ActivityStatus::READY != GetActivity())
	{
		DBG_F_PRINTLN("GSM NOT ACTIVE");

		if (m_activityReady)
		{
			m_activityReady = false;
			m_lastActivityReady = millis();

			ResponsiveDelay(200);
			return;
		}
		if (millis() - m_lastActivityReady > 20000UL)
		{
			m_activityReady = true;
			PowerONInit();

			return;
		}

		ResponsiveDelay(200);
		return;
	}

	DBG_F_PRINTLN("Sending MSG...");
	m_sendAttempts++;
	if (m_sendAttempts > 3)
	{
		DBG_F_PRINT("Delete this msg");
		m_sendAttempts = 0;
		m_outSms = false;
		return;
	}
	String sendSMSCommand = S(COMM_SEND_SMS) + '"' + m_outMsgNumber + '"';
	{
		String dummy = astd::move(m_outMsgNumber);
	}
	SendCommand(sendSMSCommand);
	bool readStatus = false;
	String response = ReadString(1000, readStatus);
	if (!response.startsWith(S(RESP_READY_FOR_SMS)))
	{
		DBG_F_PRINT("Error in sending sms, response : ");
		DBG_PRINTLN(response);
		SafeModeLoop(response);
		return;
	}

	m_GSM->print(m_outMsg);
	{
		String dummy = astd::move(m_outMsg);
	}

	m_GSM->print((char)(GSMDriverConsts::MessageEnd));
	m_GSM->print((char)(GSMDriverConsts::CarriageReturn));

	response = ReadString(7000, readStatus);
	if (false == response.startsWith(S(RESP_SEND_SMS)))
	{
		DBG_F_PRINT("Error wait sending response 1, response : ");
		DBG_PRINTLN(response);
		SafeModeLoop(response);
		return;
	}

	response = ReadString(1000, readStatus);
	if (response != S(RESP_OK))
	{
		DBG_F_PRINT("Error wait sending response 2, response : ");
		DBG_PRINTLN(response);
		SafeModeLoop(response);
		return;
	}

	DBG_F_PRINTLN("SMS send OK");
	m_sendAttempts = 0;
	m_outSms = false;
}

/////////////////////////////////////////////////////////////////
void GSMDriver::SendSMS(String& number, String& msg)
{
	if (msg.length() == 0)
	{
		return;
	}

	m_outSms = true;
	m_outMsgNumber = astd::move(number);
	m_outMsg = astd::move(msg);
	return;
}

/////////////////////////////////////////////////////////////////
bool GSMDriver::HasNewCall(String& dilerNumber)
{
	if (false == m_inCall)
	{
		return false;
	}
	dilerNumber = astd::move(m_callerNumber);
	m_inCall = false;
	return true;
}

/////////////////////////////////////////////////////////////////
bool GSMDriver::HasNewSMS(String& sender, String& msgBody)
{
	if (false == m_newSms)
	{
		return false;
	}
	sender = astd::move(m_senderNumber);
	msgBody = astd::move(m_msgBody);

	m_newSms = false;
	return true;
}

void AddTwoDigitsString(String& str, int num)
{
	if (num < 9)
	{
		str += '0';
	}
	str += String(num);
}

/////////////////////////////////////////////////////////////////
void GSMDriver::Loop()
{
	bool readStatus = false;
	if (m_GSM->available())
	{
		String gsmInput = ReadString(100, readStatus);
		if ((readStatus) && (gsmInput.length() > 0))
		{
			DBG_F_PRINTLN("...");
			StringParser(gsmInput);
			return;
		}
	}

	if (m_outSms)
	{
		if (millis() - m_lastSendTime > 1500)
		{
			SendSMS();
			m_lastSendTime = millis();
			return;
		}
	}
}

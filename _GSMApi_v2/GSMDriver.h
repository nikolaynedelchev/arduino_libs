#pragma once
#include <Arduino.h>
#include <Stream.h>
#include <avr/pgmspace.h>
#include <_Strings.h>

class Stream;
typedef void(*SystemFunction)();

#define DELAY_START_FUNC()	if (m_delayStartFunc) { m_delayStartFunc(); }
#define DELAY_END_FUNC()	if (m_delayEndFunc) { m_delayEndFunc(); }
#define DELAY_FUNC()		if (m_delaySysFunc) { m_delaySysFunc(); }

enum class GSMDriverStatus
{
	NOT_INITED,
	INITIALIZING,
	WORKING,
	READY
};

enum class GSMDriverConsts : uint8_t
{
	OK = 0,
	Connect = 1,
	Ring = 2,
	NoCarrier = 3,
	Error = 4,
	LineFeed = 10,
	CarriageReturn = 13,
	MessageEnd = 26,
	InternationalNumberType = 145,
};

class GSMDriver
{
public:
	GSMDriver(	Stream *stream, 
				SystemFunction powerONFunc = nullptr,
				SystemFunction resetFunc = nullptr,
				SystemFunction delaySysFunc = nullptr,
				SystemFunction delayStartFunc = nullptr,
				SystemFunction delayEndFunc = nullptr);

	~GSMDriver();
	bool Init();

	void				Loop();
	GSMDriverStatus		Status() const { return m_status; };
	void				SendSMS(String& number, String& msg);
	bool				IsSendSMSReady() const { return !m_outSms; };
	bool				HasNewCall(String& dilerNumber);
	bool				HasNewSMS(String& sender, String& msgBody);

private:
	enum class ActivityStatus
	{
		READY = 0,
		UNKNOWN = 2,
		RINGING = 3,
		CALL_IN_PROGRESS = 4,
		ERROR
	};

	void				PowerONInit();
	void				ResponsiveDelay(uint32_t duration);
	void				SendCommand(String& comm);
	String				ReadString(uint32_t timeout, bool& good, bool readMsg = false);
	ActivityStatus		GetActivity();
	void				SafeModeLoop(String& gsmInput);
	void				ReadNewSMS(String& gsmInput);
	void				ProcessIncomingCall(String& gsmInput);
	void				StringParser(String& gsmInput);
	void				ParseNewSms(String& gsmInput);
	void				SendSMS();


	bool				m_activityReady;
	uint32_t			m_lastActivityReady;
	GSMDriverStatus		m_status;
	Stream				*m_GSM;
	SystemFunction		m_powerOnFunc;
	SystemFunction		m_resetFunc;
	SystemFunction		m_delaySysFunc;
	SystemFunction		m_delayStartFunc;
	SystemFunction		m_delayEndFunc;
	bool				m_inCall;
	bool				m_newSms;
	bool				m_outSms;
	String				m_callerNumber;
	String				m_senderNumber;
	String				m_msgBody;
	String				m_outMsg;
	String				m_outMsgNumber;
	uint8_t				m_sendAttempts;
	uint32_t			m_lastSendTime;
};


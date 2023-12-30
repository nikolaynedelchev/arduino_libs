#pragma once

#include "../_GSMApi_v2/SmsMsg.h"

class DummyGsm
{
public:
    DummyGsm();
	~DummyGsm() = default;
    using PrinterPtr = void(*)(const String&);

    void InitPrint(PrinterPtr printPtr, PrinterPtr printLnPtr);

	void Update();
	void SendMsg(SmsMsg&& msg);
	bool RcvMsg(String& number, String& msg);
	bool RcvCall(String& number);

    void _injectSms(String& number, String& msg);
    void _injectCall(String& number);
    bool CheckUserInput(const String& userInput);
    
private:

    //
    void _print(const String& str);
    void _printLn(const String& str);

    bool m_waitingSms;
    bool m_waitingCall;
    String m_smsNumber;
    String m_smsMsg;
    String m_callNumber;

    PrinterPtr m_printPtr;
    PrinterPtr m_printLnPtr;
};


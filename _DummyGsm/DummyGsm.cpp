#include "DummyGsm.h"
#include "_Debug.h"
#include <_a_std.h>
#include <_Strings.h>

DummyGsm::DummyGsm()
    : m_waitingSms(false)
    , m_waitingCall(false)
    , m_printPtr(nullptr)
    , m_printLnPtr(nullptr)
{

}

void DummyGsm::InitPrint(PrinterPtr printPtr, PrinterPtr printLnPtr)
{
    m_printPtr = printPtr;
    m_printLnPtr = printLnPtr;
}

void DummyGsm::Update()
{

}
void DummyGsm::SendMsg(SmsMsg&& msg)
{
    _print("NEW MSG: ");
    _print(msg.number);
    _print(", msg: ");
    _printLn(msg.msg);
}

bool DummyGsm::RcvMsg(String& number, String& msg)
{
    if (m_waitingSms == false)
    {
        return false;
    }
    m_waitingSms = false;
    number = astd::move(m_smsNumber);
    msg = astd::move(m_smsMsg);
    return true;
}

bool DummyGsm::RcvCall(String& number)
{
    if (m_waitingCall == false)
    {
        return false;
    }
    m_waitingCall = false;
    number = astd::move(m_callNumber);
    return true;
}

void DummyGsm::_injectSms(String& number, String& msg)
{
    m_waitingSms = true;
    m_smsNumber = astd::move(number);
    m_smsMsg = astd::move(msg);
}

void DummyGsm::_injectCall(String& number)
{
    m_waitingCall = true;
    m_callNumber = astd::move(number);
}

void DummyGsm::_print(const String& str)
{
    if (m_printPtr != nullptr)
    {
        m_printPtr(str);
    }
}

void DummyGsm::_printLn(const String& str)
{
    if (m_printLnPtr != nullptr)
    {
        m_printLnPtr(str);
    }
}

bool DummyGsm::CheckUserInput(const String& userInput)
{
    if (userInput.startsWith("gsm_sms"))
    {
        String copy = userInput;
        m_smsNumber = RemoveFirstQuotesString(copy);
        m_smsMsg = RemoveFirstQuotesString(copy);
        m_waitingSms = true;
        return true;
    }
    else if (userInput.startsWith("gsm_call"))
    {
        String copy = userInput;
        m_callNumber = RemoveFirstQuotesString(copy);
        m_waitingCall = true;
        return true;
    }
    return false;
}

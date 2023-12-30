#include "MsgConstructor.h"
#include <_a_std.h>

void MsgConstructor::inc(uint8_t& idx)
{
	idx++;
	if (idx == s_cap)
	{
		idx = 0;
	}
}

void MsgConstructor::AddMsg(SmsMsg&& msg)
{
	if (m_size == s_cap)
	{
		inc(m_back);
		m_size--;
	}
	m_messages[m_front] = astd::move(msg);
	inc(m_front);
	m_size++;
}

bool MsgConstructor::GetMsg(SmsMsg& msg)
{
	if (IsEmpty())
	{
		return false;
	}
	msg = astd::move(m_messages[m_back]);
	inc(m_back);
	m_size--;
	return true;
}

bool MsgConstructor::IsEmpty() const
{
	return m_size == 0;
}

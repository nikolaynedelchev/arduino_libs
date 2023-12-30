#include "StreamServer.h"


#define CHECK_EOL(c) (((char)10 == (c)) || ('\n' == (c)) || ((char)13 == (c)))
//////////////////////////////////////////////////////////////
StreamServer::StreamServer()
	: m_stream(nullptr)
	, m_inited(false)
	, m_readInProcess(false)
	, m_readStartTime(0)
	, m_str(String())
	, m_blockMode(false)
{
}

//////////////////////////////////////////////////////////////
StreamServer::~StreamServer()
{
}

//////////////////////////////////////////////////////////////
void StreamServer::Init(Stream* stream)
{
	m_stream = stream;
	m_inited = true;
}

//////////////////////////////////////////////////////////////
bool StreamServer::Update()
{
	if ((false == m_inited) || (nullptr == m_stream))
	{
		return false;
	}

	if (0 == m_stream->available())
	{
		if (m_readInProcess && millis() - m_readStartTime > 30000)
		{
			m_readInProcess = false;
			m_str = String();
		}
		return false;
	}

	uint32_t lastRead = millis();
	uint32_t startRead = millis();
	while (true)
	{
		if (millis() - startRead > 15000)
		{
			return false;
		}

		if (!m_stream->available())
		{
			if (m_blockMode)
			{
				if (millis() - lastRead > 10000)
				{
					return false;
				}
			}
			else
			{
				if (millis() - lastRead > 5)
				{
					return false;
				}
			}
		}
		else
		{
			char c = (char)m_stream->read();
			lastRead = millis();
			if ('~' == c)
			{
				m_str = String();
				m_blockMode = true;
				m_readStartTime = millis();
				m_readInProcess = true;
			}
			else if (m_readInProcess)
			{
				if (CHECK_EOL(c))
				{
					m_readInProcess = false;
					return true;
				}
				m_str += c;
			}
			else
			{
				if (CHECK_EOL(c))
				{
					m_readInProcess = false;
					m_str = String("");
					return true;
				}
				m_readInProcess = true;
				m_str = String();
				m_str += c;
				m_blockMode = false;
				m_readStartTime = millis();
			}
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////
String StreamServer::GetString()
{
	return m_str;
}

#pragma once
#include "arduino.h"

class StreamServer
{
public:
	StreamServer();
	~StreamServer();

	void Init(Stream* stream);
	bool Update();
	String GetString();

private:
	Stream* m_stream;
	bool m_inited;
	bool m_readInProcess;
	uint32_t m_readStartTime;
	String m_str;
	bool m_blockMode;
};


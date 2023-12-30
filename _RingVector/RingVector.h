#pragma once
#include <Arduino.h>

template <typename T, uint16_t BUFF_SZ>
class RingVector
{
private:
	T m_buff[BUFF_SZ];
	uint16_t m_size;
	uint16_t m_index;

public:
	RingVector() : m_size(0), m_index(0) {}
	~RingVector() {}
	
	uint16_t size() { return m_size; };
	void push_front(const T& el)
	{
		m_buff[m_index] = el;
		m_index++;
		m_size++;
		if (m_size > BUFF_SZ)
		{
			m_size = BUFF_SZ;
		}
		if (m_index >= BUFF_SZ)
		{
			m_index -= BUFF_SZ;
		}
	}
	void delete_back()
	{
		if (m_size > 0)
		{
			m_size--;
		}
	}

	RingVector<T, BUFF_SZ>& operator<<(const T& obj)
	{
		push_front(obj);
		return *this;
	};

	T& operator[](uint16_t idx)
	{
		if (0 == m_size)
		{
			return m_buff[0];
		}
		if (idx >= m_size)
		{
			idx = m_size - 1;
		}
		int16_t i = (int16_t)m_index - (int16_t)idx - 1;
		if (i < 0)
		{
			i += BUFF_SZ;
		}
		return m_buff[i];
	}

};

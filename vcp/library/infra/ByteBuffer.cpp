#include "string.h"
#include "ByteBuffer.h"

namespace Infra
{

CByteBuffer::CByteBuffer()
:m_begin(0)
,m_end(0)
,m_capacity(0)
,m_pBuffer(nullptr)
{

}

CByteBuffer::CByteBuffer(unsigned int size)
:m_begin(0)
,m_end(0)
,m_capacity(size)
{
	m_pBuffer = alloac(size);
}

CByteBuffer::~CByteBuffer()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

void CByteBuffer::resize(unsigned int size)
{
	if (size == 0)
	{
		clear();
	}
	else
	{
		adjust(size);
	}
}

void CByteBuffer::append(const unsigned char* p, unsigned int len)
{
	if (!p || len == 0)
	{
		return ;
	}

	if (len > m_capacity - m_end)
	{
		adjust(m_end - m_begin + len);
	}
	
	memcpy(m_pBuffer + m_end, p, len);
	m_end += len;
}

void CByteBuffer::remove(int len)
{
	if (len > 0)
	{
		size_t pos = m_end - len < m_begin ? m_begin : m_end - len;
		memset(m_pBuffer + pos, 0, m_end - pos);
		m_end = pos;
	}
	else if (len < 0)
	{
		size_t pos = m_begin - len > m_end ? m_end : m_begin - len;
		memset(m_pBuffer + m_begin, 0, pos - m_begin);
		m_begin = pos;
	}
}

void CByteBuffer::clear()
{
	m_begin = 0;
	m_end = 0;
	m_capacity = 0;
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

const unsigned char* CByteBuffer::getBuffer()
{
	return m_pBuffer + m_begin;
}

unsigned int CByteBuffer::size() const
{
	return m_end - m_begin;
}

unsigned int CByteBuffer::capacity() const
{
	return m_capacity;
}

void CByteBuffer::adjust(unsigned int size)
{
	unsigned char* p = alloac(size);
	unsigned int cpyLen = size < m_end - m_begin ? size : m_end - m_begin;
	if (cpyLen > 0)
	{
		memcpy(p, m_pBuffer + m_begin, cpyLen);
	}
	delete[] m_pBuffer;
	m_pBuffer = p;
	m_capacity = size;
	m_end = cpyLen;
	m_begin = 0;
}

unsigned char* CByteBuffer::alloac(unsigned int size)
{
	unsigned char* p = new unsigned char[size];
	memset(p, 0, size);
	return p;
}

} //Infra

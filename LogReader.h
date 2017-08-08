#pragma once
#include "Filter.h"

//����� ��� �������� ������������� ������
//� ����� - ������ 0
class CDataBuffer
{
public:
	CDataBuffer()
	: m_data(nullptr)
	, m_sz(0)
	{
	}

	~CDataBuffer()
	{
		Clear();
	}
		
	//�������� ������
	bool Push(const char *data, size_t sz)
	{
		if (!sz)
			return true;

		char *p = (char *)realloc(m_data, m_sz + sz + 1); //�������� ������ +'\0'
		if (!p)
			return false;

		memcpy(p + m_sz, data, sz);

		m_data = p;
		m_sz += sz;
				
		m_data[m_sz] = 0;
		return true;
	}

	//�������� �����
	void Clear()
	{
		free(m_data);
		m_data = nullptr;
		m_sz = 0;
	}

	char *data() const
	{
		return m_data;
	}

	bool empty() const
	{
		return m_sz == 0;
	}

	size_t size() const
	{
		return m_sz;
	}

	void swap(CDataBuffer &src)
	{
		auto *data = m_data;
		auto sz = m_sz;

		m_data = src.m_data;
		m_sz = src.m_sz;
		
		src.m_data = data;
		src.m_sz = sz;
	}
protected:
	char *m_data;
	size_t m_sz;
};

class CLogReader
{
public:
	static const size_t BufferSize = 2 * 1024;
	CLogReader();

	~CLogReader()
	{
		Close();
	}

	//�������� �����, false - ������
	//name - ��� �����
	//buf_cap - ������ ������ ������
	bool Open(LPCSTR name, size_t buf_cap = BufferSize);

	//�������� �����
	void Close();

	//��������� ������� �����, false - ������
	bool SetFilter(const char *filter);

	//������ ��������� ��������� ������, buf - �����, bufsize - ������������ �����
	//false - ����� ����� ��� ������
	bool GetNextLine(char *buf, const size_t sz);

	bool TestLine(const char *psz, const char *endl);

protected:
	HANDLE m_hFile;

	size_t m_buf_cap; //������ ������
	DWORD m_buf_sz; //����������, ��������� � ��������� ��������
	size_t m_buf_idx; //������� �������� � ������
	char *m_buf; //����� ������ ��� ������ �����

	CDataBuffer m_data; //�� ����������� � ���������� �������� ������

	CFilter m_filter;
};

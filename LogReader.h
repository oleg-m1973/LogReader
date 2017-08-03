#pragma once
#include "Filter.h"
#include <vector>

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
	std::unique_ptr<char[]> m_buf; //����� ������ ��� ������ �����

	std::vector<char> m_data; //�� ����������� � ���������� �������� ������

	std::unique_ptr<CFilter> m_spFilter;
};

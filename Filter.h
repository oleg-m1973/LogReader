#pragma once
#include <vector>

//�������� �������
// ? - ���� ����� ������
// * - ������������������ ����� �������� �������������� �����
// + - ��������� ���������� �������, ���� ���������
// \ - ������������� �������� ������
// \t - ���������
// \r - ������ '\r'
// \s - ����� ������


class CFilter;

//������� ����� ��� �������� ������� �������
class CFilterToken
{
friend class CFilter;
public:
	CFilterToken()
	: m_repeat(false)
	{
	}

	virtual ~CFilterToken()
	{
	}
	
	//��������� ������������ ������ �������� �������
	//psz - ������ ������, endl - ����� ������, ������� ����������� 0
	//���������� ��������� �� ��������� ������ ��� nullptr � ����� �������
	const char *Test(const char *psz, const char *end)
	{
		if (!m_repeat)
			return _Test(psz, end);

		//���������, ���� �������������
		while(psz != end)
		{
			auto p = _Test(psz, end);
			if (!p)
				break;

			psz = p;
		}

		return psz;
	}

protected:
	//���������� �� Test(), ��������� ������������ ������ �������� �������, �� ��� ��������
	virtual const char *_Test(const char *psz, const char *endl) = 0;

	bool m_repeat; //��������� ���� �������������
};

//������ �������
//���� ������ �������� �������� ��������, ������� � ������ ������ ��� *, � �� ��������� * ��� ����� ������
class CFilter
{
public:
	CFilter()
	: m_bol(true)
	{
	}

	bool Parse(const char *psz);
	bool Test(const char *psz, const char *endl);

	void Reset()
	{
		m_bol = true;
		m_tokens.clear();
		m_spNext.reset();
	}
protected:
	const char *ApplyTokens(const char *psz, const char *endl);

	bool m_bol; //��������� � ������ ������
	std::vector<std::unique_ptr<CFilterToken>> m_tokens; //�������� �������
	std::unique_ptr<CFilter> m_spNext; //��������� ������
};

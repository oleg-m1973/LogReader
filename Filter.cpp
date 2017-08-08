#include "stdafx.h"
#include "Filter.h"

////////////////////////////////////////////////////////////////////////////////////////
//CFilterToken
//���������� ������� - ��������� �������

namespace 
{
//������ ������ ������ ����� ��������� �������
class CFilterSymbol
: public CFilterToken
{
public:
	CFilterSymbol()
	: m_ch(0)
	{
	}

	virtual const char *Read(const char *psz)
	{
		m_ch = *psz;
		if (m_ch != '\\')
			return psz + 1;
		
		m_ch = psz[1];
		switch (m_ch)
		{
		case 't': m_ch = '\t'; break;
		case 'r': m_ch = '\r'; break;
		case 0: return nullptr;
		}

		return psz + 2;
	}

protected:
	virtual const char *_Test(const char *psz, const char *endl)
	{
		return *psz == m_ch? ++psz: nullptr;
	}

	char m_ch;
};

// ? - ����� ������, ����� '\0'
class CFilterAnySymbol
: public CFilterToken
{
public:
	virtual const char *Read(const char *psz)
	{
		return psz + 1; // "?"
	}

protected:
	virtual const char *_Test(const char *psz, const char *endl)
	{
		return *psz? ++psz: nullptr;
	}
};

// \s - ����� ������, isspace
class CFilterSpace
: public CFilterToken
{
public:
	virtual const char *Read(const char *psz)
	{
		return psz + 2; // "\s"
	}

protected:
	virtual const char *_Test(const char *psz, const char *endl)
	{
		return isspace(*psz)? ++psz: nullptr;
	}
};

// [] - ���� �� ��������
class CFilterOneOf
: public CFilterToken
{
public:
	CFilterOneOf()
	: m_psz(nullptr)
	{
	}

	~CFilterOneOf()
	{
		delete m_psz;
	}

	virtual const char *Read(const char *psz)
	{
		if (*psz != '[' || !psz[1])
			return nullptr;

		m_psz = new char[strlen(++psz) + 1];
		for (char *p = m_psz; *psz && *psz != ']'; ++psz, ++p)
		{
			char ch = *psz;
			if (ch == '\\' && psz[1] ==']')
				ch = (*++psz);

			*p = ch;
		};

		return *psz == ']'? psz + 1: nullptr;
	}

protected:
	virtual const char *_Test(const char *psz, const char *endl)
	{
		//���� ������ [] ���������� ������� ������
		return m_psz? (strchr(m_psz, *psz) != nullptr? ++psz: nullptr): psz; 
	}

	char *m_psz;
};

} //namespace

////////////////////////////////////////////////////////////////////////////////////////
//CFilter

//��������� ������ �� ������
bool CFilter::Parse(const char *psz)
{
	Reset();
	while (*psz)
	{
		char ch = *psz;
		switch (ch)
		{
		case '?': psz = AddToken<CFilterAnySymbol>(psz); break; 
		case '[': psz = AddToken<CFilterOneOf>(psz); break; 
		case '\\':
			switch (psz[1])
			{
			case 's': psz = AddToken<CFilterSpace>(psz); break;
			default: psz = AddToken<CFilterSymbol>(psz); break;
			}
			break;
		case '+': //������������� ���� ���������� ��� ����������� ��������, ����� ������� �� ��������
			if (m_last)
				m_last->m_repeat = true;
			++psz;
			break;
		case '*':
			{
				if (IsEmpty()) //���������, ���� ����������� �������� �������. �� ������ ****
				{
					m_bol = false; //���������� ������� ������ � ������ ������
					++psz;
					continue;
				}

				m_next = new CFilter(); //������ ��������� ������
				if (!m_next->Parse(psz))
					return false;
			}
			return true; //�������� ���������
		default: 
			psz = AddToken<CFilterSymbol>(psz);
		}

		if (!psz) //������ ��� ������ �������� �������
			return false; 
	}

	//���� ������ �� �������� * � �����, ��������� ������� ��� ������ �� ����� ������, ������ 0
	if (!IsEmpty())
		AddToken<CFilterSymbol>("\0");

	return true;
}

//��������� ����������� ������ psz �������
bool CFilter::Test(const char *psz, const char *endl)
{
	if (IsEmpty()) //���� ��������� ������� ���, �� ��� ����������� *, ����� �� ������ �� ����� ������
		return true;

	//����� �� ������, �� ������ �������, ���� �� ����� ������� ������������
	for (;;  ++psz)
	{
		//��������� ������ �� ������������
		auto p = ApplyTokens(psz, endl);
		if (p) //���� ������������ ���� ���������
		{
			psz = p;
			break;
		}
		
		//���� �� ����� ������ ��� ����� ������ � ������ ������
		if (psz == endl || m_bol) 
			return false;
	};

	//���� ������ ������������ ������� ��������� ��������� ������, ���� �� ����
	return psz != nullptr && (!m_next || m_next->Test(psz, endl));
}

//��������� ������ �� ����������� ���� ��������� �������,
//���������� ��������� �� ��������� ������ ��� nullptr ������� ��������
const char *CFilter::ApplyTokens(const char *psz, const char *endl)
{
	for (auto *token = m_first; token != nullptr; token = token->m_next)
	{
		if (psz == endl)
			return nullptr;

		auto p = token->Test(psz, endl);
		if (!p)
			return nullptr;

		psz = p;
	}

	return psz;
}

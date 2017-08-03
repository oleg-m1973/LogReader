#include "stdafx.h"
#include "Filter.h"
#include <locale>
#include <algorithm>

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
	CFilterSymbol(char ch)
	: m_ch(ch) 
	{
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
	CFilterOneOf(const char *&psz)
	{
		if (*psz != '[')
			return;

		m_symbols.reserve(16);
		for (++psz; *psz && *psz != ']'; ++psz)
		{
			char ch = *psz;
			if (ch == '\\' && psz[1] ==']')
				ch = (*++psz);

			m_symbols.emplace_back(ch);
		};

		std::sort(m_symbols.begin(), m_symbols.end());
		if (*psz == ']')
			++psz;
	}

protected:
	virtual const char *_Test(const char *psz, const char *endl)
	{
		return 
			m_symbols.empty()? psz: 
			std::binary_search(m_symbols.begin(), m_symbols.end(), *psz)? ++psz: nullptr;
	}

	std::vector<char> m_symbols;
};

} //namespace

////////////////////////////////////////////////////////////////////////////////////////
//CFilter

//��������� ������ �� ������
bool CFilter::Parse(const char *psz)
{
	Reset();
	std::unique_ptr<CFilterToken> sp;
	for (;*psz; ++psz)
	{
		char ch = *psz;
		switch (ch)
		{
		case '?': sp.reset(new CFilterAnySymbol()); break; 
		case '[': sp.reset(new CFilterOneOf(psz)); break; 
		case '\\':
			ch = psz[1];
			if (ch)
			{
				++psz;
				switch (ch)
				{
				case 's': sp.reset(new CFilterSpace()); break;
				case 't': sp.reset(new CFilterSymbol('\t')); break;
				case 'r': sp.reset(new CFilterSymbol('\r')); break;
				default: 
					sp.reset(new CFilterSymbol(ch));
				}
			}
			break;
		case '+': //������������� ���� ���������� ��� ����������� ��������, ����� ������� �� ��������
			if (!m_tokens.empty())
				m_tokens.back()->m_repeat = true;
			break;
		case '*':
			{
				if (!sp && m_tokens.empty()) //���������, ���� ����������� �������� �������. �� ������ ****
				{
					m_bol = false; //���������� ������� ������ � ������ ������
					continue;
				}

				if (sp) //��������� ������� �������, ���� ����
					m_tokens.emplace_back(std::move(sp));

				m_spNext.reset(new CFilter()); //������ ��������� ������
				if (!m_spNext->Parse(psz))
					return false;
			}
			return true; //�������� ���������
		default: 
			sp.reset(new CFilterSymbol(ch));
		}

		if (sp)
			m_tokens.emplace_back(std::move(sp));
	}

	//���� ������ �� �������� * � �����, ��������� ������� ��� ������ �� ����� ������, ������ 0
	if (!m_tokens.empty())
	{
		sp.reset(new CFilterSymbol('\0'));
		m_tokens.emplace_back(std::move(sp));
	}

	return true;
}

//��������� ����������� ������ psz �������
bool CFilter::Test(const char *psz, const char *endl)
{
	if (m_tokens.empty()) //���� ��������� ������� ���, �� ��� ����������� *, ����� �� ������ �� ����� ������
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
	return psz != nullptr && (!m_spNext || m_spNext->Test(psz, endl));
}

//��������� ������ �� ����������� ���� ��������� �������,
//���������� ��������� �� ��������� ������ ��� nullptr ������� ��������
const char *CFilter::ApplyTokens(const char *psz, const char *endl)
{
	for (auto it = m_tokens.begin(), end = m_tokens.end(); it != end; ++it)
	{
		if (psz == endl)
			return nullptr;

		auto p = (*it)->Test(psz, endl);
		if (!p)
			return nullptr;

		psz = p;
	}

	return psz;
}

#pragma once
#include <vector>

//Элементы шаблона
// ? - Один любой символ
// * - Последовательность любых символов неограниченной длины
// + - Повторять предыдущий элемент, пока совпадает
// \ - Подставляется следущий символ
// \t - Табуляция
// \r - Символ '\r'
// \s - Любой пробел


class CFilter;

//Базовый класс для элемента шаблона фильтра
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
	
	//Проверяет соответствие строки эелемнту шаблона
	//psz - начало строки, endl - конец строки, включая завершающий 0
	//Возвращает указатель на следующий символ или nullptr в случе неудачи
	const char *Test(const char *psz, const char *end)
	{
		if (!m_repeat)
			return _Test(psz, end);

		//Повторяем, пока соответствует
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
	//Вызывается из Test(), проверяет соответствие строки эелемнту шаблона, но без повторов
	virtual const char *_Test(const char *psz, const char *endl) = 0;

	bool m_repeat; //Повторять пока соответствует
};

//Шаблон фильтра
//Один фильтр содержит содержит элементы, начиная с начала строки или *, и до следующей * или конца строки
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

	bool m_bol; //Проверять с начала строки
	std::vector<std::unique_ptr<CFilterToken>> m_tokens; //Элементы шаблона
	std::unique_ptr<CFilter> m_spNext; //Следующий фильтр
};

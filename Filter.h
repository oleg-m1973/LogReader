#pragma once

//Элементы шаблона
// ? - Один любой символ
// * - Последовательность любых символов неограниченной длины
// + - Повторять предыдущий элемент, пока совпадает
// \ - Подставляется следущий символ
// \t - Табуляция
// \r - Символ '\r'
// \s - Любой пробел
// [...]  - Совпадение с любым из символов между []

class CFilter;

//Базовый класс для элемента шаблона фильтра
class CFilterToken
{
friend class CFilter;
public:
	CFilterToken()
	: m_repeat(false)
	, m_next(nullptr)
	{
	}

	virtual ~CFilterToken()
	{
		delete m_next;
	}
	
	//Начитывает элемент из строки шаблона
	//Возвращает указатель на следующий элемент, или nullptr в случае ошибки
	virtual const char *Read(const char *psz) = 0;

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
			if (!p || p == psz)
				break;

			psz = p;
		}

		return psz;
	}

protected:
	//Вызывается из Test(), проверяет соответствие строки эелемнту шаблона, но без повторов
	virtual const char *_Test(const char *psz, const char *endl) = 0;

	bool m_repeat; //Повторять пока соответствует
	CFilterToken *m_next;
};

//Шаблон фильтра
//Один фильтр содержит содержит элементы, начиная с начала строки или *, и до следующей * или конца строки
class CFilter
{
public:
	CFilter()
	: m_bol(true)
	, m_first(nullptr)
	, m_last(nullptr)
	, m_next(nullptr)
	{
	}

	~CFilter()
	{
		Reset();
	}
	bool Parse(const char *psz);
	bool Test(const char *psz, const char *endl);

	void Reset()
	{
		m_bol = true;
		delete m_first;
		delete m_next;

		m_next = nullptr;
		m_first = m_last = nullptr;
	}

protected:
	bool IsEmpty() const
	{
		return m_last == nullptr;
	}

	//Добавляет новый элемент шаблона
	//Возвращает указатель на следующий символ или nullptr в случае ошибки
	template <typename T>
	const char *AddToken(const char *psz)
	{
		auto *p = new T();
		if (m_last)
			m_last = m_last->m_next = p;
		else
			m_first = m_last = p;

		return p->Read(psz);
	}

	const char *ApplyTokens(const char *psz, const char *endl);

	bool m_bol; //Проверять с начала строки

	//Элементы шаблона
	CFilterToken *m_first;  //Первый
	CFilterToken *m_last; //Последний
	
	CFilter *m_next; //Следующий фильтр
};

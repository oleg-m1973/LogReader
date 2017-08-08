#pragma once
#include "Filter.h"

//Буфер для хранения незавершённой строки
//В конце - всегда 0
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
		
	//Добавить данные
	bool Push(const char *data, size_t sz)
	{
		if (!sz)
			return true;

		char *p = (char *)realloc(m_data, m_sz + sz + 1); //Выделяем память +'\0'
		if (!p)
			return false;

		memcpy(p + m_sz, data, sz);

		m_data = p;
		m_sz += sz;
				
		m_data[m_sz] = 0;
		return true;
	}

	//Очистить буфер
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

	//открытие файла, false - ошибка
	//name - имя файла
	//buf_cap - размер буфера чтения
	bool Open(LPCSTR name, size_t buf_cap = BufferSize);

	//закрытие файла
	void Close();

	//установка фильтра строк, false - ошибка
	bool SetFilter(const char *filter);

	//запрос очередной найденной строки, buf - буфер, bufsize - максимальная длина
	//false - конец файла или ошибка
	bool GetNextLine(char *buf, const size_t sz);

	bool TestLine(const char *psz, const char *endl);

protected:
	HANDLE m_hFile;

	size_t m_buf_cap; //Размер буфера
	DWORD m_buf_sz; //Количество, считанное в последней итурации
	size_t m_buf_idx; //Текущее смещение в буфере
	char *m_buf; //Буфер данных для нового блока

	CDataBuffer m_data; //Не законченная в предыдущей итерации строка

	CFilter m_filter;
};

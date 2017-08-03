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
	std::unique_ptr<char[]> m_buf; //Буфер данных для нового блока

	std::vector<char> m_data; //Не законченная в предыдущей итерации строка

	std::unique_ptr<CFilter> m_spFilter;
};

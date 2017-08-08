#include "stdafx.h"
#include "LogReader.h"

CLogReader::CLogReader()
: m_hFile(INVALID_HANDLE_VALUE)
, m_buf(nullptr)
{
}

//открытие файла, false - ошибка
bool CLogReader::Open(LPCSTR name, size_t buf_cap)
{
	Close();

	m_buf_cap = buf_cap? buf_cap: BufferSize;
	m_buf_sz = 0;
	m_buf_idx = 0;
	m_buf = new char[m_buf_cap];

	m_hFile = ::CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	return m_hFile != INVALID_HANDLE_VALUE;
}

void CLogReader::Close()
{
	::CloseHandle(m_hFile);
	m_data.Clear();
	delete[] m_buf;
}

//установка фильтра строк, false - ошибка
bool CLogReader::SetFilter(const char *filter)
{
	if (!filter || !*filter)
	{
		m_filter.Reset();
		return true;
	}

	return m_filter.Parse(filter);
}

bool CLogReader::TestLine(const char *psz, const char *endl)
{
	const bool res = m_filter.Test(psz, endl);
	return res; 
}

bool CLogReader::GetNextLine(char *buf, const size_t sz) 
{
	size_t line = m_buf_idx; //Начало строки
	for (;;)
	{
		//Если текущий буфер обработан, считываем новые данные
		if (m_buf_idx == m_buf_sz)
		{
			//Сохраняем незаконченную строку
			if (!m_data.Push(m_buf + line, m_buf_idx - line))
				return false; //Недостаточно памяти

			//Сбрасываем индексы на начало буфера
			m_buf_idx = 0;
			line = 0;

			if (!::ReadFile(m_hFile, m_buf, DWORD(m_buf_cap), &m_buf_sz, nullptr) || !m_buf_sz) 
				break; //Ошибка или конец файла
		}

		for (; m_buf_idx < m_buf_sz; ++m_buf_idx)
		{
			if (m_buf[m_buf_idx] != '\n')
				continue;

			CLineBuffer data; 
			data.swap(m_data); //Сбрасываем буфер

			char *psz = m_buf + line; //Указатель на начало строки
			char *endl = m_buf + m_buf_idx + 1; //Конец строки, включая 0
			m_buf[m_buf_idx] = 0;

			if (!data.empty()) //Если есть незаконченная строка 
			{
				//Добавляем остаток строки
				data.Push(psz, m_buf_idx - line);
				psz = data.data();
				endl = psz + data.size() + 1;
			}

			if (TestLine(psz, endl))
			{
				strncpy(buf, psz, sz);
				++m_buf_idx; //Пропускаем последний endl
				return true;
			}

			line = m_buf_idx + 1; //Начало следующей строки
		}
	}

	if (!m_data.empty()) //Если в буфере осталась строка
	{
		CLineBuffer data; 
		data.swap(m_data); //Сбрасываем буфер
		if (TestLine(data.data(), data.data() + data.size() + 1))
		{
			strncpy(buf, data.data(), sz);
			return true;
		}
	}

	return false;
}


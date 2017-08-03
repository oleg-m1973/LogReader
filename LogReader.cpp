#include "stdafx.h"
#include "LogReader.h"

CLogReader::CLogReader()
: m_hFile(INVALID_HANDLE_VALUE)
{
}

//открытие файла, false - ошибка
bool CLogReader::Open(LPCSTR name, size_t buf_cap)
{
	Close();

	m_buf_cap = buf_cap? buf_cap: BufferSize;
	m_buf_sz = 0;
	m_buf_idx = 0;
	m_buf.reset(new char[m_buf_cap]);
	m_data.reserve(m_buf_cap);

	m_hFile = ::CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	return m_hFile != INVALID_HANDLE_VALUE;
}

void CLogReader::Close()
{
	::CloseHandle(m_hFile);
	m_data.clear();
	m_buf.reset();
}

//установка фильтра строк, false - ошибка
bool CLogReader::SetFilter(const char *filter)
{
	m_spFilter.reset(new CFilter());
	return m_spFilter->Parse(filter);
}

bool CLogReader::TestLine(const char *psz, const char *endl)
{
	const bool res = !m_spFilter || m_spFilter->Test(psz, endl);
	return res; 
}

bool CLogReader::GetNextLine(char *buf, const size_t sz) 
{
	try
	{
		size_t line = m_buf_idx; //Начало строки
		for (;;)
		{
			//Если текущий буфер обработан, считываем новые данные
			if (m_buf_idx == m_buf_sz)
			{
				m_data.insert(m_data.end(), m_buf.get() + line, m_buf.get() + m_buf_idx); //Сохраняем незаконченную строку

				//Сбрасываем индексы на начало буфера
				m_buf_idx = 0;
				line = 0;

				if (!::ReadFile(m_hFile, m_buf.get(), DWORD(m_buf_cap), &m_buf_sz, nullptr) || !m_buf_sz) 
					break; //Ошибка или конец файла
			}

			for (; m_buf_idx < m_buf_sz; ++m_buf_idx)
			{
				if (m_buf[m_buf_idx] != '\n')
					continue;

				auto data = std::move(m_data); //Сбрасываем буфер
				char *psz = m_buf.get() + line; //Указатель на начало строки
				char *endl = m_buf.get() + m_buf_idx;
				if (data.empty()) //Если строка полностью находится в буфере
					m_buf[m_buf_idx] = 0;
				else //Если есть незаконченная строка 
				{
					//Добавляем остаток строки
					data.insert(data.end(), psz, endl);
					data.push_back(0);
					psz = data.data();
					endl = psz + data.size();
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
			auto data = std::move(m_data); //Сбрасываем буфер
			data.push_back(0);
			if (TestLine(data.data(), data.data() + data.size()))
			{
				strncpy(buf, data.data(), sz);
				return true;
			}
		}
	}
	catch(...)
	{
		//throw;
	}
	return false;
}


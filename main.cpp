#include "stdafx.h"
#include "LogReader.h"

int main(int argc, char *argv[])
{
	setlocale(LC_CTYPE, "Russian");

	if (argc < 3)
	{
		std::cerr << "Usage: LogReader pattern file" << std::endl;
		return -1;
	}

	const char *pattern = argv[1];
	const char *file = argv[2];
	

	std::clog << "File: " << file << std::endl;
	std::clog << "Pattern: " << pattern << std::endl;

	CLogReader reader;

	if (!reader.SetFilter(pattern))
	{
		std::cerr << "Invalid pattern " << pattern << std::endl;
		return -1;
	}

	if (!reader.Open(file))
	{
		std::cerr << "Can't open file " << file << std::endl;
		return -1;
	}

	static const size_t _sz = 10 * 1024;
	char buf[_sz];

	auto tm = ::GetTickCount();
	size_t n = 0;
	for (; reader.GetNextLine(buf, _sz); ++n)
	{
		std::cout << buf << std::endl;
	}

	tm = ::GetTickCount() - tm;
	std::clog << "Found " << n <<  " lines, " << (double(tm) / 1000.0) << " sec" << std::endl;
	return 0;
}


#include "stdafx.h"
#include "LogReader.h"

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: LogReader pattern file\n");
		return -1;
	}

	const char *pattern = argv[1];
	const char *file = argv[2];


	fprintf(stderr, "File: %s\n", file);
	fprintf(stderr, "Pattern: %s\n", pattern);

	CLogReader reader;

	if (!reader.SetFilter(pattern))
	{
		fprintf(stderr, "Invalid pattern %s\n", pattern);
		return -1;
	}

	if (!reader.Open(file))
	{
		fprintf(stderr, "Can't open file %s\n", file);
		return -1;
	}

	static const size_t _sz = 10 * 1024;
	char buf[_sz];

	auto tm = ::GetTickCount();
	size_t n = 0;
	for (; reader.GetNextLine(buf, _sz); ++n)
	{
		fprintf(stdout, "%s\n", buf);
	}

	tm = ::GetTickCount() - tm;
	fprintf(stderr, "Found %u lines, %f sec\n", n, double(tm) / 1000.0);
	return 0;
}


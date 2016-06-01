// Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rw.h"

#define USAGE_STR	L" rwtools /from:value /offset1:value /to:value /offset2:value /block:value /size:value /nobuffer\n"	\
					L"   Usage:\n"											\
					L"     from       read data from\n"						\
					L"     offset1    offset of data source, default 0\n"	\
					L"     to         write data to, default console\n"		\
					L"     offset2    offset of data target, default 0\n"	\
					L"     size       read/write total size\n"				\
					L"     block      size of byte once, defalut 1MB\n"		\
					L"     nobuffer   open with no buffering\n\n"			\
					L"   Eg: \n"																				\
					L"     rwtools /from:\\\\.\\Z: /offset1:0 /block:4096\n"									\
					L"     rwtools /from:\\\\.\\PHYSICALDRIVE1 /offset1:0 /to:D:\\data.dat\n"					\
					L"     rwtools /from:Z:\\test.dat /offset1:0 /to:\\\\.\\Z: /block:4096 /nobuffer\n"			\
					L"     rwtools /from:\\\\.\\Z: /offset1:0 /to:D:\\data.dat /offset2:4096 /block:8192 /size:16384\n\n"	\


int _tmain(int argc, _TCHAR* argv[])
{
	RwTools rw;
	RW_PARAM param;

	if (!rw.ParseParam(argc, argv, param))
	{
		wprintf_s(USAGE_STR);
		return -1;
	}

	return rw.Run(param);
}
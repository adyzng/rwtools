#pragma once
#include "stdafx.h"

#define PHYSICAL_DRIVE_PRE				L"PhysicalDrive"

#define DEFAULT_SECTOR_SZIE				512			// 
#define DEFAULT_BLOCK_SIZE				(1 << 20)	// 1MB

#define BYTES_PER_ROW_FOR_SHOW_BINARY	16


typedef struct _rw_param
{
	BOOL		NoBuffer;
	ULONG		BlockSize;
	ULONGLONG	TotalSize;
	ULONGLONG	OffsetSrc;
	ULONGLONG	OffsetDst;
	wstring		DstName;
	wstring		SrcName;

	_rw_param() 
		:NoBuffer(FALSE),
		BlockSize(0),
		TotalSize(0),
		OffsetSrc(0),
		OffsetDst(0)
	{
		DstName.clear();
		SrcName.clear();
	}

}RW_PARAM, *PRW_PARAM;


class RwTools
{
public:
	DWORD	Run(const RW_PARAM& param);
	BOOL	ParseParam(int argc, _TCHAR* argv[], RW_PARAM& param);

private:
	BOOL	KeyValue(wstring strKeyValue, wstring& strKey, wstring& strValue);

	DWORD	SetFilePos(HANDLE hFile, ULONGLONG Offset);
	HANDLE	SimpleCreateFile(const wstring& Name, BOOL CreateAlways, BOOL NoBuffuring = TRUE, BOOL bExclusive = FALSE);

	DWORD	ReadBySector(HANDLE hFile, LPVOID lpBuffer, ULONGLONG Offset, DWORD nBytesToRead, LPDWORD lpBytesRead);
	DWORD	WriteBySector(HANDLE hFile, LPVOID lpBuffer, ULONGLONG Offset, DWORD nBytesToWrite, LPDWORD lpBytesWrite);

	void	DumpMemory(ULONGLONG offset, unsigned char* pbMemStart, unsigned long ulMemSize, BOOLEAN bShowASCCode);
};
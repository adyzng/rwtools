#include "stdafx.h"
#include <winioctl.h>
#include "rw.h"


void RwTools::DumpMemory(ULONGLONG offset, unsigned char* pbMemStart, unsigned long ulMemSize, BOOLEAN bShowASCCode)
{
	// Format: Address(8bytes+1) 01 02 ... 0N ; 12...N, thus total charactors in a row should be 10 + ulBytePerRow * 4 + 4
	// ......... 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
	// 00000000h 4D 69 63 72 6F 73 6F 66 74 20 43 2F 43 2B 2B 20 ; Microsoft C/C++
	unsigned char pbAscCode[20] = { 0 };
	unsigned long ulRow = 0;
	WCHAR szOutput[MAX_PATH] = { 0 };
	WCHAR szTemp[32] = { 0 };

	for (ulRow = 0; ulRow < (ulMemSize / BYTES_PER_ROW_FOR_SHOW_BINARY); ulRow++)
	{
		unsigned char* pzCurMemRow = pbMemStart + (ulRow * BYTES_PER_ROW_FOR_SHOW_BINARY);
		memset(pbAscCode, 0, sizeof(pbAscCode));

		UINT ulSize = BYTES_PER_ROW_FOR_SHOW_BINARY;
		ZeroMemory(szOutput, sizeof(szOutput));

		for (UINT uiIdx = 0; uiIdx < ulSize; uiIdx++)
		{
			if ((pzCurMemRow[uiIdx] > 0x20) && (pzCurMemRow[uiIdx] < 0x7F))
				pbAscCode[uiIdx] = pzCurMemRow[uiIdx];
			else
				pbAscCode[uiIdx] = '.';
		}

		swprintf_s(szTemp, ARRAYSIZE(szTemp), L"[%016I64X]  ", offset);
		wcscat_s(szOutput, ARRAYSIZE(szOutput), szTemp);

		for (UINT i = 0; i < ulSize; i++)
		{
			swprintf_s(szTemp, ARRAYSIZE(szTemp), L"%02X ", pzCurMemRow[i]);
			wcscat_s(szOutput, ARRAYSIZE(szOutput), szTemp);
		}

		wprintf_s(L"%s", szOutput);
		wprintf_s(L";  %S\n", pbAscCode);

		offset += BYTES_PER_ROW_FOR_SHOW_BINARY;
	}
}

DWORD RwTools::SetFilePos(HANDLE hFile, ULONGLONG Offset)
{
	LARGE_INTEGER liToMove = { 0 };
	LARGE_INTEGER liNewPos = { 0 };
	liToMove.QuadPart = Offset;

	DWORD dwRet = 0;
	if (!SetFilePointerEx(hFile, liToMove, &liNewPos, FILE_BEGIN) || (liNewPos.QuadPart != liToMove.QuadPart))
	{
		dwRet = GetLastError();
		wprintf_s(L"Failed to set file pointer to [%I64u]. Error %u\n", liToMove.QuadPart, dwRet);
	}

	return dwRet;
}

HANDLE RwTools::SimpleCreateFile(const wstring& Name, BOOL CreateAlways, BOOL NoBuffuring/* = TRUE*/, BOOL bExclusive/* = FALSE*/)
{
	DWORD FlagsAndAttr = FILE_ATTRIBUTE_NORMAL;
	if (NoBuffuring)
		FlagsAndAttr = FILE_FLAG_NO_BUFFERING;

	DWORD CreateDispos = OPEN_EXISTING;
	if (CreateAlways)
		CreateDispos = CREATE_ALWAYS;

	DWORD ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if (bExclusive)
		ShareMode = 0;

	HANDLE hFile = CreateFileW(
		Name.c_str(),
		GENERIC_WRITE | GENERIC_READ,
		ShareMode,
		NULL,
		CreateDispos,
		FlagsAndAttr,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL)
	{
		hFile = NULL;
		DWORD dwError = GetLastError();
		wprintf_s(L"Failed to open %s. Error %u\n", Name.c_str(), dwError);
	}

	return hFile;
}

DWORD RwTools::ReadBySector(HANDLE hFile, LPVOID lpBuffer, ULONGLONG Offset, DWORD nBytesToRead, LPDWORD lpBytesRead)
{
	DWORD		dwRet = 0;
	BOOL		bSetPos = TRUE;
	DWORD		dwSectorCnt = 0;
	DWORD		dwBytesRet = 0;
	DWORD		dwBytesRetTotal = 0;
	PBYTE		pTempBuffer = (PBYTE)lpBuffer;
	ULONGLONG	Remain = nBytesToRead;

	while (Remain)
	{
		DWORD sizeOnce = (DWORD)min(Remain, DEFAULT_SECTOR_SZIE);

		if (bSetPos)
		{
			bSetPos = FALSE;
			SetFilePos(hFile, Offset);
		}

		if (!ReadFile(hFile, pTempBuffer, sizeOnce, &dwBytesRet, NULL))
		{
			bSetPos = TRUE;
			dwRet = GetLastError();
			wprintf_s(L"[Offset: %I64u, Sector: %u] Read failed. Error %u\n", Offset, dwSectorCnt, dwRet);
		}
		else
		{
			dwBytesRetTotal += dwBytesRet;
		}

		pTempBuffer += sizeOnce;
		Offset += sizeOnce;
		dwSectorCnt++;
	}

	if (lpBytesRead)
	{
		*lpBytesRead = dwBytesRetTotal;
	}

	return dwRet;
}

DWORD RwTools::WriteBySector(HANDLE hFile, LPVOID lpBuffer, ULONGLONG Offset, DWORD nBytesToWrite, LPDWORD lpBytesWrite)
{
	DWORD		dwRet = 0;
	BOOL		bSetPos = TRUE;
	DWORD		dwSectorCnt = 0;
	DWORD		dwBytesRet = 0;
	DWORD		dwBytesRetTotal = 0;
	PBYTE		pTempBuffer = (PBYTE)lpBuffer;
	ULONGLONG	Remain = nBytesToWrite;

	while (Remain)
	{
		DWORD sizeOnce = (DWORD)min(Remain, DEFAULT_SECTOR_SZIE);

		if (bSetPos)
		{
			bSetPos = FALSE;
			SetFilePos(hFile, Offset);
		}

		if (!WriteFile(hFile, pTempBuffer, sizeOnce, &dwBytesRet, NULL))
		{
			bSetPos = TRUE;
			dwRet = GetLastError();
			wprintf_s(L"[Offset: %I64u, Sector: %u] Write failed. Error %u\n", Offset, dwSectorCnt, dwRet);
		}
		else
		{
			dwBytesRetTotal += dwBytesRet;
		}

		pTempBuffer += sizeOnce;
		Offset += sizeOnce;
		dwSectorCnt++;
	}

	if (lpBytesWrite)
		*lpBytesWrite = dwBytesRetTotal;

	return dwRet;
}

BOOL RwTools::KeyValue(wstring strKeyValue, wstring& strKey, wstring& strValue)
{
	if (strKeyValue.empty() || ((strKeyValue[0] != '/') && (strKeyValue[0] != '-')))
		return FALSE;

	size_t pos	= strKeyValue.find(L':');
	if (pos == wstring::npos)
	{
		strKey = strKeyValue.substr(1);
		strValue = L"";
	}
	else
	{
		strKey = strKeyValue.substr(1, pos - 1);
		strValue = strKeyValue.substr(pos + 1);
	}

	return TRUE; 
}

BOOL RwTools::ParseParam(int argc, _TCHAR* argv[], RW_PARAM& param)
{
	BOOL bValid = TRUE;
	wstring strKey, strValue;

	for (int idx = 1; idx < argc; idx++)
	{
		if (!KeyValue(argv[idx], strKey, strValue))
		{
			bValid = FALSE;
			break;
		}

		if (_wcsicmp(strKey.c_str(), L"from") == 0){
			param.SrcName = strValue;
			if (strValue.empty()) break;
		}
		else if (_wcsicmp(strKey.c_str(), L"to") == 0){
			param.DstName = strValue;
			if (strValue.empty()) break;
		}
		else if (_wcsicmp(strKey.c_str(), L"offset1") == 0){
			param.OffsetSrc = _wtoi64(strValue.c_str());
		}
		else if (_wcsicmp(strKey.c_str(), L"offset2") == 0){
			param.OffsetDst = _wtoi64(strValue.c_str());
		}
		else if (_wcsicmp(strKey.c_str(), L"block") == 0){
			param.BlockSize = _wtol(strValue.c_str());
		}
		else if (_wcsicmp(strKey.c_str(), L"size") == 0){
			param.TotalSize = _wtoi64(strValue.c_str());
		}
		else if (_wcsicmp(strKey.c_str(), L"nobuffer") == 0){
			param.NoBuffer = TRUE;
		}
		else if (_wcsicmp(strKey.c_str(), L"debug") == 0){
			while (1) Sleep(200);
		}
		else{
			bValid = FALSE;
			wprintf_s(L"Invalid parameter : %s\n", argv[idx]);
			break;
		}
	}

	if (!bValid)
	{
		wprintf_s(L"Invalid parameter : %s-%s\n", strKey.c_str(), strValue.c_str());
		return bValid;
	}

	if (param.SrcName.empty())
		return FALSE;

	if (param.BlockSize == 0)
		param.BlockSize = DEFAULT_BLOCK_SIZE;

	if (param.TotalSize == 0)
		param.TotalSize = param.BlockSize;
	param.TotalSize = max(param.TotalSize, DEFAULT_SECTOR_SZIE);

	if (param.TotalSize < param.BlockSize)
		param.BlockSize = (ULONG)param.TotalSize;
	param.BlockSize = max(param.BlockSize, DEFAULT_SECTOR_SZIE);

	return bValid;
}

DWORD RwTools::Run(const RW_PARAM& param)
{
	DWORD dwRet = 0;
	if (param.SrcName.empty() || param.TotalSize == 0)
		return ERROR_INVALID_PARAMETER;

	HANDLE hSrc = NULL;
	HANDLE hDst = NULL;
	PBYTE pBuffer = NULL;
	DWORD dwBytesRet = 0;

	do 
	{
		const DWORD BlockSize = param.BlockSize ? param.BlockSize : DEFAULT_BLOCK_SIZE;
		pBuffer = new BYTE[BlockSize];

		if (pBuffer == NULL)
		{
			dwRet = GetLastError();
			wprintf_s(L"Failed to allocate %u bytes buffer. Error %u.\n", BlockSize, dwRet);
			break;
		}

		hSrc = SimpleCreateFile(param.SrcName, FALSE, param.NoBuffer);
		if (hSrc == NULL)
		{
			dwRet = GetLastError();
			break;
		}
		
		if (!param.DstName.empty())
		{
			BOOL bCreateAlways = FALSE;
			BOOL bTargetIsFile = FALSE;
			
			size_t pos = param.DstName.rfind(L':');
			if ((pos != wstring::npos) && pos < (param.DstName.length() - 1))
				bTargetIsFile = TRUE;

			if (bTargetIsFile && (param.OffsetDst == 0) )
				bCreateAlways = TRUE;

			hDst = SimpleCreateFile(param.DstName, bCreateAlways, param.NoBuffer);
			if (hDst == NULL)
			{
				dwRet = GetLastError();
				break;
			}

			if (!bTargetIsFile)
			{
				if (!DeviceIoControl(hDst, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwBytesRet, NULL))
				{
					dwRet = GetLastError();
					wprintf_s(L"Failed to lock target volume %s. Error %u\n", param.DstName.c_str(), dwRet);
					break;
				}
			}
		}

		BOOL		bSetPosSrc = TRUE;
		BOOL		bSetPosDst = TRUE;
		ULONGLONG	Remain = param.TotalSize;
		ULONGLONG	OffsetSrc = param.OffsetSrc;
		ULONGLONG	OffsetDst = param.OffsetDst;
		
		while (Remain)
		{
			DWORD SizeOnce = (DWORD)min(BlockSize, Remain);
			ZeroMemory(pBuffer, SizeOnce);

			if (bSetPosSrc)
			{
				bSetPosSrc = FALSE;
				if (dwRet = SetFilePos(hSrc, OffsetSrc))
					break;
			}

			if (!ReadFile(hSrc, pBuffer, SizeOnce, &dwBytesRet, NULL))
			{
				bSetPosSrc = TRUE;
				dwRet = GetLastError();
				wprintf_s(L"Failed to read [offset: %I64u, Block: %u]. Error %u\n", OffsetSrc, SizeOnce, dwRet);

				ReadBySector(hSrc, pBuffer, OffsetSrc, SizeOnce, &dwBytesRet);
			}

			if (hDst != NULL)
			{
				if (bSetPosDst)
				{
					bSetPosDst = FALSE;
					if (dwRet = SetFilePos(hDst, OffsetDst))
						break;
				}
				
				if (!WriteFile(hDst, pBuffer, SizeOnce, &dwBytesRet, NULL))
				{
					bSetPosSrc = TRUE;
					dwRet = GetLastError();
					wprintf_s(L"Failed to write [offset: %I64u, Block: %u]. Error %u\n", OffsetDst, SizeOnce, dwRet);

					WriteBySector(hDst, pBuffer, OffsetDst, SizeOnce, &dwBytesRet);
				}
			}
			else
			{
				wprintf_s(L"Read Data: [offset: %I64u, Block: %u]\n", OffsetSrc, SizeOnce);
				DumpMemory(OffsetSrc, pBuffer, min(SizeOnce, 512), TRUE);
			}

			OffsetSrc += SizeOnce;
			OffsetDst += SizeOnce;
			Remain -= SizeOnce;

			wprintf_s(L"Current [%d B @ %I64d], Total [%I64d B / %I64d B]\n", SizeOnce, OffsetSrc, (param.TotalSize - Remain), param.TotalSize);
		}

	} while (FALSE);

	if (pBuffer)
	{
		delete[] pBuffer;
		pBuffer = NULL;
	}

	if (hDst)
	{
		CloseHandle(hDst);
		hDst = NULL;
	}

	if (hSrc)
	{
		CloseHandle(hSrc);
		hSrc = NULL;
	}

	return dwRet;
}

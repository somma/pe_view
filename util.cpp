/**----------------------------------------------------------------------------
 * util.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:7:15 15:39 created
**---------------------------------------------------------------------------*/

#include <stdafx.h>
#include "util.h"

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
void write_to_console(_In_ DWORD log_level, _In_ wchar_t* function, _In_ wchar_t* format, ...)
{
	static HANDLE	_stdout_handle = INVALID_HANDLE_VALUE;
	static WORD		_old_color = 0x0000;
	
	//> initialization for console text color manipulation.
	if (INVALID_HANDLE_VALUE == _stdout_handle)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(_stdout_handle, &csbi);
		_old_color = csbi.wAttributes;
	}

	std::wstringstream strm_prefix;
	switch (log_level)
	{
	case LL_DEBG: 
		{
			strm_prefix << L"[DEBG] " << function << L", "; 
			break;
		}
	case LL_INFO: 
		{
			strm_prefix << L"[INFO] " << function << L", "; 
			SetConsoleTextAttribute(_stdout_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			break;
		}
	case LL_ERRR: 
		{
			strm_prefix << L"[ERR ] " << function << L", "; 
			SetConsoleTextAttribute(_stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
			break;
		}
	case LL_NONE:
	default:
		{
			strm_prefix << function << L", ";
			break;
		}
	}

	DWORD len;
	va_list args;
	WCHAR msg[4096];	
	
	va_start(args, format);
	if(TRUE != SUCCEEDED(StringCchVPrintf(msg, sizeof(msg), format, args))){ return; }
	va_end(args);

	WriteConsole(_stdout_handle, strm_prefix.str().c_str(), (DWORD)strm_prefix.str().size(), &len, NULL);
	WriteConsole(_stdout_handle, msg, (DWORD)wcslen(msg), &len, NULL);	
	WriteConsole(_stdout_handle, L"\n", (DWORD)wcslen(L"\n"), &len, NULL);
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _old_color);

#ifdef _USE_AHNTRACE_
	switch (log_level)
	{
	case LL_DEBG: DEBUGLOG(msg); break;
	case LL_INFO: INFOLOG(msg); break;	
	case LL_ERRR: ERRORLOG(msg); break;
	default:
		break;		
	}
#endif//_USE_AHNTRACE_
}



/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool is_file_existsW(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = {0};

	//
	// CreateFile()이 아닌 GetFileAttributesEx()를 이용하면 파일이 다른 process에 의해 lock되어 있어도
	// 파일 존재여부를 정확히 체크할 수 있다.
	//
	if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info)==0) 
		return false;
	else
		return true;
}
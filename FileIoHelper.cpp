/*-----------------------------------------------------------------------------
 * FileIoHelper.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * - 01.09.2010 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "util.h"
#include "FileIoHelper.h"

/**----------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
bool OpenFileContext(IN PCWSTR FilePath, OUT PFILE_CTX& Ctx)
{
    _ASSERTE(NULL != FilePath);
    if (NULL == FilePath) return false;
    if (!is_file_existsW(FilePath)) return false;;

	_ASSERT(NULL == Ctx);
	if (NULL != Ctx) return false;;

    Ctx = (PFILE_CTX) malloc(sizeof(FILE_CTX));
	if (NULL == Ctx) return false;

    RtlZeroMemory(Ctx, sizeof(FILE_CTX));
	bool ret = false;

#pragma warning(disable: 4127)
    do 
    {
        Ctx->FileHandle = CreateFileW(
                            (LPCWSTR)FilePath, 
                            GENERIC_READ, 
                            NULL,
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL
                            );
        if (INVALID_HANDLE_VALUE == Ctx->FileHandle)
        {
            log_err
                L"CreateFile(%ws) failed, gle = %u", 
                FilePath, 
                GetLastError()
            log_end
            break;
        }

        // check file size 
        // 
        LARGE_INTEGER fileSize;
        if (TRUE != GetFileSizeEx(Ctx->FileHandle, &fileSize))
        {
            log_err
                L"%ws, can not get file size, gle = %u", 
                FilePath, 
                GetLastError() 
            log_end
            break;
        }

        // [ WARN ]
        // 
        // 4Gb 이상의 파일의 경우 MapViewOfFile()에서 오류가 나거나 
        // 파일 포인터 이동이 문제가 됨
        // FilIoHelperClass 모듈을 이용해야 함
        // 
        _ASSERTE(fileSize.HighPart == 0);
		if (fileSize.HighPart > 0) 
		{
			log_err
				L"file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
				fileSize.QuadPart
			log_end
			break;
		}

        Ctx->FileSize = (DWORD)fileSize.QuadPart;
        Ctx->FileMap = CreateFileMapping(
                                Ctx->FileHandle, 
                                NULL, 
                                PAGE_READONLY, 
                                0, 
                                0, 
                                NULL
                                );
        if (NULL == Ctx->FileMap)
        {
            log_err
                L"CreateFileMapping(%ws) failed, gle = %u", 
                FilePath, 
                GetLastError() 
            log_end
            break;
        }

        Ctx->FileView = (PUCHAR) MapViewOfFile(
                                        Ctx->FileMap, 
                                        FILE_MAP_READ, 
                                        0, 
                                        0, 
                                        0
                                        );
        if(Ctx->FileView == NULL)
        {
            log_err
                L"MapViewOfFile(%ws) failed, gle = %u", 
                FilePath, 
                GetLastError() 
            log_end
            break;
        }    
        
		ret = true;
    } while (FALSE);
#pragma warning(default: 4127)

    if (!ret)
    {
        if (INVALID_HANDLE_VALUE != Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
        if (NULL!= Ctx->FileMap) CloseHandle(Ctx->FileMap);
        if (NULL!= Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
        free(Ctx);
    }

    return ret;
}



/**----------------------------------------------------------------------------
    \brief  

    \param  
    \return         
    \code
    \endcode        
-----------------------------------------------------------------------------*/
void CloseFileContext(IN PFILE_CTX& Ctx)
{
	_ASSERTE(NULL != Ctx);
	if (NULL == Ctx) return;

    if (INVALID_HANDLE_VALUE!= Ctx->FileHandle) CloseHandle(Ctx->FileHandle);
    if (NULL!= Ctx->FileMap) CloseHandle(Ctx->FileMap);
    if (NULL!= Ctx->FileView) UnmapViewOfFile(Ctx->FileView);
    free(Ctx);
}
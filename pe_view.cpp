/**----------------------------------------------------------------------------
 * pe_view.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com)
 *-----------------------------------------------------------------------------
 * 2014:7:11 0:16 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "FileIoHelper.h"

#define GetImgDirEntryRVA( inh, IDE ) (inh->OptionalHeader.DataDirectory[IDE].VirtualAddress)


// RVA 값을 포함하는 섹션 헤더를 찾는 함수
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS inh)
{
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(inh);
        
    for (UINT32 i = 0; i < inh->FileHeader.NumberOfSections; i++, section++ )
    {
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		DWORD size = section->Misc.VirtualSize;
		if ( 0 == size )
			size = section->SizeOfRawData;
			
        // Is the RVA within this section?
        if ( (rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
            return section;
    }
    
    return 0;
}

// RVA 값을 통해 image_base 기준, 파일 포인터를 찾는 함수
LPVOID GetPtrFromRVA(DWORD rva, const PIMAGE_NT_HEADERS inh, const UCHAR* image_base)
{
	PIMAGE_SECTION_HEADER ish = GetEnclosingSectionHeader( rva, inh );
	if (NULL == ish) return NULL;

	int delta = (int)(ish->VirtualAddress - ish->PointerToRawData);
	return (PVOID) ( image_base + rva - delta );
}


void show_usage(_In_ wchar_t* me);
bool parse_pe_header(_In_ const UCHAR* file_base, _In_ UINT32 file_size);
void dump_import_section(_In_ const UCHAR* image_base, _In_ const PIMAGE_NT_HEADERS inh);
void dump_import(
			_In_ PIMAGE_THUNK_DATA p_int, 
			_In_ PIMAGE_THUNK_DATA p_iat, 
			_In_ const PIMAGE_NT_HEADERS inh, 
			_In_ PIMAGE_IMPORT_DESCRIPTOR iid, 
			_In_ const UCHAR* image_base
			);


void dump_export_section(_In_ const UCHAR* image_base, _In_ const PIMAGE_NT_HEADERS inh);

/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		show_usage(argv[1]);
		return -1;
	}

	const wchar_t* file_path = argv[1];


	// open file
	PFILE_CTX context = NULL;
	if (true != OpenFileContext(file_path, context))
	{
		log_err L"OpenFileContext() failed, file_path = %s", file_path log_end
		return -1;
	}
	
	const UCHAR*	file_base = context->FileView;
	UINT32			file_size = context->FileSize;
	
	do 
	{
		// parse pe header
		if (true != parse_pe_header(file_base, file_size))
		{
			log_err L"parse_pe_header() failed, file_base = 0x%p", file_base log_end
			return false;
		}
		
    
	} while (false);


	// close file
	CloseFileContext(context);
	return 0;
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
void show_usage(_In_ wchar_t* me)
{
	log_info L"usage: %s  [file_path]", me log_end
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
bool parse_pe_header(_In_ const UCHAR* file_base, _In_ UINT32 file_size)
{
	_ASSERTE(NULL != file_base);
	if (NULL == file_base) return false;
		
	PIMAGE_DOS_HEADER idh = (PIMAGE_DOS_HEADER)file_base;
	if(idh->e_magic != IMAGE_DOS_SIGNATURE) return false;

    // check e_lfanew offset
    // 
    DWORD_PTR p = (DWORD_PTR)idh + idh->e_lfanew;
    if (TRUE == IsBadReadPtr((void*)p, sizeof(DWORD)/* sizeof (IMAGE_NT_SIGNATURE) */))
    {
        log_err
            L"invalid e_lfanew offset, idh=0x%08x, e_lfanew=0x%08x", 
            idh, idh->e_lfanew 
        log_end        
        return false;    
    }


	PIMAGE_NT_HEADERS inh = (PIMAGE_NT_HEADERS) ((DWORD_PTR)idh + idh->e_lfanew);
    if (IMAGE_NT_SIGNATURE != inh->Signature) 
    {
        log_err
            L"invalid inh->Signature (0x%08x)", inh->Signature
        log_end        
        return false;    
    }

	// dump file_header
	//inh->FileHeader

	// check file is 64bit
	bool is_64 = false;
	if (inh->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		is_64 = true;
	else
		is_64 = false;


	// dump optional header

	// dump section table


	// dump exe debug directory

	// dump resources

	// ............


	// dump import section
	dump_import_section(file_base, inh);

	// dump export section
	dump_export_section(file_base, inh);





	return true;
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
void dump_import_section(_In_ const UCHAR* image_base, _In_ const PIMAGE_NT_HEADERS inh)
{
	UINT32 iat_start_rva = GetImgDirEntryRVA(inh, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (0 == iat_start_rva) return;

	PIMAGE_IMPORT_DESCRIPTOR iid = (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRVA(iat_start_rva, inh, image_base);
	if (NULL == iid) return;

	// iterate all IMAGE_IMPORT_DESCRIPTOR
	for(;;)
	{
		if ( (0 == iid->TimeDateStamp) && (0 == iid->Name) ) break;

		log_info
			L"section name: %s", GetPtrFromRVA(iid->Name, inh, image_base)
		log_end

		// todo
		//...
		//...
		
		
		// dump IAT/INT
		DWORD rvaINT = iid->OriginalFirstThunk;
		DWORD rvaIAT = iid->FirstThunk;

		if (rvaINT == 0)
		{
			rvaINT = rvaIAT;

			if (rvaINT == 0) return;					// oops!
		}

		PIMAGE_THUNK_DATA p_int = (PIMAGE_THUNK_DATA)GetPtrFromRVA(rvaINT, inh, image_base);
		PIMAGE_THUNK_DATA p_iat = (PIMAGE_THUNK_DATA)GetPtrFromRVA(rvaIAT, inh, image_base);
		if (NULL == p_int || NULL == p_iat) return;		// oops!

		dump_import(p_int, p_iat, inh, iid, image_base);

		iid++;	// next!
	}
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
void 
dump_import(
	_In_ PIMAGE_THUNK_DATA p_int, 
	_In_ PIMAGE_THUNK_DATA p_iat, 
	_In_ const PIMAGE_NT_HEADERS inh, 
	_In_ PIMAGE_IMPORT_DESCRIPTOR iid, 
	_In_ const UCHAR* image_base
	)
{
	bool is_64 = (inh->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	for(;;)
	{
		if (0 == p_int->u1.AddressOfData) return;


		ULONGLONG ordinal = -1;
		if (true == is_64)
		{
			if ( IMAGE_SNAP_BY_ORDINAL64(p_int->u1.Ordinal) ) 
				ordinal = IMAGE_ORDINAL64(p_int->u1.Ordinal);
		}
		else
		{
			if ( IMAGE_SNAP_BY_ORDINAL32(p_int->u1.Ordinal) )
				ordinal = IMAGE_ORDINAL32(p_int->u1.Ordinal);
		}



		if (-1 == ordinal)
		{
			log_info L"ordinal = %4u", ordinal log_end
		}
		else
		{
			// import by name
			// 32/64 이지만 RVA 이므로 32 비트로 강제 캐스팅해도 문제 없음
			PIMAGE_IMPORT_BY_NAME iibn = (PIMAGE_IMPORT_BY_NAME) 
										 GetPtrFromRVA(
											(DWORD) p_int->u1.AddressOfData, 
											inh, 
											image_base);

			log_info L"hint = %4u, name = %s", iibn->Hint, iibn->Name log_end
		}
		
		// goto next thunk
		++p_int;
		++p_iat;
	}//for
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
void dump_export_section(_In_ const UCHAR* image_base, _In_ const PIMAGE_NT_HEADERS inh)
{

}

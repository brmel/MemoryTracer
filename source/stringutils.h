//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================
#pragma once

struct tstring_caseless_compare_t
	{
	bool operator() (const Z_STRING& l, const Z_STRING& r) const
		{
#ifdef UNICODE
		return _wcsicmp(l.c_str(), r.c_str()) < 0;
#else
		return _stricmp(l.c_str(), r.c_str()) < 0;
#endif
		}
	};

const Z_STRING format_size(const unsigned long long size);
const Z_STRING format_number(const unsigned long long number);
const Z_STRING format_process_memory_data_type(const int type);
const Z_STRING format_process_memory_block_type(const int type);
const Z_STRING format_process_memory_group_type(const int type);

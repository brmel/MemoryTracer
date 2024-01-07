//= ========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//==========================================================================================================

#include "main.h"
#include "csvfile.h"

//==========================================================================================================
//
Csvfile::Csvfile(const Z_STRING& filename, const Z_STRING separator /*= Z_TEXT(";")*/)
   : m_Fs()
   , m_IsFirst(true)
   , m_Separator(separator)
   , m_EscapeSeq(Z_TEXT("\""))
   , m_SpecialChars(Z_TEXT("\""))
   {
   m_Fs.exceptions(std::ios::failbit | std::ios::badbit);
   m_Fs.open(filename);
   }

//==========================================================================================================
//
Csvfile::~Csvfile()
   {
   Flush();
   m_Fs.close();
   }

//==========================================================================================================
//
Z_STRING Csvfile::Escape(const Z_STRING& val)
   {
   std::wostringstream result;
   result;

   Z_STRING::size_type to, from = 0u, len = val.length();

   while(from < len &&
         Z_STRING::npos != (to = val.find_first_of(m_SpecialChars, from)))
      {
      result << val.substr(from, to - from) << m_EscapeSeq << val[to];
      from = to + 1;
      }

   result << val.substr(from);
   return result.str();
   }


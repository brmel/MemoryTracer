//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================
#pragma once

class Csvfile;

inline static Csvfile& Csv_Endrow(Csvfile& file);
inline static Csvfile& Csv_Flush(Csvfile& file);

//=========================================================================================================
//
class Csvfile
   {
   public:
      Csvfile(const Z_STRING& filename, const Z_STRING separator = Z_TEXT(","));
      ~Csvfile();

      void Flush() { m_Fs.flush(); }
      void Endrow() { m_Fs << std::endl; m_IsFirst = true; }

      Csvfile& operator << (Csvfile& (*val)(Csvfile&))
         {
         return val(*this);
         }

      Csvfile& operator << (const Z_TEXT_CHAR* val)
         {
         return Write(Escape(val));
         }

      Csvfile& operator << (const Z_STRING& val)
         {
         return Write(Escape(val));
         }

      template<typename T>
      Csvfile& operator << (const T& val)
         {
         return Write(val);
         }

   private:
      template<typename T>
      Csvfile& Write(const T& val);

      Z_STRING Escape(const Z_STRING& val);

   private:
      std::wofstream m_Fs;
      bool           m_IsFirst;
      const Z_STRING m_Separator;
      const Z_STRING m_EscapeSeq;
      const Z_STRING m_SpecialChars;
   };

//=========================================================================================================
//
inline static Csvfile& Csv_Endrow(Csvfile& file)
   {
   file.Endrow();
   return file;
   }

//=========================================================================================================
//
inline static Csvfile& Csv_Flush(Csvfile& file)
   {
   file.Flush();
   return file;
   }

//=========================================================================================================
//
template<typename T>
Csvfile& Csvfile::Write(const T& val)
   {
   if(!m_IsFirst)
      {
      m_Fs << m_Separator;
      }
   else
      {
      m_IsFirst = false;
      }
   m_Fs << val;
   return *this;
   }

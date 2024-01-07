//= ========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//==========================================================================================================

#include "main.h"
#include "memoryexporter.h"
#include "processmemory.h"

//==========================================================================================================
//
const std::map<EProcessMemoryGroupType, Z_STRING> EXPORTED_HEADER_MEM_TYPE
   {
      {PMGT_TOTAL        , Z_STRING(Z_TEXT("PMGT_TOTAL      "))},
      {PMGT_IMAGE        , Z_STRING(Z_TEXT("PMGT_IMAGE      "))},
      {PMGT_MAPPED_FILE  , Z_STRING(Z_TEXT("PMGT_MAPPED_FILE"))},
      {PMGT_SHAREABLE    , Z_STRING(Z_TEXT("PMGT_SHAREABLE  "))},
      {PMGT_HEAP         , Z_STRING(Z_TEXT("PMGT_HEAP       "))},
      {PMGT_STACK        , Z_STRING(Z_TEXT("PMGT_STACK      "))},
      {PMGT_PRIVATE      , Z_STRING(Z_TEXT("PMGT_PRIVATE    "))},
      {PMGT_UNUSABLE     , Z_STRING(Z_TEXT("PMGT_UNUSABLE   "))},
      {PMGT_FREE         , Z_STRING(Z_TEXT("PMGT_FREE       "))},
   };

const std::map<EProcessMemoryDataType, Z_STRING> EXPORTED_HEADER_MEM_STATE
   {
      {PMDT_SIZE         , Z_STRING(Z_TEXT("PMDT_SIZE        "))},
      {PMDT_COMMITTED    , Z_STRING(Z_TEXT("PMDT_COMMITTED   "))},
      {PMDT_PRIVATE_BYTE , Z_STRING(Z_TEXT("PMDT_PRIVATE_BYTE"))},
      {PMDT_WS_TOTAL     , Z_STRING(Z_TEXT("PMDT_WS_TOTAL    "))},
      {PMDT_WS_PRIVATE   , Z_STRING(Z_TEXT("PMDT_WS_PRIVATE  "))},
      {PMDT_WS_SHAREABLE , Z_STRING(Z_TEXT("PMDT_WS_SHAREABLE"))},
      {PMDT_WS_SHARED    , Z_STRING(Z_TEXT("PMDT_WS_SHARED   "))},
      {PMDT_BLOCKS       , Z_STRING(Z_TEXT("PMDT_BLOCKS      "))},
      {PMDT_LARGEST      , Z_STRING(Z_TEXT("PMDT_LARGEST     "))},
   };

//==========================================================================================================
//
CMemoryExporter::CMemoryExporter(Z_INT ProcessId)
   : m_ProcessId(ProcessId)
   , m_File_MemType     (std::to_wstring(ProcessId) + Z_TEXT("_MemType.csv"))
   , m_File_MemState    (std::to_wstring(ProcessId) + Z_TEXT("_MemState.csv"))
   , m_File_MemWsPrivate(std::to_wstring(ProcessId) + Z_TEXT("_PrivateByte.csv"))
   {
   ExportHeader();
   }

//==========================================================================================================
//
CMemoryExporter::~CMemoryExporter()
   {}

//==========================================================================================================
//
void CMemoryExporter::Run(const CProcessMemory& ProcessMemory)
   {
   ExportData_MemType(ProcessMemory);
   ExportData_MemState(ProcessMemory);
   ExportData_MemWsPrivate(ProcessMemory);
   }

//==========================================================================================================
//
void CMemoryExporter::ExportData_MemType(const CProcessMemory& ProcessMemory)
   {
   for(EProcessMemoryGroupType group_type = PMGT__FIRST; group_type < PMGT__LAST; group_type = (EProcessMemoryGroupType)((int)group_type + 1))
      {
      EProcessMemoryDataType type = PMDT_SIZE;
      m_File_MemType << ProcessMemory.GetData(group_type, type);
      }
   m_File_MemType << Csv_Endrow;
   }

//==========================================================================================================
//
void CMemoryExporter::ExportData_MemState(const CProcessMemory& ProcessMemory)
   {
   EProcessMemoryGroupType group_type = PMGT_TOTAL;
   for(EProcessMemoryDataType type = PMDT_SIZE; type < PMDT__LAST; type = (EProcessMemoryDataType)((int)type + 1))
      {
      m_File_MemState << ProcessMemory.GetData(group_type, type);
      }
   m_File_MemState << Csv_Endrow;
   }

//==========================================================================================================
//
void CMemoryExporter::ExportData_MemWsPrivate(const CProcessMemory& ProcessMemory)
   {
   for(EProcessMemoryGroupType group_type = PMGT__FIRST; group_type < PMGT__LAST; group_type = (EProcessMemoryGroupType)((int)group_type + 1))
      {
      EProcessMemoryDataType type = PMDT_PRIVATE_BYTE;
      m_File_MemWsPrivate << ProcessMemory.GetData(group_type, type);
      }
   m_File_MemWsPrivate << Csv_Endrow;
   }

//==========================================================================================================
//
void CMemoryExporter::ExportHeader()
   {
   ExportHeader_MemType ();
   ExportHeader_MemState();
   ExportHeader_MemWsPrivate();
   }

//==========================================================================================================
//
void CMemoryExporter::ExportHeader_MemType()
   {
   for(size_t i = 0; i < EXPORTED_HEADER_MEM_TYPE.size(); i++)
      {
      // TBM Why we use at() here
      m_File_MemType << EXPORTED_HEADER_MEM_TYPE.at(static_cast<EProcessMemoryGroupType>(i));
      }
   m_File_MemType << Csv_Endrow;
   }

//==========================================================================================================
//
void CMemoryExporter::ExportHeader_MemState()
   {
   for(size_t i = 0; i < EXPORTED_HEADER_MEM_STATE.size(); i++)
      {
      m_File_MemState << EXPORTED_HEADER_MEM_STATE.at(static_cast<EProcessMemoryDataType>(i+1));
      }
   m_File_MemState << Csv_Endrow;
   }

//==========================================================================================================
//
void CMemoryExporter::ExportHeader_MemWsPrivate()
   {
   for(size_t i = 0; i < EXPORTED_HEADER_MEM_TYPE.size(); i++)
      {
      m_File_MemWsPrivate << EXPORTED_HEADER_MEM_TYPE.at(static_cast<EProcessMemoryGroupType>(i));
      }
   m_File_MemWsPrivate << Csv_Endrow;
   }

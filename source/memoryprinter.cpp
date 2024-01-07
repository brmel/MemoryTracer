//= ========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//==========================================================================================================

#include "main.h"
#include "stringutils.h"
#include "memoryprinter.h"
#include "process.h"
#include "processmemory.h"
#include "processlist.h"

#define SUMMARY_ROW_SIZE (100)
#define SUMMARY_SIZE (2 * SUMMARY_ROW_SIZE)

//==========================================================================================================
//
CMemoryPrinter::CMemoryPrinter(const CProcess& Process, const CProcessMemory& ProcessMemory, Z_INT Level)
   : m_Process(Process)
   , m_ProcessMemory(ProcessMemory)
   , m_Level(Level)
   {
   assert(m_Level >= 0 && m_Level <= 5);
   }

//==========================================================================================================
//
CMemoryPrinter::~CMemoryPrinter()
   {}

//=========================================================================================================
//
void CMemoryPrinter::Run() const
   {
   if(m_Level >= 1)
      {
      PrintSnapshot_Level_1();
      }
   if(m_Level >= 2)
      {
      PrintSnapshot_Level_2();
      }
   if(m_Level >= 3)
      {
      PrintSnapshot_Level_3();
      }

   if(m_Level == 4)
      {
      PrintSnapshot_Level_4_5(false);
      }
   else if(m_Level == 5)
      {
      PrintSnapshot_Level_4_5(true);
      }
   }

//=========================================================================================================
//
void CMemoryPrinter::PrintSnapshot_Level_1() const
   {
   std::tcout << std::endl;
   std::tcout << "Process: " << m_Process.GetImageFileName() << std::endl;
   std::tcout << "PID:     " << m_Process.GetProcessId()     << std::endl;
   }

//=========================================================================================================
//
void CMemoryPrinter::PrintSnapshot_Level_2() const
   {
   unsigned long long vm_total = m_ProcessMemory.GetData(PMGT_TOTAL, PMDT_COMMITTED);
   unsigned long long ws_total = m_ProcessMemory.GetData(PMGT_TOTAL, PMDT_WS_TOTAL);
   unsigned long long vm_current = 0;
   unsigned long long ws_current = 0;
   Z_STRING vm_summary;
   Z_STRING ws_summary;

   if(vm_total > 0)
      {
      for(EProcessMemoryGroupType group_type = PMGT_IMAGE; group_type < PMGT_FREE; group_type = (EProcessMemoryGroupType)((int)group_type + 1))
         {
         vm_current += m_ProcessMemory.GetData(group_type, PMDT_COMMITTED);
         ws_current += m_ProcessMemory.GetData(group_type, PMDT_WS_TOTAL);
         Z_STRING vm_temp((int)(SUMMARY_SIZE * vm_current / vm_total) - vm_summary.size(), format_process_memory_group_type(group_type)[0]);
         Z_STRING ws_temp((int)(SUMMARY_SIZE * ws_current / vm_total) - ws_summary.size(), format_process_memory_group_type(group_type)[0]);
         vm_summary += vm_temp;
         ws_summary += ws_temp;
         }
      }
   vm_summary.resize(SUMMARY_SIZE, '.');
   ws_summary.resize(SUMMARY_SIZE, '.');

   std::tcout << std::endl;
   std::tcout << std::setw(80) << std::left << "Virtual Memory Summary:" << std::setw(20) << std::right << format_size(vm_total) << std::endl;
   for(int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE)
      {
      std::tcout << vm_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
      }

   std::tcout << std::endl;
   std::tcout << std::setw(80) << std::left << "Working Set Summary:" << std::setw(20) << std::right << format_size(ws_total) << std::endl;
   for(int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE)
      {
      std::tcout << ws_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
      }
   }

//=========================================================================================================
//
void CMemoryPrinter::PrintSnapshot_Level_3() const
   {
   std::tcout << std::endl;
   std::tcout << std::left << std::setw(11) << "Type" << "  " << std::right << std::setw(12) << "Size" << "  " << std::setw(12) << "Committed" << "  " << std::setw(12) << "PrivateByte" << "  " << std::setw(12) << "Total WS" << "  " << std::setw(12) << "Private WS" << "  " << std::setw(12) << "Shareable WS" << "  " << std::setw(12) << "Shared WS" << "  " << std::setw(6) << "Blocks" << "  " << std::setw(12) << "Largest" << std::endl;
   std::tcout << std::setw(13 + 14 * 7 + 8 - 2) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;
   
   for(EProcessMemoryGroupType group_type = PMGT__FIRST; group_type < PMGT__LAST; group_type = (EProcessMemoryGroupType)((int)group_type + 1))
      {
      std::tcout << std::setw(11) << std::setfill(L' ') << std::left << format_process_memory_group_type(group_type);
      for(EProcessMemoryDataType type = PMDT_SIZE; type < PMDT__LAST; type = (EProcessMemoryDataType)((int)type + 1))
         {
         std::tcout << "  " << std::setw(type == PMDT_BLOCKS ? 6 : 12) << std::setfill(L' ') << std::right << (type == PMDT_BLOCKS ? format_number(m_ProcessMemory.GetData(group_type, type)) : format_size(m_ProcessMemory.GetData(group_type, type)));
         }
      std::tcout << std::endl;
      }
   }

//=========================================================================================================
//
void CMemoryPrinter::PrintSnapshot_Level_4_5(bool AddLevel5) const
   {
   std::tcout << std::endl;
   std::tcout << std::left << std::setw(16) << "Address" << "    " << std::setw(11) << "Type" << "  " << std::right << std::setw(12) << "Size" << "  " << std::setw(12) << "Committed" << "  " << std::setw(12) << "PrivateByte" << "  " << std::setw(12) << "Total WS" << "  " << std::setw(12) << "Private WS" << "  " << std::setw(12) << "Shareable WS" << "  " << std::setw(12) << "Shared WS" << "  " << std::setw(6) << "Blocks" << "  " << std::left << std::setw(7) << "Access" << "  " << "Details" << std::endl;
   std::tcout << std::setw(20 + 13 + 14 * 6 + 8 + 9 * 2 - 2) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

   for(const auto& it_group : m_ProcessMemory.GetGroups())
      {
      const CProcessMemoryGroup& group = it_group.second;

      std::tcout << std::setw(16) << std::setfill(L'0') << std::right << std::hex << group.base();
      std::tcout << "  ";
      std::tcout << "  " << std::setw(11) << std::setfill(L' ') << std::left << format_process_memory_group_type(group.GetType());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.size());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.committed());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.private_byte());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_private());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_shareable());
      std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_shared());
      std::tcout << "  " << std::setw(6) << std::setfill(L' ') << std::right << format_number(group.blocks());
      std::tcout << "  " << std::setw(7) << std::setfill(L' ') << std::left << group.GetProtectionStr();
      std::tcout << "  " << std::setfill(L' ') << std::left << std::filesystem::path(group.GetDetails()).filename();
      std::tcout << std::endl;

      if(AddLevel5 && group.GetType() != PMGT_FREE && group.GetType() != PMGT_UNUSABLE)
         {
         for(const auto& block : group.GetBlockList())
            {
            std::tcout << "  " << std::setw(16) << std::setfill(L'0') << std::right << std::hex << block.base();
            std::tcout << "  " << std::setw(11) << std::setfill(L' ') << std::left << format_process_memory_block_type(block.GetType());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.size());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.committed());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.privateByte());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_private());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_shareable());
            std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_shared());
            std::tcout << "  " << std::setw(6) << std::setfill(L' ') << std::right << format_number(0);
            std::tcout << "  " << std::setw(7) << std::setfill(L' ') << std::left << block.GetProtectionStr();
            std::tcout << "  " << std::setfill(L' ') << std::left << block.GetDetails();
            std::tcout << std::endl;
            }
         std::tcout << std::endl;
         std::tcout << std::endl;
         }
      }
   }

//=========================================================================================================
//
void CMemoryPrinter::PrintProcessList()
   {
   CProcessList Processes;

   std::tcout << "Accessible processes: " << Processes.Get().size() << std::endl;
   std::tcout << std::endl;
   std::tcout << " " << std::setw(30) << std::left << "Username" << "  " << std::right << std::setw(6) << "PID" << "  " << "Image Name" << std::endl;
   std::tcout << " " << std::setw(78) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

   for(std::list<CProcess>::const_iterator process = Processes.Get().begin(); process != Processes.Get().end(); process++)
      {
      std::tcout << " " << std::setw(30) << std::left << process->GetUser_Name() << "  " << std::right << std::setw(6) << process->GetProcessId() << "  " << process->GetImageFileName() << std::endl;
      }
   }
//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "snapshot.h"
#include "process.h"
#include "memoryprinter.h"

//=========================================================================================================
//
CSnapshot::CSnapshot(Z_UINT32 Pid)
   : m_Pid(Pid)
   {
   assert(CProcess::IsProcessValid(Pid));

   if(!CProcess::IsProcessValid(Pid))
      {
      CMemoryPrinter::PrintProcessList();
      }
   else
      {
      m_Process.emplace(Pid);
      m_ProcessMemory.emplace(*m_Process);
      m_Exporter.emplace(m_Process->GetProcessId());    
      }
   }

//=========================================================================================================
//
CSnapshot::~CSnapshot()
   {}

//=========================================================================================================
//
void CSnapshot::PrintNow(Z_INT Level)
   {
   assert(m_Process.has_value());
   assert(m_ProcessMemory.has_value());

   if(m_Process && m_ProcessMemory)
      {
      bool IsOkay = Update();
      if(IsOkay)
         {
         PrintNow_Imp(Level);
         }
      }
   }

//=========================================================================================================
//
void CSnapshot::ExportNow()
   {
   assert(m_Process.has_value());
   assert(m_ProcessMemory.has_value());
   if(m_Process && m_ProcessMemory)
      {
      bool IsOkay = Update();
      if(IsOkay)
         {
         ExportNow_Imp();
         }
      }
   }

//=========================================================================================================
//
bool CSnapshot::Update()
   {
   bool IsOkay = true;
   m_ProcessMemory->Update();

   return IsOkay;
   }

//=========================================================================================================
//
void CSnapshot::PrintNow_Imp(Z_INT Level) const
   {
   CMemoryPrinter Printer(*m_Process, *m_ProcessMemory, Level);
   Printer.Run();
   }

//=========================================================================================================
//
void CSnapshot::ExportNow_Imp()
   {
   assert(m_Exporter.has_value());
   m_Exporter->Run(*m_ProcessMemory);
   }

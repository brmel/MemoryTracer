//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "../include/snapshotmngr.h"
#include "snapshot.h"
#include "memoryprinter.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::seconds;

//=========================================================================================================
//
CSnapshotMngr::CSnapshotMngr(Z_UINT32 Pid)
   {
   if(CProcess::IsProcessValid(Pid))
      {
      m_pProcessH = new CSnapshot(Pid);
      }
   else
      {
      CMemoryPrinter::PrintProcessList();
      }
   }
 
//=========================================================================================================
//
CSnapshotMngr::~CSnapshotMngr()
   {
   if(m_pProcessH)
      {
      delete m_pProcessH;
      m_pProcessH = nullptr;
      }
   }

//=========================================================================================================
//
void CSnapshotMngr::PrintNow(Z_INT Level/* = 3*/)
   {
   assert(m_pProcessH);

   if(m_pProcessH)
      {
      auto pProcess = static_cast<CSnapshot*>(m_pProcessH);
      pProcess->PrintNow(Level);
      }
   }

//=========================================================================================================
//
void CSnapshotMngr::ExportNow()
   {
   assert(m_pProcessH);

   if(m_pProcessH)
      {
      auto pProcess = static_cast<CSnapshot*>(m_pProcessH);
      pProcess->ExportNow();
      }
   }

//=========================================================================================================
//
void CSnapshotMngr::Export(Z_INT UserAllDuration /*= 60*/, Z_INT FrequencyTSeconds /*= 2*/)
   {
   assert(FrequencyTSeconds > 0);
   assert(UserAllDuration > 0);

   max(FrequencyTSeconds, (Z_INT)1);
   max(UserAllDuration, (Z_INT)1);

   if(m_pProcessH)
      {
      auto pProcess = static_cast<CSnapshot*>(m_pProcessH);
      bool TimeLeft = true;
      auto StartTime = high_resolution_clock::now();

      do
         {
         pProcess->ExportNow();
         std::this_thread::sleep_for(std::chrono::seconds(FrequencyTSeconds));
         auto TimeNow = high_resolution_clock::now();

         auto Seconds_Int = duration_cast<seconds>(TimeNow - StartTime);
         Z_INT CurrentDuration = Seconds_Int.count();
         TimeLeft = CurrentDuration < UserAllDuration;
         } while(TimeLeft);
      }
   }
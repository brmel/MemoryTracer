//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

#include "os.h"

//=========================================================================================================
//
class CSnapshotMngr
	{
	public:
		CSnapshotMngr(Z_UINT32 Pid);
		~CSnapshotMngr();

		void PrintNow(Z_INT Level = 3);

		void ExportNow();

		void Export(Z_INT Duration = 120, Z_INT PeriodTSeconds = 2);

	private:
		void* m_pProcessH {nullptr};
	};
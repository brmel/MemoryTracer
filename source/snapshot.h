//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once
#include "processmemory.h"
#include "memoryexporter.h"

class CSnapshot
	{
	public:
		CSnapshot(Z_UINT32 Pid);
		~CSnapshot();

		void PrintNow(Z_INT Level);
		void ExportNow();

	private:
		bool Update();

		void PrintNow_Imp(Z_INT Level) const;
		void ExportNow_Imp();

	private:
		Z_UINT32                      m_Pid {0};
		std::optional<CProcess>       m_Process;
		std::optional<CProcessMemory> m_ProcessMemory;

		std::optional<CMemoryExporter> m_Exporter;
	};
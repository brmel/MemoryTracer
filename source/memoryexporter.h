//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================
#pragma once

#include "csvfile.h"

class CProcess;
class CProcessMemory;

//=========================================================================================================
//
class CMemoryExporter
	{
	public:
		CMemoryExporter(Z_INT ProcessId);
		~CMemoryExporter();

		void Run(const CProcessMemory& ProcessMemory);

	private:
		void ExportHeader();
		void ExportHeader_MemType ();
		void ExportHeader_MemState();
		void ExportHeader_MemWsPrivate();

		void ExportData_MemType (const CProcessMemory& ProcessMemory);
		void ExportData_MemState(const CProcessMemory& ProcessMemory);
		void ExportData_MemWsPrivate(const CProcessMemory& ProcessMemory);

	private:
		Z_INT m_ProcessId {0};

		Csvfile m_File_MemType;
		Csvfile m_File_MemState;
		Csvfile m_File_MemWsPrivate;
	};

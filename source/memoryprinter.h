//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================
#pragma once

class CProcess;
class CProcessMemory;

//=========================================================================================================
//
class CMemoryPrinter
	{
	public:
		CMemoryPrinter(const CProcess& Process, const CProcessMemory& ProcessMemory, Z_INT Level);
		~CMemoryPrinter();

		void Run() const;

		static void PrintProcessList();

	private:
		void PrintSnapshot_Level_1() const;
		void PrintSnapshot_Level_2() const;
		void PrintSnapshot_Level_3() const;
		void PrintSnapshot_Level_4_5(bool AddLevel5) const;

	private:
		const CProcess&       m_Process;
		const CProcessMemory& m_ProcessMemory;
		Z_INT                 m_Level {0};
	};

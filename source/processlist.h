//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once
#include "process.h"

//=========================================================================================================
//
class CProcessList
	{
	public:
		CProcessList();
		~CProcessList();

		const std::list<CProcess>& Get() const { return m_Processes; }

	private:
		std::list<CProcess> m_Processes;
	};
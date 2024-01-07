//= ========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "processlist.h"

//=========================================================================================================
//
CProcessList::CProcessList()
	{
	// NT API Support:
	// 5.0  EnumProcesses
	std::vector<Z_UINT32> Pids(102400);

	Z_UINT32 PidsNeeded = 0;
	if(!EnumProcesses(&*Pids.begin(), Z_UINT32(Pids.end() - Pids.begin()), &PidsNeeded))
		{
		std::tcerr << "EnumProcesses failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		return;
		}
	Pids.resize(PidsNeeded / sizeof(DWORD));

	for(std::vector<DWORD>::iterator i = Pids.begin(); i < Pids.end(); i++)
		{
		CProcess Process(*i);
		//if ((process.image_filename().size() > 0) && (process.username().size() > 0)) {
		m_Processes.push_back(Process);
		//}
		}

	m_Processes.sort(std::less<DWORD>());
	}

//=========================================================================================================
//
CProcessList::~CProcessList()
	{}
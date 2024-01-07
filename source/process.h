//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================
#pragma once

//=========================================================================================================
//
class CProcess
	{
	public:
		CProcess(const Z_UINT32 Pid);
		~CProcess();

		const Z_UINT32 GetProcessId()     const { return m_ProcessId;	  }
		const FILETIME GetCreationTime()  const { return m_CreationTime; }
		const Z_STRING GetImageFilePath() const { return m_ImageFilePath;}
		const Z_STRING GetImageFileName() const { return m_ImageFileName;}
		const Z_STRING GetUser_Name()     const { return m_UserName;	  }

		operator DWORD() const { return m_ProcessId; }

		static bool IsProcessValid(Z_UINT32 Pid);

	private: 
		Z_UINT32 m_ProcessId {0};	      // To uniquely identify a process over time, both its PID and start time are
		FILETIME m_CreationTime {NULL};  // required, since PIDs get reused. Note that this pair is NOT unique when
		Z_STRING m_ImageFilePath;	      // persisted.
		Z_STRING m_ImageFileName;
		Z_STRING m_UserName;
	};

//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "process.h"

//=========================================================================================================
// Create a process wrapper for a given PID. The process' creation time,
// Image file path and file name and owning username are all collected.
//
CProcess::CProcess(Z_UINT32 Pid):
	m_ProcessId(Pid)
	{
	// NT API Support:
	//   5.0  GetModuleFileNameEx
	//   5.1  GetProcessImageFileName
	//   5.0  GetProcessTimes
	//   5.0  GetTokenInformation
	//   5.0  LookupAccountSid
	//   5.0  OpenProcess
	//   5.0  OpenProcessToken
	//   6.0  QueryFullProcessImageName

	if(IsProcessValid(Pid))
		{
		HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, Pid);

		FILETIME ctime = {0, 0};
		FILETIME etime = {0, 0};
		FILETIME ktime = {0, 0};
		FILETIME utime = {0, 0};
		if(GetProcessTimes(hProcess, &ctime, &etime, &ktime, &utime))
			{
			m_CreationTime = ctime;
			}
		else
			{
			std::tcerr << std::dec << Pid << ": GetProcessTimes failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}

#if _WIN32_WINNT < 0x0600
		Z_STRING Image(MAX_PATH, '\0');
		// This needs PROCESS_VM_READ.
		Z_UINT32 ImageLength = GetModuleFileNameEx(hProcess, NULL, &Image[0], Image.size());
		if(ImageLength > 0)
			{
			Image.resize(ImageLength);
			}
		else
			{
			std::tcerr << std::dec << Pid << ": GetModuleFileNameEx failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
#else

		Z_STRING Image(MAX_PATH, '\0');
		Z_UINT32 ImageLength = (Z_UINT32)Image.size();

		// This needs PROCESS_QUERY_LIMITED_INFORMATION.
		if(QueryFullProcessImageName(hProcess, 0, &Image[0], &ImageLength))
			{
			Image.resize(ImageLength);
			}
		else
			{
			std::tcerr << std::dec << Pid << ": QueryFullProcessImageName failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
#endif

		m_ImageFilePath.assign(Image);
		Z_STRING::size_type last_slash = m_ImageFilePath.rfind('\\');
		if(last_slash != Z_STRING::npos)
			{
			m_ImageFileName = m_ImageFilePath.substr(++last_slash, m_ImageFilePath.size());
			}

		HANDLE hProcessToken;
		if(OpenProcessToken(hProcess, TOKEN_QUERY, &hProcessToken))
			{
			Z_UINT32 DataLength = 0;
			if(!GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &DataLength) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
				{
				std::vector<Z_UINT8> Data(DataLength);
				if(GetTokenInformation(hProcessToken, TokenUser, Data.data(), DataLength, &DataLength))
					{
					TOKEN_USER* user = static_cast<TOKEN_USER*>((void*)Data.data());
					Z_STRING name(MAX_NAME, '\0');
					Z_UINT32 name_length = (Z_UINT32)name.size();
					Z_STRING domain(MAX_NAME, '\0');
					Z_UINT32 DomainLength = (Z_UINT32)domain.size();
					SID_NAME_USE type;
					if(LookupAccountSid(NULL, user->User.Sid, &name[0], &name_length, &domain[0], &DomainLength, &type))
						{
						name.resize(name_length);
						domain.resize(DomainLength);
						m_UserName = _T("");
						if(domain.size())
							{
							m_UserName += domain;
							m_UserName += _T("\\");
							}
						m_UserName += name;
						}
					else
						{
						std::tcerr << std::dec << Pid << ": LookupAccountSid failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
						}
					}
				else
					{
					std::tcerr << std::dec << Pid << ": GetTokenInformation(2) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
					}
				}
			else
				{
				std::tcerr << std::dec << Pid << ": GetTokenInformation failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				}
			CloseHandle(hProcessToken);
			}
		else
			{
			std::tcerr << std::dec << Pid << ": OpenProcessToken failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}

		CloseHandle(hProcess);
		}
		
	}

//=========================================================================================================
//
bool CProcess::IsProcessValid(Z_UINT32 Pid)
	{
#if _WIN32_WINNT < 0x0600
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
#else
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
#endif

	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, Pid);
	bool IsOkay = hProcess != NULL;
	if(hProcess != NULL)
		{
		CloseHandle(hProcess);
		}

	return IsOkay;
	}

//=========================================================================================================
//
CProcess::~CProcess()
	{}

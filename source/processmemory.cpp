//= ========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "utils.h"
#include "processmemory.h"

//=========================================================================================================
//
CProcessMemory::CProcessMemory(const CProcess& process, Z_INT ReservSize /* = 1000*/)
	: m_Process(process)
	{
	m_Modules.reserve(ReservSize);
	m_Heaps.reserve(ReservSize);
	m_Stacks.reserve(ReservSize);
	m_Groups.reserve(ReservSize);
	}

//=========================================================================================================
//
CProcessMemory::~CProcessMemory()
	{}

//=========================================================================================================
//
void CProcessMemory::Update()
	{
	LogicReset();

	// NT API Support:
	//   5.0  CreateToolhelp32Snapshot
	//   5.0  GetMappedFileName
	//   5.0  OpenProcess
	//   5.0  VirtualQueryEx

	EnablePrivilege(SE_DEBUG_NAME);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, m_Process.GetProcessId());
	if(NULL == hProcess)
		{
		std::tcerr << std::dec << m_Process.GetProcessId() << ": OpenProcess failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		DisablePrivilege(SE_DEBUG_NAME);
		return;
		}

	// Get list of images.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 | TH32CS_SNAPTHREAD | TH32CS_SNAPHEAPLIST, m_Process.GetProcessId());
	
	UpdateModules(hSnapshot);
	//UpdateHeap(hSnapshot);
	UpdateStack(hSnapshot);
	
	CloseHandle(hSnapshot);

	std::sort(m_Modules.begin(), m_Modules.end(), std::less<Z_UINT64>());
	std::sort(m_Stacks.begin() , m_Stacks.end(), std::less<Z_UINT64>());
	std::sort(m_Heaps.begin()  , m_Heaps.end(), std::less<Z_UINT64>());

	CollectVirtualMemoryAllocations(hProcess);
	CollectWorkingSetPages(hProcess);

	CloseHandle(hProcess);
	this->DisablePrivilege(SE_DEBUG_NAME);
	}

//=========================================================================================================
//
void CProcessMemory::LogicReset()
	{
	m_Modules.clear();
	m_Heaps.clear();
	m_Stacks.clear();
	m_Groups.clear();
	}

//=========================================================================================================
//
void CProcessMemory::CollectVirtualMemoryAllocations(HANDLE& hProcess)
	{
	BOOL isWow64;
	IsWow64Process(hProcess, &isWow64);

	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);

	// Windows 32bit limit: 0xFFFFFFFF.
	// Windows 64bit limit: 0x7FFFFFFFFFFF.
	Z_UINT64 maxAddress = systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 && !isWow64 ? 0x800000000000 : 0x100000000;

	MEMORY_BASIC_INFORMATION info = {0};

	for(Z_UINT64 address = 0; address < maxAddress; address += info.RegionSize)
		{
		size_t info_size = VirtualQueryEx(hProcess, (void*)address, &info, sizeof(info));

		if(info_size == 0) break;
		if(info_size != sizeof(info))
			{
			std::tcerr << std::dec << m_Process.GetProcessId() << "VirtualQueryEx returned unexpected info_size (" << std::hex << info_size << ", expected " << std::hex << sizeof(info) << ")" << std::endl;
			}
		if(info.BaseAddress != (void*)address)
			{
			std::tcerr << std::dec << m_Process.GetProcessId() << "VirtualQueryEx returned unexpected BaseAddress (" << std::hex << info.BaseAddress << ", expected " << std::hex << (void*)address << ")" << std::endl;
			}

		// Account for unusable regions of memory. Unusable pages are reported as MEM_FREE by VirtualQueryEx.
		if(info.State == MEM_FREE && systemInfo.dwAllocationGranularity > systemInfo.dwPageSize)
			{
			Z_UINT64 unusable_end = ((address + systemInfo.dwAllocationGranularity - 1) / systemInfo.dwAllocationGranularity) * systemInfo.dwAllocationGranularity;
			if(unusable_end > address)
				{
				auto UnusableRegionSize = min(unusable_end - address, info.RegionSize);
				m_Groups.emplace_back(address, CProcessMemoryGroup(PMGT_UNUSABLE, address, UnusableRegionSize));
				address = address + UnusableRegionSize - info.RegionSize;
				continue;
				}
			}

		if((Z_UINT64)info.AllocationBase + info.RegionSize > maxAddress) break;

		Z_UINT64 allocation_base = (Z_UINT64)info.AllocationBase;
		if(info.State == MEM_FREE) allocation_base = (Z_UINT64)info.BaseAddress;

		// Set up the memory group (== allocation) with details and type.
		if(m_Groups.empty() || (m_Groups.back().first != allocation_base))
			{
			assert(m_Groups.empty() || m_Groups.back().first < allocation_base);
			m_Groups.emplace_back(allocation_base, CProcessMemoryGroup());
			}

		assert(!m_Groups.empty());
		auto& memory_group = m_Groups.back();

		if(memory_group.second.GetType() == PMGT__LAST)
			{
			memory_group.second = CProcessMemoryGroup(*this, hProcess, &info);
			}

		// Set up the memory block with current state.
		// TBM : Here we have a 2d table and we need to keep memory. How can we do that ?
		// We need to reserve memory for each block 
		memory_group.second.AddBlock(CProcessMemoryBlock(*this, hProcess, memory_group.second, &info));
		}

#ifndef NDEBUG
	for(size_t i = 0; i < m_Groups.size() - 1; i++)
		{
		// assert that groupes are sorted
		assert(m_Groups[i] < m_Groups[i+1]);
		}
#endif // !NDEBUG
	}

//=========================================================================================================
//
void CProcessMemory::CollectWorkingSetPages(HANDLE& hProcess)
	{
	PERFORMACE_INFORMATION performance_info = {0};
	if(GetPerformanceInfo(&performance_info, sizeof(performance_info)))
		{
		PROCESS_MEMORY_COUNTERS memory_counters = {0};
		if(GetProcessMemoryInfo(hProcess, &memory_counters, sizeof(memory_counters)))
			{
			Z_UINT32 working_set_page_count = (Z_UINT32)memory_counters.WorkingSetSize / (Z_UINT32)performance_info.PageSize;
			Z_UINT32 buffer_length = sizeof(PSAPI_WORKING_SET_INFORMATION) + working_set_page_count * sizeof(PSAPI_WORKING_SET_BLOCK);
			std::vector<byte> buffer(2 * buffer_length);
			if(QueryWorkingSet(hProcess, buffer.data(), (DWORD)buffer.size()))
				{
				PSAPI_WORKING_SET_INFORMATION* ws_info = (PSAPI_WORKING_SET_INFORMATION*)(void*)buffer.data();
				PSAPI_WORKING_SET_BLOCK* ws_block = (PSAPI_WORKING_SET_BLOCK*)ws_info->WorkingSetInfo;
				for(Z_UINT32 page = 0; page < ws_info->NumberOfEntries; page++, ws_block++)
					{
					Z_UINT64 target_address = ws_block->VirtualPage * performance_info.PageSize;

					const auto& up_group = std::upper_bound(m_Groups.begin(), m_Groups.end(), target_address,
																		 [](Z_UINT64 value, const std::pair<Z_UINT64, CProcessMemoryGroup>& groupe)
																		 {
																		 return value < groupe.first;
																		 });

					const auto& it_group = up_group - 1;

					if(it_group != m_Groups.end())
						{
						const CProcessMemoryGroup& group = it_group->second;
						for(const auto& it_block : group.GetBlockList())
							{
							if((it_block.base() <= target_address) && (it_block.base() + it_block.size() > target_address))
								{
								CProcessMemoryBlock& block = const_cast<CProcessMemoryBlock&>(it_block);
								block.Add_WS_Page(ws_block, performance_info.PageSize, group.GetType());
								break;
								}
							}
						}
					}
				}
			else
				{
				std::tcerr << std::dec << m_Process.GetProcessId() << ": QueryWorkingSet failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				}
			}
		else
			{
			std::tcerr << std::dec << m_Process.GetProcessId() << ": GetProcessMemoryInfo failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
		}
	else
		{
		std::tcerr << std::dec << m_Process.GetProcessId() << ": GetProcessMemoryInfo failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
	}

//=========================================================================================================
//
void CProcessMemory::EnablePrivilege(const Z_STRING privilege_name)
	{
	HANDLE hToken;
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
		return;
		}

	LUID luid;
	if(!LookupPrivilegeValue(NULL, privilege_name.c_str(), &luid))
		{
		return;
		}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
		return;
		}
	if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
		return;
		}
	}

//=========================================================================================================
//
void CProcessMemory::DisablePrivilege(const Z_STRING privilege_name)
	{
	HANDLE hToken;
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
		return;
		}

	LUID luid;
	if(!LookupPrivilegeValue(NULL, privilege_name.c_str(), &luid))
		{
		return;
		}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;
	if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
		return;
		}
	if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
		return;
		}
	}

//=========================================================================================================
//
const Z_UINT64 CProcessMemory::GetData(const EProcessMemoryGroupType group_type, const EProcessMemoryDataType type) const
	{
	if((group_type == PMGT_TOTAL) && (type == PMDT_LARGEST)) return 0;
	if(type == PMDT_LARGEST)
		{
		Z_UINT64 largest = 0;
		for(const auto& group : m_Groups)
			{
			if((group.second.GetType() == group_type) || ((group_type == PMGT_TOTAL) && (group.second.GetType() != PMGT_FREE)))
				{
				if(largest < group.second.GetData(type))
					{
					largest = group.second.GetData(type);
					}
				}
			}
		return largest;
		}

	Z_UINT64 total = 0;
	for(const auto& group : m_Groups)
		{
		if((group.second.GetType() == group_type) || ((group_type == PMGT_TOTAL) && (group.second.GetType() != PMGT_FREE)))
			{
			total += group.second.GetData(type);
			}
		}
	return total;
	}

//=========================================================================================================
//
CProcessMemoryGroup::CProcessMemoryGroup(): m_Type(PMGT__LAST), m_Details(_T(""))
	{}

//=========================================================================================================
//
CProcessMemoryGroup::CProcessMemoryGroup(const CProcessMemory& memory, const HANDLE hProcess, PMEMORY_BASIC_INFORMATION info)
	{
	if(info->State == MEM_FREE)
		{
		m_Type = PMGT_FREE;
		}
	else
		{
		Z_STRING filepath;
		if((info->Type == MEM_MAPPED) || (info->Type == MEM_IMAGE))
			{
			filepath.assign(MAX_PATH, '\0');
			Z_UINT32 FileSize = (Z_UINT32)filepath.size();
			Z_UINT32 filepath_length = GetMappedFileName(hProcess, info->BaseAddress, &*filepath.begin(), FileSize);
			filepath.resize(filepath_length);
			if(filepath.size())
				{
				filepath = MapDevicePathToDrivePath(filepath);
				}
			}

		if(info->Type == MEM_IMAGE)
			{
			m_Type = PMGT_IMAGE;
			m_Details = filepath;
			}
		else if(info->Type == MEM_MAPPED)
			{
			if(filepath.size())
				{
				m_Type = PMGT_MAPPED_FILE;
				m_Details = filepath;
				}
			else
				{
				m_Type = PMGT_SHAREABLE;
				}
			}
		else if(info->Type == MEM_PRIVATE)
			{
			m_Type = PMGT_PRIVATE;
			}
		else
			{
			std::tcerr << "VirtualQueryEx returned unexpected info.Type: " << info->Type << std::endl;
			}

		if((m_Type == PMGT_SHAREABLE) || (m_Type == PMGT_PRIVATE))
			{
			// Shareable and private memory may be a process heap.
			for(const auto& heap : memory.GetHeaps())
				{
				if(((Z_UINT64)info->BaseAddress <= heap.GetBase()) && ((Z_UINT64)info->BaseAddress + (Z_UINT64)info->RegionSize >= heap.GetBase()))
					{
					Z_STRING heap_details(_T("Heap ID:           "));
					_itow_s(heap.GetId(), &heap_details[9], 10, 10);
					heap_details.resize(heap_details.find(_T('\0')));
					if(heap.GetDefault())
						{
						heap_details += _T(" (Default)");
						}
					m_Type = PMGT_HEAP;
					m_Details = heap_details;
					break;
					}
				}
			}
		}
	}

//=========================================================================================================
//
CProcessMemoryGroup::CProcessMemoryGroup(const EProcessMemoryGroupType type, const Z_UINT64 base, const Z_UINT64 size)
	{
	m_Type = type;
	m_Blocks.emplace_back(PMBT_FREE, base, size);
	}

//=========================================================================================================
//
CProcessMemoryGroup::~CProcessMemoryGroup()
	{}

//=========================================================================================================
//
const Z_UINT64 CProcessMemoryGroup::GetData(const EProcessMemoryDataType type) const
	{
	if((type == PMDT_BASE) && m_Blocks.size()) return m_Blocks.begin()->base();
	if(type == PMDT_BASE) return 0;
	if((type == PMDT_BLOCKS) && (this->GetType() == PMGT_UNUSABLE)) return 0;
	if(type == PMDT_BLOCKS) return m_Blocks.size();
	if(type == PMDT_LARGEST) return GetData(PMDT_SIZE);

	Z_UINT64 total = 0;
	for(const auto& block : m_Blocks)
		{
		total += block.GetData(type);
		}
	return total;
	}

//=========================================================================================================
//
const ProcessMemoryProtection CProcessMemoryGroup::GetProtection() const
	{
	ProcessMemoryProtection rv = PMP_NONE;
	for(const auto& block : m_Blocks)
		{
		if(block.GetType() == PMBT_COMMITTED)
			{
			rv |= block.GetProtection();
			}
		}
	return rv;
	}

//=========================================================================================================
//
const Z_STRING CProcessMemoryGroup::GetProtectionStr() const
	{
	ProcessMemoryProtection p = GetProtection();
	Z_STRING rv;
	if(p & PMP_READ)          rv += _T("r"); else rv += _T("-");
	if(p & PMP_WRITE)         rv += _T("w"); else rv += _T("-");
	if(p & PMP_EXECUTE)       rv += _T("x"); else rv += _T("-");
	if(p & PMP_COPY)          rv += _T("c"); else rv += _T("-");
	if(p & PMP_GUARD)         rv += _T("G"); else rv += _T("-");
	if(p & PMP_NO_CACHE)      rv += _T("C"); else rv += _T("-");
	if(p & PMP_WRITE_COMBINE) rv += _T("W"); else rv += _T("-");
	return rv;
	}

//=========================================================================================================
//
const void CProcessMemoryGroup::AddBlock(const CProcessMemoryBlock & block)
	{
	m_Blocks.push_back(block);
	}

//=========================================================================================================
//
CProcessMemoryBlock::CProcessMemoryBlock(const CProcessMemory & memory, const HANDLE hProcess, CProcessMemoryGroup & group, const PMEMORY_BASIC_INFORMATION info): m_Protection(PMP_NONE)
	{
	UNREFERENCED_PARAMETER(hProcess);

	for(int i = PMDT__FIRST; i < PMDT__LAST; i++) m_Data[i] = 0;
	m_Data[PMDT_BASE] = (Z_UINT64)info->BaseAddress;
	m_Data[PMDT_SIZE] = (Z_UINT64)info->RegionSize;
	m_Data[PMDT_LARGEST] = (Z_UINT64)info->RegionSize;
	m_Type = info->State == MEM_COMMIT ? PMBT_COMMITTED : info->State == MEM_RESERVE ? PMBT_RESERVED : PMBT_FREE;

	auto GroupeType = group.GetType();
	if(m_Type == PMBT_COMMITTED)
		{
		if(GroupeType == PMGT_HEAP || GroupeType == PMGT_STACK || GroupeType == PMGT_PRIVATE)
			{
			m_Data[PMDT_PRIVATE_BYTE] = (Z_UINT64)info->RegionSize;
			}
		}

	switch(info->Protect & 0x00FF)
		{
		case PAGE_NOACCESS:
			m_Protection = PMP_NONE;
			break;
		case PAGE_READONLY:
			m_Protection = PMP_READ;
			break;
		case PAGE_READWRITE:
			m_Protection = PMP_READ | PMP_WRITE;
			break;
		case PAGE_WRITECOPY:
			m_Protection = PMP_READ | PMP_WRITE | PMP_COPY;
			break;
		case PAGE_EXECUTE:
			m_Protection = PMP_EXECUTE;
			break;
		case PAGE_EXECUTE_READ:
			m_Protection = PMP_EXECUTE | PMP_READ;
			break;
		case PAGE_EXECUTE_READWRITE:
			m_Protection = PMP_EXECUTE | PMP_READ | PMP_WRITE;
			break;
		case PAGE_EXECUTE_WRITECOPY:
			m_Protection = PMP_EXECUTE | PMP_READ | PMP_COPY;
			break;
		}

	if(info->Protect & PAGE_GUARD)        m_Protection |= PMP_GUARD;
	if(info->Protect & PAGE_NOCACHE)      m_Protection |= PMP_NO_CACHE;
	if(info->Protect & PAGE_WRITECOMBINE) m_Protection |= PMP_WRITE_COMBINE;

	if(m_Type == PMBT_COMMITTED)
		{
		if(GroupeType == PMGT_IMAGE && ((m_Protection & PMP_WRITE) || (m_Protection & PMP_COPY)))
			{
			m_Data[PMDT_PRIVATE_BYTE] = (Z_UINT64)info->RegionSize;
			m_ImageAddedAsPrivate = true;
			}
		}

	if((m_Type != PMBT_FREE) && (GroupeType == PMGT_PRIVATE))
		{
		// Private memory may be a stack.
		for(const auto& stack : memory.GetStacks())
			{
			if(((Z_UINT64)info->BaseAddress <= stack.GetBase()) && ((Z_UINT64)info->BaseAddress + (Z_UINT64)info->RegionSize >= stack.GetBase()))
				{
				Z_STRING stack_details(_T("Thread ID:           "));
				_itow_s(stack.GetId(), &stack_details[11], 10, 10);
				stack_details.resize(stack_details.find(_T('\0')));
#ifdef _WIN64
				if(stack.GetType() == PST_32BIT)
					{
					stack_details += _T(" (Wow64)");
					}
#endif
				group.SetType(PMGT_STACK);
				group.SetDetails(stack_details);
				break;
				}
			}
		}
	}

//=========================================================================================================
//
CProcessMemoryBlock::CProcessMemoryBlock(const EProcessMemoryBlockType type, const Z_UINT64 base, const Z_UINT64 size)
	{
	m_Type = type;
	for(int i = PMDT__FIRST; i < PMDT__LAST; i++) m_Data[i] = 0;
	m_Data[PMDT_BASE] = base;
	m_Data[PMDT_SIZE] = size;
	m_Data[PMDT_LARGEST] = size;
	}

//=========================================================================================================
//
CProcessMemoryBlock::~CProcessMemoryBlock()
	{}

//=========================================================================================================
//
const Z_UINT64 CProcessMemoryBlock::GetData(const EProcessMemoryDataType type) const
	{
	if(type == PMDT_COMMITTED) return (m_Type == PMBT_COMMITTED ? m_Data[PMDT_SIZE] : 0);
	if(type == PMDT_WS_TOTAL)  return m_Data[PMDT_WS_PRIVATE] + m_Data[PMDT_WS_SHAREABLE];
	return m_Data[type];
	}

//=========================================================================================================
//
const Z_STRING CProcessMemoryBlock::GetProtectionStr() const
	{
	if(m_Type == PMBT_COMMITTED)
		{
		ProcessMemoryProtection p = GetProtection();
		Z_STRING rv;
		if(p & PMP_READ)          rv += _T("r"); else rv += _T("-");
		if(p & PMP_WRITE)         rv += _T("w"); else rv += _T("-");
		if(p & PMP_EXECUTE)       rv += _T("x"); else rv += _T("-");
		if(p & PMP_COPY)          rv += _T("c"); else rv += _T("-");
		if(p & PMP_GUARD)         rv += _T("G"); else rv += _T("-");
		if(p & PMP_NO_CACHE)      rv += _T("C"); else rv += _T("-");
		if(p & PMP_WRITE_COMBINE) rv += _T("W"); else rv += _T("-");
		return rv;
		}
	return _T("");
	}

//=========================================================================================================
//
const void CProcessMemoryBlock::Add_WS_Page(
	const PSAPI_WORKING_SET_BLOCK * ws_block,
	const SIZE_T page_size, 
	EProcessMemoryGroupType GroupeType)
	{
	if(ws_block->Shared)
		{
		m_Data[PMDT_WS_SHAREABLE] += page_size;
		if(ws_block->ShareCount > 1)
			{
			m_Data[PMDT_WS_SHARED] += page_size;
			}

		// heap that is shared
		if(GroupeType == PMGT_HEAP)
			{
			m_Data[PMDT_PRIVATE_BYTE] -= page_size;
			}
		}
	else
		{
		m_Data[PMDT_WS_PRIVATE] += page_size;
		if(GroupeType == PMGT_IMAGE && (!m_ImageAddedAsPrivate))
			{
			// Copy on write pages
			m_Data[PMDT_PRIVATE_BYTE] += page_size;
			}
		}
	}

//=========================================================================================================
//
void CProcessMemory::UpdateModules(HANDLE& hSnapshot)
	{
	MODULEENTRY32 module = {sizeof(module)};
	if(Module32First(hSnapshot, &module))
		{
		do
			{
			m_Modules.emplace_back((Z_UINT64)module.modBaseAddr, (Z_UINT64)module.modBaseSize, module.szExePath);
			} while(Module32Next(hSnapshot, &module));
		}
	else
		{
		std::tcerr << std::dec << m_Process.GetProcessId() << ": Module32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
	}

//=========================================================================================================
//
void CProcessMemory::UpdateStack(HANDLE& hSnapshot)
	{
	if(INVALID_HANDLE_VALUE != hSnapshot)
		{
		THREADENTRY32 thread = {sizeof(thread)};
		if(Thread32First(hSnapshot, &thread))
			{
			do
				{
				if(thread.th32OwnerProcessID == m_Process.GetProcessId())
					{
					HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, false, thread.th32ThreadID);
					if(NULL != hThread)
						{
						if(GetCurrentThreadId() != thread.th32ThreadID)
							{
							SuspendThread(hThread);
							}
#ifdef _WIN64
						CONTEXT context_64bit = {}; context_64bit.ContextFlags = CONTEXT_CONTROL;
						if(GetThreadContext(hThread, &context_64bit))
							{
							m_Stacks.push_back(CProcessStack(thread.th32ThreadID, context_64bit.Rsp, PST_64BIT));
							}
						else if(GetLastError() != ERROR_INVALID_PARAMETER)
							{
							std::tcerr << std::dec << m_Process.GetProcessId() << ": GetThreadContext(64bit) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
							}
						WOW64_CONTEXT context_32bit = {WOW64_CONTEXT_ALL};
						if(Wow64GetThreadContext(hThread, &context_32bit))
							{
#elif _WIN32
							CONTEXT context_32bit = {CONTEXT_CONTROL};
							if(GetThreadContext(hThread, &context_32bit))
							{
#endif
							m_Stacks.push_back(CProcessStack(thread.th32ThreadID, context_32bit.Esp, PST_32BIT));
							}
						else if(GetLastError() != ERROR_INVALID_PARAMETER)
							{
							std::tcerr << std::dec << m_Process.GetProcessId() << ": GetThreadContext(32bit) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
							}
						if(GetCurrentThreadId() != thread.th32ThreadID)
							{
							ResumeThread(hThread);
							}
						}
					else
						{
						std::tcerr << std::dec << m_Process.GetProcessId() << ": OpenThread failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
						}
					}
				} while(Thread32Next(hSnapshot, &thread));
			}
		else
			{
			std::tcerr << std::dec << m_Process.GetProcessId() << ": Thread32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
		}
	}

//=========================================================================================================
//
void CProcessMemory::UpdateHeap(HANDLE& hSnapshot)
	{
	HEAPLIST32 heap = {sizeof(heap)};
	Z_UINT32 heap_index = 0;
	if(Heap32ListFirst(hSnapshot, &heap))
		{
		do
			{
			m_Heaps.push_back(CProcessHeap(heap_index++, (Z_UINT64)heap.th32HeapID, heap.dwFlags & HF32_DEFAULT));

			// TODO: http://www.securityxploded.com/enumheaps.php
			//HEAPENTRY32 heap_block = { sizeof(HEAPENTRY32) };
			//if (Heap32First(&heap_block, heap.th32ProcessID, heap.th32HeapID)) {
			//	do {
			//		std::tcout << "   BLOCK(" << std::hex << std::setfill(_T('0')) << std::setw(16) << heap_block.dwAddress << "-" << std::setw(16) << (heap_block.dwAddress + heap_block.dwBlockSize) << ") " << std::setfill(_T(' ')) << (heap_block.dwFlags & LF32_FIXED ? "FIXED" : "     ") << " " << (heap_block.dwFlags & LF32_FREE ? "FREE" : "    ") << " " << (heap_block.dwFlags & LF32_MOVEABLE ? "MOVEABLE" : "        ") << " lock=" << heap_block.dwLockCount << ", resvd=" << heap_block.dwResvd << ", handle=" << heap_block.hHandle; // << std::endl;
			//		if (1) {
			//			byte* buffer_base = (byte*)heap_block.dwAddress;
			//			SIZE_T buffer_length = heap_block.dwBlockSize < 0x1000 ? heap_block.dwBlockSize : 0x1000;
			//			byte* buffer = new byte[buffer_length];
			//			ReadProcessMemory(hProcess, buffer_base, buffer, buffer_length, &buffer_length);
			//			if (buffer_length > 0) {
			//				std::tcout << std::hex << std::setfill(_T('0'));
			//				for (int i = 0; i < buffer_length / sizeof(buffer[0]); i++) {
			//					if (i % 60 == 0) std::tcout << std::endl << "     " << (buffer_base + i) << ":";
			//					if (i % 4 == 0) std::tcout << " ";
			//					std::tcout << std::setw(2 * sizeof(buffer[0])) << buffer[i];
			//				}
			//				std::tcout << std::setfill(_T(' ')) << std::endl;
			//			} else {
			//				std::tcerr << std::dec << m_Process.GetProcessId() << ": ReadProcessMemory failed: " << buffer_length << " bytes, " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			//			}
			//		}
			//	} while (Heap32Next(&heap_block));
			//} else {
			//	std::tcerr << std::dec << m_Process.GetProcessId() << ": Heap32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			//}

			} while(Heap32ListNext(hSnapshot, &heap));
		}
	else
		{
		std::tcerr << std::dec << m_Process.GetProcessId() << ": Heap32ListFirst failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
	}

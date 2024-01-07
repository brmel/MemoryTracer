//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

#include "process.h"
#include "processmodule.h"
#include "processstack.h"
#include "processheap.h"

//=========================================================================================================
//
typedef unsigned int ProcessMemoryProtection;
#define PMP_NONE          0x0000
#define PMP_READ          0x0001
#define PMP_WRITE         0x0002
#define PMP_COPY          0x0004
#define PMP_EXECUTE       0x0008
#define PMP_ACCESS_MASK   0x000F
#define PMP_GUARD         0x0100
#define PMP_NO_CACHE      0x0200
#define PMP_WRITE_COMBINE 0x0400
#define PMP_FLAG_MASK     0x0700

//=========================================================================================================
//
enum EProcessMemoryDataType
	{
	PMDT__FIRST = 0,
	PMDT_BASE = PMDT__FIRST,
	PMDT_SIZE,
	PMDT_COMMITTED,
	PMDT_PRIVATE_BYTE,
	PMDT_WS_TOTAL,
	PMDT_WS_PRIVATE,
	PMDT_WS_SHAREABLE,
	PMDT_WS_SHARED,
	PMDT_BLOCKS,
	PMDT_LARGEST,
	PMDT__LAST
	};

//=========================================================================================================
//
enum EProcessMemoryBlockType
	{
	PMBT__FIRST = 0,
	PMBT_FREE = PMBT__FIRST,
	PMBT_RESERVED,
	PMBT_COMMITTED,
	PMBT__LAST
	};

//=========================================================================================================
//
enum EProcessMemoryGroupType
	{
	PMGT__FIRST = 0,
	PMGT_TOTAL = PMGT__FIRST,
	PMGT_IMAGE,
	PMGT_MAPPED_FILE,
	PMGT_SHAREABLE,
	PMGT_HEAP,
	PMGT_STACK,
	PMGT_PRIVATE,
	PMGT_UNUSABLE,
	PMGT_FREE,
	PMGT__LAST
	};

class CProcessMemory;
class CProcessMemoryGroup;

//=========================================================================================================
//
class CProcessMemoryBlock
	{
	public:
		CProcessMemoryBlock(const CProcessMemory& memory, const HANDLE hProcess, CProcessMemoryGroup& group, const PMEMORY_BASIC_INFORMATION info);
		CProcessMemoryBlock(const EProcessMemoryBlockType type, const Z_UINT64 base, const Z_UINT64 size);
		~CProcessMemoryBlock();

		const Z_UINT64 GetData(const EProcessMemoryDataType type) const;

#define DATA_ACCESS_PMB(name, key) const Z_UINT64 name() const { return m_Data[PMDT_ ## key]; }
		DATA_ACCESS_PMB(base        , BASE);
		DATA_ACCESS_PMB(size        , SIZE);
		DATA_ACCESS_PMB(committed   , COMMITTED);
		DATA_ACCESS_PMB(privateByte , PRIVATE_BYTE);
		DATA_ACCESS_PMB(ws		    , WS_TOTAL);
		DATA_ACCESS_PMB(ws_private  , WS_PRIVATE);
		DATA_ACCESS_PMB(ws_shareable, WS_SHAREABLE);
		DATA_ACCESS_PMB(ws_shared   , WS_SHARED);
		DATA_ACCESS_PMB(blocks      , BLOCKS);
		DATA_ACCESS_PMB(largest     , LARGEST);

		const EProcessMemoryBlockType GetType()			 const { return m_Type; }
		const ProcessMemoryProtection GetProtection()	 const { return m_Protection; }
		const Z_STRING                GetProtectionStr() const;
		const Z_STRING                GetDetails()       const { return m_Details; }

		const void SetDetails(const Z_STRING details) { m_Details = details; }

		const void Add_WS_Page(const PSAPI_WORKING_SET_BLOCK* ws_block, const SIZE_T page_size, EProcessMemoryGroupType GroupeType);
		operator Z_UINT64() const { return m_Data[PMDT_BASE]; }

	private:
		Z_UINT64                m_Data[/* EProcessMemoryDataType */PMDT__LAST];
		EProcessMemoryBlockType m_Type;
		ProcessMemoryProtection m_Protection {PMP_NONE};
		Z_STRING                m_Details;
		bool                    m_ImageAddedAsPrivate {false};
	};

//=========================================================================================================
//
class CProcessMemoryGroup
	{
	public:
		CProcessMemoryGroup();
		CProcessMemoryGroup(const CProcessMemory& memory, const HANDLE hProcess, const PMEMORY_BASIC_INFORMATION info);
		CProcessMemoryGroup(const EProcessMemoryGroupType type, const Z_UINT64 base, const Z_UINT64 size);
		~CProcessMemoryGroup();

		const Z_UINT64 GetData(const EProcessMemoryDataType type) const;

#define DATA_ACCESS_PMG(name, key) const Z_UINT64 name() const { return GetData(PMDT_ ## key); }
		DATA_ACCESS_PMG(base        , BASE);
		DATA_ACCESS_PMG(size        , SIZE);
		DATA_ACCESS_PMG(committed   , COMMITTED);
		DATA_ACCESS_PMG(private_byte, PRIVATE_BYTE);
		DATA_ACCESS_PMG(ws			 , WS_TOTAL);
		DATA_ACCESS_PMG(ws_private  , WS_PRIVATE);
		DATA_ACCESS_PMG(ws_shareable, WS_SHAREABLE);
		DATA_ACCESS_PMG(ws_shared   , WS_SHARED);
		DATA_ACCESS_PMG(blocks      , BLOCKS);
		DATA_ACCESS_PMG(largest     , LARGEST);

		const EProcessMemoryGroupType GetType()           const { return m_Type; }
		const ProcessMemoryProtection GetProtection()     const;
		const Z_STRING                GetProtectionStr() const;
		const Z_STRING                GetDetails()        const { return m_Details; }
							               
		const void SetType(const EProcessMemoryGroupType type) { m_Type = type; }
		const void SetDetails(const Z_STRING details) { m_Details = details; }

		const std::vector<CProcessMemoryBlock>& GetBlockList() const { return m_Blocks; }
		const void AddBlock(const CProcessMemoryBlock& block);
		operator Z_UINT64() const { return GetData(PMDT_BASE); }

	private:
		EProcessMemoryGroupType          m_Type {PMGT__FIRST};
		Z_STRING                         m_Details;
		std::vector<CProcessMemoryBlock> m_Blocks;
	};

//=========================================================================================================
//
class CProcessMemory
	{
	public:
		CProcessMemory(const CProcess& process, Z_INT ReservSize = 1000);
		~CProcessMemory();

		void Update();

		const DWORD GetProcessId() const { return m_Process.GetProcessId(); }

		const std::vector<CProcessModule>& GetModules() const { return m_Modules; }
		const std::vector<CProcessHeap>&   GetHeaps()   const { return m_Heaps; }
		const std::vector<CProcessStack>&  GetStacks()  const { return m_Stacks; }

		const std::vector<std::pair<Z_UINT64, CProcessMemoryGroup>>& GetGroups() const { return m_Groups; }
		const Z_UINT64 GetData(const EProcessMemoryGroupType group_type, const EProcessMemoryDataType type) const;

	private:
		void EnablePrivilege(const Z_STRING privilege_name);
		void DisablePrivilege(const Z_STRING privilege_name);

		void CollectVirtualMemoryAllocations(HANDLE& hProcess);
		void CollectWorkingSetPages(HANDLE& hProcess);

		void UpdateModules(HANDLE& hSnapshot);
		void UpdateStack(HANDLE& hSnapshot);
		void UpdateHeap(HANDLE& hSnapshot);

		void LogicReset();

	private:
		const CProcess&             m_Process;
		std::vector<CProcessModule> m_Modules;
		std::vector<CProcessHeap>   m_Heaps;
		std::vector<CProcessStack>  m_Stacks;
		std::vector<std::pair<Z_UINT64, CProcessMemoryGroup>> m_Groups;
	};

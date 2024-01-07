//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

enum EProcessStackType
	{
	PST__FIRST = 0,
	PST_32BIT = PST__FIRST,
	PST_64BIT,
	PST__LAST
	};

//=========================================================================================================
//
class CProcessStack
	{
	public:
		CProcessStack(const Z_UINT32 id, const Z_UINT64 base, const EProcessStackType type);
		~CProcessStack();

		const Z_UINT32 GetId()            const { return  m_Id;	 }
		const Z_UINT64 GetBase()          const { return  m_Base; }
		const EProcessStackType GetType() const { return  m_Type; }

		operator Z_UINT64() const { return m_Base; }

	private:
		Z_UINT32          m_Id;
		Z_UINT64          m_Base;
		EProcessStackType m_Type;
	};
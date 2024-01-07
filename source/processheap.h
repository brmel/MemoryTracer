//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

//=========================================================================================================
//
class CProcessHeap
	{
	public:
		CProcessHeap(const Z_UINT32 id, const Z_UINT64 base, const bool Default);
		~CProcessHeap();

		const Z_UINT32 GetId()      const { return m_Id; }
		const Z_UINT64 GetBase()    const { return m_Base; }
		const Z_UINT64 GetDefault() const { return m_Default; }

		operator Z_UINT64() const { return m_Base; }

	private:
		Z_UINT32 m_Id;
		Z_UINT64 m_Base;
		bool     m_Default;
	};


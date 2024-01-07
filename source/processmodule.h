//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

//=========================================================================================================
//
class CProcessModule
	{
	public:
		CProcessModule(const Z_UINT64 Base, const Z_UINT64 Size, const Z_STRING& ImageFilePath);
		~CProcessModule();

		const Z_UINT64 GetBase() const { return m_Base; }
		const Z_UINT64 GetSize() const { return m_Size; }
		const Z_STRING GetImageFilePath() const { return m_ImageFilePath; }

		operator Z_UINT64() const { return m_Base; }

	private:
		Z_UINT64 m_Base;
		Z_UINT64 m_Size;
		Z_STRING m_ImageFilePath;
	};

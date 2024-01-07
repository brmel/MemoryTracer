//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "processmodule.h"

//=========================================================================================================
//
CProcessModule::CProcessModule(const Z_UINT64 Base, const Z_UINT64 Size, const Z_STRING& ImageFilePath)
	: m_Base(Base)
	, m_Size(Size)
	, m_ImageFilePath(ImageFilePath)
	{}

//=========================================================================================================
//
CProcessModule::~CProcessModule()
	{}

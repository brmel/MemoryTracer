//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#include "main.h"
#include "processheap.h"

//=========================================================================================================
//
CProcessHeap::CProcessHeap(Z_UINT32 Id, Z_UINT64 Base, bool Default)
	: m_Id(Id)
	, m_Base(Base)
	, m_Default(Default)
	{}

//=========================================================================================================
//
CProcessHeap::~CProcessHeap()
	{}

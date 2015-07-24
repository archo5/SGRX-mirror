

#include "scritem.hpp"



SGRX_ScriptedItem* SGRX_ScriptedItem::Create( SGS_CTX, sgsVariable func )
{
	SGRX_ScriptedItem* SI = SGS_PUSHCLASS( C, SGRX_ScriptedItem, () );
	sgs_ObjAcquire( C, SI->m_sgsObject );
	sgsVariable obj( C, sgsVariable::PickAndPop );
	
	// call the initialization function
	obj.thiscall( func );
	
	return SI;
}

void SGRX_ScriptedItem::Acquire()
{
	sgs_ObjAcquire( C, m_sgsObject );
}

void SGRX_ScriptedItem::Release()
{
	sgs_ObjRelease( C, m_sgsObject );
}




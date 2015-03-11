

#pragma once
#define __STDC_FORMAT_MACROS 1
#define HEADER_SGSCRIPT_H "sgscript.h"
#include "../ext/src/sgscript/sgscript.h"
#include "../ext/src/sgscript/sgs_cppbc.h"

#include "utils.hpp"


struct ScriptVarIterator
{
	ScriptVarIterator( SGS_CTX, sgs_Variable* var );
	ScriptVarIterator( sgsVariable& var );
	void _Init( SGS_CTX, sgs_Variable* var );
	
	sgsVariable GetKey();
	sgsVariable GetValue();
	bool Advance();
	
	sgsVariable m_iter;
};

struct ScriptContext
{
	ScriptContext();
	~ScriptContext();
	
	bool ExecFile( const StringView& path );
	bool ExecBuffer( const StringView& data );
	
	// creates a new dict, sets metaobject of current to new, sets new as env
	void PushEnv();
	// tries to reverse the operation done by the previous function, returns false if no metaobj
	bool PopEnv();
	
	ScriptVarIterator GlobalIterator();
	
	SGS_CTX;
};


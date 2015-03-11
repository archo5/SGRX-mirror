

#pragma once
#define __STDC_FORMAT_MACROS 1
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
	
	ScriptVarIterator GlobalIterator();
	
	SGS_CTX;
};


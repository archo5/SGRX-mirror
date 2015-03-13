

#include "script.hpp"


ScriptVarIterator::ScriptVarIterator( SGS_CTX, sgs_Variable* var )
{
	_Init( C, var );
}

ScriptVarIterator::ScriptVarIterator( sgsVariable& var )
{
	_Init( var.C, &var.var );
}

void ScriptVarIterator::_Init( SGS_CTX, sgs_Variable* var )
{
	if( !C )
		return;
	sgs_Variable it;
	if( SGS_SUCCEEDED( sgs_GetIteratorP( C, var, &it ) ) )
	{
		m_iter = sgsVariable( C, &it );
	}
}

sgsVariable ScriptVarIterator::GetKey()
{
	sgsVariable out( m_iter.C );
	if( m_iter.C )
		sgs_IterGetDataP( m_iter.C, &m_iter.var, &out.var, NULL );
	return out;
}

sgsVariable ScriptVarIterator::GetValue()
{
	sgsVariable out( m_iter.C );
	if( m_iter.C )
		sgs_IterGetDataP( m_iter.C, &m_iter.var, NULL, &out.var );
	return out;
}

bool ScriptVarIterator::Advance()
{
	return m_iter.C && sgs_IterAdvanceP( m_iter.C, &m_iter.var ) > 0;
}


ScriptContext::ScriptContext()
{
	C = sgs_CreateEngine();
}

ScriptContext::~ScriptContext()
{
	sgs_DestroyEngine( C );
}

bool ScriptContext::ExecFile( const StringView& path )
{
	return SGS_SUCCEEDED( sgs_ExecFile( C, StackPath( path ) ) );
}

bool ScriptContext::ExecBuffer( const StringView& data )
{
	return SGS_SUCCEEDED( sgs_ExecBuffer( C, data.data(), data.size() ) );
}

String ScriptContext::Serialize( sgsVariable var )
{
	String out;
	sgs_PushVar( C, var );
	if( SGS_SUCCEEDED( sgs_Serialize( C ) ) )
	{
		out = sgs_GetVar<String>()( C, -1 );
	}
	sgs_Pop( C, 1 );
	return out;
}

sgsVariable ScriptContext::Unserialize( const StringView& sv )
{
	sgsVariable out;
	sgs_PushStringBuf( C, sv.data(), sv.size() );
	if( SGS_SUCCEEDED( sgs_Serialize( C ) ) )
	{
		out = sgs_GetVar<sgsVariable>()( C, -1 );
	}
	sgs_Pop( C, 1 );
	return out;
}

sgsVariable ScriptContext::CreateDict( int args )
{
	sgsVariable out( C );
	sgs_InitDict( C, &out.var, args );
	return out;
}

bool ScriptContext::GlobalCall( const char* name, int args, int ret )
{
	return SGS_SUCCEEDED( sgs_GlobalCall( C, name, args, ret ) );
}

void ScriptContext::PushEnv()
{
	sgs_Variable cur_env, new_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_InitDict( C, &new_env, 0 );
	sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &new_env ), sgs_GetObjectStructP( &cur_env ) );
	sgs_SetEnv( C, &new_env );
	sgs_Release( C, &new_env );
	sgs_Release( C, &cur_env );
}

bool ScriptContext::PopEnv()
{
	sgs_Variable cur_env;
	
	sgs_GetEnv( C, &cur_env );
	sgs_VarObj* upper_env = sgs_ObjGetMetaObj( sgs_GetObjectStructP( &cur_env ) );
	if( upper_env )
		sgs_ObjSetMetaObj( C, sgs_GetObjectStructP( &cur_env ), NULL );
	sgs_Release( C, &cur_env );
	if( upper_env )
	{
		sgs_Variable new_env;
		sgs_InitObjectPtr( &new_env, upper_env );
		sgs_SetEnv( C, &new_env );
		sgs_Release( C, &new_env );
		return true;
	}
	return false;
}

ScriptVarIterator ScriptContext::GlobalIterator()
{
	sgs_Variable env;
	sgs_GetEnv( C, &env );
	ScriptVarIterator svi( C, &env );
	sgs_Release( C, &env );
	return svi;
}

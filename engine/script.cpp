

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
	sgs_Variable kv;
	if( m_iter.C && SGS_SUCCEEDED( sgs_IterGetDataP( m_iter.C, &m_iter.var, &kv, NULL ) ) )
		return sgsVariable( m_iter.C, &kv );
	return sgsVariable();
}

sgsVariable ScriptVarIterator::GetValue()
{
	sgs_Variable kv;
	if( m_iter.C && SGS_SUCCEEDED( sgs_IterGetDataP( m_iter.C, &m_iter.var, NULL, &kv ) ) )
		return sgsVariable( m_iter.C, &kv );
	return sgsVariable();
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

ScriptVarIterator ScriptContext::GlobalIterator()
{
	sgs_Variable env;
	sgs_GetEnv( C, &env );
	ScriptVarIterator svi( C, &env );
	sgs_Release( C, &env );
	return svi;
}

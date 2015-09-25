
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#define SGS_INTERNAL_STRINGTABLES

#include "sgs_int.h"



#if SGS_DEBUG && SGS_DEBUG_FLOW
#  define DBGINFO( text ) sgs_Msg( C, SGS_INFO, text )
#else
#  define DBGINFO( text )
#endif


static const char* g_varnames[] = { "null", "bool", "int", "real", "string", "func", "cfunc", "obj", "ptr" };


static void* int_memory( SGS_SHCTX, void* ptr, size_t size )
{
	void* p;
	if( size )
	{
		size += 16;
		S->memsize += size;
		S->numallocs++;
		S->numblocks++;
	}
	if( ptr )
	{
		ptr = ((char*)ptr) - 16;
		S->memsize -= *(size_t*)ptr;
		S->numfrees++;
		S->numblocks--;
	}
	p = S->memfunc( S->mfuserdata, ptr, size );
	if( p )
	{
		memcpy( p, &size, sizeof(size_t) );
		p = ((char*)p) + 16;
	}
	return p;
}


sgs_Context* sgs_CreateEngineExt( sgs_MemFunc memfunc, void* mfuserdata )
{
	///
	// SHARED STATE
	SGS_CTX;
	sgs_ShCtx* S = (sgs_ShCtx*) memfunc( mfuserdata, NULL, sizeof( sgs_ShCtx ) );
	
	S->version = SGS_VERSION_INT;
	S->sfs_fn = sgs_StdScriptFSFunc;
	S->sfs_ctx = NULL;
	
	S->memfunc = memfunc;
	S->mfuserdata = mfuserdata;
	S->memsize = sizeof( sgs_ShCtx ) + sizeof( sgs_Context );
	S->numallocs = 2;
	S->numblocks = 2;
	S->numfrees = 0;
	
	S->objs = NULL;
	S->objcount = 0;
	S->redblue = 0;
	S->gcrun = SGS_FALSE;
	S->objpool_size = 0;
	
	///
	// CONTEXT
	C = (sgs_Context*) memfunc( mfuserdata, NULL, sizeof( sgs_Context ) );
	
	S->state_list = C;
	S->statecount = 1;
	C->shared = S;
	C->prev = NULL;
	C->next = NULL;
	
	C->output_fn = sgs_StdOutputFunc;
	C->output_ctx = stdout;
	C->erroutput_fn = sgs_StdOutputFunc;
	C->erroutput_ctx = stderr;
	C->serialize_state = NULL;
	C->minlev = SGS_INFO;
	C->apilev = SGS_ERROR + 1;
	C->msg_fn = sgs_StdMsgFunc;
	C->msg_ctx = stderr;
	C->last_errno = 0;
	C->hook_fn = NULL;
	C->hook_ctx = NULL;
	
	C->state = 0;
	C->fctx = NULL;
	C->filename = NULL;
	
	C->stack_mem = 32;
	C->stack_base = sgs_Alloc_n( sgs_Variable, C->stack_mem );
	C->stack_off = C->stack_base;
	C->stack_top = C->stack_base;
	
	C->clstk_mem = 32;
	C->clstk_base = sgs_Alloc_n( sgs_Closure*, C->clstk_mem );
	C->clstk_off = C->clstk_base;
	C->clstk_top = C->clstk_base;
	
	C->sf_first = NULL;
	C->sf_last = NULL;
	C->sf_cached = NULL;
	C->sf_count = 0;
	C->num_last_returned = 0;
	
	// SHARED/FINALIZE
#if SGS_OBJPOOL_SIZE > 0
	S->objpool_data = sgs_Alloc_n( sgs_ObjPoolItem, SGS_OBJPOOL_SIZE );
#else
	S->objpool_data = NULL;
#endif
	sgs_vht_init( &S->typetable, C, 32, 32 );
	sgs_vht_init( &S->stringtable, C, 256, 256 );
	sgs_vht_init( &S->ifacetable, C, 64, 64 );
	// ---
	
	sgsSTD_GlobalInit( C );
	sgsSTD_PostInit( C );
	return C;
}


static void ctx_destroy( SGS_CTX )
{
	SGS_SHCTX_USE;
	
	if( C->state & SGS_STATE_DESTROYING )
	{
		return;
	}
	C->state |= SGS_STATE_DESTROYING;
	
	/* clear the stack */
	while( C->stack_base != C->stack_top )
	{
		C->stack_top--;
		sgs_Release( C, C->stack_top );
	}
	sgs_Dealloc( C->stack_base );
	
	while( C->clstk_base != C->clstk_top )
	{
		C->clstk_top--;
		if( --(*C->clstk_top)->refcount < 1 )
		{
			sgs_Release( C, &(*C->clstk_top)->var );
			sgs_Dealloc( *C->clstk_top );
		}
	}
	sgs_Dealloc( C->clstk_base );
	
	sgsSTD_GlobalFree( C );
	
	/* free the call stack */
	{
		sgs_StackFrame* sf = C->sf_cached, *sfn;
		while( sf )
		{
			sgs_Release( C, &sf->func );
			sfn = sf->cached;
			sgs_Dealloc( sf );
			sf = sfn;
		}
		C->sf_cached = NULL;
	}
	
	// LAST ONE CLEANS UP
	if( C->prev == NULL && C->next == NULL )
	{
		sgs_GCExecute( C );
		sgs_BreakIf( S->objs || S->objcount );
		
		sgs_vht_free( &S->ifacetable, C ); /* table should be empty here */
		sgs_vht_free( &S->typetable, C );
		sgs_VHTVar* p = S->stringtable.vars;
		sgs_VHTVar* pend = p + S->stringtable.size;
		while( p < pend )
		{
			sgs_iStr* unfreed_string = p->key.data.S;
#if SGS_DEBUG && SGS_DEBUG_VALIDATE
			if( unfreed_string->refcount > 0 )
			{
				fprintf( stderr, "unfreed string: (rc=%d)[%d]\"%.*s\"\n",
					(int) unfreed_string->refcount, (int) unfreed_string->size,
					(int) unfreed_string->size, sgs_str_cstr( unfreed_string ) );
			}
#endif
			sgs_BreakIf( unfreed_string->refcount > 0 && "string not freed" );
			sgs_BreakIf( unfreed_string->refcount < 0 && "memory error" );
			sgs_Dealloc( unfreed_string );
			p++;
		}
		sgs_vht_free( &S->stringtable, C );
	}
	// ----
	
	if( S->state_list == C ) S->state_list = C->next;
	if( C->prev ) C->prev->next = C->next;
	if( C->next ) C->next->prev = C->prev;
	S->statecount--;
	
	// free the context
	S->memfunc( S->mfuserdata, C, 0 );
	S->memsize -= sizeof(*C);
	S->numfrees++;
	S->numblocks--;
}

static void shctx_destroy( SGS_SHCTX )
{
#if SGS_OBJPOOL_SIZE > 0
	{
		int32_t i;
		for( i = 0; i < S->objpool_size; ++i )
			int_memory( S, S->objpool_data[ i ].obj, 0 );
		int_memory( S, S->objpool_data, 0 );
	}
#endif

#ifdef SGS_DEBUG_LEAKS
	sgs_BreakIf( S->memsize > sizeof( sgs_ShCtx ) &&
		"not all resources have been freed" );
	sgs_BreakIf( S->memsize < sizeof( sgs_ShCtx ) &&
		"some resorces may have been freed more than once" );
#endif
	
	S->memfunc( S->mfuserdata, S, 0 );
}

void sgs_DestroyEngine( SGS_CTX )
{
	sgs_ShCtx* S = C->shared;
	
	while( S->state_list )
		ctx_destroy( S->state_list );
	
	shctx_destroy( S );
}


static void copy_append_frame( SGS_CTX, sgs_StackFrame* sf )
{
	sgs_StackFrame* nsf = sgs_Alloc( sgs_StackFrame );
	memcpy( nsf, sf, sizeof(*nsf) );
	sgs_Acquire( C, &nsf->func );
	
	nsf->next = NULL;
	nsf->cached = NULL;
	if( C->sf_last )
	{
		nsf->prev = C->sf_last;
		nsf->prev->next = nsf;
	}
}

sgs_Context* sgs_ForkState( SGS_CTX, int copystate )
{
	SGS_SHCTX_USE;
	sgs_Context* NC = (sgs_Context*) S->memfunc( S->mfuserdata, NULL, sizeof(*NC) );
	S->memsize += sizeof(*NC);
	S->numallocs++;
	S->numblocks++;
	memcpy( NC, C, sizeof(*NC) );
	
	// realloc / acquire
	if( copystate == SGS_FALSE )
	{
		NC->stack_mem = 32;
		NC->clstk_mem = 32;
	}
	
	NC->stack_base = sgs_Alloc_n( sgs_Variable, NC->stack_mem );
	NC->clstk_base = sgs_Alloc_n( sgs_Closure*, NC->clstk_mem );
	
	if( copystate == SGS_FALSE )
	{
		NC->stack_top = NC->stack_off = NC->stack_base;
		NC->clstk_top = NC->clstk_off = NC->clstk_base;
	}
	else
	{
		// WP: assuming top > base
		memcpy( NC->stack_base, C->stack_base, sizeof(sgs_Variable) * (size_t)( C->stack_top - C->stack_base ) );
		NC->stack_off = NC->stack_base + ( C->stack_off - C->stack_base );
		NC->stack_top = NC->stack_base + ( C->stack_top - C->stack_base );
		{
			sgs_Variable* vp = NC->stack_base, *vpend = NC->stack_top;
			while( vp != vpend )
				sgs_Acquire( C, vp++ );
		}
		
		// WP: assuming top > base
		memcpy( NC->clstk_base, C->clstk_base, sizeof(sgs_Closure*) * (size_t)( C->clstk_top - C->clstk_base ) );
		NC->clstk_off = NC->clstk_base + ( C->clstk_off - C->clstk_base );
		NC->clstk_top = NC->clstk_base + ( C->clstk_top - C->clstk_base );
		{
			sgs_Closure** cp = NC->clstk_base, **cpend = NC->clstk_top;
			while( cp != cpend )
				(*cp++)->refcount++;
		}
	}
	
	// - global var. dict.
	sgs_ObjAcquire( C, NC->_G );
	
	// - call stack
	NC->sf_first = NULL;
	NC->sf_last = NULL;
	NC->sf_cached = NULL;
	NC->sf_count = 0;
	if( copystate )
	{
		sgs_StackFrame* sf = C->sf_first;
		while( sf != C->sf_last )
		{
			copy_append_frame( NC, sf );
			sf = sf->next;
		}
	}
	
	// add state to list
	NC->prev = NULL;
	NC->next = S->state_list;
	if( NC->next )
		NC->next->prev = NC;
	S->state_list = NC;
	S->statecount++;
	
	return NC;
}

void sgs_FreeState( SGS_CTX )
{
	SGS_SHCTX_USE;
	ctx_destroy( C );
	
	if( S->state_list == NULL )
		shctx_destroy( S );
}

SGSBOOL sgs_PauseState( SGS_CTX )
{
	if( C->sf_last == NULL )
		return SGS_FALSE; /* nothing to pause */
	if( C->state & SGS_STATE_PAUSED )
		return SGS_FALSE; /* already paused, but possibly not at the expected location */
	
	sgs_StackFrame* sf = C->sf_last;
	if( sf && !sf->iptr )
		sf = sf->prev; /* should be able to use this inside a C function */
	if( !sf || !sf->iptr )
		return SGS_FALSE; /* not in SGS function */
	{
		sgs_StackFrame* tsf = sf->prev;
		while( tsf && tsf->iptr )
			tsf = tsf->prev;
		if( tsf )
			return SGS_FALSE; /* cannot have any C functions in the middle */
	}
	
	C->state |= SGS_STATE_PAUSED;
	
	return SGS_TRUE;
}

SGSBOOL sgs_ResumeStateExp( SGS_CTX, int args, int expect )
{
	int rvc = 0;
	int ret = sgs_ResumeStateRet( C, args, &rvc );
	if( ret )
		sgs_SetDeltaSize( C, expect - rvc );
	return ret;
}


const char* sgs_CodeString( int type, int val )
{
	if( type == SGS_CODE_ER )
	{
		if( val < SGS_EINPROC )
			return "UNKNOWN ERROR";
		if( SGS_SUCCEEDED( val ) )
			val = 0;
		return sgs_ErrNames[  -  val ];
	}
	else if( type == SGS_CODE_VT )
	{
		if( val < 0 || val >= SGS_VT__COUNT )
			return NULL;
		return sgs_VarNames[ val ];
	}
	else if( type == SGS_CODE_OP )
	{
		if( val < 0 || val >= SGS_SI_COUNT )
			return NULL;
		return sgs_OpNames[ val ];
	}
	else
		return NULL;
}


void sgs_GetOutputFunc( SGS_CTX, sgs_OutputFunc* outf, void** outc )
{
	*outf = C->output_fn;
	*outc = C->output_ctx;
}

void sgs_SetOutputFunc( SGS_CTX, sgs_OutputFunc func, void* ctx )
{
	sgs_BreakIf( func == NULL );
	if( func == SGSOUTPUTFN_DEFAULT )
		func = sgs_StdOutputFunc;
	if( !ctx && func == sgs_StdOutputFunc )
		ctx = stdout;
	C->output_fn = func;
	C->output_ctx = ctx;
}

void sgs_Write( SGS_CTX, const void* ptr, size_t size )
{
	C->output_fn( C->output_ctx, C, ptr, size );
}

SGSBOOL sgs_Writef( SGS_CTX, const char* what, ... )
{
	char buf[ SGS_OUTPUT_STACKBUF_SIZE ];
	sgs_MemBuf info = sgs_membuf_create();
	int cnt;
	va_list args;
	char* ptr = buf;
	
	va_start( args, what );
	cnt = SGS_VSPRINTF_LEN( what, args );
	va_end( args );
	
	if( cnt < 0 )
		return SGS_FALSE;
	
	if( cnt >= SGS_OUTPUT_STACKBUF_SIZE )
	{
		/* WP: false alarm */
		sgs_membuf_resize( &info, C, (size_t) cnt + 1 );
		ptr = info.ptr;
	}
	
	va_start( args, what );
	vsprintf( ptr, what, args );
	va_end( args );
	ptr[ cnt ] = 0;
	
	sgs_WriteStr( C, ptr );
	
	sgs_membuf_destroy( &info, C );
	return SGS_TRUE;
}

void sgs_GetErrOutputFunc( SGS_CTX, sgs_OutputFunc* outf, void** outc )
{
	*outf = C->erroutput_fn;
	*outc = C->erroutput_ctx;
}

void sgs_SetErrOutputFunc( SGS_CTX, sgs_OutputFunc func, void* ctx )
{
	sgs_BreakIf( func == NULL );
	if( func == SGSOUTPUTFN_DEFAULT )
		func = sgs_StdOutputFunc;
	if( !ctx && func == sgs_StdOutputFunc )
		ctx = stderr;
	C->erroutput_fn = func;
	C->erroutput_ctx = ctx;
}

void sgs_ErrWrite( SGS_CTX, const void* ptr, size_t size )
{
	C->erroutput_fn( C->erroutput_ctx, C, ptr, size );
}

SGSBOOL sgs_ErrWritef( SGS_CTX, const char* what, ... )
{
	char buf[ SGS_OUTPUT_STACKBUF_SIZE ];
	sgs_MemBuf info = sgs_membuf_create();
	int cnt;
	va_list args;
	char* ptr = buf;
	
	va_start( args, what );
	cnt = SGS_VSPRINTF_LEN( what, args );
	va_end( args );
	
	if( cnt < 0 )
		return SGS_FALSE;
	
	if( cnt >= SGS_OUTPUT_STACKBUF_SIZE )
	{
		/* WP: false alarm */
		sgs_membuf_resize( &info, C, (size_t) cnt + 1 );
		ptr = info.ptr;
	}
	
	va_start( args, what );
	vsprintf( ptr, what, args );
	va_end( args );
	ptr[ cnt ] = 0;
	
	sgs_ErrWriteStr( C, ptr );
	
	sgs_membuf_destroy( &info, C );
	return SGS_TRUE;
}


void sgs_GetMsgFunc( SGS_CTX, sgs_MsgFunc* outf, void** outc )
{
	*outf = C->msg_fn;
	*outc = C->msg_ctx;
}

void sgs_SetMsgFunc( SGS_CTX, sgs_MsgFunc func, void* ctx )
{
	sgs_BreakIf( func == NULL );
	if( func == SGSMSGFN_DEFAULT )
		func = sgs_StdMsgFunc;
	else if( func == SGSMSGFN_DEFAULT_NOABORT )
		func = sgs_StdMsgFunc_NoAbort;
	if( !ctx && ( func == sgs_StdMsgFunc || func == sgs_StdMsgFunc_NoAbort ) )
		ctx = NULL;
	C->msg_fn = func;
	C->msg_ctx = ctx;
}

SGSZERO sgs_Msg( SGS_CTX, int type, const char* what, ... )
{
	char buf[ SGS_OUTPUT_STACKBUF_SIZE ];
	sgs_MemBuf info = sgs_membuf_create();
	int off = 0, cnt = 0, slen = 0;
	va_list args;
	char* ptr = buf;
	
	/* error level filter */
	if( type < C->minlev ) return 0;
	/* error suppression */
	if( C->sf_last && C->sf_last->errsup > 0 ) return 0;
	
	va_start( args, what );
	cnt = SGS_VSPRINTF_LEN( what, args );
	va_end( args );
	
	sgs_BreakIf( cnt < 0 );
	if( cnt < 0 )
	{
		C->msg_fn( C->msg_ctx, C, SGS_ERROR, "sgs_Msg ERROR: failed to print the message" );
		return 0;
	}
	
	if( C->sf_last && C->sf_last->nfname )
	{
		/* WP: it is expected that native functions ..
		.. will not set names of more than 2GB length */
		slen = (int) strlen( C->sf_last->nfname );
		off = slen + SGS_MSG_EXTRABYTES;
		cnt += off;
	}

	if( cnt >= SGS_OUTPUT_STACKBUF_SIZE )
	{
		/* WP: cnt is never negative */
		sgs_membuf_resize( &info, C, (size_t) cnt + 1 );
		ptr = info.ptr;
	}
	
	if( C->sf_last && C->sf_last->nfname )
	{
		/* WP: slen is generated from size_t, thus is never negative */
		memcpy( ptr, C->sf_last->nfname, (size_t) slen );
		memcpy( ptr + slen, SGS_MSG_EXTRASTRING, SGS_MSG_EXTRABYTES );
	}

	va_start( args, what );
	vsprintf( ptr + off, what, args );
	va_end( args );
	ptr[ cnt ] = 0;

	C->msg_fn( C->msg_ctx, C, type, ptr );

	sgs_membuf_destroy( &info, C );
	
	return 0;
}


void sgs_WriteErrorInfo( SGS_CTX, int flags, sgs_ErrorOutputFunc func, void* ctx, int type, const char* msg )
{
	if( flags & SGS_ERRORINFO_STACK )
	{
		sgs_StackFrame* p = sgs_GetFramePtr( C, SGS_FALSE );
		SGS_UNUSED( ctx );
		while( p != NULL )
		{
			const char* file, *name;
			int ln;
			if( !p->next && !p->code )
				break;
			sgs_StackFrameInfo( C, p, &name, &file, &ln );
			if( ln )
				func( ctx, "- \"%s\" in %s, line %d\n", name, file, ln );
			else
				func( ctx, "- \"%s\" in %s\n", name, file );
			p = p->next;
		}
	}
	if( flags & SGS_ERRORINFO_ERROR )
	{
		static const char* errpfxs[ 3 ] = { "Info", "Warning", "Error" };
		type = type / 100 - 1;
		if( type < 0 ) type = 0;
		if( type > 2 ) type = 2;
		func( ctx, "%s: %s\n", errpfxs[ type ], msg );
	}
}

static void serialize_output_func( void* ud, SGS_CTX, const void* ptr, size_t datasize )
{
	sgs_MemBuf* B = (sgs_MemBuf*) ud;
	sgs_membuf_appbuf( B, C, ptr, datasize );
}

void sgs_PushErrorInfo( SGS_CTX, int flags, int type, const char* msg )
{
	sgs_OutputFunc oldfn = C->output_fn;
	void* oldctx = C->output_ctx;
	
	sgs_MemBuf B = sgs_membuf_create();
	C->output_fn = serialize_output_func;
	C->output_ctx = &B;
	
	sgs_WriteErrorInfo( C, flags, (sgs_ErrorOutputFunc) sgs_Writef, C, type, msg );
	
	/* WP: hopefully the error messages will not be more than 2GB long */
	sgs_PushStringBuf( C, B.ptr, (sgs_SizeVal) B.size );
	
	sgs_membuf_destroy( &B, C );
	C->output_fn = oldfn;
	C->output_ctx = oldctx;
}


SGSBOOL sgs_GetHookFunc( SGS_CTX, sgs_HookFunc* outf, void** outc )
{
	if( C->hook_fn )
	{
		*outf = C->hook_fn;
		*outc = C->hook_ctx;
		return 1;
	}
	return 0;
}

void sgs_SetHookFunc( SGS_CTX, sgs_HookFunc func, void* ctx )
{
	C->hook_fn = func;
	C->hook_ctx = ctx;
}


SGSBOOL sgs_GetScriptFSFunc( SGS_CTX, sgs_ScriptFSFunc* outf, void** outc )
{
	if( C->shared->sfs_fn )
	{
		*outf = C->shared->sfs_fn;
		*outc = C->shared->sfs_ctx;
		return 1;
	}
	return 0;
}

void sgs_SetScriptFSFunc( SGS_CTX, sgs_ScriptFSFunc func, void* ctx )
{
	C->shared->sfs_fn = func;
	C->shared->sfs_ctx = ctx;
}


void* sgs_Memory( SGS_CTX, void* ptr, size_t size )
{
	SGS_SHCTX_USE;
	return int_memory( S, ptr, size );
}


static int ctx_decode( SGS_CTX, const char* buf, size_t size, sgs_CompFunc** out )
{
	sgs_CompFunc* func = NULL;

	if( sgsBC_ValidateHeader( buf, size ) < SGS_HEADER_SIZE )
	{
		/* invalid / unsupported / unrecognized file */
		return 0;
	}

	{
		const char* ret;
		ret = sgsBC_Buf2Func( C, C->filename ? C->filename : "", buf, size, &func );
		if( ret )
		{
			/* just invalid, error! */
			sgs_Msg( C, SGS_ERROR, "Failed to read bytecode file (%s)", ret );
			return -1;
		}
	}

	*out = func;
	return 1;
}

static int ctx_compile( SGS_CTX, const char* buf, size_t size, sgs_CompFunc** out )
{
	sgs_CompFunc* func = NULL;
	sgs_TokenList tlist = NULL;
	sgs_FTNode* ftree = NULL;

	C->state = 0;

	DBGINFO( "...running the tokenizer" );
	tlist = sgsT_Gen( C,  buf, size );
	if( !tlist || C->state & SGS_HAS_ERRORS )
		goto error;
#if SGS_DEBUG && SGS_DEBUG_DATA
	sgsT_DumpList( tlist, NULL );
#endif

	DBGINFO( "...running the function tree builder" );
	ftree = sgsFT_Compile( C, tlist );
	if( !ftree || C->state & SGS_HAS_ERRORS )
		goto error;
#if SGS_DEBUG && SGS_DEBUG_DATA
	sgsFT_Dump( ftree );
#endif

	DBGINFO( "...generating the opcode" );
	func = sgsBC_Generate( C, ftree );
	if( !func || C->state & SGS_HAS_ERRORS )
		goto error;
	DBGINFO( "...cleaning up tokens/function tree" );
	sgsFT_Destroy( C, ftree ); ftree = NULL;
	sgsT_Free( C, tlist ); tlist = NULL;
#if SGS_DUMP_BYTECODE || ( SGS_DEBUG && SGS_DEBUG_DATA )
	sgsBC_Dump( func );
#endif

	*out = func;
	return 1;

error:
	if( func )	sgsBC_Free( C, func );
	if( ftree ) sgsFT_Destroy( C, ftree );
	if( tlist ) sgsT_Free( C, tlist );

	return 0;
}

static SGSRESULT ctx_execute( SGS_CTX, const char* buf, size_t size, int clean, int* rvc )
{
	int rr;
	sgs_CompFunc* func;
	sgs_Variable funcvar;
	
	if( !( rr = ctx_decode( C, buf, size, &func ) ) &&
		!ctx_compile( C, buf, size, &func ) )
		return SGS_ECOMP;
	
	if( rr < 0 )
		return SGS_EINVAL;
	
	funcvar.type = SGS_VT_FUNC;
	funcvar.data.F = sgsBC_ConvertFunc( C, func, "<main>", 6, 0 );
	/* func was freed here */
	DBGINFO( "...cleaning up bytecode/constants" );
	
	DBGINFO( "...executing the generated function" );
	sgs_XCallP( C, &funcvar, 0, rvc );
	
	DBGINFO( "...finished!" );
	sgs_Release( C, &funcvar );
	return SGS_SUCCESS;
}

SGSRESULT sgs_EvalBuffer( SGS_CTX, const char* buf, size_t size, int* rvc )
{
	DBGINFO( "sgs_EvalBuffer called!" );
	
	if( size > 0x7fffffff )
		return SGS_EINVAL;
	
	return ctx_execute( C, buf, size, !rvc, rvc );
}

SGSRESULT sgs_EvalFile( SGS_CTX, const char* file, int* rvc )
{
	int ret;
	size_t ulen;
	char* data;
	const char* ofn;
	unsigned char magic[4];
	sgs_ScriptFSData fsd = {0};
	SGS_SHCTX_USE;
	DBGINFO( "sgs_EvalFile called!" );
	
	fsd.filename = file;
	ret = S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_OPEN, &fsd );
	if( SGS_FAILED( ret ) )
	{
		return ret == SGS_ENOTFND ? ret : SGS_EINPROC;
	}
	ulen = fsd.size;
	
	if( ulen > 4 )
	{
		fsd.output = magic;
		fsd.size = 4;
		ret = S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_READ, &fsd );
		if( SGS_FAILED( ret ) )
		{
			S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_CLOSE, &fsd );
			return SGS_EINPROC;
		}
		
		if( ( magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F' ) /* ELF binary */
		 || ( magic[0] == 'M' && magic[1] == 'Z' ) /* DOS binary / DOS stub for PE binary */
		 || ( magic[0] == 0xCA && magic[1] == 0xFE && magic[2] == 0xBA && magic[3] == 0xBE ) /* Mach-O binary 1 */
		 || ( magic[0] == 0xCE && magic[1] == 0xFA && magic[2] == 0xED && magic[3] == 0xFE ) /* Mach-O binary 2 */
		 || ( magic[0] == 0xCF && magic[1] == 0xFA && magic[2] == 0xED && magic[3] == 0xFE ) /* Mach-O binary 3 */
		 )
		{
			S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_CLOSE, &fsd );
			return SGS_EINVAL;
		}
	}
	
	data = sgs_Alloc_n( char, ulen );
	fsd.output = data;
	fsd.size = ulen;
	ret = S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_READ, &fsd );
	S->sfs_fn( S->sfs_ctx, C, SGS_SFS_FILE_CLOSE, &fsd );
	if( SGS_FAILED( ret ) )
	{
		sgs_Dealloc( data );
		return SGS_EINPROC;
	}
	
	ofn = C->filename;
	C->filename = file;
	ret = ctx_execute( C, data, ulen, rvc ? SGS_FALSE : SGS_TRUE, rvc );
	C->filename = ofn;
	
	sgs_Dealloc( data );
	return ret;
}

SGSRESULT sgs_Compile( SGS_CTX, const char* buf, size_t size, char** outbuf, size_t* outsize )
{
	sgs_MemBuf mb;
	sgs_CompFunc* func;
	
	if( size > 0x7fffffff )
		return SGS_EINVAL;
	
	if( !ctx_compile( C, buf, size, &func ) )
		return SGS_ECOMP;
	
	mb = sgs_membuf_create();
	if( !sgsBC_Func2Buf( C, func, &mb ) )
	{
		sgs_membuf_destroy( &mb, C );
		return SGS_EINPROC;
	}
	
	*outbuf = mb.ptr;
	*outsize = mb.size;
	
	sgsBC_Free( C, func );
	
	return SGS_SUCCESS;
}


static void _recfndump( const char* constptr, size_t constsize,
	const char* codeptr, size_t codesize, int gt, int args, int tmp, int clsr )
{
	const sgs_Variable* var = (const sgs_Variable*) (const void*) SGS_ASSUME_ALIGNED( constptr, 4 );
	const sgs_Variable* vend = (const sgs_Variable*) (const void*) SGS_ASSUME_ALIGNED( constptr + constsize, 4 );
	while( var < vend )
	{
		if( var->type == SGS_VT_FUNC )
		{
			_recfndump( (const char*) sgs_func_consts( var->data.F ), var->data.F->instr_off,
				(const char*) sgs_func_bytecode( var->data.F ), var->data.F->size - var->data.F->instr_off,
				var->data.F->gotthis, var->data.F->numargs, var->data.F->numtmp, var->data.F->numclsr );
		}
		var++;
	}
	printf( "\nFUNC: type=%s args=%d tmp=%d closures=%d\n", gt ? "method" : "function", args, tmp, clsr );
	sgsBC_DumpEx( constptr, constsize, codeptr, codesize );
}

SGSRESULT sgs_DumpCompiled( SGS_CTX, const char* buf, size_t size )
{
	int rr;
	sgs_CompFunc* func;
	
	if( size > 0x7fffffff )
		return SGS_EINVAL;
	
	if( !( rr = ctx_decode( C, buf, size, &func ) ) &&
		!ctx_compile( C, buf, size, &func ) )
		return SGS_ECOMP;
	
	if( rr < 0 )
		return SGS_EINVAL;
	
	_recfndump( func->consts.ptr, func->consts.size, func->code.ptr, func->code.size,
		func->gotthis, func->numargs, func->numtmp, func->numclsr );
	
	sgsBC_Free( C, func );
	return SGS_SUCCESS;
}


SGSRESULT sgs_Abort( SGS_CTX )
{
	sgs_StackFrame* sf = C->sf_last;
	if( sf && !sf->iptr )
		sf = sf->prev; /* should be able to use this inside a C function */
	if( !sf || !sf->iptr )
		return SGS_ENOTFND;
	while( sf && sf->iptr )
	{
		sf->iptr = sf->iend;
		sf->flags |= SGS_SF_ABORTED;
		sf = sf->prev;
	}
	return SGS_SUCCESS;
}



static void ctx_print_safe( SGS_CTX, const char* str, size_t size )
{
	const char* strend = str + size;
	while( str < strend )
	{
		if( *str == ' ' || isgraph( *str ) )
			sgs_Write( C, str, 1 );
		else
		{
			static const char* hexdigs = "0123456789ABCDEF";
			char buf[ 4 ] = { '\\', 'x', 0, 0 };
			{
				buf[2] = hexdigs[ (*str & 0xf0) >> 4 ];
				buf[3] = hexdigs[ *str & 0xf ];
			}
			sgs_Write( C, buf, 4 );
		}
		str++;
	}
}

static void dumpobj( SGS_CTX, sgs_VarObj* p )
{
	sgs_Writef( C, "OBJECT %p refcount=%d data=%p iface=%p prev=%p next=%p redblue=%s",
		p, p->refcount, p->data, p->iface, p->prev, p->next, p->redblue ? "R" : "B" );
}

/* g_varnames ^^ */

static void dumpvar( SGS_CTX, sgs_Variable* var )
{
	if( var->type > SGS_VT_PTR )
	{
		sgs_Writef( C, "INVALID TYPE %d\n", (int) var->type );
		return;
	}
	/* WP: var->type base type info uses bits 1-8 */
	sgs_Writef( C, "%s (size:%d)", g_varnames[ var->type ], sgsVM_VarSize( var ) );
	switch( var->type )
	{
	case SGS_VT_NULL: break;
	case SGS_VT_BOOL: sgs_Writef( C, " = %s", var->data.B ? "true" : "false" ); break;
	case SGS_VT_INT: sgs_Writef( C, " = %" PRId64, var->data.I ); break;
	case SGS_VT_REAL: sgs_Writef( C, " = %f", var->data.R ); break;
	case SGS_VT_STRING:
		sgs_Writef( C, " [rc:%d] = \"", var->data.S->refcount );
		ctx_print_safe( C, sgs_var_cstr( var ), SGS_MIN( var->data.S->size, 16 ) );
		sgs_Writef( C, var->data.S->size > 16 ? "...\"" : "\"" );
		break;
	case SGS_VT_FUNC:
		sgs_Writef( C, " [rc:%d] '%s'[%d]%s tmp=%d clsr=%d", var->data.F->refcount,
			var->data.F->sfuncname->size ? sgs_str_cstr( var->data.F->sfuncname ) : "<anonymous>",
			(int) var->data.F->numargs, var->data.F->gotthis ? " (method)" : "",
			(int) var->data.F->numtmp, (int) var->data.F->numclsr );
		break;
	case SGS_VT_CFUNC: sgs_Writef( C, " = %p", var->data.C ); break;
	case SGS_VT_OBJECT: sgs_Writef( C, " = " ); dumpobj( C, var->data.O ); break;
	case SGS_VT_PTR: sgs_Writef( C, " = %p", var->data.P ); break;
	}
}

ptrdiff_t sgs_Stat( SGS_CTX, int type )
{
	SGS_SHCTX_USE;
	switch( type )
	{
	/* WP: not important */
	case SGS_STAT_VERSION: return (ptrdiff_t) S->version;
	case SGS_STAT_STATECOUNT: return (ptrdiff_t) S->statecount;
	case SGS_STAT_OBJCOUNT: return (ptrdiff_t) S->objcount;
	case SGS_STAT_MEMSIZE: return (ptrdiff_t) S->memsize;
	case SGS_STAT_NUMALLOCS: return (ptrdiff_t) S->numallocs;
	case SGS_STAT_NUMFREES: return (ptrdiff_t) S->numfrees;
	case SGS_STAT_NUMBLOCKS: return (ptrdiff_t) S->numblocks;
	case SGS_STAT_DUMP_STACK:
		{
			sgs_Variable* p = C->stack_base;
			sgs_WriteStr( C, "\nVARIABLE -- ---- STACK ---- BASE ----\n" );
			while( p < C->stack_top )
			{
				if( p == C->stack_off )
				{
					sgs_WriteStr( C, "VARIABLE -- ---- STACK ---- OFFSET ----\n" );
				}
				sgs_Writef( C, "VARIABLE %02d ", p - C->stack_base );
				dumpvar( C, p );
				sgs_WriteStr( C, "\n" );
				p++;
			}
			sgs_WriteStr( C, "VARIABLE -- ---- STACK ---- TOP ----\n" );
		}
		return SGS_SUCCESS;
	case SGS_STAT_DUMP_GLOBALS:
		{
			sgs_VHTVar *p, *pend;
			sgsSTD_GlobalIter( C, &p, &pend );
			sgs_WriteStr( C, "\nGLOBAL ---- LIST ---- START ----\n" );
			while( p < pend )
			{
				sgs_iStr* str = p->key.data.S;
				sgs_WriteStr( C, "GLOBAL '" );
				ctx_print_safe( C, sgs_str_cstr( str ), str->size );
				sgs_WriteStr( C, "' = " );
				dumpvar( C, &p->val );
				sgs_WriteStr( C, "\n" );
				p++;
			}
			sgs_WriteStr( C, "GLOBAL ---- LIST ---- END ----\n" );
		}
		return SGS_SUCCESS;
	case SGS_STAT_DUMP_OBJECTS:
		{
			sgs_VarObj* p = S->objs;
			sgs_WriteStr( C, "\nOBJECT ---- LIST ---- START ----\n" );
			while( p )
			{
				dumpobj( C, p );
				sgs_WriteStr( C, "\n" );
				p = p->next;
			}
			sgs_WriteStr( C, "OBJECT ---- LIST ---- END ----\n" );
		}
		return SGS_SUCCESS;
	case SGS_STAT_DUMP_FRAMES:
		{
			sgs_StackFrame* p = sgs_GetFramePtr( C, SGS_FALSE );
			sgs_WriteStr( C, "\nFRAME ---- LIST ---- START ----\n" );
			while( p != NULL )
			{
				const char* file, *name;
				int ln;
				sgs_StackFrameInfo( C, p, &name, &file, &ln );
				sgs_Writef( C, "FRAME \"%s\" in %s, line %d\n", name, file, ln );
				p = p->next;
			}
			sgs_WriteStr( C, "FRAME ---- LIST ---- END ----\n" );
		}
		return SGS_SUCCESS;
	case SGS_STAT_DUMP_STATS:
		{
			sgs_WriteStr( C, "\nSTATS ---- ---- ----\n" );
			sgs_Writef( C, "# allocs: %d\n", S->numallocs );
			sgs_Writef( C, "# frees: %d\n", S->numfrees );
			sgs_Writef( C, "# mem blocks: %d\n", S->numblocks );
			sgs_Writef( C, "# mem bytes: %d\n", S->memsize );
			sgs_Writef( C, "# objects: %d\n", S->objcount );
			sgs_Writef( C, "GC state: %s\n", S->redblue ? "red" : "blue" );
			sgs_WriteStr( C, "---- ---- ---- -----\n" );
		}
		return SGS_SUCCESS;
	case SGS_STAT_XDUMP_STACK:
		{
			ptrdiff_t i = 0, ssz = C->stack_top - C->stack_base;
			sgs_WriteStr( C, "\nVARIABLE -- ---- STACK ---- BASE ----\n" );
			while( i < ssz )
			{
				sgs_Variable* p = C->stack_base + i;
				if( p == C->stack_off )
				{
					sgs_WriteStr( C, "VARIABLE -- ---- STACK ---- OFFSET ----\n" );
				}
				sgs_Writef( C, "VARIABLE %02d ", (int) i );
				dumpvar( C, p );
				sgs_WriteStr( C, "\n" );
				sgs_PushVariable( C, p );
				if( SGS_SUCCEEDED( sgs_DumpVar( C, 6 ) ) )
				{
					sgs_WriteStr( C, sgs_ToString( C, -1 ) );
					sgs_Pop( C, 1 );
					sgs_WriteStr( C, "\n" );
				}
				i++;
			}
			sgs_WriteStr( C, "VARIABLE -- ---- STACK ---- TOP ----\n" );
		}
		return SGS_SUCCESS;
	default:
		return SGS_EINVAL;
	}
}

int32_t sgs_Cntl( SGS_CTX, int what, int32_t val )
{
	int32_t x;
	switch( what )
	{
	/* WP: last bit of C->state is not used */
	case SGS_CNTL_STATE: x = (int32_t) C->state; C->state = (uint32_t) val; return x;
	case SGS_CNTL_GET_STATE: return (int32_t) C->state;
	case SGS_CNTL_MINLEV: x = C->minlev; C->minlev = val; return x;
	case SGS_CNTL_GET_MINLEV: return C->minlev;
	case SGS_CNTL_APILEV: x = C->apilev; C->apilev = val; return x;
	case SGS_CNTL_GET_APILEV: return C->apilev;
	case SGS_CNTL_ERRNO: x = C->last_errno; C->last_errno = val ? 0 : errno; return x;
	case SGS_CNTL_SET_ERRNO: x = C->last_errno; C->last_errno = val; return x;
	case SGS_CNTL_GET_ERRNO: return C->last_errno;
	case SGS_CNTL_ERRSUP:
		if( C->sf_last )
		{
			x = C->sf_last->errsup;
			C->sf_last->errsup = (int16_t) val;
			return x;
		}
		return 0;
	case SGS_CNTL_GET_ERRSUP: return C->sf_last ? C->sf_last->errsup : 0;
	case SGS_CNTL_SERIALMODE:
		x = C->state & SGS_SERIALIZE_MODE2 ? 2 : 1;
		if( val == 2 ) C->state |= SGS_SERIALIZE_MODE2;
		else C->state &= (uint32_t) ~SGS_SERIALIZE_MODE2;
		return x;
	case SGS_CNTL_NUMRETVALS: return C->num_last_returned;
	case SGS_CNTL_GET_PAUSED: return C->state & SGS_STATE_PAUSED ? 1 : 0;
	}
	return 0;
}

void sgs_StackFrameInfo( SGS_CTX, sgs_StackFrame* frame, const char** name, const char** file, int* line )
{
	int L = 0;
	const char* N = "<non-callable type>";
	const char* F = "<buffer>";

	SGS_UNUSED( C );
	if( frame->func.type == SGS_VT_NULL )
	{
		N = "<main>";
		L = frame->lntable[ frame->lptr - frame->code ];
		if( frame->filename )
			F = frame->filename;
	}
	else if( frame->func.type == SGS_VT_FUNC )
	{
		N = "<anonymous function>";
		if( frame->func.data.F->sfuncname->size )
			N = sgs_str_cstr( frame->func.data.F->sfuncname );
		L = !frame->func.data.F->lineinfo ? 1 :
			frame->func.data.F->lineinfo[ frame->lptr - frame->code ];
		if( frame->func.data.F->sfilename->size )
			F = sgs_str_cstr( frame->func.data.F->sfilename );
		else if( frame->filename )
			F = frame->filename;
	}
	else if( frame->func.type == SGS_VT_CFUNC )
	{
		N = frame->nfname ? frame->nfname : "[C function]";
		F = "[C code]";
	}
	else if( frame->func.type == SGS_VT_OBJECT )
	{
		sgs_VarObj* O = frame->func.data.O;
		N = O->iface->name ? O->iface->name : "<object>";
		F = "[C code]";
	}
	if( name ) *name = N;
	if( file ) *file = F;
	if( line ) *line = L;
}

sgs_StackFrame* sgs_GetFramePtr( SGS_CTX, int end )
{
	return end ? C->sf_last : C->sf_first;
}


SGSRESULT sgs_RegisterType( SGS_CTX, const char* name, sgs_ObjInterface* iface )
{
	size_t len;
	sgs_VHTVar* p;
	SGS_SHCTX_USE;
	if( !iface )
		return SGS_EINVAL;
	len = strlen( name );
	if( len > 0x7fffffff )
		return SGS_EINVAL;
	/* WP: error condition */
	p = sgs_vht_get_str( &S->typetable, name, (uint32_t) len, sgs_HashFunc( name, len ) );
	if( p )
		return SGS_EINPROC;
	{
		sgs_Variable tmp;
		tmp.type = SGS_VT_PTR;
		tmp.data.P = iface;
		sgs_PushStringBuf( C, name, (sgs_SizeVal) len );
		sgs_vht_set( &S->typetable, C, C->stack_top-1, &tmp );
		sgs_Pop( C, 1 );
	}
	return SGS_SUCCESS;
}

SGSRESULT sgs_UnregisterType( SGS_CTX, const char* name )
{
	SGS_SHCTX_USE;
	size_t len = strlen( name );
	if( len > 0x7fffffff )
		return SGS_EINVAL;
	/* WP: error condition */
	sgs_VHTVar* p = sgs_vht_get_str( &S->typetable, name, (uint32_t) len, sgs_HashFunc( name, len ) );
	if( !p )
		return SGS_ENOTFND;
	sgs_vht_unset( &S->typetable, C, &p->key );
	return SGS_SUCCESS;
}

sgs_ObjInterface* sgs_FindType( SGS_CTX, const char* name )
{
	SGS_SHCTX_USE;
	size_t len = strlen( name );
	if( len > 0x7fffffff )
		return NULL;
	/* WP: error condition */
	sgs_VHTVar* p = sgs_vht_get_str( &S->typetable, name, (uint32_t) len, sgs_HashFunc( name, len ) );
	if( p )
		return (sgs_ObjInterface*) p->val.data.P;
	return NULL;
}

SGSRESULT sgs_PushInterface( SGS_CTX, sgs_CFunc igfn )
{
	sgs_VHTVar* vv;
	sgs_Variable key;
	SGS_SHCTX_USE;
	
	sgs_InitCFunction( &key, igfn );
	vv = sgs_vht_get( &S->ifacetable, &key );
	if( vv )
	{
		sgs_PushVariable( C, &vv->val );
		return SGS_SUCCESS;
	}
	else
	{
		sgs_VarObj* obj;
		sgs_Variable val;
		sgs_StkIdx ssz;
		SGSRESULT res;
		
		ssz = sgs_StackSize( C );
		res = sgs_CallP( C, &key, 0, 1 );
		if( SGS_FAILED( res ) ||
			sgs_ItemType( C, ssz ) != SGS_VT_OBJECT ||
			!sgs_PeekStackItem( C, ssz, &val ) )
		{
			sgs_SetStackSize( C, ssz );
			return SGS_EINPROC;
		}
		sgs_vht_set( &S->ifacetable, C, &key, &val );
		obj = sgs_GetObjectStruct( C, ssz );
		obj->is_iface = 1;
		obj->refcount--; /* being in interface table doesn't count */
		return SGS_SUCCESS;
	}
}

SGSRESULT sgs_InitInterface( SGS_CTX, sgs_Variable* var, sgs_CFunc igfn )
{
	SGSRESULT res = sgs_PushInterface( C, igfn );
	if( res == SGS_SUCCESS )
		sgs_StoreVariable( C, var );
	return res;
}


void sgs_FuncName( SGS_CTX, const char* fnliteral )
{
	if( C->sf_last )
		C->sf_last->nfname = fnliteral;
}

int sgs_HasFuncName( SGS_CTX )
{
	return !!C->sf_last && !!C->sf_last->nfname;
}




#define SS_IS_LAUNCHER 1

#if defined _WIN32
#include <windows.h>
#endif


__declspec(dllimport) int SGRX_EntryPoint( int argc, char** argv, int debug );


#undef main
int main( int argc, char* argv[] )
{
	int ret;
	
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	int argc = 1;
	char* argv[] = { "<app>" };
#elif defined _WIN32
#define CUSTOM_ARGENC
	int i, totalsize = 0, isDebug = 0;
	char *p;
	LPWSTR *argv_w;
	
	argc = 0;
	argv = NULL;
	
	argv_w = CommandLineToArgvW( GetCommandLineW(), &argc );
	if( !argv_w )
		return -201;
	
	for( i = 0; i < argc; ++i )
		totalsize += WideCharToMultiByte( CP_UTF8, 0, argv_w[i], lstrlenW( argv_w[i] ) + 1, NULL, 0, NULL, NULL );
	
	argv = (char**) malloc( sizeof(*argv) * argc + totalsize );
	p = (char*)( argv + argc );
	
	for( i = 0; i < argc; ++i )
	{
		int size = WideCharToMultiByte( CP_UTF8, 0, argv_w[i], lstrlenW( argv_w[i] ) + 1, p, totalsize, NULL, NULL );
		totalsize -= size;
		argv[ i ] = p;
		if( !strcmp( p, "D" ) )
			isDebug = 1;
		if( !strcmp( p, "DV" ) )
			isDebug = 2;
		p += size;
	}
	
	if( isDebug == 0 )
		FreeConsole();
	
	LocalFree( argv_w );
#endif
	
	ret = SGRX_EntryPoint( argc, argv, isDebug );
	
#ifdef CUSTOM_ARGENC
	free( argv );
#endif
	
	return ret;
}


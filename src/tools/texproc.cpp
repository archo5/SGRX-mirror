

#include "../engine/engine.hpp"
#include "../engine/renderer.hpp"


enum EOpType
{
	OT_Unknown,
	OT_PackTex,
};

const char* findprop( int argc, char* argv[], const char* propname, bool req = false )
{
	for( int i = 1; i < argc - 1; ++i )
	{
		if( !strcmp( argv[ i ], propname ) )
			return argv[ i + 1 ];
	}
	if( req )
	{
		fprintf( stderr, "ERROR: missing required argument (%s)\n", propname );
		exit( 1 );
	}
	return NULL;
}
const char* nullorempty( const char* str )
{
	return str ? str : "";
}


int main( int argc, char* argv[] )
{
	EOpType type = OT_Unknown;
	for( int i = 1; i < argc; ++i )
	{
		if( !strcmp( argv[ i ], "--packtex" ) ){ type = OT_PackTex; break; }
	}
	
	if( type == OT_Unknown )
	{
		fprintf( stderr, "ERROR: unknown operation type\n" );
		exit( 1 );
	}
	
	if( type == OT_PackTex )
	{
		puts( "TEXTURE PROCESSOR: PACK TEXTURE" );
		
		const char* tex_path = findprop( argc, argv, "-i", true );
		const char* fn_out = findprop( argc, argv, "-o", true );
		
		ByteArray filedata;
		if( LoadBinaryFile( tex_path, filedata ) == false )
		{
			fprintf( stderr, "ERROR: failed to read texture file: %s\n", tex_path );
			exit( 1 );
		}
		
		TextureData TD;
		if( TextureData_Load( &TD, filedata, tex_path ) == false )
		{
			fprintf( stderr, "ERROR: failed to parse texture: %s\n", tex_path );
			exit( 1 );
		}
		
		ByteArray outfile;
		outfile.append( "STX\0", 4 );
		outfile.append( &TD.info, sizeof(TD.info) );
		uint32_t datasize = TD.data.size();
		outfile.append( &datasize, 4 );
		outfile.append( TD.data );
		if( SaveBinaryFile( fn_out, outfile.data(), outfile.size() ) == false )
		{
			fprintf( stderr, "ERROR: failed to save texture: %s\n", fn_out );
			exit( 1 );
		}
	}
	
	puts( "finished" );
	return 0;
}



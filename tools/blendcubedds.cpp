
#include "../engine/utils.hpp"
#include "../engine/renderer.hpp"

#include "../engine/dds.c"


int main( int argc, char* argv[] )
{
	puts( "--- convert Blender cubemaps to DDS ---" );
	
	const char* input_file = NULL;
	const char* output_file = NULL;
	
	for( int i = 1; i < argc - 1; ++i )
	{
		if( !strcmp( argv[ i ], "-i" ) ){ input_file = argv[ ++i ]; }
		else if( !strcmp( argv[ i ], "-o" ) ){ output_file = argv[ ++i ]; }
		else
		{
			fprintf( stderr, "unexpected argument: %s\n", argv[ i ] );
			return 1;
		}
	}
	if( !input_file )
	{
		fprintf( stderr, "no input file (-i) specified\n" );
		return 1;
	}
	if( !output_file )
	{
		fprintf( stderr, "no output file (-i) specified\n" );
		return 1;
	}
	
	StringView path = input_file;
	ByteArray ba;
	TextureData TD;
	if( !LoadBinaryFile( path, ba ) )
	{
		fprintf( stderr, "failed to load file: %s\n", input_file );
		return 1;
	}
	if( !TextureData_Load( &TD, ba, path ) )
	{
		fprintf( stderr, "failed to parse file: %s\n", input_file );
		return 1;
	}
	
	if( TD.info.width < 3 || TD.info.height < 2 || TD.info.width / 3 != TD.info.height )
	{
		fprintf( stderr, "invalid dimensions, expecting 3:2\n" );
		return 1;
	}
	
	int WIDTH = TD.info.height / 2;
	
	dds_byte header[ DDS_HEADER_MAX_SIZE ];
	size_t sz = dds_gen_header( header, sizeof(header), true, DDS_FMT_B8G8R8A8, WIDTH, WIDTH, 1, 1 );
	
	ByteArray OUT( header, sz );
	
	// TODO
	
	return 0;
}


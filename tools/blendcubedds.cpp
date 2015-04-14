
#include "../engine/utils.hpp"
#include "../engine/renderer.hpp"

#include "../engine/dds.c"


// DDS: The faces are written in the order: positive x, negative x, positive y, negative y, positive z, negative z
// Blender:  0;0=-X  1;0=-Z  2;0=+X  0;1=-Y  1;1=+Y  2;1=+Z

struct cubeface
{
	int img_x;
	int img_y;
};
cubeface faces[6] =
{
	{2,0},
	{0,0},
	{1,1},
	{0,1},
	{2,1},
	{1,0},
};


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
	
	if( TD.info.width < 3 || TD.info.height < 2 || TD.info.width / 3 != TD.info.height / 2 )
	{
		fprintf( stderr, "invalid dimensions, expecting 3:2\n" );
		return 1;
	}
	
	int WIDTH = TD.info.height / 2;
	
	dds_byte header[ DDS_HEADER_MAX_SIZE ];
	size_t dds_sz = dds_gen_header( header, sizeof(header), true, DDS_FMT_B8G8R8A8, WIDTH, WIDTH, 1, 1 );
	
	ByteArray OUT( header, dds_sz );
	
	size_t off = TextureData_GetMipDataOffset( &TD.info, 0, 0 );
	size_t sz = TextureData_GetMipDataSize( &TD.info, 0 );
	if( sz == 0 )
	{
		fprintf( stderr, "failed to get mip size\n" );
		return 1;
	}
	
	if( size_t(WIDTH * WIDTH * 4) != sz / 6 )
	{
		fprintf( stderr, "unsupported input format\n" );
		return 1;
	}
	
	size_t dstrow_stride = WIDTH * 4;
	size_t srcrow_stride = dstrow_stride * 3;
	for( int i = 0; i < 6; ++i )
	{
		cubeface CF = faces[i];
		size_t img_off = off + srcrow_stride * WIDTH * CF.img_y + dstrow_stride * CF.img_x;
		for( int y = 0; y < WIDTH; ++y )
		{
			OUT.append( &TD.data[ img_off ], dstrow_stride );
			if( TD.info.format == DDS_FMT_R8G8B8A8 )
			{
				for( int x = 0; x < WIDTH; ++x )
				{
					TSWAP( OUT[ OUT.size() - dstrow_stride + x * 4 ], OUT[ OUT.size() - dstrow_stride + x * 4 + 2 ] );
				}
			}
			img_off += srcrow_stride;
		}
	}
	
	TextureData TDVER;
	if( !TextureData_Load( &TDVER, OUT, output_file ) )
	{
		fprintf( stderr, "failed to validate output file\n" );
		return 1;
	}
	
	SaveBinaryFile( output_file, OUT.data(), OUT.size() );
	
	puts( "Done!" );
	
	return 0;
}


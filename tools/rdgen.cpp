

#define USE_ARRAY
#include "../engine/renderer.hpp"
#include "../engine/enganim.hpp"


template< class T > struct FreeOnEnd {};
template<> struct FreeOnEnd< FILE* >
{
	FreeOnEnd( FILE* p ) : m_p( p ){}
	~FreeOnEnd(){ if( m_p ) fclose( m_p ); }
	FILE* m_p;
};
#define AUTOFREE( T, p, iv ) T p = iv; FreeOnEnd<T> autofree_##p( p )


struct AMVertex
{
	Vec3 pos;
	uint8_t weights[4];
	uint8_t indices[4];
};


int main( int argc, char* argv[] )
{
	puts( "--- ragdoll generator ---" );
	
	const char* input_file = NULL;
	
	for( int i = 1; i < argc - 1; ++i )
	{
		if( !strcmp( argv[ i ], "-i" ) ){ input_file = argv[ ++i ]; }
		else
		{
			fprintf( stderr, "unexpected argument: %s\n", argv[ i ] );
			return 1;
		}
	}
	if( !input_file ){ fprintf( stderr, "no input file (-i) specified" ); return 1; }
	
	Array< AMVertex > mesh_vertex_data;
	MeshFileData mesh_file_data;
	
	AUTOFREE( FILE*, fp, fopen( input_file, "r" ) );
	while( !feof( fp ) )
	{
		char type[ 9 ] = {0};
		if( fscanf( fp, "%8s", type ) < 1 )
			break;
		
		if( !strcmp( type, "MESH" ) )
		{
			printf( "reading MESH... " );
			
			char mesh_path[ 1025 ] = {0};
			
			if( fscanf( fp, "%1024s", mesh_path ) < 1 )
			{
				perror( "failed to read MESH path" );
				return 1;
			}
			
			ByteArray mesh_data;
			if( !LoadBinaryFile( mesh_path, mesh_data ) )
			{
				fprintf( stderr, "failed to read MESH from file: %s\n", mesh_path );
				return 1;
			}
			printf( "file loaded" );
			
			const char* mesh_load_error = MeshData_Parse( (char*) mesh_data.data(), mesh_data.size(), &mesh_file_data );
			if( mesh_load_error )
			{
				fprintf( stderr, "failed to parse MESH - %s (file: %s)\n", mesh_load_error, mesh_path );
				return 1;
			}
			printf( ", mesh parsed" );
			
			VDeclInfo vertex_decl;
			const char* vdecl_load_error = VDeclInfo_Parse( &vertex_decl, StackString< 256 >( StringView( mesh_file_data.formatData, mesh_file_data.formatSize ) ) );
			if( vdecl_load_error )
			{
				fprintf( stderr, "failed to parse MESH vertex format - %s (file: %s)\n", vdecl_load_error, mesh_path );
				return 1;
			}
			printf( ", vdecl parsed" );
			
			int p_off = -1, bw_off = -1, bi_off = -1;
			for( int i = 0; i < (int) vertex_decl.count; ++i )
			{
				if( vertex_decl.usages[ i ] == VDECLUSAGE_POSITION )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 ){ fprintf( stderr, "MESH position data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
					p_off = vertex_decl.offsets[ i ];
				}
				else if( vertex_decl.usages[ i ] == VDECLUSAGE_BLENDWT )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_BCOL4 ){ fprintf( stderr, "MESH blend weight data not UINT8[4] (file: %s)\n", mesh_path ); return 1; }
					bw_off = vertex_decl.offsets[ i ];
				}
				else if( vertex_decl.usages[ i ] == VDECLUSAGE_BLENDIDX )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_BCOL4 ){ fprintf( stderr, "MESH blend index data not UINT8[4] (file: %s)\n", mesh_path ); return 1; }
					bi_off = vertex_decl.offsets[ i ];
				}
			}
			if( p_off < 0 ){ fprintf( stderr, "MESH has no position data (file: %s)\n", mesh_path ); return 1; }
			if( bw_off < 0 ){ fprintf( stderr, "MESH has no blend weight data (file: %s)\n", mesh_path ); return 1; }
			if( bi_off < 0 ){ fprintf( stderr, "MESH has no blend index data (file: %s)\n", mesh_path ); return 1; }
			
			printf( ", validated" );
			
			size_t vcount = mesh_file_data.vertexDataSize / vertex_decl.size;
			mesh_vertex_data.resize( vcount );
			for( size_t i = 0; i < vcount; ++i )
			{
				char* vbase = mesh_file_data.vertexData + i * vertex_decl.size;
				AMVertex V = { *(Vec3*) ( vbase + p_off ) };
				memcpy( V.weights, vbase + bw_off, 4 );
				memcpy( V.indices, vbase + bi_off, 4 );
				mesh_vertex_data.push_back( V );
			}
			printf( ", vertex data extracted" );
			
			puts( " - done" );
		}
		else
		{
			fprintf( stderr, "unrecognized command: %s\n", type );
			return 1;
		}
	}
}


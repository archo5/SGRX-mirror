

#define USE_QUAT
#define USE_MAT4
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

struct AMBone
{
	String name;
	Mat4 boneOffset;
	Mat4 skinOffset;
	Mat4 invSkinOffset;
	int parent_id;
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
	Array< AMBone > mesh_bones;
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
			
			if( mesh_file_data.numBones == 0 )
			{
				fprintf( stderr, "MESH error - no bones (file: %s)\n", mesh_path );
				return 1;
			}
			
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
			
			for( uint8_t i = 0; i < mesh_file_data.numBones; ++i )
			{
				AMBone B =
				{
					StringView( mesh_file_data.bones[ i ].boneName, mesh_file_data.bones[ i ].boneNameSize ),
					mesh_file_data.bones[ i ].boneOffset,
					mesh_file_data.bones[ i ].parent_id == 255 ? -1 : mesh_file_data.bones[ i ].parent_id
				};
				mesh_bones.push_back( B );
			}
			printf( ", bone data extracted" );
			
			int numBones = mesh_bones.size();
			for( int b = 0; b < numBones; ++b )
			{
				if( mesh_bones[ b ].parent_id < -1 || mesh_bones[ b ].parent_id >= b )
				{
					fprintf( stderr, "RecalcBoneMatrices: each parent_id must point to a previous bone or no bone (-1) [error in bone %d: %d]", b, mesh_bones[ b ].parent_id );
					return 1;
				}
			}
			
			Mat4 skinOffsets[ MAX_MESH_BONES ];
			for( int b = 0; b < numBones; ++b )
			{
				if( mesh_bones[ b ].parent_id >= 0 )
					skinOffsets[ b ].Multiply( mesh_bones[ b ].boneOffset, skinOffsets[ mesh_bones[ b ].parent_id ] );
				else
					skinOffsets[ b ] = mesh_bones[ b ].boneOffset;
				mesh_bones[ b ].skinOffset = skinOffsets[ b ];
			}
			for( int b = 0; b < numBones; ++b )
			{
				if( !skinOffsets[ b ].InvertTo( mesh_bones[ b ].invSkinOffset ) )
				{
					fprintf( stderr, "RecalcBoneMatrices: failed to invert skin offset matrix #%d\n", b );
					mesh_bones[ b ].invSkinOffset.SetIdentity();
				}
			}
			
			puts( " - done" );
		}
		else if( !strcmp( type, "ANIM" ) )
		{
			printf( "reading ANIM... " );
			
			char anim_path[ 1025 ] = {0};
			char anim_name[ 256 ] = {0};
			int anim_frame = -1;
			
			// READ COMMAND
			if( fscanf( fp, "%1024s %255s %d", anim_path, anim_name, &anim_frame ) < 3 )
			{
				perror( "failed to read ANIM path" );
				return 1;
			}
			
			// LOAD ANIMATION FILE
			ByteArray ba;
			if( !LoadBinaryFile( anim_path, ba ) )
			{
				fprintf( stderr, "Failed to load animation file: %s\n", anim_path );
				return 1;
			}
			AnimFileParser afp( ba );
			if( afp.error )
			{
				fprintf( stderr, "Failed to parse animation file (%s) - %s\n", anim_path, afp.error );
				return 1;
			}
			
			// FIND ANIMATION
			size_t animID = 0;
			for( ; animID < afp.animData.size(); ++animID )
			{
				if( !strcmp( afp.animData[ animID ].name, anim_name ) )
					break;
			}
			if( animID == afp.animData.size() )
			{
				fprintf( stderr, "Animation '%s' was not found in file '%s'\n", anim_path, anim_name );
				return 1;
			}
			AnimFileParser::Anim* anim = &afp.animData[ animID ];
			
			if( anim_frame < 0 || anim_frame >= (int) anim->frameCount )
			{
				fprintf( stderr, "ANIM frame (%d) out of bounds (0-%d)\n", anim_frame, (int) anim->frameCount - 1 );
				return 1;
			}
			printf( ", animation is loaded" );
			
			float* trackData[ MAX_MESH_BONES ] = {0};
			for( size_t i = 0; i < mesh_bones.size(); ++i )
			{
				for( uint8_t t = 0; t < anim->trackCount; ++t )
				{
					AnimFileParser::Track& T = afp.trackData[ anim->trackDataOff + t ];
					if( StringView( T.name, T.nameSize ) == mesh_bones[ i ].name )
					{
						trackData[ i ] = T.dataPtr;
						break;
					}
				}
			}
			
			Mat4 matrices[ MAX_MESH_BONES ];
			// apply animation
			for( size_t i = 0; i < mesh_bones.size(); ++i )
			{
				float* T = trackData[ i ];
				Vec3 pos = T ? V3( T[ anim_frame * 10 + 0 ], T[ anim_frame * 10 + 1 ], T[ anim_frame * 10 + 2 ] ) : V3(0);
				Quat rot = T ? QUAT( T[ anim_frame * 10 + 3 ], T[ anim_frame * 10 + 4 ], T[ anim_frame * 10 + 5 ], T[ anim_frame * 10 + 6 ] ) : Quat::Identity;
				Vec3 scl = T ? V3( T[ anim_frame * 10 + 7 ], T[ anim_frame * 10 + 8 ], T[ anim_frame * 10 + 9 ] ) : V3(0);
				Mat4& M = matrices[ i ];
				M = Mat4::CreateSRT( scl, rot, pos ) * mesh_bones[ i ].boneOffset;
				if( mesh_bones[ i ].parent_id >= 0 )
					M = M * matrices[ mesh_bones[ i ].parent_id ];
			}
			for( size_t i = 0; i < mesh_bones.size(); ++i )
			{
				Mat4& M = matrices[ i ];
				M = mesh_bones[ i ].invSkinOffset * M;
			}
			
			puts( " - done" );
		}
		else if( !strcmp( type, "REASSIGN" ) )
		{
		}
		else if( !strcmp( type, "JOINT" ) )
		{
		}
		else if( !strcmp( type, "BODY" ) )
		{
		}
		else if( !strcmp( type, "OUT" ) )
		{
		}
		else
		{
			fprintf( stderr, "unrecognized command: %s\n", type );
			return 1;
		}
	}
}


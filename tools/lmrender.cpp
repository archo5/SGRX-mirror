

#define USE_VEC3
#define USE_ARRAY
#define USE_HASHTABLE
#include "../engine/utils.hpp"
#include "../engine/renderer.hpp"
#include "../engine/script.hpp"

#include "../ext/src/libpng/png.h"
#include "../ext/src/lighter/lighter.h"


#define MAX_HARD_LIGTHMAP_SIZE 8192


template< class T > struct FreeOnEnd {};
template<> struct FreeOnEnd< FILE* >
{
	FreeOnEnd( FILE* p ) : m_p( p ){}
	~FreeOnEnd(){ if( m_p ) fclose( m_p ); }
	FILE* m_p;
};
template<> struct FreeOnEnd< ltr_Scene* >
{
	FreeOnEnd( ltr_Scene* p ) : m_p( p ){}
	~FreeOnEnd(){ if( m_p ) ltr_DestroyScene( m_p ); }
	ltr_Scene* m_p;
};
#define AUTOFREE( T, p, iv ) T p = iv; FreeOnEnd<T> autofree_##p( p )


static FINLINE float cvcompress( float x )
{
	return x;
}


int dumpimg( const char* filename, float* buffer, int width, int height )
{
	int code = 0;
	float* rgb;
	png_structp png_ptr;
	png_infop info_ptr;
	
	// Open file for writing (binary mode)
	AUTOFREE( FILE*, fp, fopen( filename, "wb" ) );
	if( fp == NULL )
	{
		fprintf( stderr, "Could not open file %s for writing\n", filename );
		return 1;
	}
	// Initialize write structure
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( png_ptr == NULL )
	{
		fprintf( stderr, "Could not allocate write struct\n" );
		return 1;
	}
	
	// Initialize info structure
	info_ptr = png_create_info_struct( png_ptr );
	if( info_ptr == NULL )
	{
		fprintf( stderr, "Could not allocate info struct\n" );
		return 1;
	}
	// Setup Exception handling
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		fprintf( stderr, "Error during png creation\n" );
		code = 1;
		goto fail;
	}
	png_init_io( png_ptr, fp );
	
	// Write header (8 bit colour depth)
	png_set_IHDR( png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
	
	png_write_info( png_ptr, info_ptr );
	// Allocate memory for one row (3 bytes per pixel - RGB)
	
	png_byte row[ MAX_HARD_LIGTHMAP_SIZE * 3 ];
	
	// Write image data
	rgb = buffer;
	for( int y = 0; y < height; ++y )
	{
		png_bytep prow = row;
		for( int x = 0; x < width; ++x )
		{
			prow[ 0 ] = clamp( cvcompress( rgb[ 0 ] ), 0, 1 ) * 255.0f;
			prow[ 1 ] = clamp( cvcompress( rgb[ 1 ] ), 0, 1 ) * 255.0f;
			prow[ 2 ] = clamp( cvcompress( rgb[ 2 ] ), 0, 1 ) * 255.0f;
			prow += 3;
			rgb += 3;
		}
		png_write_row( png_ptr, row );
	}
	
	// End write
	png_write_end( png_ptr, NULL );
	
fail:
	if( info_ptr != NULL ) png_free_data( png_ptr, info_ptr, PNG_FREE_ALL, -1 );
	if( png_ptr != NULL ) png_destroy_write_struct( &png_ptr, (png_infopp) NULL );
	return code;
}


Vec3 GetColorFromTexture( const StringView& path )
{
	ByteArray ba;
	TextureData TD;
	if( LoadBinaryFile( path, ba ) && TextureData_Load( &TD, ba, path ) )
	{
		// TODO
	}
	return V3(1);
}

struct MeshPartKey
{
	String meshPath;
	uint32_t partID;
	
	bool operator == ( const MeshPartKey& o ) const
	{
		return meshPath == o.meshPath && partID == o.partID;
	}
};
Hash HashVar( const MeshPartKey& mpk )
{
	return HashVar( mpk.meshPath ) ^ HashVar( mpk.partID );
}

HashTable< String, Vec3 > g_TexSamples;
HashTable< MeshPartKey, String > g_MeshPartTextures;

LTRBOOL Radiosity_Sample_Fn( ltr_Config* config, ltr_SampleRequest* req )
{
	MeshPartKey mpk = { String( req->mesh_ident, req->mesh_ident_size ), req->part_id };
	String* texname = g_MeshPartTextures.getptr( mpk );
	if( !texname )
		return 0;
	
	Vec3* texsample = g_TexSamples.getptr( *texname );
	if( !texsample )
		return 0;
	
	req->out_diffuse_color[0] = texsample->x;
	req->out_diffuse_color[1] = texsample->y;
	req->out_diffuse_color[2] = texsample->z;
	return 1;
}


int main( int argc, char* argv[] )
{
	puts( "--- lightmap renderer ---" );
	AUTOFREE( ltr_Scene*, scene, ltr_CreateScene() );
	
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
	if( !input_file )
	{
		fprintf( stderr, "no input file (-i) specified" );
		return 1;
	}
	
	ltr_Config scene_config;
	ltr_GetConfig( &scene_config, scene );
	scene_config.sample_fn = Radiosity_Sample_Fn;
	
	ltr_Mesh* last_mesh = NULL;
	
	AUTOFREE( FILE*, fp, fopen( input_file, "r" ) );
	if( !fp )
	{
		perror( "failed to open input file" );
		fprintf( stderr, "file: %s\n", input_file );
		return 1;
	}
	
	ScriptContext m_scriptCtx;
	sgsVariable m_textureInfo;
	
	char sample_output_file[ 256 ] = {0};
	
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
			
			MeshFileData mf_data;
			const char* mesh_load_error = MeshData_Parse( (char*) mesh_data.data(), mesh_data.size(), &mf_data );
			if( mesh_load_error )
			{
				fprintf( stderr, "failed to parse MESH - %s (file: %s)\n", mesh_load_error, mesh_path );
				return 1;
			}
			printf( ", mesh parsed" );
			
			VDeclInfo vertex_decl;
			const char* vdecl_load_error = VDeclInfo_Parse( &vertex_decl, StackString< 256 >( StringView( mf_data.formatData, mf_data.formatSize ) ) );
			if( vdecl_load_error )
			{
				fprintf( stderr, "failed to parse MESH vertex format - %s (file: %s)\n", vdecl_load_error, mesh_path );
				return 1;
			}
			printf( ", vdecl parsed" );
			
			int p_off = -1, n_off = -1, t0_off = -1, t1_off = -1;
			for( int i = 0; i < (int) vertex_decl.count; ++i )
			{
				if( vertex_decl.usages[ i ] == VDECLUSAGE_POSITION )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 ){ fprintf( stderr, "MESH position data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
					p_off = vertex_decl.offsets[ i ];
				}
				else if( vertex_decl.usages[ i ] == VDECLUSAGE_NORMAL )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 ){ fprintf( stderr, "MESH normal data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
					n_off = vertex_decl.offsets[ i ];
				}
				else if( vertex_decl.usages[ i ] == VDECLUSAGE_TEXTURE0 )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT2 ){ fprintf( stderr, "MESH texcoord_0 data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
					t0_off = vertex_decl.offsets[ i ];
				}
				else if( vertex_decl.usages[ i ] == VDECLUSAGE_TEXTURE1 )
				{
					if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT2 ){ fprintf( stderr, "MESH texcoord_1 data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
					t1_off = vertex_decl.offsets[ i ];
				}
			}
			if( p_off < 0 ){ fprintf( stderr, "MESH has no position data (file: %s)\n", mesh_path ); return 1; }
			if( n_off < 0 ){ fprintf( stderr, "MESH has no normal data (file: %s)\n", mesh_path ); return 1; }
			if( t0_off < 0 && t1_off < 0 ){ fprintf( stderr, "MESH has no texture coordinate data (file: %s)\n", mesh_path ); return 1; }
			if( t0_off < 0 ) t0_off = t1_off;
			else if( t1_off < 0 ) t1_off = t0_off;
			
			printf( ", validated" );
			
			Array< uint32_t > mesh_indices;
			if( mf_data.dataFlags & MDF_INDEX_32 )
			{
				mesh_indices.assign( (uint32_t*) mf_data.indexData, mf_data.indexDataSize / sizeof( uint32_t ) );
			}
			else
			{
				mesh_indices.resize( mf_data.indexDataSize / sizeof( uint16_t ) );
				for( uint32_t i = 0; i < mf_data.indexDataSize / sizeof( uint16_t ); ++i )
					mesh_indices[ i ] = *( (uint16_t*) mf_data.indexData + i );
			}
			
			printf( ", indices converted" );
			
			last_mesh = ltr_CreateMesh( scene, mesh_path, strlen( mesh_path ) );
			
			printf( ", reading parts..." );
			
			for( int i = 0; i < (int) mf_data.numParts; ++i )
			{
				const MeshFilePartData& mfpd = mf_data.parts[ i ];
				if( mfpd.indexCount == 0 || mfpd.vertexCount == 0 )
				{
					fprintf( stderr, "[WARNING: mesh has an empty part %d]", i );
					continue;
				}
				
				StringView texname = mfpd.materialTextureCount ? StringView( mfpd.materialStrings[1], mfpd.materialStringSizes[1] ) : StringView();
				String texname_norm = String_Replace( texname, "\\", "/" );
				if( !g_TexSamples.getptr( texname ) )
					g_TexSamples[ texname ] = GetColorFromTexture( texname );
				MeshPartKey mpk = { mesh_path, i };
				g_MeshPartTextures[ mpk ] = texname;
				
				ltr_MeshPartInfo mpinfo =
				{
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + p_off ), // POSITION
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + n_off ), // NORMAL
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + t0_off ), // TEXCOORD0
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + t1_off ), // TEXCOORD1
					vertex_decl.size, vertex_decl.size, vertex_decl.size, vertex_decl.size, // stride
					&mesh_indices[ mfpd.indexOffset ], mfpd.vertexCount, mfpd.indexCount, mf_data.dataFlags & MDF_TRIANGLESTRIP != 0 ? 1 : 0,
					!m_textureInfo.getprop( m_scriptCtx.CreateStringVar( texname_norm ) ).getprop( "noshadow" ).get<bool>()
				};
				
				if( !ltr_MeshAddPart( last_mesh, &mpinfo ) )
				{
					fprintf( stderr, "failed to add MESH part %d (file: %s)\n", i, mesh_path );
					return 1;
				}
				
				printf( " -%d", i );
			}
			
			puts( " - done" );
		}
		else if( !strcmp( type, "INST" ) )
		{
			puts( "reading INST" );
			
			char lightmap_name[ 1025 ] = {0}; // internal copy is made
			ltr_MeshInstanceInfo mi_info =
			{
				{ { 1, 0, 0, 0 },
				  { 0, 1, 0, 0 },
				  { 0, 0, 1, 0 },
				  { 0, 0, 0, 1 } },
				1.0f, 1, lightmap_name, 0
			};
			
			if( !last_mesh )
			{
				fprintf( stderr, "INST used before MESH\n" );
				return 1;
			}
			
			char key[ 65 ] = {0};
			for(;;)
			{
				if( fscanf( fp, "%64s", key ) < 1 )
				{
					perror( "failed to read the INST key" );
					return 1;
				}
				
				if( !strcmp( key, "END" ) )
					break;
				else if( !strcmp( key, "MESH" ) || !strcmp( key, "INST" ) || !strcmp( key, "LIGHT" ) || !strcmp( key, "SAMPLE" ) || !strcmp( key, "CONFIG" ) )
				{
					fprintf( stderr, "INST: missing 'END' of parameters\n" );
					return 1;
				}
				else if( !strcmp( key, "lightmap" ) )
				{
					if( fscanf( fp, "%1024s", lightmap_name ) < 1 )
					{
						perror( "INST: failed to read lightmap name" );
						return 1;
					}
				}
				else if( !strcmp( key, "importance" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { mi_info.importance = v; } else { perror( "INST: failed to read importance" ); return 1; } }
				else if( !strcmp( key, "shadow" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { mi_info.shadow = !!v; } else { perror( "INST: failed to read shadow" ); return 1; } }
				else if( !strcmp( key, "matrix" ) )
				{
					if( fscanf( fp, "%f %f %f %f  %f %f %f %f  %f %f %f %f",
						&mi_info.matrix[0][0], &mi_info.matrix[1][0], &mi_info.matrix[2][0], &mi_info.matrix[3][0],
						&mi_info.matrix[0][1], &mi_info.matrix[1][1], &mi_info.matrix[2][1], &mi_info.matrix[3][1],
						&mi_info.matrix[0][2], &mi_info.matrix[1][2], &mi_info.matrix[2][2], &mi_info.matrix[3][2] ) < 12 )
					{
						perror( "INST: failed to read matrix" );
						return 1;
					}
				}
				else { fprintf( stderr, "unrecognized INST key: %s\n", key ); return 1; }
			}
			
			if( !lightmap_name[0] )
			{
				fprintf( stderr, "INST: lightmap name is not set\n" );
				return 1;
			}
			mi_info.ident_size = strlen( mi_info.ident );
			
			ltr_MeshAddInstance( last_mesh, &mi_info );
		}
		else if( !strcmp( type, "LIGHT" ) )
		{
			puts( "reading LIGHT" );
			
			ltr_LightInfo light_info = {0};
			
			if( fscanf( fp, "%8s", type ) < 1 )
			{
				perror( "failed to read light type" );
				return 1;
			}
			
			if( !strcmp( type, "POINT" ) ) light_info.type = LTR_LT_POINT;
			else if( !strcmp( type, "SPOT" ) ) light_info.type = LTR_LT_SPOT;
			else if( !strcmp( type, "DIRECT" ) ) light_info.type = LTR_LT_DIRECT;
			else
			{
				fprintf( stderr, "unrecognized light type: %s\n", type );
				return 1;
			}
			
			char key[ 65 ] = {0};
			for(;;)
			{
				if( fscanf( fp, "%64s", key ) < 1 )
				{
					perror( "failed to read the LIGHT key" );
					return 1;
				}
				
				if( !strcmp( key, "END" ) )
					break;
				else if( !strcmp( key, "MESH" ) || !strcmp( key, "INST" ) || !strcmp( key, "LIGHT" ) || !strcmp( key, "SAMPLE" ) || !strcmp( key, "CONFIG" ) )
				{
					fprintf( stderr, "LIGHT: missing 'END' of parameters\n" );
					return 1;
				}
				else if( !strcmp( key, "position" ) )
				{
					ltr_VEC3 vec; if( fscanf( fp, "%f %f %f", &vec[0], &vec[1], &vec[2] ) == 3 ) { memcpy( light_info.position, vec, sizeof(vec) ); } else { perror( "LIGHT: failed to read position" ); return 1; }
				}
				else if( !strcmp( key, "direction" ) )
				{
					ltr_VEC3 vec; if( fscanf( fp, "%f %f %f", &vec[0], &vec[1], &vec[2] ) == 3 ) { memcpy( light_info.direction, vec, sizeof(vec) ); } else { perror( "LIGHT: failed to read direction" ); return 1; }
				}
				else if( !strcmp( key, "up_direction" ) )
				{
					ltr_VEC3 vec; if( fscanf( fp, "%f %f %f", &vec[0], &vec[1], &vec[2] ) == 3 ) { memcpy( light_info.up_direction, vec, sizeof(vec) ); } else { perror( "LIGHT: failed to read up_direction" ); return 1; }
				}
				else if( !strcmp( key, "color" ) )
				{
					ltr_VEC3 vec; if( fscanf( fp, "%f %f %f", &vec[0], &vec[1], &vec[2] ) == 3 ) { memcpy( light_info.color_rgb, vec, sizeof(vec) ); } else { perror( "LIGHT: failed to read color" ); return 1; }
				}
				else if( !strcmp( key, "range" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.range = v; } else { perror( "LIGHT: failed to read range" ); return 1; } }
				else if( !strcmp( key, "power" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.power = v; } else { perror( "LIGHT: failed to read power" ); return 1; } }
				else if( !strcmp( key, "light_radius" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.light_radius = v; } else { perror( "LIGHT: failed to read light_radius" ); return 1; } }
				else if( !strcmp( key, "shadow_sample_count" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { light_info.shadow_sample_count = v; } else { perror( "LIGHT: failed to read shadow_sample_count" ); return 1; } }
				else if( !strcmp( key, "spot_angle_out" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.spot_angle_out = v; } else { perror( "LIGHT: failed to read spot_angle_out" ); return 1; } }
				else if( !strcmp( key, "spot_angle_in" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.spot_angle_in = v; } else { perror( "LIGHT: failed to read spot_angle_in" ); return 1; } }
				else if( !strcmp( key, "spot_curve" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { light_info.spot_curve = v; } else { perror( "LIGHT: failed to read spot_curve" ); return 1; } }
				else { fprintf( stderr, "unrecognized LIGHT key: %s\n", key ); return 1; }
			}
			
			ltr_LightAdd( scene, &light_info );
		}
		else if( !strcmp( type, "SAMPLE" ) )
		{
			puts( "reading SAMPLE" );
			
			ltr_VEC3 pos;
			// easy to remember order: X,Y,Z / +,-
			ltr_VEC3 sdirs[6] = { {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };
			if( fscanf( fp, "%f %f %f", &pos[0], &pos[1], &pos[2] ) == 3 )
			{
				ltr_SampleInfo sampleinfo;
				memset( &sampleinfo, 0, sizeof(sampleinfo) );
				memcpy( sampleinfo.position, pos, sizeof(pos) );
				for( int i = 0; i < 6; ++i )
				{
					memcpy( sampleinfo.normal, sdirs[i], sizeof(ltr_VEC3) );
					ltr_SampleAdd( scene, &sampleinfo );
				}
			}
			else
			{
				perror( "SAMPLE: failed to read sample data" );
				return 1;
			}
		}
		else if( !strcmp( type, "CONFIG" ) )
		{
			puts( "reading CONFIG" );
			
			char key[ 65 ] = {0};
			if( fscanf( fp, "%64s", key ) < 1 )
			{
				perror( "failed to read the CONFIG key" );
				return 1;
			}
			
			if(0);
			// limits & factors
			else if( !strcmp( key, "max_tree_memory" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.max_tree_memory = v; } else { perror( "CONFIG: failed to read max_tree_memory" ); return 1; } }
			else if( !strcmp( key, "max_lightmap_size" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.max_lightmap_size = v; } else { perror( "CONFIG: failed to read max_lightmap_size" ); return 1; } }
			else if( !strcmp( key, "default_width" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.default_width = v; } else { perror( "CONFIG: failed to read default_width" ); return 1; } }
			else if( !strcmp( key, "default_height" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.default_height = v; } else { perror( "CONFIG: failed to read default_height" ); return 1; } }
			else if( !strcmp( key, "global_size_factor" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.global_size_factor = v; } else { perror( "CONFIG: failed to read global_size_factor" ); return 1; } }
			// lightmap generation
			else if( !strcmp( key, "clear_color" ) )
			{
				ltr_VEC3 col; if( fscanf( fp, "%f %f %f", &col[0], &col[1], &col[2] ) == 3 ) { memcpy( scene_config.clear_color, col, sizeof(col) ); } else { perror( "CONFIG: failed to read clear_color" ); return 1; }
			}
			else if( !strcmp( key, "ambient_color" ) )
			{
				ltr_VEC3 col; if( fscanf( fp, "%f %f %f", &col[0], &col[1], &col[2] ) == 3 ) { memcpy( scene_config.ambient_color, col, sizeof(col) ); } else { perror( "CONFIG: failed to read ambient_color" ); return 1; }
			}
			// RADIOSITY effect
			else if( !strcmp( key, "bounce_count" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.bounce_count = v; } else { perror( "CONFIG: failed to read bounce_count" ); return 1; } }
			// AMBIENT OCCLUSION effect
			else if( !strcmp( key, "ao_distance" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.ao_distance = v; } else { perror( "CONFIG: failed to read ao_distance" ); return 1; } }
			else if( !strcmp( key, "ao_multiplier" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.ao_multiplier = v; } else { perror( "CONFIG: failed to read ao_multiplier" ); return 1; } }
			else if( !strcmp( key, "ao_falloff" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.ao_falloff = v; } else { perror( "CONFIG: failed to read ao_falloff" ); return 1; } }
			else if( !strcmp( key, "ao_effect" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.ao_effect = v; } else { perror( "CONFIG: failed to read ao_effect" ); return 1; } }
			else if( !strcmp( key, "ao_divergence" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.ao_divergence = v; } else { perror( "CONFIG: failed to read ao_divergence" ); return 1; } }
			else if( !strcmp( key, "ao_color" ) )
			{
				ltr_VEC3 col; if( fscanf( fp, "%f %f %f", &col[0], &col[1], &col[2] ) == 3 ) { memcpy( scene_config.ao_color_rgb, col, sizeof(col) ); } else { perror( "CONFIG: failed to read ao_color" ); return 1; }
			}
			else if( !strcmp( key, "ao_num_samples" ) ){ int v = 0; if( fscanf( fp, "%d", &v ) == 1 ) { scene_config.ao_num_samples = v; } else { perror( "CONFIG: failed to read ao_num_samples" ); return 1; } }
			// GAUSSIAN BLUR effect
			else if( !strcmp( key, "blur_size" ) ){ float v = 0; if( fscanf( fp, "%f", &v ) == 1 ) { scene_config.blur_size = v; } else { perror( "CONFIG: failed to read blur_size" ); return 1; } }
			// SAMPLE output file
			else if( !strcmp( key, "samples_out" ) ){ if( fscanf( fp, "%255s", sample_output_file ) == 1 ) { ; } else { perror( "CONFIG: failed to read samples_out" ); return 1; } }
			// TEXTURE specification file
			else if( !strcmp( key, "texture_spec" ) )
			{
				char tex_spec_file[ 256 ] = {0};
				if( fscanf( fp, "%255s", tex_spec_file ) == 1 )
				{
					if( !m_scriptCtx.EvalFile( tex_spec_file, &m_textureInfo ) )
					{
						fprintf( stderr, "CONFIG: failed to read from texture_spec file\n" );
						return 1;
					}
				}
				else { perror( "CONFIG: failed to read texture_spec" ); return 1; }
			}
			// -
			else { fprintf( stderr, "unrecognized CONFIG key: %s\n", key ); return 1; }
		}
		else
		{
			fprintf( stderr, "unrecognized command: %s\n", type );
			return 1;
		}
	}
	
	if( scene_config.max_lightmap_size > MAX_HARD_LIGTHMAP_SIZE )
	{
		fprintf( stderr, "max. lightmap size exceeds %d\n", MAX_HARD_LIGTHMAP_SIZE );
		return 1;
	}
	
	ltr_SetConfig( scene, &scene_config );
	puts( "configuration successfully parsed" );
	
	if( !last_mesh )
	{
		puts( "nothing to do" );
		return 0;
	}
	
	// --- DO WORK ---
	ltr_WorkInfo winfo;
	while( ltr_DoWork( scene, &winfo ) == 0 )
	{
		printf( "%s [%d/%d] ... %d%%\n", winfo.stage, (int) winfo.part,
			(int) winfo.item_count, (int) ( winfo.part * 100 / TMAX( winfo.item_count, 1 ) ) );
	}
	
	// --- RETURN OUTPUT ---
	ltr_WorkOutput wout;
	ltr_WorkOutputInfo woutinfo;
	ltr_GetWorkOutputInfo( scene, &woutinfo );
	if( *sample_output_file )
	{
		ByteArray ba;
		ByteWriter bw( &ba );
		uint32_t sample_count = woutinfo.sample_count / 6;
		bw << sample_count;
		for( uint32_t i = 0; i < sample_count; ++i )
		{
			Vec3 v;
			v = Vec3::CreateFromPtr( woutinfo.samples[ i * 6 ].position );
			bw << v;
			for( int s = 0; s < 6; ++s )
			{
				v = Vec3::CreateFromPtr( woutinfo.samples[ i * 6 + s ].out_color );
				bw << v;
			}
		}
		if( !SaveBinaryFile( sample_output_file, ba.data(), ba.size() ) )
		{
			fprintf( stderr, "Failed to save sample file to %s\n", sample_output_file );
			return 1;
		}
	}
	for( u32 lm = 0; lm < woutinfo.lightmap_count; ++lm )
	{
		if( !ltr_GetWorkOutput( scene, lm, &wout ) )
		{
			fprintf( stderr, "Failed to get work output for lightmap %d\n", (int) lm );
			return 1;
		}
		dumpimg( wout.inst_ident, wout.lightmap_rgb, wout.width, wout.height );
	}
	
	puts( "finished" );
	return 0;
}


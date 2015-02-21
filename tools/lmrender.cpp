

#define USE_ARRAY
#include "../engine/utils.hpp"
#include "../engine/renderer.hpp"

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
			prow[ 0 ] = rgb[ 0 ] * 255.0f;
			prow[ 1 ] = rgb[ 1 ] * 255.0f;
			prow[ 2 ] = rgb[ 2 ] * 255.0f;
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
	
	ltr_Mesh* last_mesh = NULL;
	
	AUTOFREE( FILE*, fp, fopen( input_file, "r" ) );
	if( !fp )
	{
		perror( "failed to open input file" );
		fprintf( stderr, "file: %s\n", input_file );
		return 1;
	}
	
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
				
				ltr_MeshPartInfo mpinfo =
				{
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + p_off ), // POSITION
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + n_off ), // NORMAL
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + t0_off ), // TEXCOORD0
					(float*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + t1_off ), // TEXCOORD1
					vertex_decl.size, vertex_decl.size, vertex_decl.size, vertex_decl.size, // stride
					&mesh_indices[ mfpd.indexOffset ], mfpd.vertexCount, mfpd.indexCount, mf_data.dataFlags & MDF_TRIANGLESTRIP != 0 ? 1 : 0,
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
				else if( !strcmp( key, "MESH" ) || !strcmp( key, "INST" ) || !strcmp( key, "LIGHT" ) || !strcmp( key, "CONFIG" ) )
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
				else if( !strcmp( key, "MESH" ) || !strcmp( key, "INST" ) || !strcmp( key, "LIGHT" ) || !strcmp( key, "CONFIG" ) )
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


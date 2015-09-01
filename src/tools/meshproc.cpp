

#include "../engine/engine.hpp"
#include "../engine/renderer.hpp"
#include <bullet/src/LinearMath/btConvexHull.h>


enum EOpType
{
	OT_Unknown,
	OT_GenConvexHull,
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
		if( !strcmp( argv[ i ], "--convex" ) ){ type = OT_GenConvexHull; break; }
	}
	
	if( type == OT_Unknown )
	{
		fprintf( stderr, "ERROR: unknown operation type\n" );
		exit( 1 );
	}
	
	if( type == OT_GenConvexHull )
	{
		puts( "MESH PROCESSOR: GENERATE CONVEX HULL" );
		
		const char* mesh_path = findprop( argc, argv, "-i", true );
		const char* fn_out = findprop( argc, argv, "-o", true );
		const char* str_maxverts = findprop( argc, argv, "-maxverts" );
		const char* str_mindiff = findprop( argc, argv, "-mindiff" );
		
		int maxverts = String_ParseInt( nullorempty( str_maxverts ) );
		float mindiff = String_ParseFloat( nullorempty( str_mindiff ) );
		
		//
		// MESH LOADING
		//
		ByteArray mesh_data;
		if( !LoadBinaryFile( mesh_path, mesh_data ) )
		{
			fprintf( stderr, "failed to read MESH from file: %s\n", mesh_path );
			exit( 1 );
		}
		printf( "file loaded" );
		
		MeshFileData mf_data;
		const char* mesh_load_error = MeshData_Parse( (char*) mesh_data.data(), mesh_data.size(), &mf_data );
		if( mesh_load_error )
		{
			fprintf( stderr, "failed to parse MESH - %s (file: %s)\n", mesh_load_error, mesh_path );
			exit( 1 );
		}
		printf( ", mesh parsed" );
		
		VDeclInfo vertex_decl;
		const char* vdecl_load_error = VDeclInfo_Parse( &vertex_decl, StackString< 256 >( StringView( mf_data.formatData, mf_data.formatSize ) ) );
		if( vdecl_load_error )
		{
			fprintf( stderr, "failed to parse MESH vertex format - %s (file: %s)\n", vdecl_load_error, mesh_path );
			exit( 1 );
		}
		printf( ", vdecl parsed" );
		
		int p_off = -1;
		for( int i = 0; i < (int) vertex_decl.count; ++i )
		{
			if( vertex_decl.usages[ i ] == VDECLUSAGE_POSITION )
			{
				if( vertex_decl.types[ i ] != VDECLTYPE_FLOAT3 ){ fprintf( stderr, "MESH position data not FLOAT[3] (file: %s)\n", mesh_path ); return 1; }
				p_off = vertex_decl.offsets[ i ];
			}
		}
		if( p_off < 0 ){ fprintf( stderr, "MESH has no position data (file: %s)\n", mesh_path ); return 1; }
		
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
		
		printf( ", reading parts..." );
		
		Array< Vec3 > mesh_vertices;
		for( int i = 0; i < (int) mf_data.numParts; ++i )
		{
			const MeshFilePartData& mfpd = mf_data.parts[ i ];
			if( mfpd.indexCount == 0 || mfpd.vertexCount == 0 )
			{
				fprintf( stderr, "[WARNING: mesh has an empty part %d]\n", i );
				continue;
			}
			
			char* vposdata = (char*)( mf_data.vertexData + vertex_decl.size * mfpd.vertexOffset + p_off );
			for( uint32_t idx = 0; idx < mfpd.indexCount; ++idx )
			{
				Vec3 pos = *(Vec3*)(vposdata + vertex_decl.size * mesh_indices[ mfpd.indexOffset + idx ]);
				mesh_vertices.push_back( pos );
			}
			
			printf( " -%d", i );
		}
		
		puts( " - done" );
		
		//
		// GENERATE CONVEX HULL
		//
		printf( "input point count: %d\n", (int) mesh_vertices.size() );
		Array< btVector3 > mesh_verts_btv;
		for( size_t i = 0; i < mesh_vertices.size(); ++i )
		{
			Vec3 v = mesh_vertices[ i ];
			mesh_verts_btv.push_back( btVector3( v.x, v.y, v.z ) );
		}
		HullDesc bhdesc( QF_DEFAULT, mesh_verts_btv.size(), mesh_verts_btv.data() );
		if( str_maxverts && maxverts >= 3 )
			bhdesc.mMaxVertices = maxverts;
		if( str_mindiff )
			bhdesc.mNormalEpsilon = mindiff;
		
		HullLibrary bhlib;
		HullResult bhres;
		if( bhlib.CreateConvexHull( bhdesc, bhres ) == QE_FAIL )
		{
			fprintf( stderr, "ERROR: failed to generate convex hull!\n" );
			exit( 1 );
		}
		printf( "output point count: %d\n", (int) bhres.m_OutputVertices.size() );
		
		SGRX_ConvexPointSet::Data data;
		data.points.resize( bhres.m_OutputVertices.size() );
		for( int i = 0; i < bhres.m_OutputVertices.size(); ++i )
		{
			btVector3 v = bhres.m_OutputVertices[ i ];
			data.points[ i ] = V3( v.x(), v.y(), v.z() );
		}
		
		bhlib.ReleaseResult( bhres );
		
		//
		// CHULL SAVING
		//
		
		ByteArray cpset_data;
		ByteWriter bw( &cpset_data );
		data.Serialize( bw );
		if( SaveBinaryFile( fn_out, cpset_data.data(), cpset_data.size() ) == false )
		{
			fprintf( stderr, "ERROR: failed to write to output file (%s)\n", fn_out );
			exit( 1 );
		}
	}
	
	puts( "finished" );
	return 0;
}



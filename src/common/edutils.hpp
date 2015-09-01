

#pragma once
#include <engine.hpp>



struct OBJExporter
{
	struct FaceVertex
	{
		int pos;
		int tex;
		int nrm;
	};
	
	void AddVertex( Vec3 pos, Vec2 tex, Vec3 nrm )
	{
		FaceVertex fv =
		{
			positions.find_or_add( pos ),
			texcoords.find_or_add( tex ),
			normals.find_or_add( nrm ),
		};
		facedata.push_back( fv );
	}
	bool Save( const StringView& path, const StringView& comment = "" )
	{
#define OBJEX_BFR_SIZE 8192
		char bfr[ OBJEX_BFR_SIZE ];
		
		String out;
		if( comment )
		{
			out.append( "# " );
			out.append( comment );
			out.append( "\n" );
		}
		for( size_t i = 0; i < positions.size(); ++i )
		{
			sgrx_snprintf( bfr, OBJEX_BFR_SIZE, "v %.6g %.6g %.6g\n", positions[i].x, positions[i].y, positions[i].z );
			out.append( bfr );
		}
		for( size_t i = 0; i < texcoords.size(); ++i )
		{
			sgrx_snprintf( bfr, OBJEX_BFR_SIZE, "vt %.6g %.6g\n", texcoords[i].x, texcoords[i].y );
			out.append( bfr );
		}
		for( size_t i = 0; i < normals.size(); ++i )
		{
			sgrx_snprintf( bfr, OBJEX_BFR_SIZE, "vn %.6g %.6g %.6g\n", normals[i].x, normals[i].y, normals[i].z );
			out.append( bfr );
		}
		size_t fc = ( facedata.size() / 3 ) * 3;
		for( size_t i = 0; i < fc; i += 3 )
		{
			const FaceVertex& fv0 = facedata[ i ], fv1 = facedata[ i + 1 ], fv2 = facedata[ i + 2 ];
			sgrx_snprintf( bfr, OBJEX_BFR_SIZE, "f %d/%d/%d %d/%d/%d %d/%d/%d\n"
				, fv0.pos + 1, fv0.tex + 1, fv0.nrm + 1
				, fv1.pos + 1, fv1.tex + 1, fv1.nrm + 1
				, fv2.pos + 1, fv2.tex + 1, fv2.nrm + 1
			);
			out.append( bfr );
		}
		
		return FS_SaveTextFile( path, out );
	}
	
	Array< Vec3 > positions;
	Array< Vec3 > normals;
	Array< Vec2 > texcoords;
	Array< FaceVertex > facedata;
};



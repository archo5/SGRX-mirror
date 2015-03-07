

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


Quat rotation_between_vectors( const Vec3& v1, const Vec3& v2 )
{
	Vec3 nv1 = v1.Normalized();
	Vec3 nv2 = v2.Normalized();
	return Quat::CreateAxisAngle( Vec3Cross( nv1, nv2 ), Vec3Dot( nv1, nv2 ) );
}


struct AMVertex
{
	Vec3 pos;
	uint8_t weights[4];
	uint8_t indices[4];
};

struct AMBone
{
	String name;
	String path;
	Mat4 boneOffset;
	Mat4 skinOffset;
	Mat4 invSkinOffset;
	Mat4 local_to_world;
	Mat4 world_to_local;
	int parent_id;
	
	uint8_t weight_threshold;
	Vec3 world_axisZ; // general direction (Z)
	Vec3 world_axisY; // second g.d. (Y)
	Vec3 world_axisX; // cross product (X)
	Vec3 world_center;
	Vec3 world_bb_min;
	Vec3 world_bb_max;
	Vec3 world_extents;
	float capsule_height;
	float capsule_radius;
};

bool string_match_pattern( const char* pattern, const StringView& str )
{
	if( *pattern == '\0' && str.size() == 0 )
		return true;
	
	if( *pattern == '*' && pattern[1] != '\0' && str.size() == 0 )
		return false;
	
	if( *pattern == '?' || *pattern == str.ch() )
		return string_match_pattern( pattern + 1, str.part(1) );
	
	if( *pattern == '*' )
		return string_match_pattern( pattern + 1, str ) || string_match_pattern( pattern, str.part(1) );
	return false;
}

void bones_recalc_paths( Array< AMBone >& bones )
{
	char buf[ 32768 ];
	for( size_t i = 0; i < bones.size(); ++i )
	{
		AMBone& B = bones[ i ];
		if( B.parent_id >= 0 )
		{
			AMBone& P = bones[ B.parent_id ];
			snprintf( buf, sizeof(buf), "%.*s%.*s/>", (int) P.path.size() - 1, P.path.data(), (int) B.name.size(), B.name.data() );
		}
		else
			snprintf( buf, sizeof(buf), "</%.*s/>", (int) B.name.size(), B.name.data() );
		B.path = buf;
	}
}

void bones_match_pattern( Array< AMBone >& in, const char* pattern, Array< AMBone* >& out )
{
	for( size_t i = 0; i < in.size(); ++i )
	{
		if( string_match_pattern( pattern, in[i].path ) && out.find_first_at( &in[i] ) == NOT_FOUND )
			out.push_back( &in[i] );
	}
}

int find_bone( Array< AMBone >& in, const char* name )
{
	StringView nm = name;
	for( size_t i = 0; i < in.size(); ++i )
		if( in[i].name == nm )
			return i;
	return -1;
}

void replace_bone_indices( Array< AMVertex >& verts, int from, int to )
{
	for( size_t i = 0; i < verts.size(); ++i )
	{
		AMVertex& V = verts[ i ];
		if( V.weights[0] == from ) V.weights[0] = to;
		if( V.weights[1] == from ) V.weights[1] = to;
		if( V.weights[2] == from ) V.weights[2] = to;
		if( V.weights[3] == from ) V.weights[3] = to;
	}
}

void merge_equal_bone_indices( Array< AMVertex >& verts )
{
	for( size_t i = 0; i < verts.size(); ++i )
	{
		AMVertex& V = verts[ i ];
		
		uint8_t weights[4] = {0};
		uint8_t indices[4] = {0};
		int wicount = 0;
		for( int cwi = 0; cwi < 4; ++cwi )
		{
			if( !V.weights[ cwi ] )
				continue;
			int dwi = 0;
			for( ; dwi < wicount; ++dwi )
				if( indices[ dwi ] == V.indices[ cwi ] )
					break;
			weights[ dwi ] += V.weights[ cwi ];
			indices[ dwi ] = V.indices[ cwi ];
			if( dwi >= wicount )
				wicount++;
		}
		
		memcpy( V.weights, weights, sizeof(weights) );
		memcpy( V.indices, indices, sizeof(indices) );
	}
}

void calc_bone_volume_info( int bid, AMBone& B, Array< AMVertex >& verts )
{
	Array< Vec3 > points;
	
	// gather related points
	for( size_t i = 0; i < verts.size(); ++i )
	{
		if( ( verts[ i ].indices[0] == bid && verts[ i ].weights[0] >= B.weight_threshold ) ||
			( verts[ i ].indices[1] == bid && verts[ i ].weights[1] >= B.weight_threshold ) ||
			( verts[ i ].indices[2] == bid && verts[ i ].weights[2] >= B.weight_threshold ) ||
			( verts[ i ].indices[3] == bid && verts[ i ].weights[3] >= B.weight_threshold ) )
		{
			points.push_back( verts[ i ].pos );
		}
	}
	
	//
	// A X I S   1
	//
	Vec3 main_center_3d = {0,0,0};
	{
		// find longest direction and center
		Vec3 center = {0,0,0};
		Vec3 curdir = {0,0,0};
		float curlen = 0;
		
		for( size_t i = 0; i < points.size(); ++i )
		{
			center += points[i];
			for( size_t j = i + 1; j < points.size(); ++j )
			{
				Vec3 newdir = points[j] - points[i];
				float newlen = newdir.LengthSq();
				if( newlen > curlen )
				{
					curdir = newdir;
					curlen = newlen;
				}
			}
		}
		if( points.size() )
			center /= points.size();
		main_center_3d = center;
	
		// find centers at both sides of plane
		Vec3 PN = curdir.Normalized();
		float PD = Vec3Dot( PN, center );
		Vec3 cp0 = {0,0,0}, cp1 = {0,0,0};
		int ci0 = 0, ci1 = 0;
		
		for( size_t i = 0; i < points.size(); ++i )
		{
			Vec3 P = points[i];
			if( Vec3Dot( PN, P ) > PD )
			{
				cp0 += P;
				ci0++;
			}
			else
			{
				cp1 += P;
				ci1++;
			}
		}
		
		if( ci0 ) cp0 /= ci0;
		if( ci1 ) cp1 /= ci1;
		
		// axis1 = direction between centers
		B.world_axisZ = ( cp1 - cp0 ).Normalized();
	}
	
	//
	// A X I S   2
	//
	// pick projection axes
	Vec3 proj_x = Vec3Cross( B.world_axisZ, B.world_axisZ.Shuffle() ).Normalized();
	Vec3 proj_y = Vec3Cross( B.world_axisZ, proj_x ).Normalized();
	
	// project points into 2D
	Array< Vec2 > point2ds;
	point2ds.resize( points.size() );
	Vec2 proj_center = { Vec3Dot( main_center_3d, proj_x ), Vec3Dot( main_center_3d, proj_y ) };
	for( size_t i = 0; i < points.size(); ++i )
	{
		point2ds[ i ] = V2( Vec3Dot( points[i], proj_x ), Vec3Dot( points[i], proj_y ) ) - proj_center;
	}
	
	{
		// find longest direction and center
		Vec2 center = {0,0};
		Vec2 curdir = {0,0};
		float curlen = 0;
		
		for( size_t i = 0; i < point2ds.size(); ++i )
		{
			center += point2ds[i];
			for( size_t j = i + 1; j < point2ds.size(); ++j )
			{
				Vec2 newdir = point2ds[j] - point2ds[i];
				float newlen = newdir.LengthSq();
				if( newlen > curlen )
				{
					curdir = newdir;
					curlen = newlen;
				}
			}
		}
		if( point2ds.size() )
			center /= point2ds.size();
	
		// find centers at both sides of plane
		Vec2 PN = curdir.Normalized();
		float PD = Vec2Dot( PN, center );
		Vec2 cp0 = {0,0}, cp1 = {0,0};
		int ci0 = 0, ci1 = 0;
		
		for( size_t i = 0; i < point2ds.size(); ++i )
		{
			Vec2 P = point2ds[i];
			if( Vec2Dot( PN, P ) > PD )
			{
				cp0 += P;
				ci0++;
			}
			else
			{
				cp1 += P;
				ci1++;
			}
		}
		
		if( ci0 ) cp0 /= ci0;
		if( ci1 ) cp1 /= ci1;
		
		// axis1 = direction between centers
		Vec2 world_axis_2d = ( cp1 - cp0 ).Normalized();
		B.world_axisY = world_axis_2d.x * proj_x + world_axis_2d.y * proj_y;
		B.world_axisX = Vec3Cross( B.world_axisZ, B.world_axisY ).Normalized();
		
		B.world_center = main_center_3d + proj_x * center.x + proj_y * center.y;
	}
	
	//
	// O T H E R
	//
	Vec3 bbmin = {0,0,0};
	Vec3 bbmax = {0,0,0};
	Vec3 C = B.world_center;
	Vec3 center_dots = { Vec3Dot( B.world_axisX, C ), Vec3Dot( B.world_axisY, C ), Vec3Dot( B.world_axisZ, C ) };
	for( size_t i = 0; i < points.size(); ++i )
	{
		Vec3 P = points[ i ];
		Vec3 dots = { Vec3Dot( B.world_axisX, P ), Vec3Dot( B.world_axisY, P ), Vec3Dot( B.world_axisZ, P ) };
		dots -= center_dots;
		bbmin = Vec3::Min( bbmin, dots );
		bbmax = Vec3::Max( bbmax, dots );
	}
	B.world_bb_min = bbmin;
	B.world_bb_max = bbmax;
	B.world_extents = Vec3::Max( -bbmin, bbmax );
	B.capsule_radius = TMAX( B.world_extents.x, B.world_extents.y );
	B.capsule_height = TMAX( B.world_extents.z - B.capsule_radius * 2, 0.0f );
	
	printf( "Bone %.*s:\n", (int) B.name.size(), B.name.data() );
	printf( "- center: %g %g %g\n", B.world_center.x, B.world_center.y, B.world_center.z );
	printf( "- axis-X: %g %g %g\n", B.world_axisX.x, B.world_axisX.y, B.world_axisX.z );
	printf( "- axis-Y: %g %g %g\n", B.world_axisY.x, B.world_axisY.y, B.world_axisY.z );
	printf( "- axis-Z: %g %g %g\n", B.world_axisZ.x, B.world_axisZ.y, B.world_axisZ.z );
	printf( "- bbmin: %g %g %g\n", bbmin.x, bbmin.y, bbmin.z );
	printf( "- bbmax: %g %g %g\n", bbmax.x, bbmax.y, bbmax.z );
	printf( "- extents: %g %g %g\n", B.world_extents.x, B.world_extents.y, B.world_extents.z );
	printf( "- capsule radius: %g, height: %g\n", B.capsule_radius, B.capsule_height );
}


struct SkeletonInfo
{
	enum BodyType
	{
		BodyType_Capsule = 1
	};
	
	struct HitBox
	{
		String name;
		Quat rotation;
		Vec3 position;
		Vec3 extents;
		float multiplier;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name );
			arch( rotation );
			arch( position );
			arch( extents );
			arch( multiplier );
		}
	};
	
	struct Body
	{
		String name;
		Quat rotation;
		Vec3 position;
		uint8_t type;
		float capsule_radius;
		float capsule_height;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name );
			arch( rotation );
			arch( position );
			arch( type );
			if( type == BodyType_Capsule )
			{
				arch( capsule_radius );
				arch( capsule_height );
			}
		}
	};
	
	struct Joint
	{
		String name1;
		String name2;
		Vec3 local_offset1;
		Vec3 local_offset2;
		uint8_t type;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name1 );
			arch( name2 );
			arch( local_offset1 );
			arch( local_offset2 );
			arch( type );
		}
	};
	
	Array< HitBox > hitboxes;
	Array< Body > bodies;
	Array< Joint > joints;
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SGRXSKRI" );
		SerializeVersionHelper<T> vh( arch, 1 );
		vh( hitboxes );
		vh( bodies );
		vh( joints );
	}
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
					// name, path
					StringView( mesh_file_data.bones[ i ].boneName, mesh_file_data.bones[ i ].boneNameSize ), "",
					// bone, skin, invskin, w2l, l2w
					mesh_file_data.bones[ i ].boneOffset, Mat4::Identity, Mat4::Identity, Mat4::Identity, Mat4::Identity,
					// parent ID
					mesh_file_data.bones[ i ].parent_id == 255 ? -1 : mesh_file_data.bones[ i ].parent_id,
					// weight, w_axisZ, w_axisY, w_axisX
					127, V3(0), V3(0), V3(0),
					// w_center, w_min, w_max, w_ext
					V3(0), V3(0), V3(0), V3(0),
					// capsule height, radius
					0, 0,
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
				mesh_bones[ b ].local_to_world = mesh_bones[ b ].skinOffset;
				mesh_bones[ b ].world_to_local = mesh_bones[ b ].invSkinOffset;
			}
			printf( ", matrices calculated" );
			
			bones_recalc_paths( mesh_bones );
			printf( ", paths generated" );
			
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
				AnimFileParser::Anim& A = afp.animData[ animID ];
				if( StringView( A.name, A.nameSize ) == anim_name )
					break;
			}
			if( animID == afp.animData.size() )
			{
				fprintf( stderr, "Animation '%s' was not found in file '%s'\n", anim_name, anim_path );
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
				mesh_bones[ i ].local_to_world = M;
				if( !M.InvertTo( mesh_bones[ i ].world_to_local ) )
				{
					fprintf( stderr, "ANIM: failed to invert animation matrix #%d\n", (int) i );
					mesh_bones[ i ].world_to_local.SetIdentity();
				}
				M = mesh_bones[ i ].invSkinOffset * M;
			}
			
			puts( " - done" );
		}
		else if( !strcmp( type, "REASSIGN" ) )
		{
			char src_pattern[ 1025 ] = {0};
			char dst_bone[ 256 ] = {0};
			
			// READ COMMAND
			if( fscanf( fp, "%1024s %255s", src_pattern, dst_bone ) < 2 )
			{
				perror( "failed to read REASSIGN <pattern> <target>" );
				return 1;
			}
			
			bool need_parent = !strcmp( dst_bone, "parent" );
			
			Array< AMBone* > rebones;
			
			bones_match_pattern( mesh_bones, src_pattern, rebones );
			// bones are already sorted with children after parents
			
			printf( "REASSIGN: pattern \"%s\" found %d bones\n", src_pattern, (int) rebones.size() );
			
			for( size_t i = rebones.size(); i > 0; )
			{
				i--;
				AMBone* B = rebones[ i ];
				printf( "- matched \"%.*s\"\n", (int) B->path.size(), B->path.data() );
				int pos = need_parent ? B->parent_id : find_bone( mesh_bones, dst_bone );
				if( pos < 0 )
				{
					fprintf( stderr, need_parent ? "WARNING: parent bone not found!\n" : "WARNING: bone \"%s\" not found!\n", dst_bone );
					continue;
				}
				
				size_t bone_idx = B - mesh_bones.data();
				if( pos > bone_idx )
					pos--;
				replace_bone_indices( mesh_vertex_data, bone_idx, pos );
				
				printf( "Removing reassigned bone: \"%.*s\"\n", (int) B->name.size(), B->name.data() );
				for( size_t j = bone_idx + 1; j < mesh_bones.size(); ++j )
				{
					if( mesh_bones[ j ].parent_id == bone_idx )
						mesh_bones[ j ].parent_id = mesh_bones[ bone_idx ].parent_id;
					else if( mesh_bones[ j ].parent_id > bone_idx )
						mesh_bones[ j ].parent_id--;
				}
				mesh_bones.erase( bone_idx );
			}
			merge_equal_bone_indices( mesh_vertex_data );
			bones_recalc_paths( mesh_bones );
		}
		else if( !strcmp( type, "JOINT" ) )
		{
		}
		else if( !strcmp( type, "BODY" ) )
		{
		}
		else if( !strcmp( type, "OUT" ) )
		{
			char out_path[ 1025 ] = {0};
			if( fscanf( fp, "%1024s", out_path ) < 1 )
			{
				perror( "failed to read OUT <path>" );
				return 1;
			}
			
			for( size_t i = 0; i < mesh_bones.size(); ++i )
				calc_bone_volume_info( i, mesh_bones[ i ], mesh_vertex_data );
			
			// REMOVE UNUSED BONES
			for( size_t i = mesh_bones.size(); i > 0; )
			{
				i--;
				AMBone& B = mesh_bones[ i ];
				if( B.world_extents.NearZero() )
				{
					printf( "Removing unused bone: \"%.*s\"\n", (int) B.name.size(), B.name.data() );
					for( size_t j = i + 1; j < mesh_bones.size(); ++j )
					{
						if( mesh_bones[ j ].parent_id == i )
							mesh_bones[ j ].parent_id = mesh_bones[ i ].parent_id;
						else if( mesh_bones[ j ].parent_id > i )
							mesh_bones[ j ].parent_id--;
					}
					mesh_bones.erase( i );
				}
			}
			
			// SAVE DATA
			SkeletonInfo skinfo;
			for( size_t i = 0; i < mesh_bones.size(); ++i )
			{
				const AMBone& B = mesh_bones[ i ];
				const String& name = B.name;
				Quat rotation = rotation_between_vectors( V3(0,0,1), B.world_to_local.TransformNormal( B.world_axisZ ) );
				Vec3 position = B.world_to_local.TransformPos( B.world_center );
				
				SkeletonInfo::HitBox hbox = { name, rotation, position, B.world_extents, 1 };
				SkeletonInfo::Body body = { name, rotation, position, SkeletonInfo::BodyType_Capsule, B.capsule_height, B.capsule_radius };
				
				skinfo.hitboxes.push_back( hbox );
				skinfo.bodies.push_back( body );
				
				if( B.parent_id >= 0 )
				{
					const AMBone& BP = mesh_bones[ B.parent_id ];
					Vec3 parent_local_pos = BP.world_to_local.TransformPos( B.local_to_world.TransformPos( V3(0) ) );
					SkeletonInfo::Joint joint = { BP.name, B.name, parent_local_pos, V3(0), 0 };
				}
			}
			
			ByteArray ba;
			ByteWriter bw( &ba );
			bw << skinfo;
			
			if( SaveBinaryFile( out_path, ba.data(), ba.size() ) )
			{
				printf( "File saved to \"%s\"!\n", out_path );
			}
			else
			{
				fprintf( stderr, "FAILED to save file \"%s\"!\n", out_path );
				return 1;
			}
		}
		else
		{
			fprintf( stderr, "unrecognized command: %s\n", type );
			return 1;
		}
	}
}


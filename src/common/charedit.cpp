

#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <imgui.hpp>
#include <physics.hpp>


inline Quat EA2Q( Vec3 v ){ return Quat::CreateFromXYZ( DEG2RAD( v ) ); }
inline Vec3 Q2EA( Quat q ){ return RAD2DEG( q.ToXYZ() ); }


PhyWorldHandle g_PhyWorld;
SceneHandle g_EdScene;
AnimCharHandle g_AnimChar;
AnimCharInst* g_AnimCharInst;


IMGUIRenderView* g_NUIRenderView;
IMGUIFilePicker* g_NUICharFilePicker;
IMGUIMeshPicker* g_NUIMeshPicker;
IMGUIAnimPicker* g_NUIAnimPicker;



struct AMVertex
{
	Vec3 pos;
	uint8_t weights[4];
	uint8_t indices[4];
};
Array< AMVertex > g_MeshVertices;

struct AMBone
{
	// IN
	int id;
	String name;
	uint8_t weight_threshold;
	// OUT
	Vec3 world_axisZ; // general direction (Z)
	Vec3 world_axisY; // second g.d. (Y)
	Vec3 world_axisX; // cross product (X)
	Vec3 world_center;
	Vec3 bb_min;
	Vec3 bb_max;
	Vec3 bb_extents;
	float capsule_halfheight;
	float capsule_radius;
};

void reload_mesh_vertices()
{
	Array< AMVertex >& outverts = g_MeshVertices;
	
	LOG << "Reloading mesh vertices...";
	outverts.clear();
	if( g_AnimChar )
	{
		SGRX_IMesh* M = g_AnimCharInst->m_cachedMesh;
		if( M && M->m_vdata.size() && M->m_vertexDecl )
		{
			const VDeclInfo& VDI = M->m_vertexDecl.GetInfo();
			int off_p = VDI.GetOffset( VDECLUSAGE_POSITION );
			int off_i = VDI.GetOffset( VDECLUSAGE_BLENDIDX );
			int off_w = VDI.GetOffset( VDECLUSAGE_BLENDWT );
			if( off_p != -1 && off_i != -1 && off_w != -1 &&
				VDI.GetType( VDECLUSAGE_POSITION ) == VDECLTYPE_FLOAT3 &&
				VDI.GetType( VDECLUSAGE_BLENDIDX ) == VDECLTYPE_BCOL4 &&
				VDI.GetType( VDECLUSAGE_BLENDWT ) == VDECLTYPE_BCOL4 )
			{
				AMVertex vtx;
				size_t vcount = M->m_vdata.size() / VDI.size;
				for( size_t i = 0; i < vcount; ++i )
				{
					size_t boff = i * VDI.size;
					memcpy( &vtx.pos, &M->m_vdata[ boff + off_p ], sizeof(vtx.pos) );
					memcpy( vtx.weights, &M->m_vdata[ boff + off_w ], sizeof(vtx.weights) );
					memcpy( vtx.indices, &M->m_vdata[ boff + off_i ], sizeof(vtx.indices) );
					outverts.push_back( vtx );
				}
				LOG << "RELOADED VERTICES: " << outverts.size();
			}
		}
	}
}

void calc_bone_volume_info( AMBone& B )
{
	Array< AMVertex >& verts = g_MeshVertices;
	
	
	Array< Vec3 > points;
	
	// gather related points
	for( size_t i = 0; i < verts.size(); ++i )
	{
		if( ( verts[ i ].indices[0] == B.id && verts[ i ].weights[0] >= B.weight_threshold ) ||
			( verts[ i ].indices[1] == B.id && verts[ i ].weights[1] >= B.weight_threshold ) ||
			( verts[ i ].indices[2] == B.id && verts[ i ].weights[2] >= B.weight_threshold ) ||
			( verts[ i ].indices[3] == B.id && verts[ i ].weights[3] >= B.weight_threshold ) )
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
			if( Vec3Dot( PN, P ) < PD )
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
			if( Vec2Dot( PN, P ) < PD )
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
		// this order of cross product arguments is required for matrix->quaternion to work properly
		B.world_axisX = Vec3Cross( B.world_axisY, B.world_axisZ ).Normalized();
		
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
	Vec3 bbcenter = ( bbmin + bbmax ) * 0.5f;
	B.world_center += B.world_axisX * bbcenter.x + B.world_axisY * bbcenter.y + B.world_axisZ * bbcenter.z;
	B.bb_min = bbmin - bbcenter;
	B.bb_max = bbmax - bbcenter;
	B.bb_extents = B.bb_max;
	B.capsule_radius = TMAX( B.bb_extents.x, B.bb_extents.y );
	B.capsule_halfheight = TMAX( ( B.bb_extents.z - B.capsule_radius ), 0.0f );
	
	printf( "Bone %.*s:\n", (int) B.name.size(), B.name.data() );
	printf( "- center: %g %g %g\n", B.world_center.x, B.world_center.y, B.world_center.z );
	printf( "- axis-X: %g %g %g\n", B.world_axisX.x, B.world_axisX.y, B.world_axisX.z );
	printf( "- axis-Y: %g %g %g\n", B.world_axisY.x, B.world_axisY.y, B.world_axisY.z );
	printf( "- axis-Z: %g %g %g\n", B.world_axisZ.x, B.world_axisZ.y, B.world_axisZ.z );
	printf( "- bbmin: %g %g %g\n", B.bb_min.x, B.bb_min.y, B.bb_min.z );
	printf( "- bbmax: %g %g %g\n", B.bb_max.x, B.bb_max.y, B.bb_max.z );
	printf( "- extents: %g %g %g\n", B.bb_extents.x, B.bb_extents.y, B.bb_extents.z );
	printf( "- capsule radius: %g, height: %g\n", B.capsule_radius, B.capsule_halfheight );
}

void calc_char_bone_info( int bid, int thres, float minbs, int mask )
{
	if( g_AnimChar == NULL || bid < 0 || bid >= (int) g_AnimChar->bones.size() )
	{
		LOG_ERROR << "Cannot calculate bone info for bone " << bid << " at this time";
		return;
	}
	SGRX_MeshBone* meshbones = g_AnimCharInst->m_cachedMesh->m_bones;
	AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ bid ];
	AMBone B = { g_AnimCharInst->_GetMeshBoneID( bid ), BI.name, (uint8_t) thres };
	
	calc_bone_volume_info( B );
	
	Mat4 world_to_local = B.id < 0 ? Mat4::Identity : meshbones[ B.id ].invSkinOffset;
	Vec3 local_pos = world_to_local.TransformPos( B.world_center );
	Vec3 lo1 = world_to_local.TransformNormal( B.world_axisX );
	Vec3 lo2 = world_to_local.TransformNormal( B.world_axisY );
	Vec3 lo3 = world_to_local.TransformNormal( B.world_axisZ );
	Quat local_rot = Mat4::Basis( lo1, lo2, lo3 ).GetRotationQuaternion();
	
	if( mask & 1 )
	{
		BI.body.position = local_pos;
		BI.body.rotation = local_rot;
		switch( BI.body.type )
		{
		case AnimCharacter::BodyType_Capsule:
			BI.body.size = V3( B.capsule_radius, 0, B.capsule_halfheight );
			break;
		case AnimCharacter::BodyType_Sphere:
			BI.body.size = V3( Vec3Dot( B.bb_extents, V3(1.0f/3.0f) ) );
			break;
		default:
			BI.body.size = B.bb_extents;
			break;
		}
	}
	if( mask & 2 )
	{
		BI.hitbox.position = local_pos;
		BI.hitbox.rotation = local_rot;
		BI.hitbox.extents = B.bb_extents;
	}
	if( mask & 4 )
	{
		Vec3 pbs = V3(0), bs = g_AnimChar->bones[ bid ].body.size.Abs();
		int pbone = g_AnimCharInst->_FindParentBone( bid );
		while( pbone >= 0 )
		{
			pbs = g_AnimChar->bones[ pbone ].body.size.Abs();
			if( pbs.x < minbs || pbs.y < minbs || pbs.z < minbs )
				pbone = g_AnimCharInst->_FindParentBone( pbone );
			else
				break;
		}
		
		if( pbone < 0 || bs.x < minbs || bs.y < minbs || bs.z < minbs ||
			pbs.x < minbs || pbs.y < minbs || pbs.z < minbs )
		{
			BI.joint.type = AnimCharacter::JointType_None;
			BI.joint.parent_name = "";
			BI.joint.parent_id = -1;
		}
		else
		{
			int pbone_m = g_AnimCharInst->_GetMeshBoneID( pbone );
			ASSERT( pbone_m >= 0 );
			
			BI.joint.type = AnimCharacter::JointType_ConeTwist;
			BI.joint.parent_name = g_AnimChar->bones[ pbone ].name;
			BI.joint.parent_id = pbone;
			
			Mat4 world_to_parent = meshbones[ pbone_m ].invSkinOffset;
			
			// calculate own position
			Vec3 bone_origin = meshbones[ B.id ].skinOffset.TransformPos( V3(0) );
			
			// calculate child bone count / avg. position
			int numch = 0;
			Vec3 avg_child_origin = V3(0);
			for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			{
				if( g_AnimCharInst->_FindParentBone( i ) == bid )
				{
					int cmbid = g_AnimCharInst->_GetMeshBoneID( i );
					avg_child_origin += meshbones[ cmbid ].skinOffset.TransformPos( V3(0) );
					numch++;
				}
			}
			if( numch > 0 )
				avg_child_origin /= numch;
			
			// generate world-space joint frame
			Vec3 jwZ = V3(0);
			if( numch > 0 )
				jwZ = ( avg_child_origin - bone_origin );
			if( jwZ == V3(0) )
				jwZ = B.world_axisZ;
			jwZ.Normalize();
			Vec3 jwX = Vec3Dot( jwZ, B.world_axisZ.Normalized() ) > 0.707f ? B.world_axisX : B.world_axisZ;
			Vec3 jwY = Vec3Cross( jwZ, jwX ).Normalized();
			jwX = Vec3Cross( jwY, jwZ ).Normalized(); // orthogonalize
			
			// copy joint frame to parent+own bone space
			Mat4 jlb = Mat4::Basis(
				world_to_local.TransformNormal( jwX ),
				world_to_local.TransformNormal( jwY ),
				world_to_local.TransformNormal( jwZ ) );
			BI.joint.self_position = world_to_local.TransformPos( bone_origin );
			BI.joint.self_rotation = jlb.GetRotationQuaternion();
			
			Mat4 jpb = Mat4::Basis(
				world_to_parent.TransformNormal( jwX ),
				world_to_parent.TransformNormal( jwY ),
				world_to_parent.TransformNormal( jwZ ) );
			BI.joint.prnt_position = world_to_parent.TransformPos( bone_origin );
			BI.joint.prnt_rotation = jpb.GetRotationQuaternion();
			
			printf( "Joint Data for %.*s:\n", (int) B.name.size(), B.name.data() );
			printf( "- child count: %d\n", numch );
			printf( "- origin: %g %g %g\n", bone_origin.x, bone_origin.y, bone_origin.z );
			printf( "- axis-X: %g %g %g\n", jwX.x, jwX.y, jwX.z );
			printf( "- axis-Y: %g %g %g\n", jwY.x, jwY.y, jwY.z );
			printf( "- axis-Z: %g %g %g\n", jwZ.x, jwZ.y, jwZ.z );
			printf( "- avg. child origin: %g %g %g\n", avg_child_origin.x, avg_child_origin.y, avg_child_origin.z );
		}
	}
}


// TRANSFORM TARGET
enum ETransformTargetType
{
	TT_None = 0,
	TT_BoneHitbox,
	TT_BoneBody,
	TT_BoneJointSelfFrame,
	TT_BoneJointParentFrame,
	TT_Attachment,
};
enum ETransformOpType
{
	TO_None = 0,
	TO_MoveCam,
	TO_RotateTrackpad,
	TO_RotateCamRoll,
};
struct XFormData
{
	XFormData() : boneID(-1), itemID(-1), position(V3(0)), rotation(Quat::Identity),
		boneToWorld(Mat4::Identity), worldToBone(Mat4::Identity)
	{}
	int boneID;
	int itemID;
	Vec3 position;
	Quat rotation;
	Mat4 boneToWorld;
	Mat4 worldToBone;
};
struct XFormStateInfo
{
	XFormStateInfo() : type(TT_None), op(TO_None), curCursorPos(V2(0)), startCursorPos(V2(0)){}
	
	String name;
	ETransformTargetType type;
	ETransformOpType op;
	Vec2 curCursorPos;
	Vec2 startCursorPos;
	Vec2 viewSize;
	XFormData xfdata;
	
	void OnMoveKey()
	{
		if( type == TT_None )
			return;
		if( op != TO_MoveCam )
		{
			if( GetXFormData() == false )
				return;
		}
		op = TO_MoveCam;
		OnUpdate( curCursorPos, viewSize );
	}
	void OnRotateKey()
	{
		if( type == TT_None )
			return;
		if( op != TO_RotateTrackpad && op != TO_RotateCamRoll )
		{
			if( GetXFormData() == false )
				return;
		}
		if( op == TO_RotateTrackpad ) op = TO_RotateCamRoll;
		else op = TO_RotateTrackpad;
		OnUpdate( curCursorPos, viewSize );
	}
	void OnConfirmKey()
	{
		op = TO_None;
	}
	void OnCancelKey()
	{
		if( op != TO_None )
		{
			SetXFormPos( xfdata.position );
			SetXFormRot( xfdata.rotation );
			op = TO_None;
		}
	}
	bool GetXFormData()
	{
		startCursorPos = curCursorPos;
		StringView boneName = name;
		
		int iid = -1;
		switch( type )
		{
		case TT_None:
			return false;
		case TT_BoneHitbox:
		case TT_BoneBody:
		case TT_BoneJointSelfFrame:
			break;
		case TT_BoneJointParentFrame:
			iid = g_AnimCharInst->m_cachedMesh.FindBone( boneName );
			if( iid < 0 )
				return false;
			boneName = g_AnimChar->bones[ iid ].joint.parent_name;
			break;
		case TT_Attachment:
			iid = g_AnimChar->FindAttachment( name );
			if( iid < 0 )
				return false;
			boneName = g_AnimChar->attachments[ iid ].bone;
			break;
		}
		
		int bid = xfdata.boneID = g_AnimCharInst->m_cachedMesh.FindBone( boneName );
		if( bid < 0 )
			return false;
		if( iid < 0 )
			iid = bid;
		xfdata.itemID = iid;
		
		SGRX_MeshInstance* MI = g_AnimCharInst->m_cachedMeshInst;
		xfdata.boneToWorld = g_AnimCharInst->m_cachedMesh->m_bones[ bid ].skinOffset
			* MI->skin_matrices[ bid ] * MI->matrix;
		xfdata.worldToBone = xfdata.boneToWorld.Inverted();
		
		switch( type )
		{
		case TT_None:
			break;
		case TT_BoneHitbox:
			xfdata.position = g_AnimChar->bones[ iid ].hitbox.position;
			xfdata.rotation = g_AnimChar->bones[ iid ].hitbox.rotation;
			break;
		case TT_BoneBody:
			xfdata.position = g_AnimChar->bones[ iid ].body.position;
			xfdata.rotation = g_AnimChar->bones[ iid ].body.rotation;
			break;
		case TT_BoneJointSelfFrame:
			xfdata.position = g_AnimChar->bones[ iid ].joint.self_position;
			xfdata.rotation = g_AnimChar->bones[ iid ].joint.self_rotation;
			break;
		case TT_BoneJointParentFrame:
			xfdata.position = g_AnimChar->bones[ iid ].joint.prnt_position;
			xfdata.rotation = g_AnimChar->bones[ iid ].joint.prnt_rotation;
			break;
		case TT_Attachment:
			xfdata.position = g_AnimChar->attachments[ iid ].position;
			xfdata.rotation = g_AnimChar->attachments[ iid ].rotation;
			break;
		}
		
		return true;
	}
	void _OnDataChanged()
	{
	}
	template< class T > void _UPD( T& a, const T& b )
	{
		if( a != b )
		{
			a = b;
			_OnDataChanged();
		}
	}
	void SetXFormPos( Vec3 pos )
	{
		int iid = xfdata.itemID;
		switch( type )
		{
		case TT_None:
			break;
		case TT_BoneHitbox:
			_UPD( g_AnimChar->bones[ iid ].hitbox.position, pos );
			break;
		case TT_BoneBody:
			_UPD( g_AnimChar->bones[ iid ].body.position, pos );
			break;
		case TT_BoneJointSelfFrame:
			_UPD( g_AnimChar->bones[ iid ].joint.self_position, pos );
			break;
		case TT_BoneJointParentFrame:
			_UPD( g_AnimChar->bones[ iid ].joint.prnt_position, pos );
			break;
		case TT_Attachment:
			_UPD( g_AnimChar->attachments[ iid ].position, pos );
			break;
		}
	}
	void SetXFormRot( Quat rot )
	{
		int iid = xfdata.itemID;
		switch( type )
		{
		case TT_None:
			break;
		case TT_BoneHitbox:
			_UPD( g_AnimChar->bones[ iid ].hitbox.rotation, rot );
			break;
		case TT_BoneBody:
			_UPD( g_AnimChar->bones[ iid ].body.rotation, rot );
			break;
		case TT_BoneJointSelfFrame:
			_UPD( g_AnimChar->bones[ iid ].joint.self_rotation, rot );
			break;
		case TT_BoneJointParentFrame:
			_UPD( g_AnimChar->bones[ iid ].joint.prnt_rotation, rot );
			break;
		case TT_Attachment:
			_UPD( g_AnimChar->attachments[ iid ].rotation, rot );
			break;
		}
	}
	void OnUpdate( Vec2 cp, Vec2 vsz );
}
g_XFormState;

void XFormStateInfo::OnUpdate( Vec2 cp, Vec2 vsz )
{
	curCursorPos = cp;
	viewSize = vsz;
	
	switch( op )
	{
	case TO_MoveCam:
		{
			// calculate projection plane
			Vec3 pporigin = xfdata.boneToWorld.TransformPos( xfdata.position );
			Vec3 ppdir = -g_EdScene->camera.direction.Normalized();
			Vec4 pplane = V4( ppdir, Vec3Dot( ppdir, pporigin ) );
			
			// calculate start/curr rays & intersect with plane
			Vec3 ro_start, ro_dir;
			Vec3 rc_start, rc_dir;
			float ro_dists[2], rc_dists[2];
			if( g_EdScene->camera.GetCursorRay( startCursorPos.x, startCursorPos.y, ro_start, ro_dir ) &&
				g_EdScene->camera.GetCursorRay( curCursorPos.x, curCursorPos.y, rc_start, rc_dir ) &&
				RayPlaneIntersect( ro_start, ro_dir, pplane, ro_dists ) &&
				RayPlaneIntersect( rc_start, rc_dir, pplane, rc_dists ) )
			{
				// get intersection points
				Vec3 ro_isp = ro_start + ro_dir * ro_dists[0];
				Vec3 rc_isp = rc_start + rc_dir * rc_dists[0];
				
				// transform position
				Vec3 nwpos = pporigin + rc_isp - ro_isp;
				SetXFormPos( xfdata.worldToBone.TransformPos( nwpos ) );
			}
		}
		break;
	case TO_RotateTrackpad:
		{
			Vec2 cdiff = curCursorPos - startCursorPos;
			Vec2 rf = cdiff * viewSize * 0.01f;
			
			SGRX_Camera& CAM = g_EdScene->camera;
			Vec3 wdir_x = Vec3Cross( CAM.direction, CAM.updir );
			Vec3 wdir_y = Vec3Cross( CAM.direction, wdir_x );
			
			Vec3 bdir_x = xfdata.worldToBone.TransformNormal( wdir_x ).Normalized();
			Vec3 bdir_y = xfdata.worldToBone.TransformNormal( wdir_y ).Normalized();
			
			SetXFormRot( Quat::CreateAxisAngle( -bdir_y, rf.x ) *
				Quat::CreateAxisAngle( bdir_x, rf.y ) * xfdata.rotation );
		}
		break;
	case TO_RotateCamRoll:
		{
			Vec2 scntr = V2(0.5f) * viewSize;
			Vec2 oss_cp = startCursorPos * viewSize;
			Vec2 css_cp = curCursorPos * viewSize;
			
			float odcp = ( oss_cp - scntr ).Angle();
			float cdcp = ( css_cp - scntr ).Angle();
			
			SGRX_Camera& CAM = g_EdScene->camera;
			Vec3 bdir_z = xfdata.worldToBone.TransformNormal( CAM.direction ).Normalized();
			
			SetXFormRot( Quat::CreateAxisAngle( bdir_z, cdcp - odcp ) * xfdata.rotation );
		}
		break;
	case TO_None:
		break;
	}
}


// Floor mesh

struct FMVertex
{
	Vec3 pos;
	Vec3 nrm;
	Vec2 tex;
};

MeshInstHandle g_FloorMeshInst;
PhyRigidBodyHandle g_FloorBody;

static void floor_mesh_update( float size, float height )
{
	FMVertex verts[4] =
	{
		{ { -size, -size, height }, {0,0,1}, { +size, -size } },
		{ { +size, -size, height }, {0,0,1}, { -size, -size } },
		{ { +size, +size, height }, {0,0,1}, { -size, +size } },
		{ { -size, +size, height }, {0,0,1}, { +size, +size } },
	};
	uint16_t idcs[6] = { 0, 2, 1, 3, 2, 0 };
	SGRX_MeshPart part = { 0, 4, 0, 6 };
	VertexDeclHandle vdh = GR_GetVertexDecl( "pf3nf30f2" );
	g_FloorMeshInst->GetMesh()->SetVertexData( verts, sizeof(verts), vdh );
	g_FloorMeshInst->GetMesh()->SetIndexData( idcs, sizeof(idcs), false );
	g_FloorMeshInst->GetMesh()->SetAABBFromVertexData( verts, sizeof(verts), vdh );
	g_FloorMeshInst->GetMesh()->SetPartData( &part, 1 );
	g_FloorBody->GetShape()->SetScale( V3(size) );
	g_FloorBody->SetPosition( V3( 0, 0, height ) );
}



String g_fileName;

bool g_showBodies = true;
bool g_showJoints = true;
bool g_showHitboxes = true;
bool g_showAttachments = true;
bool g_showMasks = true;
bool g_showPhysics = true;
float g_maskPreviewTickSize = 0.04f;

float g_recalcMinBoneSize = 0.05f;
int g_recalcWeightThreshold = 96;
bool g_recalcHitboxes = true;
bool g_recalcBodies = true;
bool g_recalcJoints = true;
Array< bool > g_recalcWhichBones;

size_t g_hoverBone = NOT_FOUND;
size_t g_hoverAtch = NOT_FOUND;
size_t g_hoverMask = NOT_FOUND;

void SetHovered( size_t b, size_t a, size_t m )
{
	g_hoverBone = b;
	g_hoverAtch = a;
	g_hoverMask = m;
}



void PostViewUI()
{
	// xform editing
	ImVec2 gcp = ImGui::GetMousePos() - ImGui::GetWindowPos();
	ImVec2 gwsz = ImGui::GetWindowSize();
	Vec2 ncp = { safe_fdiv( gcp.x, gwsz.x ), safe_fdiv( gcp.y, gwsz.y ) };
	g_XFormState.OnUpdate( ncp, V2( gwsz.x, gwsz.y ) );
	
	if( g_NUIRenderView->CanAcceptKeyboardInput() )
	{
		if( ImGui::IsKeyPressed( SDLK_1, false ) ) g_showBodies = !g_showBodies;
		if( ImGui::IsKeyPressed( SDLK_2, false ) ) g_showJoints = !g_showJoints;
		if( ImGui::IsKeyPressed( SDLK_3, false ) ) g_showHitboxes = !g_showHitboxes;
		if( ImGui::IsKeyPressed( SDLK_4, false ) ) g_showAttachments = !g_showAttachments;
		if( ImGui::IsKeyPressed( SDLK_5, false ) ) g_showMasks = !g_showMasks;
		if( ImGui::IsKeyPressed( SDLK_6, false ) ) g_showPhysics = !g_showPhysics;
		if( ImGui::IsKeyPressed( SDLK_g, false ) ) g_XFormState.OnMoveKey();
		if( ImGui::IsKeyPressed( SDLK_r, false ) ) g_XFormState.OnRotateKey();
		if( ImGui::IsKeyPressed( SDLK_RETURN, false ) ||
			ImGui::IsMouseClicked( 0 ) ) g_XFormState.OnConfirmKey();
		if( ImGui::IsKeyPressed( SDLK_ESCAPE, false ) ||
			ImGui::IsMouseClicked( 1 ) ) g_XFormState.OnCancelKey();
	}
	
	// attachment names
	if( g_showAttachments )
	{
		for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
		{
			Mat4 wm;
			if( g_AnimCharInst->GetAttachmentMatrix( i, wm ) )
			{
				bool infront = true;
				Vec3 spos = g_EdScene->camera.WorldToScreen( wm.TransformPos( V3(0,0,0) ), &infront );
				if( infront )
				{
					ImGui::SetCursorPos( ImGui::GetWindowSize() * ImVec2( spos.x, spos.y ) );
					ImGui::Text( "%s", StackString<256>(g_AnimChar->attachments[ i ].name).str );
				}
			}
		}
	}
	
	ImVec2 r0 = ImGui::GetWindowPos() + ImVec2( 1, 1 );
	ImDrawList* idl = ImGui::GetWindowDrawList();
	idl->PushClipRectFullScreen();
	idl->AddRectFilled( r0, r0 + ImVec2( 610, 16 ), ImColor( 0.f, 0.f, 0.f, 0.5f ), 8.0f, 0x4 );
	
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2(0,0) );
	ImGui::SetCursorScreenPos( r0 + ImVec2( 4, -1 ) );
	ImGui::Text( "Render items: [Bodies (1): " );
	ImGui::SameLine(); IMGUIYesNo( g_showBodies );
	ImGui::SameLine(); ImGui::Text( ", Joints (2): " );
	ImGui::SameLine(); IMGUIYesNo( g_showJoints );
	ImGui::SameLine(); ImGui::Text( ", Hitboxes (3): " );
	ImGui::SameLine(); IMGUIYesNo( g_showHitboxes );
	ImGui::SameLine(); ImGui::Text( ", Attachments (4): " );
	ImGui::SameLine(); IMGUIYesNo( g_showAttachments );
	ImGui::SameLine(); ImGui::Text( ", Masks (5): " );
	ImGui::SameLine(); IMGUIYesNo( g_showMasks );
	ImGui::SameLine(); ImGui::Text( ", Physics (6): " );
	ImGui::SameLine(); IMGUIYesNo( g_showPhysics );
	ImGui::SameLine(); ImGui::Text( "]" );
	ImGui::PopStyleVar();
	
	if( g_AnimCharInst->skinName.size() == 0 )
	{
		ImGui::Text( "Warning! Empty skin selected." );
		ImGui::SameLine();
		if( ImGui::Button( "Reset to default" ) )
			g_AnimCharInst->SetSkin( "!default" );
	}
	
	idl->PopClipRect();
}

void DebugDraw()
{
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
	
	if( g_AnimCharInst->m_cachedMesh && g_AnimCharInst->m_cachedMeshInst )
	{
		if( g_showBodies )
		{
			br.Reset();
			for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			{
				Mat4 wm;
				if( g_AnimCharInst->_GetBodyMatrix( i, wm ) )
				{
					Vec3 size = g_AnimChar->bones[ i ].body.size;
					if( i == g_hoverBone )
						br.Col( 0.8f, 0.1f, 0.9f, 0.8f );
					else
						br.Col( 0.5f, 0.1f, 0.9f, 0.5f );
					switch( g_AnimChar->bones[ i ].body.type )
					{
					case AnimCharacter::BodyType_Sphere:
						br.SphereOutline( wm.TransformPos( V3(0) ), size.x, 32 );
						break;
					case AnimCharacter::BodyType_Capsule:
						br.CapsuleOutline( wm.TransformPos( V3(0) ), size.x,
							wm.TransformNormal( V3(0,0,1) ).Normalized(), ( size.z - size.x ) * 2, 32 );
						break;
					case AnimCharacter::BodyType_Box:
						br.AABB( -size, size, wm );
						break;
					}
				}
			}
		}
		
		if( g_showJoints )
		{
			br.Reset();
			for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			{
				float a = 0.5f;
				float d = 0.1f;
				
				AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ i ];
				if( BI.joint.type == AnimCharacter::JointType_None )
					continue;
				
				Mat4 wm;
				if( g_AnimCharInst->_GetJointMatrix( i, true, wm ) )
				{
					br.SetPrimitiveType( PT_Lines );
					br.Col( 0.9f, 0.9f, 0.9f, a );
					br.Pos( wm.TransformPos( V3(0,0,0) ) );
					br.Col( 1, 0, 0, a );
					br.Pos( wm.TransformPos( V3(d,0,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 1, 0, a );
					br.Pos( wm.TransformPos( V3(0,d,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 0, 1, a );
					br.Pos( wm.TransformPos( V3(0,0,d) ) );
					
					switch( BI.joint.type )
					{
					case AnimCharacter::JointType_Hinge:
						br.Col( 1, 0.5f, 0, a );
						br.ConeOutline(
							wm.TransformPos( V3(0,0,0) ),
							wm.TransformNormal( V3(0,0,1) ),
							wm.TransformNormal( V3(0,-1,0) ),
							0.1f,
							V2( BI.joint.turn_limit_1, 0 ),
							32 );
						break;
					case AnimCharacter::JointType_ConeTwist:
						br.Col( 1, 1, 0, a );
						br.ConeOutline(
							wm.TransformPos( V3(0,0,0) ),
							wm.TransformNormal( V3(0,0,1) ),
							wm.TransformNormal( V3(0,-1,0) ),
							0.1f,
							V2( BI.joint.turn_limit_1, BI.joint.turn_limit_2 ),
							32 );
						break;
					case AnimCharacter::JointType_None: break;
					}
				}
				
				if( g_AnimCharInst->_GetJointMatrix( i, false, wm ) )
				{
					br.SetPrimitiveType( PT_Lines );
					br.Col( 0.6f, 0.6f, 0.6f, a );
					br.Pos( wm.TransformPos( V3(0,0,0) ) );
					br.Col( 1, 0, 0, a );
					br.Pos( wm.TransformPos( V3(d,0,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 1, 0, a );
					br.Pos( wm.TransformPos( V3(0,d,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 0, 1, a );
					br.Pos( wm.TransformPos( V3(0,0,d) ) );
					
#if 0
					switch( BI.joint.type )
					{
					default:;
					}
#endif
				}
			}
		}
		
		if( g_showHitboxes )
		{
			SceneRaycastCallback_Closest srcc;
			Vec3 p0 = g_NUIRenderView->crpos;
			Vec3 p1 = p0 + g_NUIRenderView->crdir * 1000;
			g_AnimCharInst->RaycastAll( p0, p1, &srcc );
			
			br.Reset();
			for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			{
				bool hit = srcc.m_hit && g_AnimCharInst->_GetMeshBoneID( i ) == srcc.m_closest.boneID;
				Mat4 wm;
				Vec3 ext;
				if( g_AnimCharInst->_GetHitboxOBB( i, wm, ext ) )
				{
					if( i == g_hoverBone )
					{
						if( hit )
							br.Col( 0.9f, 0.8f, 0.1f, 0.9f );
						else
							br.Col( 0.1f, 0.8f, 0.9f, 0.8f );
					}
					else
					{
						if( hit )
							br.Col( 0.9f, 0.5f, 0.1f, 0.6f );
						else
							br.Col( 0.1f, 0.5f, 0.9f, 0.5f );
					}
					br.AABB( -ext, ext, wm );
				}
			}
		}
		
		// draw attachments
		if( g_showAttachments )
		{
			br.Reset();
			br.SetPrimitiveType( PT_Lines );
			for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
			{
				Mat4 wm;
				if( g_AnimCharInst->GetAttachmentMatrix( i, wm ) )
				{
					float a, d = 0.8f;
					if( i == g_hoverAtch )
						br.Col( 0.9f, 0.9f, 0.9f, a = 0.8f );
					else
						br.Col( 0.6f, 0.6f, 0.6f, a = 0.5f );
					br.Pos( wm.TransformPos( V3(0,0,0) ) );
					br.Col( 1, 0, 0, a );
					br.Pos( wm.TransformPos( V3(d,0,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 1, 0, a );
					br.Pos( wm.TransformPos( V3(0,d,0) ) );
					
					br.Prev( 1 );
					br.Col( 0, 0, 1, a );
					br.Pos( wm.TransformPos( V3(0,0,d) ) );
				}
			}
		}
		
		// draw current mask
		if( g_showMasks && g_AnimCharInst->m_cachedMesh && g_AnimCharInst->m_cachedMeshInst )
		{
			br.Reset();
			
			for( size_t i = 0; i < g_AnimChar->masks.size(); ++i )
			{
				if( i != g_hoverMask )
					continue; // displaying more than one at a time would lead to excessive noise
				
				float maskdata[ SGRX_MAX_MESH_BONES ] = {0};
				
				AnimCharacter::Mask& M = g_AnimChar->masks[ i ];
				for( size_t j = 0; j < M.cmds.size(); ++j )
				{
					AnimCharacter::MaskCmd& MC = M.cmds[ j ];
					GR_SetFactors( ArrayView<float>( maskdata, SGRX_MAX_MESH_BONES ),
						g_AnimCharInst->m_cachedMesh, MC.bone, MC.weight, MC.children, MC.mode );
				}
				
				for( int bone_id = 0; bone_id < g_AnimCharInst->m_cachedMesh->m_numBones; ++bone_id )
				{
					Mat4 bmtx = g_AnimCharInst->m_cachedMeshInst->matrix;
					if( g_AnimCharInst->m_cachedMeshInst->IsSkinned() )
						bmtx = g_AnimCharInst->m_cachedMeshInst->skin_matrices[ bone_id ] * bmtx;
					bmtx = g_AnimCharInst->m_cachedMesh->m_bones[ bone_id ].skinOffset * bmtx;
					
					Vec3 pos = bmtx.TransformPos( V3(0) );
					br.Col( 0.1f + ( 1 - maskdata[ bone_id ] ) * 0.8f, 0.1f + maskdata[ bone_id ] * 0.8f, 0.1f );
					br.Tick( pos, g_maskPreviewTickSize );
				}
			}
		}
		
		// draw physics
		if( g_showPhysics )
		{
			g_PhyWorld->DebugDraw();
		}
		
		// draw bones
		if( 0 )
		{
			br.Reset();
			SGRX_IMesh* mesh = g_AnimCharInst->m_cachedMeshInst->GetMesh();
			for( int i = 0; i < mesh->m_numBones; ++i )
			{
				float sxt = ( 1 - float(i) / mesh->m_numBones );
				br.Axis( mesh->m_bones[ i ].skinOffset, 0.1f + sxt * 0.1f );
			}
		}
	}
}


bool PickBoneName( const char* label, String& name, int& id, int self = -1 )
{
	String namelist = "<None>";
	namelist.push_back( '\0' );
	for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
	{
		if( self >= 0 )
		{
			int pb = i;
			while( pb >= 0 )
			{
				if( pb == self )
					break;
				pb = g_AnimCharInst->_FindParentBone( pb );
			}
			// can't have child bones/self in the list
			if( pb >= 0 )
				continue;
		}
		
		int pb = i;
		for(;;)
		{
			pb = g_AnimCharInst->_FindParentBone( pb );
			if( pb < 0 )
				break;
			namelist.push_back( '-' );
		}
		namelist.append( g_AnimChar->bones[ i ].name );
		namelist.push_back( '\0' );
	}
	namelist.push_back( '\0' );
	id++;
	bool ret = ImGui::Combo( label, &id, namelist.data() );
	id--;
	if( ret && id >= 0 && id < (int) g_AnimChar->bones.size() )
	{
		name = g_AnimChar->bones[ id ].name;
	}
	return ret;
}

void EditBoneInfo( size_t self_id, AnimCharacter::BoneInfo& bi )
{
	static const char* bi_body_types[] = { "None", "Sphere", "Capsule", "Box" };
	static const char* bi_joint_types[] = { "None", "Hinge", "Cone/twist" };
	
	ImVec2 cursorBegin = ImGui::GetCursorPos();
	
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &bi, "Bone: %s [id=%d]",
		StackString<256>(bi.name).str,
		g_AnimCharInst->_GetMeshBoneID( self_id ) ) )
	{
		if( IMGUIEditString( "Name", bi.name, 256 ) )
			g_AnimCharInst->_RecalcBoneIDs();
		
		IMGUI_GROUP( "Hitbox", true,
		{
			IMGUIEditVec3( "Position", bi.hitbox.position, -100, 100 );
			IMGUIEditQuat( "Rotation", bi.hitbox.rotation );
			if( ImGui::Button( "Select hitbox for XForm", ImVec2( ImGui::GetContentRegionAvail().x, 20 ) ) )
			{
				g_XFormState.name = bi.name;
				g_XFormState.type = TT_BoneHitbox;
			}
			IMGUIEditVec3( "Extents", bi.hitbox.extents, 0.01f, 100 );
			IMGUIEditFloat( "Multiplier", bi.hitbox.multiplier, 0, 1000 );
		});
		IMGUI_GROUP( "Body", true,
		{
			IMGUIEditVec3( "Position", bi.body.position, -100, 100 );
			IMGUIEditQuat( "Rotation", bi.body.rotation );
			if( ImGui::Button( "Select body for XForm", ImVec2( ImGui::GetContentRegionAvail().x, 20 ) ) )
			{
				g_XFormState.name = bi.name;
				g_XFormState.type = TT_BoneBody;
			}
			IMGUI_COMBOBOX( "Type", bi.body.type, bi_body_types );
			if( bi.body.type == AnimCharacter::BodyType_Sphere ||
				bi.body.type == AnimCharacter::BodyType_Capsule )
				IMGUIEditFloat( "Radius", bi.body.size.x, 0.01f, 100 );
			if( bi.body.type == AnimCharacter::BodyType_Capsule )
				IMGUIEditFloat( "Height", bi.body.size.z, 0.01f, 100 );
			if( bi.body.type == AnimCharacter::BodyType_Box )
				IMGUIEditVec3( "Size", bi.body.size, 0.01f, 100 );
		});
		IMGUI_GROUP( "Joint", true,
		{
			PickBoneName( "Parent bone", bi.joint.parent_name, bi.joint.parent_id, self_id );
			IMGUI_COMBOBOX( "Type", bi.joint.type, bi_joint_types );
			IMGUIEditVec3( "Position [self]", bi.joint.self_position, -100, 100 );
			IMGUIEditQuat( "Rotation [self]", bi.joint.self_rotation );
			if( ImGui::Button( "Select local joint frame for XForm", ImVec2( ImGui::GetContentRegionAvail().x, 20 ) ) )
			{
				g_XFormState.name = bi.name;
				g_XFormState.type = TT_BoneJointSelfFrame;
			}
			IMGUIEditVec3( "Position [parent]", bi.joint.prnt_position, -100, 100 );
			IMGUIEditQuat( "Rotation [parent]", bi.joint.prnt_rotation );
			if( ImGui::Button( "Select parent joint frame for XForm", ImVec2( ImGui::GetContentRegionAvail().x, 20 ) ) )
			{
				g_XFormState.name = bi.name;
				g_XFormState.type = TT_BoneJointParentFrame;
			}
			if( bi.joint.type == AnimCharacter::JointType_Hinge )
			{
				IMGUIEditFloat( "Min. rotation", bi.joint.turn_limit_1, -360, 360 );
				IMGUIEditFloat( "Max. rotation", bi.joint.turn_limit_2, -360, 360 );
			}
			if( bi.joint.type == AnimCharacter::JointType_ConeTwist )
			{
				IMGUIEditFloat( "X limit", bi.joint.turn_limit_1, 0, 360 );
				IMGUIEditFloat( "Y limit", bi.joint.turn_limit_2, 0, 360 );
				IMGUIEditFloat( "Twist limit", bi.joint.twist_limit, 0, 360 );
			}
		});
		
		ImGui::TreePop();
	}
	
	ImVec2 cursorEnd = ImGui::GetCursorPos();
	ImGui::SetCursorPos( cursorBegin );
	ImGui::InvisibleButton( "hl area", ImVec2( ImGui::GetContentRegionAvailWidth(), cursorEnd.y - cursorBegin.y ) );
	if( ImGui::IsItemHovered() )
		SetHovered( self_id, NOT_FOUND, NOT_FOUND );
	ImGui::SetCursorPos( cursorEnd );
}

void EditAttachmentInfo( size_t self_id, AnimCharacter::Attachment& atch )
{
	ImVec2 cursorBegin = ImGui::GetCursorPos();
	
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &atch, "Attachment: %s", StackString<256>(atch.name).str ) )
	{
		IMGUIEditString( "Name", atch.name, 256 );
		PickBoneName( "Bone", atch.bone, atch.bone_id );
		IMGUIEditVec3( "Position", atch.position, -100, 100 );
		IMGUIEditQuat( "Rotation", atch.rotation );
		if( ImGui::Button( "Select attachment for XForm", ImVec2( ImGui::GetContentRegionAvail().x, 20 ) ) )
		{
			g_XFormState.name = atch.name;
			g_XFormState.type = TT_Attachment;
		}
		
		ImGui::TreePop();
	}
	
	ImVec2 cursorEnd = ImGui::GetCursorPos();
	ImGui::SetCursorPos( cursorBegin );
	ImGui::InvisibleButton( "hl area", ImVec2( ImGui::GetContentRegionAvailWidth(), cursorEnd.y - cursorBegin.y ) );
	if( ImGui::IsItemHovered() )
		SetHovered( NOT_FOUND, self_id, NOT_FOUND );
	ImGui::SetCursorPos( cursorEnd );
}

const char* SFModeToString( uint8_t mode )
{
	if( mode == SFM_Set ) return "Set";
	if( mode == SFM_Add ) return "Add";
	if( mode == SFM_AddDist ) return "Add-Dist";
	return "<unknown>";
}

void EditMaskCmdInfo( size_t, AnimCharacter::MaskCmd& mcmd )
{
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &mcmd, "Mask command: %s, weight = %g, mode = %s%s",
		StackString<256>(mcmd.bone).str, mcmd.weight,
		SFModeToString( mcmd.mode ),
		mcmd.children ? ", incl. children" : "" ) )
	{
		int id = -1;
		for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
		{
			if( g_AnimChar->bones[ i ].name == mcmd.bone )
			{
				id = i;
				break;
			}
		}
		PickBoneName( "Bone", mcmd.bone, id );
		IMGUIEditFloat( "Weight", mcmd.weight, -1, 1 );
		IMGUIComboBox( "Mode", mcmd.mode, "Set\0Add\0Add-Dist\0" );
		ImGui::Checkbox( "Include children?", &mcmd.children );
		
		ImGui::TreePop();
	}
}

void EditMaskInfo( size_t self_id, AnimCharacter::Mask& mask )
{
	ImVec2 cursorBegin = ImGui::GetCursorPos();
	
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &mask, "Mask: %s", StackString<256>(mask.name).str ) )
	{
		IMGUIEditString( "Name", mask.name, 256 );
		IMGUI_GROUP( "Commands", true,
		{
			IMGUIEditArray( mask.cmds, EditMaskCmdInfo, "Add command" );
		});
		
		ImGui::TreePop();
	}
	
	ImVec2 cursorEnd = ImGui::GetCursorPos();
	ImGui::SetCursorPos( cursorBegin );
	ImGui::InvisibleButton( "hl area", ImVec2( ImGui::GetContentRegionAvailWidth(), cursorEnd.y - cursorBegin.y ) );
	if( ImGui::IsItemHovered() )
		SetHovered( NOT_FOUND, NOT_FOUND, self_id );
	ImGui::SetCursorPos( cursorEnd );
}

static void GenAnimPrintName( char bfr[128], StringView anim, const char* def )
{
	if( anim )
	{
		RCString* bundle = g_NUIAnimPicker->m_nameMap.getptr( anim );
		if( bundle )
		{
			anim = *bundle;
			StringView bundlepath = anim.until( ":" );
			StringView animname = anim.after( ":" );
			StringView bundlename = bundlepath.after_last( "/" );
			StringView bundledir = bundlepath.until_last( "/" );
			sgrx_snprintf( bfr, 128, "%s@%s|%s", StackPath(animname).str,
				StackPath(bundlename).str, StackPath(bundledir).str );
		}
		else
		{
			StringView bundlepath = anim.until( ":" );
			StringView animname = anim.after( ":" );
			sgrx_snprintf( bfr, 128, "%s@%s", StackPath(animname).str,
				StackPath(bundlepath).str );
		}
	}
	else
		strcpy( bfr, def );
}


static void EditMapping( HashTable< StringView, AnimCharacter::MappedAnim >& mapping )
{
	Array< AnimCharacter::MappedAnim* > items;
	for( size_t i = 0; i < mapping.size(); ++i )
		items.push_back( &mapping.item( i ).value );
	
	// sort
	
	for( size_t i = 0; i < items.size(); ++i )
	{
		AnimCharacter::MappedAnim& MA = *items[ i ];
		
		if( ImGui::TreeNode( &MA, "Mapping: %s => %s",
			MA.name.c_str(), MA.anim.c_str() ) )
		{
			ImVec2 cp_before = ImGui::GetCursorPos();
			float width = ImGui::GetContentRegionAvail().x;
			
			// CANNOT EDIT IT THIS WAY
		//	IMGUIEditString( "Name", MA.name, 256 );
			{
				const char* caption = "Select animation for state";
				char bfr[ 128 ];
				GenAnimPrintName( bfr, MA.anim, "<click to select animation>" );
				if( ImGui::Button( bfr, ImVec2( ImGui::GetContentRegionAvailWidth() * 2.0f/3.0f, 20 ) ) )
				{
					g_NUIAnimPicker->OpenPopup( caption );
				}
				ImGui::SameLine();
				ImGui::Text( "Animation" );
				g_NUIAnimPicker->Popup( caption, MA.anim );
			}
			IMGUIEditInt( "ID", MA.id, 0, 65535 );
			
			ImVec2 cp_after = ImGui::GetCursorPos();
			ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
			if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
			{
				RCString name = MA.name;
				mapping.unset( name );
			}
			ImGui::SetCursorPos( cp_after );
			
			ImGui::TreePop();
		}
	}
	
	// create new
	static String mapname;
	if( ImGui::Button( "Add mapping",
			ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
	{
		mapname = "";
		ImGui::OpenPopup( "create_mapping" );
	}
	
	if( ImGui::BeginPopup( "create_mapping" ) )
	{
		IMGUIEditString( "Name", mapname, 256 );
		if( ImGui::Button( "Create" ) )
		{
			AnimCharacter::MappedAnim ma = { mapname, "", 0 };
			for( size_t i = 0; i < mapping.size(); ++i )
			{
				if( ma.id < mapping.item( i ).value.id )
					ma.id = mapping.item( i ).value.id;
			}
			ma.id++;
			mapping.set( ma.name, ma );
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


static void EditSkins( HashTable< StringView, AnimCharacter::Skin >& skins )
{
	if( ImGui::Button( "Reset to default" ) )
	{
		g_AnimCharInst->SetSkin( "!default" );
	}
	ImGui::SameLine();
	if( ImGui::Button( "Set to empty" ) )
	{
		g_AnimCharInst->SetSkin( "" );
	}
	
	Array< AnimCharacter::Skin* > items;
	for( size_t i = 0; i < skins.size(); ++i )
		items.push_back( &skins.item( i ).value );
	
	// sort
	
	for( size_t i = 0; i < items.size(); ++i )
	{
		AnimCharacter::Skin& SK = *items[ i ];
		
		ImGui::PushID( &SK );
		
		ImVec2 cp_before = ImGui::GetCursorPos();
		float width = ImGui::GetContentRegionAvail().x;
		
		if( ImGui::TreeNode( &SK, "Skin: %s => %s",
			SK.name.c_str(), SK.mesh.c_str() ) )
		{
			
			// CANNOT EDIT IT THIS WAY
		//	IMGUIEditString( "Name", SK.name, 256 );
			{
				const char* caption = "Select mesh for skin";
				if( ImGui::Button( SK.mesh.size() ? SK.mesh.c_str() : "<click to select mesh>",
					ImVec2( ImGui::GetContentRegionAvailWidth() * 2.0f/3.0f, 20 ) ) )
				{
					g_NUIMeshPicker->OpenPopup( caption );
				}
				ImGui::SameLine();
				ImGui::Text( "Animation" );
				g_NUIMeshPicker->Popup( caption, SK.mesh );
			}
			
			ImGui::TreePop();
		}
		
		ImVec2 cp_after = ImGui::GetCursorPos();
		ImGui::SetCursorPos( cp_before + ImVec2( width - 60, 0 ) );
		if( ImGui::Button( "[set]", ImVec2( 30, 14 ) ) )
		{
			g_AnimCharInst->SetSkin( SK.name );
		}
		ImGui::SetCursorPos( cp_before + ImVec2( width - 30, 0 ) );
		if( ImGui::Button( "[del]", ImVec2( 30, 14 ) ) )
		{
			RCString name = SK.name;
			skins.unset( name );
		}
		ImGui::SetCursorPos( cp_after );
		
		ImGui::PopID();
	}
	
	// create new
	static String skinname;
	if( ImGui::Button( "Add skin",
			ImVec2( ImGui::GetContentRegionAvailWidth() * 2.f/3.f, 20 ) ) )
	{
		skinname = "";
		ImGui::OpenPopup( "create_skin" );
	}
	
	if( ImGui::BeginPopup( "create_skin" ) )
	{
		IMGUIEditString( "Name", skinname, 256 );
		if( ImGui::Button( "Create" ) )
		{
			AnimCharacter::Skin sk = { skinname, "" };
			skins.set( sk.name, sk );
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


static HashTable< void*, RCString > g_ExprCompileResults;
void EditValExpr( const char* label, AnimCharacter::ValExpr& expr )
{
	if( IMGUIEditString( label, expr.expr, 256 ) )
	{
		MECompileResult cr = expr.Recompile( g_AnimChar );
		if( cr )
		{
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "%s at pos. %d (%s...)",
				StackString<128>(cr.error).str,
				int( cr.unparsed.data() - expr.expr.data() ),
				StackString<16>(cr.unparsed).str );
			g_ExprCompileResults.set( &expr, bfr );
		}
		else
			g_ExprCompileResults.unset( &expr );
	}
	RCString* errptr = g_ExprCompileResults.getptr( &expr );
	if( errptr )
		ImGui::Text( "Error: %s", errptr->c_str() );
	else
		ImGui::Text( "Value: %g", expr.compiled_expr.Eval( g_AnimChar ) );
}

void EditVariableInfo( size_t self_id, Handle<AnimCharacter::Variable>& var )
{
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &var, "Variable: %s", StackString<256>(var->name).str ) )
	{
		IMGUIEditString( "Name", var->name, 256 );
		float v = var->value;
		IMGUIEditFloat( "Value (initial)", v, -FLT_MAX, FLT_MAX );
		var->value = v;
		
		ImGui::TreePop();
	}
}

void EditAliasInfo( size_t self_id, Handle<AnimCharacter::Alias>& alias )
{
	ImGui::SetNextTreeNodeOpened( true, ImGuiSetCond_Appearing );
	if( ImGui::TreeNode( &alias, "Alias: %s", StackString<256>(alias->name).str ) )
	{
		IMGUIEditString( "Name", alias->name, 256 );
		EditValExpr( "Value", alias->expr );
		ImGui::TreePop();
	}
}

void EditACVariables()
{
	AnimCharacter& ac = *g_AnimChar;
	IMGUI_GROUP( "Variables", false,
	{
		ImGui::BeginChangeCheck();
		IMGUIEditArray( ac.variables, EditVariableInfo, NULL );
		if( ImGui::Button( "Add variable" ) )
		{
			ac.variables.push_back( new AnimCharacter::Variable );
		}
		if( ImGui::EndChangeCheck() )
			ac._ReindexVariables();
	});
	
	IMGUI_GROUP( "Aliases", false,
	{
		IMGUIEditArray( ac.aliases, EditAliasInfo, NULL );
		if( ImGui::Button( "Add alias" ) )
		{
			ac.aliases.push_back( new AnimCharacter::Alias );
		}
	});
}

void EditAnimChar()
{
	AnimCharacter& ac = *g_AnimChar;
	if( g_NUIMeshPicker->Property( "Pick mesh", "mesh", ac.mesh ) )
	{
		LOG << "Picked MESH: " << ac.mesh;
		g_AnimCharInst->_OnRenderUpdate();
		SGRX_IMesh* M = g_AnimCharInst->m_cachedMesh;
		if( M )
		{
			for( int i = 0; i < M->m_numBones; ++i )
			{
				StringView name = M->m_bones[ i ].name;
				size_t j = 0;
				for( ; j < ac.bones.size(); ++j )
				{
					if( ac.bones[ j ].name == name )
						break;
				}
				if( j == ac.bones.size() )
				{
					AnimCharacter::BoneInfo B;
					B.name = name;
					ac.bones.push_back( B );
				}
			}
		}
		reload_mesh_vertices();
		g_AnimCharInst->_RecalcBoneIDs();
		lmm_prepmeshinst( g_AnimCharInst->m_cachedMeshInst );
	}
	IMGUI_GROUP( "Bones", false,
	{
		IMGUIEditArray( ac.bones, EditBoneInfo, "Add bone" );
	});
	IMGUI_GROUP( "Attachments", false,
	{
		IMGUIEditArray( ac.attachments, EditAttachmentInfo, "Add attachment" );
	});
	IMGUI_GROUP( "Masks", false,
	{
		IMGUIEditArray( ac.masks, EditMaskInfo, "Add mask" );
	});
	IMGUI_GROUP( "Anim. mapping", false,
	{
		EditMapping( ac.mapping );
	});
	IMGUI_GROUP( "Skins", false,
	{
		EditSkins( ac.skins );
	});
	EditACVariables();
}


bool PickMaskName( const char* label, String& name )
{
	String namelist = "<None>";
	namelist.push_back( '\0' );
	int id = -1;
	for( size_t i = 0; i < g_AnimChar->masks.size(); ++i )
	{
		if( g_AnimChar->masks[ i ].name.size() &&
			name == g_AnimChar->masks[ i ].name )
			id = i;
		namelist.append( g_AnimChar->masks[ i ].name );
		namelist.push_back( '\0' );
	}
	namelist.push_back( '\0' );
	id++;
	bool ret = ImGui::Combo( label, &id, namelist.data() );
	id--;
	if( ret )
	{
		name = size_t(id) < g_AnimChar->masks.size() ?
			g_AnimChar->masks[ id ].name : String();
	}
	return ret;
}

const float NODE_SLOT_RADIUS = 4.0f;
const ImVec2 NODE_WINDOW_PADDING(4.0f, 4.0f);

static Handle<AnimCharacter::Node> g_NodeLinkP1;
static Handle<AnimCharacter::Node> g_NodeLinkP2;
static SGRX_GUID* g_NodeLinkP2GUID;

static Handle<AnimCharacter::State> g_StateLinkP1;
static Handle<AnimCharacter::State> g_StateLinkP2;

struct DrawNodeLink
{
	Handle<AnimCharacter::Node> node1;
	Handle<AnimCharacter::Node> node2;
	ImVec2 node2pos;
};
static Array< DrawNodeLink > g_TempNodeLinks;

struct DrawStateLink
{
	Handle<AnimCharacter::State> state1;
	Handle<AnimCharacter::State> state2;
	ImVec2 state2pos;
};
static Array< DrawStateLink > g_TempStateLinks;

static bool HoverLink( ImVec2 p1, ImVec2 p2 )
{
	Vec3 mp = V3(ImGui::GetMousePos().x, ImGui::GetMousePos().y, 0.0f);
	// Hermite spline
	ImVec2 t1 = ImVec2(+80.0f, 0.0f);
	ImVec2 t2 = ImVec2(+80.0f, 0.0f);
	ImVec2 pp = p1;
	const int STEPS = 12;
	for (int step = 0; step <= STEPS; step++)
	{
		float t = (float)step / (float)STEPS;
		float h1 = +2*t*t*t - 3*t*t + 1.0f;
		float h2 = -2*t*t*t + 3*t*t;
		float h3 =    t*t*t - 2*t*t + t;
		float h4 =    t*t*t -   t*t;
		ImVec2 pc(h1*p1.x + h2*p2.x + h3*t1.x + h4*t2.x, h1*p1.y + h2*p2.y + h3*t1.y + h4*t2.y);
		Vec3 pp3 = V3(pp.x, pp.y, 0);
		Vec3 pc3 = V3(pc.x, pc.y, 0);
		Vec3 pn3 = ( pc3 - pp3 ).Normalized();
		pp3 -= pn3 * 1.5f;
		pc3 += pn3 * 1.5f;
		if( PointLineDistance( mp, pp3, pc3 ) < 1.5f )
			return true;
		pp = pc;
	}
	return false;
}

static void DrawLink( ImDrawList* draw_list, ImVec2 p1, ImVec2 p2, ImColor col = ImColor(200,200,100) )
{
	// Hermite spline
	ImVec2 t1 = ImVec2(+80.0f, 0.0f);
	ImVec2 t2 = ImVec2(+80.0f, 0.0f);
	const int STEPS = 12;
	for (int step = 0; step <= STEPS; step++)
	{
		float t = (float)step / (float)STEPS;
		float h1 = +2*t*t*t - 3*t*t + 1.0f;
		float h2 = -2*t*t*t + 3*t*t;
		float h3 =    t*t*t - 2*t*t + t;
		float h4 =    t*t*t -   t*t;
		draw_list->PathLineTo(ImVec2(h1*p1.x + h2*p2.x + h3*t1.x + h4*t2.x, h1*p1.y + h2*p2.y + h3*t1.y + h4*t2.y));
	}
	draw_list->PathStroke(col, false, 3.0f);
}


//
//  S T A T E   E D I T O R
//

static AnimCharacter::PlayerNode* g_EditedPlayerNode = NULL;
static AnimCharacter::State* g_SelState = NULL;
static AnimCharacter::Transition* g_SelTransition = NULL;
static ImVec2 g_StateCameraPos(0,0);

static void AfterReload()
{
	g_EditedPlayerNode = NULL;
	g_SelState = NULL;
	g_SelTransition = NULL;
}

void EditTransition( AnimCharacter::Transition* tr )
{
	AnimCharacter::PlayerNode* P = g_EditedPlayerNode;
	AnimCharacter::State* a = NULL, * b = NULL;
	bool fromany = false;
	for( size_t i = 0; i < P->states.size(); ++i )
	{
		AnimCharacter::State* s = P->states[ i ];
		if( tr->source.IsNull() )
			fromany = true;
		if( s->guid == tr->source )
			a = s;
		if( s->guid == tr->target )
			b = s;
	}
	
	ImGui::Text( "Transition: %s -> %s",
		fromany ? "<any>" : ( a ? StackPath(a->GetName()).str : "<!err:a>" ),
		b ? StackPath(b->GetName()).str : "<!err:b>" );
	ImGui::Separator();
	if( ImGui::Button( "Delete" ) )
	{
		P->transitions.remove_first( tr );
		P->RehashTransitions();
		if( g_SelTransition == tr )
			g_SelTransition = NULL;
		return;
	}
	if( !fromany )
	{
		if( IMGUIEditBool( "Bidirectional", tr->bidi ) )
			P->RehashTransitions();
	}
	EditValExpr( "Condition", tr->expr );
	
	ImGui::Separator();
}

void EditACStateProps()
{
	AnimCharacter::PlayerNode* P = g_EditedPlayerNode;
	
	if( g_SelTransition )
	{
		ImGui::PushID( -1 );
		EditTransition( g_SelTransition );
		ImGui::PopID();
	}
	
	if( g_SelState )
	{
		ImGui::Text( "State '%s'", StackPath(g_SelState->GetName()).str );
		ImGui::Separator();
		
		IMGUIEditString( "Name", g_SelState->name, 256 );
		{
			const char* caption = "Select animation for state";
			char bfr[ 128 ];
			GenAnimPrintName( bfr, g_SelState->anim, "<click to select animation>" );
			if( ImGui::Button( bfr, ImVec2( ImGui::GetContentRegionAvailWidth() * 2.0f/3.0f, 20 ) ) )
			{
				g_NUIAnimPicker->OpenPopup( caption );
			}
			ImGui::SameLine();
			ImGui::Text( "Animation" );
			g_NUIAnimPicker->Popup( caption, g_SelState->anim );
		}
		IMGUIComboBox( "Playback mode", g_SelState->playMode, "Loop\0Once\0Clamp\0" );
		EditValExpr( "Speed", g_SelState->speed );
		IMGUIEditFloat( "Fade time", g_SelState->fade_time, 0, 1000 );
		IMGUIEditBool( "Fade using speed", g_SelState->fade_speed );
		
		IMGUI_GROUP( "Transitions from anywhere", true,
		{
			for( size_t i = 0; i < P->transitions.size(); ++i )
			{
				AnimCharacter::Transition* tr = P->transitions[ i ];
				if( tr->source.NotNull() || tr->target != g_SelState->guid )
					continue;
				ImGui::PushID( i );
				EditTransition( tr );
				ImGui::PopID();
			}
		});
		
		ImGui::Separator();
	}
}

static void TryAddTransition( AnimCharacter::State* a, AnimCharacter::State* b )
{
	AnimCharacter::PlayerNode* P = g_EditedPlayerNode;
	SGRX_GUID srcguid = a ? a->guid : SGRX_GUID::Null;
	for( size_t i = 0; i < P->transitions.size(); ++i )
	{
		AnimCharacter::Transition* tr = P->transitions[ i ];
		if( tr->source == srcguid && tr->target == b->guid )
			return; // already exists
	}
	
	AnimCharacter::Transition* tr = new AnimCharacter::Transition;
	tr->source = srcguid;
	tr->target = b->guid;
	P->transitions.push_back( tr );
	P->RehashTransitions();
	g_SelTransition = tr;
}
static void EditPlayerStates()
{
	AnimCharacter::PlayerNode* P = g_EditedPlayerNode;
	
	ImGui::BeginChangeCheck();
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2(1,1) );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(0,0) );
	ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImColor(40,40,40,200) );
	ImGui::BeginChild( "scrolling_region", ImVec2(0,0), true, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove );
	ImGui::PushItemWidth( 100.0f );
	
	AnimCharacter::State* node_hovered_in_list = NULL;
	AnimCharacter::State* node_hovered_in_scene = NULL;
	bool open_context_menu = false;
	
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit( 3 );
	ImVec2 left_upper_corner = ImGui::GetCursorScreenPos();
	ImVec2 offset = ImGui::GetCursorScreenPos()
		- ImGui::GetContentRegionAvail() * ImVec2( 0.5f, 0.5f )
		- g_StateCameraPos;
	
	HashTable< SGRX_GUID, ImVec2 > inPosMap;
	HashTable< SGRX_GUID, ImVec2 > outPosMap;
//	outPosMap.set( SGRX_GUID::Null, left_upper_corner );
	for( size_t i = 0; i < P->states.size(); ++i )
	{
		AnimCharacter::State* node = P->states[ i ];
		Vec2& pos = node->editor_pos;
		
		ImGui::PushID( node );
		ImVec2 node_rect_min = offset + ImVec2( pos.x, pos.y );
		draw_list->ChannelsSetCurrent( 2 );
		
		bool old_any_active = ImGui::IsAnyItemActive();
		ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
		ImGui::BeginGroup();
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2(4,4) );
		ImGui::Text( "%s", StackPath(node->GetName()).str );
		ImGui::Dummy( ImVec2(140, 1) );
		{
			char bfr[ 128 ];
			GenAnimPrintName( bfr, node->anim, "<no animation>" );
			ImGui::Text( "%s", bfr );
		}
		ImGui::PopStyleVar(1);
		ImGui::EndGroup();
		
		ImVec2 nodeSize = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
		ImVec2 node_rect_max = node_rect_min + nodeSize;
		
		bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
		
		// draw box, do links
		draw_list->ChannelsSetCurrent(1); // Background
		ImVec2 node_title_max = node_rect_min + ImVec2( nodeSize.x, 24 );
		draw_list->AddRectFilled(node_rect_min, node_rect_max, (node_hovered_in_list == node || node_hovered_in_scene == node || (node_hovered_in_list == NULL && g_SelState == node)) ? ImColor(75,75,75,200) : ImColor(60,60,60,200), 4.0f); 
		draw_list->AddRectFilled(node_rect_min, node_title_max, ImColor(20,20,20,200), 4.0f);
		draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100+(node == P->starting_state?120:0),100,100), 4.0f);
		
		// input link
		{
			ImVec2 pos = ImVec2( node_rect_min.x, node_rect_min.y + 12 );
			inPosMap.set( node->guid, pos );
			draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, ImColor(150,150,150,150));
			
			ImVec2 scpos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos( pos - ImVec2( NODE_SLOT_RADIUS, NODE_SLOT_RADIUS ) );
			ImGui::InvisibleButton( "ilink", ImVec2( NODE_SLOT_RADIUS * 2.0f, NODE_SLOT_RADIUS * 2.0f ) );
			if( ImGui::IsItemHoveredRect() )
			{
				if( ImGui::IsMouseClicked(0) )
				{
					g_StateLinkP2 = node;
				}
				else if( ImGui::IsMouseReleased(0) && g_StateLinkP1 )
				{
					TryAddTransition( g_StateLinkP1, node );
				}
			}
			if( g_StateLinkP2 == node )
			{
				DrawLink( draw_list, ImGui::GetMousePos(), pos, ImColor(200,200,100,127) );
			}
			ImGui::SetCursorScreenPos( scpos );
		}
		// output link
		{
			ImVec2 pos = ImVec2( node_rect_max.x, node_rect_min.y + 12 );
			outPosMap.set( node->guid, pos );
			draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, ImColor(150,150,150,150));
			
			ImVec2 scpos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos( pos - ImVec2( NODE_SLOT_RADIUS, NODE_SLOT_RADIUS ) );
			ImGui::InvisibleButton( "olink", ImVec2( NODE_SLOT_RADIUS * 2.0f, NODE_SLOT_RADIUS * 2.0f ) );
			if( ImGui::IsItemHoveredRect() )
			{
				if( ImGui::IsMouseClicked(0) )
				{
					g_StateLinkP1 = node;
				}
				else if( ImGui::IsMouseReleased(0) && g_StateLinkP2 )
				{
					TryAddTransition( node, g_StateLinkP2 );
				}
			}
			if( g_StateLinkP1 == node )
			{
				DrawLink( draw_list, pos, ImGui::GetMousePos(), ImColor(200,200,100,127) );
			}
			ImGui::SetCursorScreenPos( scpos );
		}
		
		// node box
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node", nodeSize);
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_scene = node;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		bool node_moving_active = ImGui::IsItemActive();
		if (node_widgets_active || node_moving_active)
			g_SelState = node;
		if (node_moving_active && ImGui::IsMouseDragging(0))
		{
			pos.x += ImGui::GetIO().MouseDelta.x;
			pos.y += ImGui::GetIO().MouseDelta.y;
		}
		
		ImGui::PopID();
	}
	
	draw_list->ChannelsSetCurrent(0); // links
	AnimCharacter::Transition* tr_hover = NULL;
	for( size_t i = 0; i < P->transitions.size(); ++i )
	{
		AnimCharacter::Transition* tr = P->transitions[ i ];
		ImVec2 p2 = inPosMap.getcopy( tr->target );
		ImVec2 p1 = tr->source.NotNull() ?
			outPosMap.getcopy( tr->source ) :
			p2 + ImVec2( -20, 0 );
		if( HoverLink( p1, p2 ) )
			tr_hover = tr;
	}
	if( ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(0) )
		g_SelTransition = tr_hover;
	for( size_t i = 0; i < P->transitions.size(); ++i )
	{
		AnimCharacter::Transition* tr = P->transitions[ i ];
		ImColor c = ImColor(150,150,100);
		if( tr == tr_hover )
			c.Value.x += 50/255.0f; c.Value.y += 50/255.0f; c.Value.z += 50/255.0f;
		if( tr == g_SelTransition )
			c.Value.z += 100/255.0f;
		ImVec2 p2 = inPosMap.getcopy( tr->target );
		ImVec2 p1 = tr->source.NotNull() ?
			outPosMap.getcopy( tr->source ) :
			p2 + ImVec2( -20, 0 );
		DrawLink( draw_list, p1, p2, c );
	}
	
	draw_list->ChannelsMerge();
	
	// Link post-processing
	if( ImGui::IsMouseReleased(0) )
	{
		g_StateLinkP1 = NULL;
		g_StateLinkP2 = NULL;
	}
	
	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
	{
		g_SelState = node_hovered_in_list = node_hovered_in_scene = NULL;
		open_context_menu = true;
	}
	if (open_context_menu)
	{
		ImGui::OpenPopup("context_menu");
		if (node_hovered_in_list != NULL)
			g_SelState = node_hovered_in_list;
		if (node_hovered_in_scene != NULL)
			g_SelState = node_hovered_in_scene;
	}
	
	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
	if (ImGui::BeginPopup("context_menu"))
	{
		AnimCharacter::State* node = g_SelState;
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (node)
		{
			ImGui::Text("State '%s'", StackString<100>(node->GetName()).str);
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
			{
				for( size_t i = 0; i < P->transitions.size(); ++i )
				{
					AnimCharacter::Transition* tr = P->transitions[ i ];
					if( tr->source == node->guid || tr->target == node->guid )
					{
						if( g_SelTransition == tr )
							g_SelTransition = NULL;
						P->transitions.erase( i-- );
					}
				}
				P->states.remove_first( node );
				P->RehashStates();
				P->RehashTransitions();
				g_SelState = NULL;
			}
			ImGui::Separator();
			if( ImGui::MenuItem( "Create transition from anywhere" ) )
			{
				TryAddTransition( NULL, node );
			}
			if( ImGui::MenuItem( "Set as current" ) )
			{
				for( size_t i = 0; i < g_AnimCharInst->m_rtnodes.size(); ++i )
				{
					AnimCharInst::NodeRT* n = g_AnimCharInst->m_rtnodes[ i ];
					if( n->src->type == AnimCharacter::NT_Player )
					{
						SGRX_CAST( AnimCharInst::PlayerNodeRT*, PNRT, n );
						PNRT->current_state = node;
						PNRT->StartCurrentState();
					}
				}
			}
			if( ImGui::MenuItem( "Set as first" ) )
			{
				P->starting_state = node;
			}
			if (ImGui::MenuItem("Copy", NULL, false, false)) {}
		}
		else
		{
			AnimCharacter::State* ns = NULL;
			if (ImGui::MenuItem("Add state"))
				ns = new AnimCharacter::State;
			if( ns )
			{
				ns->Init( V2( scene_pos ) );
				P->states.push_back( ns );
				P->RehashStates();
				P->RehashTransitions();
				g_SelState = ns;
				if( !P->starting_state )
					P->starting_state = ns;
				ImGui::TriggerChangeCheck();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Paste", NULL, false, false)) {}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	
	// MMB view scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
	{
		g_StateCameraPos -= ImGui::GetIO().MouseDelta;
	}
	
	ImGui::SetCursorScreenPos( left_upper_corner );
	if( ImGui::Button( "Close" ) )
	{
		g_EditedPlayerNode = NULL;
	}
	
	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	if( ImGui::EndChangeCheck() )
		; // WHAT
}



//
//  N O D E   E D I T O R
//

void EditNodeInput( const char* label, AnimCharacter::Node* node, SGRX_GUID& guid )
{
	ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( -4, 9 );
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImGui::PushID( label );
	ImGui::Text( "%s", label );
	draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, ImColor(150,150,150,150));
	
	AnimCharacter::Node* node_in = g_AnimChar->_FindNodeByGUID( guid );
	if( node_in )
	{
		DrawNodeLink dnl = { node_in, node, pos };
		g_TempNodeLinks.push_back( dnl );
	}
	
	ImVec2 scpos = ImGui::GetCursorScreenPos();
	ImGui::SetCursorScreenPos( pos - ImVec2( NODE_SLOT_RADIUS, NODE_SLOT_RADIUS ) );
	ImGui::InvisibleButton( "ilink", ImVec2( NODE_SLOT_RADIUS * 2.0f, NODE_SLOT_RADIUS * 2.0f ) );
	if( ImGui::IsItemHoveredRect() )
	{
		if( ImGui::IsMouseClicked(0) )
		{
			g_NodeLinkP2 = node;
			g_NodeLinkP2GUID = &guid;
		}
		else if( ImGui::IsMouseReleased(0) && g_NodeLinkP1 )
		{
			guid = g_NodeLinkP1->guid;
		}
	}
	if( g_NodeLinkP2 == node && g_NodeLinkP2GUID == &guid )
	{
		DrawLink( draw_list, ImGui::GetMousePos(), pos );
	}
	ImGui::SetCursorScreenPos( scpos );
	
	ImGui::PopID();
}

void EditACNode( AnimCharacter::Node* node )
{
	// common
	if( node->type == AnimCharacter::NT_Player )
	{
		SGRX_CAST( AnimCharacter::PlayerNode*, PN, node );
		if( ImGui::Button( "Edit states / transitions", ImVec2(140,20) ) )
		{
			g_EditedPlayerNode = PN;
		}
	}
	else if( node->type == AnimCharacter::NT_Mask )
	{
		SGRX_CAST( AnimCharacter::MaskNode*, MN, node );
		EditNodeInput( "Input", MN, MN->src );
		PickMaskName( "Mask", MN->mask_name );
	}
	else if( node->type == AnimCharacter::NT_Blend )
	{
		SGRX_CAST( AnimCharacter::BlendNode*, BN, node );
		EditNodeInput( "Input A", BN, BN->A );
		EditNodeInput( "Input B", BN, BN->B );
		EditValExpr( "Factor", BN->factor );
		IMGUIComboBox( "Mode", BN->mode, "Normal\0Additive\0" );
	}
	else if( node->type == AnimCharacter::NT_RelAbs )
	{
		SGRX_CAST( AnimCharacter::RelAbsNode*, RN, node );
		EditNodeInput( "Input", RN, RN->src );
		IMGUIEditIntFlags( "Reverse", RN->flags, ANIM_RELABS_INVERT );
	//	IMGUIEditIntFlags( "Space change", RN->flags, ANIM_RELABS_POSOFF );
	}
	else if( node->type == AnimCharacter::NT_Rotator )
	{
		SGRX_CAST( AnimCharacter::RotatorNode*, RN, node );
		EditNodeInput( "Input", RN, RN->src );
		EditValExpr( "Angle", RN->angle );
	}
}

static ImVec2 g_NodeCameraPos(0,0);
static AnimCharacter::Node* g_SelNode = NULL;
void EditNodes()
{
	AnimCharacter& ac = *g_AnimChar;
	if( g_EditedPlayerNode )
	{
		EditPlayerStates();
		return;
	}
	
	ImGui::BeginChangeCheck();
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2(1,1) );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(0,0) );
	ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, ImColor(40,40,40,200) );
	ImGui::BeginChild( "scrolling_region", ImVec2(0,0), true, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove );
	ImGui::PushItemWidth( 100.0f );
	
	AnimCharacter::Node* node_hovered_in_list = NULL;
	AnimCharacter::Node* node_hovered_in_scene = NULL;
	bool open_context_menu = false;
	
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit( 3 );
	ImVec2 offset = ImGui::GetCursorScreenPos()
		- ImGui::GetContentRegionAvail() * ImVec2( 0.5f, 0.5f )
		- g_NodeCameraPos;
	
	HashTable< AnimCharacter::Node*, ImVec2 > outPosMap;
	g_TempNodeLinks.clear();
	for( size_t i = 0; i < ac.nodes.size(); ++i )
	{
		Vec2& pos = ac.nodes[ i ]->editor_pos;
		AnimCharacter::Node* node = ac.nodes[ i ];
		
		ImGui::PushID( node );
		ImVec2 node_rect_min = offset + ImVec2( pos.x, pos.y );
		draw_list->ChannelsSetCurrent( 2 );
		
		bool old_any_active = ImGui::IsAnyItemActive();
		ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
		ImGui::BeginGroup();
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2(4,4) );
		ImGui::Text( "%s", ac.nodes[ i ]->GetName() );
		ImGui::Dummy( ImVec2(140, 1) );
		EditACNode( ac.nodes[ i ] );
		ImGui::PopStyleVar(1);
		ImGui::EndGroup();
		
		ImVec2 nodeSize = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
		ImVec2 node_rect_max = node_rect_min + nodeSize;
		
		bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
		
		// draw box, do links
		draw_list->ChannelsSetCurrent(1); // Background
		ImVec2 node_title_max = node_rect_min + ImVec2( nodeSize.x, 24 );
		draw_list->AddRectFilled(node_rect_min, node_rect_max, (node_hovered_in_list == node || node_hovered_in_scene == node || (node_hovered_in_list == NULL && g_SelNode == node)) ? ImColor(75,75,75,200) : ImColor(60,60,60,200), 4.0f); 
		draw_list->AddRectFilled(node_rect_min, node_title_max, ImColor(20,20,20,200), 4.0f);
		draw_list->AddRect(node_rect_min, node_rect_max,
			ImColor(
				100+(node == ac.output_node?120:0),
				100+(node == ac.main_player_node?120:0),100), 4.0f);
		
		// output link
		{
			ImVec2 pos = ImVec2( node_rect_max.x, node_rect_min.y + 12 );
			outPosMap.set( ac.nodes[ i ], pos );
			draw_list->AddCircleFilled(pos, NODE_SLOT_RADIUS, ImColor(150,150,150,150));
			
			ImVec2 scpos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos( pos - ImVec2( NODE_SLOT_RADIUS, NODE_SLOT_RADIUS ) );
			ImGui::InvisibleButton( "olink", ImVec2( NODE_SLOT_RADIUS * 2.0f, NODE_SLOT_RADIUS * 2.0f ) );
			if( ImGui::IsItemHoveredRect() )
			{
				if( ImGui::IsMouseClicked(0) )
				{
					g_NodeLinkP1 = node;
				}
				else if( ImGui::IsMouseReleased(0) && g_NodeLinkP2 )
				{
					*g_NodeLinkP2GUID = node->guid;
				}
			}
			if( g_NodeLinkP1 == node )
			{
				DrawLink( draw_list, pos, ImGui::GetMousePos() );
			}
			ImGui::SetCursorScreenPos( scpos );
		}
		
		// node box
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node", nodeSize);
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_scene = node;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		bool node_moving_active = ImGui::IsItemActive();
		if (node_widgets_active || node_moving_active)
			g_SelNode = node;
		if (node_moving_active && ImGui::IsMouseDragging(0))
		{
			pos.x += ImGui::GetIO().MouseDelta.x;
			pos.y += ImGui::GetIO().MouseDelta.y;
		}
		
		ImGui::PopID();
	}
	
	draw_list->ChannelsSetCurrent(0); // links
	for( size_t i = 0; i < g_TempNodeLinks.size(); ++i )
	{
		const DrawNodeLink& dnl = g_TempNodeLinks[ i ];
		DrawLink( draw_list, outPosMap.getcopy( dnl.node1 ), dnl.node2pos );
	}
	g_TempNodeLinks.clear();
	
	draw_list->ChannelsMerge();
	
	// Link post-processing
	if( ImGui::IsMouseReleased(0) )
	{
		g_NodeLinkP1 = NULL;
		g_NodeLinkP2 = NULL;
		g_NodeLinkP2GUID = NULL;
	}
	
	// Open context menu
	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
	{
		g_SelNode = node_hovered_in_list = node_hovered_in_scene = NULL;
		open_context_menu = true;
	}
	if (open_context_menu)
	{
		ImGui::OpenPopup("context_menu");
		if (node_hovered_in_list != NULL)
			g_SelNode = node_hovered_in_list;
		if (node_hovered_in_scene != NULL)
			g_SelNode = node_hovered_in_scene;
	}
	
	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
	if (ImGui::BeginPopup("context_menu"))
	{
		AnimCharacter::Node* node = g_SelNode;
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (node)
		{
			ImGui::Text("%s node", node->GetName());
			ImGui::Separator();
			if (ImGui::MenuItem("Delete"))
			{
				ac._UnlinkNode( node );
				ac.nodes.remove_first( node );
				if( ac.output_node == node )
					ac.output_node = NULL;
				if( ac.main_player_node == node )
					ac.main_player_node = NULL;
				g_SelNode = NULL;
			}
			ImGui::Separator();
			if( ImGui::MenuItem( "Set as output" ) )
			{
				ac.output_node = node;
				ac._Prepare();
			}
			if( node->type == AnimCharacter::NT_Player && ImGui::MenuItem( "Set as main player" ) )
			{
				ac.main_player_node = node;
			}
			if( ImGui::MenuItem( "Unlink output pin" ) )
			{
				ac._UnlinkNode( node );
				ac._Prepare();
			}
			if (ImGui::MenuItem("Copy", NULL, false, false)) {}
		}
		else
		{
			AnimCharacter::Node* nn = NULL;
			if (ImGui::MenuItem("Add: Player node"))
				nn = new AnimCharacter::PlayerNode;
			if (ImGui::MenuItem("Add: Mask node"))
				nn = new AnimCharacter::MaskNode;
			if (ImGui::MenuItem("Add: Blend node"))
				nn = new AnimCharacter::BlendNode;
			if (ImGui::MenuItem("Add: Ragdoll node"))
				nn = new AnimCharacter::RagdollNode;
			if (ImGui::MenuItem("Add: Rel<->Abs node"))
				nn = new AnimCharacter::RelAbsNode;
			if (ImGui::MenuItem("Add: Rotator node"))
				nn = new AnimCharacter::RotatorNode;
			if( nn )
			{
				nn->Init( V2( scene_pos ) );
				ac.nodes.push_back( nn );
				if( !ac.output_node )
					ac.output_node = nn;
				ImGui::TriggerChangeCheck();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Paste", NULL, false, false)) {}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	
	// MMB view scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
	{
		g_NodeCameraPos -= ImGui::GetIO().MouseDelta;
	}
	
	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	if( ImGui::EndChangeCheck() )
		ac._Prepare();
}


void BoneRecalcUI( AnimCharacter& ac )
{
	IMGUIEditFloat( "Min. bone size", g_recalcMinBoneSize, 0, 10 );
	IMGUIEditInt( "Weight threshold", g_recalcWeightThreshold, 0, 255 );
	
	g_recalcWhichBones.resize_using( ac.bones.size(), false );
	
	if( ImGui::Button( "Recalculate bones", ImVec2( ImGui::GetContentRegionAvailWidth(), 20 ) ) )
	{
		int mask = 0;
		if( g_recalcHitboxes ) mask |= 2;
		if( g_recalcBodies ) mask |= 1;
		if( g_recalcJoints ) mask |= 4;
		for( size_t i = 0; i < g_recalcWhichBones.size(); ++i )
		{
			if( g_recalcWhichBones[ i ] )
				calc_char_bone_info( i, g_recalcWeightThreshold, g_recalcMinBoneSize, mask );
		}
	}
	
	ImGui::Columns( 4 );
	
	// header
	ImGui::Text( "Bones" );
	ImGui::SameLine();
	if( ImGui::Button( "All" ) )
	{
		TMEMSET( g_recalcWhichBones.data(), g_recalcWhichBones.size(), true );
	}
	ImGui::SameLine();
	if( ImGui::Button( "None" ) )
	{
		TMEMSET( g_recalcWhichBones.data(), g_recalcWhichBones.size(), false );
	}
	ImGui::NextColumn();
	ImGui::Text( "Hitbox" ); ImGui::SameLine(); IMGUIEditBool( "##rchitbox", g_recalcHitboxes );
	ImGui::NextColumn();
	ImGui::Text( "Body" ); ImGui::SameLine(); IMGUIEditBool( "##rcbody", g_recalcBodies );
	ImGui::NextColumn();
	ImGui::Text( "Joint" ); ImGui::SameLine(); IMGUIEditBool( "##rcjoint", g_recalcJoints );
	ImGui::NextColumn();
	
	// bones
	for( size_t i = 0; i < ac.bones.size(); ++i )
	{
		bool& rtb = g_recalcWhichBones[ i ];
		IMGUIEditBool( StackPath(ac.bones[ i ].name).str, rtb );
		ImGui::NextColumn();
		IMGUIYesNo( rtb && g_recalcHitboxes );
		ImGui::NextColumn();
		IMGUIYesNo( rtb && g_recalcBodies );
		ImGui::NextColumn();
		IMGUIYesNo( rtb && g_recalcJoints );
		ImGui::NextColumn();
	}
	
	ImGui::Columns( 1 );
}


struct CharRenderView : IMGUIRenderView
{
	CharRenderView() : IMGUIRenderView( g_EdScene ){}
	void DebugDraw()
	{
		::DebugDraw();
	}
};


enum EditorMode
{
	EditChar,
	RecalcBones,
	AnimPreview,
	RagdollTest,
	MiscProps,
};
int g_mode = EditChar;
bool g_APChangeStates = true;
float g_phySpeed = 1;
int g_phyIters = 1;
bool g_phyIIEnable = false;
String g_phyIIBone;
int g_phyIIBoneID = -1;
Vec3 g_phyIIPos = V3(0);
Vec3 g_phyIIDir = V3(0);
float g_phyIIStrength = 1;
float g_phyIIAtten = 1;
float g_phyIIRadius = 10;


static bool ModeRB( const char* label, int mode, int key )
{
	if( ImGui::RadioButton( label, &g_mode, mode ) )
		return true;
	if( ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed( key, false ) )
	{
		g_mode = mode;
		return true;
	}
	return false;
}


struct CharEditor : IGame
{
	CharEditor(){}
	bool OnInitialize()
	{
		LOG_FUNCTION_ARG( "CharEditor" );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		// core data
		g_PhyWorld = PHY_CreateWorld();
		g_PhyWorld->SetGravity( V3( 0, 0, -9.81f ) );
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.znear = 0.1f;
		g_EdScene->camera.UpdateMatrices();
		g_EdScene->skyTexture = GR_GetTexture( "textures/sky/overcast1.dds" );
		g_AnimChar = new AnimCharacter;
		g_AnimCharInst = new AnimCharInst( g_EdScene, g_PhyWorld );
		g_AnimCharInst->SetSkin( "!default" );
		g_AnimCharInst->SetAnimChar( g_AnimChar );
		
		// TEST
#if 0
		g_AnimChar->m_anDeformer.AddModelForce( V3(-0.5f,0.5f,1), V3(1,-1,0), 0.7f, 1, 0.5f );
#endif
		
		// floor mesh
		g_FloorMeshInst = g_EdScene->CreateMeshInstance();
		g_FloorMeshInst->SetMesh( GR_CreateMesh() );
		SGRX_Material mtl;
		mtl.shader = "default";
		mtl.textures[ 0 ] = GR_GetTexture( "textures/unit.png" );
		g_FloorMeshInst->materials.assign( &mtl, 1 );
		lmm_prepmeshinst( g_FloorMeshInst );
		SGRX_PhyRigidBodyInfo frbi;
		frbi.mass = 0;
		frbi.kinematic = true;
		frbi.inertia = V3(0);
		frbi.friction = 0.95f;
		frbi.restitution = 0.05f;
		Vec3 verts[5] = { {-1,-1,0}, {1,-1,0}, {1,1,0}, {-1,1,0}, {0,0,-1} };
		frbi.shape = g_PhyWorld->CreateConvexHullShape( verts, 5 );
		g_FloorBody = g_PhyWorld->CreateRigidBody( frbi );
		floor_mesh_update( 2, 0 );
		
		SGRX_IMGUI_Init();
		
		g_NUIRenderView = new CharRenderView;
		g_NUICharFilePicker = new IMGUIFilePicker( SGRXPATH_SRC "/chars", ".chr" );
		g_NUIMeshPicker = new IMGUIMeshPicker;
		g_NUIAnimPicker = new IMGUIAnimPicker;
		
		return true;
	}
	void OnDestroy()
	{
		LOG_FUNCTION_ARG( "CharEditor" );
		
		delete g_NUIAnimPicker;
		delete g_NUIMeshPicker;
		delete g_NUICharFilePicker;
		delete g_NUIRenderView;
		
		g_FloorMeshInst = NULL;
		delete g_AnimCharInst;
		g_AnimCharInst = NULL;
		g_AnimChar = NULL;
		g_EdScene = NULL;
		g_PhyWorld = NULL;
		
		SGRX_IMGUI_Free();
	}
	void OnEvent( const Event& e )
	{
		LOG_FUNCTION_ARG( "CharEditor" );
		
		SGRX_IMGUI_Event( e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		LOG_FUNCTION_ARG( "CharEditor" );
		
		SGRX_IMGUI_NewFrame( dt );
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_AnimCharInst->FixedTick( dt, g_mode == AnimPreview && g_APChangeStates );
		for( int i = 0; i < g_phyIters; ++i )
			g_PhyWorld->Step( dt * g_phySpeed / g_phyIters );
		
		if( g_AnimCharInst->m_cachedMeshInst )
			lmm_prepmeshinst( g_AnimCharInst->m_cachedMeshInst );
		g_AnimCharInst->PreRender( 1 );
		
#if 0
		//float fac = sinf( g_AnimCharInst->m_anDeformer.forces[ 0 ].lifetime * M_PI ) * 0.5f + 0.5f;
		g_AnimCharInst->m_anDeformer.forces[ 0 ].amount = 1;//fac * 0.5f;
		g_AnimCharInst->m_anDeformer.forces[ 0 ].pos = g_EdScene->camera.position;
		g_AnimCharInst->m_anDeformer.forces[ 0 ].dir = g_EdScene->camera.direction;
#endif
		
		IMGUI_MAIN_WINDOW_BEGIN
		{
			bool needOpen = false;
			bool needSave = false;
			bool needSaveAs = false;
			
			if( ImGui::BeginMenuBar() )
			{
				if( ImGui::BeginMenu( "File" ) )
				{
					if( ImGui::MenuItem( "New" ) )
					{
						g_fileName = "";
						g_AnimChar = new AnimCharacter();
						g_AnimCharInst->SetAnimChar( g_AnimChar );
						AfterReload();
					}
					if( ImGui::MenuItem( "Open" ) ) needOpen = true;
					if( ImGui::MenuItem( "Save" ) ) needSave = true;
					if( ImGui::MenuItem( "Save As" ) ) needSaveAs = true;
					ImGui::Separator();
					if( ImGui::MenuItem( "Exit" ) ){ Game_End(); }
					ImGui::EndMenu();
				}
				ImGui::SameLine( 0, 50 );
				ImGui::Text( "Character file: %s", g_fileName.size() ? StackPath(g_fileName).str : "<none>" );
				ImGui::SameLine( 0, 50 );
				ImGui::Text( "Edit mode:" );
				ImGui::SameLine();
				ModeRB( "Character", EditChar, SDLK_1 );
				ImGui::SameLine();
				ModeRB( "Recalc. bones", RecalcBones, SDLK_2 );
				ImGui::SameLine();
				if( ModeRB( "Animation preview", AnimPreview, SDLK_3 ) )
				{
					g_AnimChar->_Prepare();
					g_AnimCharInst->_OnRenderUpdate();
					g_AnimCharInst->ResetStates();
				}
				ImGui::SameLine();
				ModeRB( "Ragdoll test", RagdollTest, SDLK_4 );
				ImGui::SameLine();
				ModeRB( "Misc. settings", MiscProps, SDLK_5 );
				ImGui::EndMenuBar();
			}
			
			IMGUI_HSPLIT3( 0.3f, 0.7f,
			{
				g_NUIRenderView->Process( dt );
				PostViewUI();
			},
			{
				EditNodes();
			},
			{
				if( g_mode == EditChar )
				{
					if( g_EditedPlayerNode )
					{
						EditACStateProps();
						EditACVariables();
					}
					else
					{
						EditAnimChar();
					}
				}
				else if( g_mode == RecalcBones )
				{
					BoneRecalcUI( *g_AnimChar );
				}
				else if( g_mode == AnimPreview )
				{
					IMGUIEditBool( "Change states", g_APChangeStates );
					if( ImGui::Button( "Reset" ) )
					{
						g_AnimCharInst->ResetStates();
					}
					IMGUI_GROUP( "Information", true,
					{
						for( size_t i = 0; i < g_AnimCharInst->m_rtnodes.size(); ++i )
						{
							AnimCharInst::NodeRT* N = g_AnimCharInst->m_rtnodes[ i ];
							if( N->src->type == AnimCharacter::NT_Player )
							{
								SGRX_CAST( AnimCharInst::PlayerNodeRT*, PNRT, N );
								ImGui::Text( "PlayerNode [#%d]: current_state=%s",
									int(i),
									StackPath(PNRT->current_state ? PNRT->current_state->GetName() : "<none>").str );
							}
						}
					});
					IMGUI_GROUP( "Skin", false,
					{
						ImGui::ListBoxHeader( "skin" );
						if( ImGui::Selectable( "<empty>", g_AnimCharInst->skinName == SV("") ) )
							g_AnimCharInst->SetSkin( "" );
						if( ImGui::Selectable( "<default>", g_AnimCharInst->skinName == SV("!default") ) )
							g_AnimCharInst->SetSkin( "!default" );
						for( size_t i = 0; i < g_AnimChar->skins.size(); ++i )
						{
							AnimCharacter::Skin& SK = g_AnimChar->skins.item( i ).value;
							if( ImGui::Selectable( SK.name.c_str(), g_AnimCharInst->skinName == SK.name ) )
								g_AnimCharInst->SetSkin( SK.name );
						}
						ImGui::ListBoxFooter();
					});
					for( size_t i = 0; i < g_AnimChar->variables.size(); ++i )
					{
						AnimCharacter::Variable* var = g_AnimChar->variables[ i ];
						IMGUIEditFloat( StackPath(var->name).str,
							*g_AnimCharInst->_GetVarValAt( i ), -10000, 10000 );
					}
				}
				else if( g_mode == RagdollTest )
				{
					static float floor_size = 2;
					static float floor_height = 0;
					
					IMGUI_GROUP( "Floor", true,
					{
						bool fup = false;
						fup |= IMGUIEditFloat( "Size", floor_size, 0.01f, 100 );
						fup |= IMGUIEditFloat( "Height", floor_height, -100, 100 );
						if( fup )
							floor_mesh_update( floor_size, floor_height );
					});
					
					IMGUI_GROUP( "Physics", true,
					{
						IMGUIEditFloat( "Speed", g_phySpeed, 0.01f, 100.0f );
						if( ImGui::Button( "x0.1" ) ){ g_phySpeed = 0.1f; }
						ImGui::SameLine();
						if( ImGui::Button( "x0.5" ) ){ g_phySpeed = 0.5f; }
						ImGui::SameLine();
						if( ImGui::Button( "x1" ) ){ g_phySpeed = 1; }
						ImGui::SameLine();
						if( ImGui::Button( "x2" ) ){ g_phySpeed = 2; }
						ImGui::SameLine();
						if( ImGui::Button( "x10" ) ){ g_phySpeed = 10; }
						
						IMGUIEditInt( "Iteration count", g_phyIters, 1, 100 );
						if( ImGui::Button( "1" ) ){ g_phyIters = 1; }
						ImGui::SameLine();
						if( ImGui::Button( "2" ) ){ g_phyIters = 2; }
						ImGui::SameLine();
						if( ImGui::Button( "5" ) ){ g_phyIters = 5; }
						ImGui::SameLine();
						if( ImGui::Button( "10" ) ){ g_phyIters = 10; }
						
						IMGUI_GROUP( "Inital impulse", true,
						{
							IMGUIEditBool( "Enable", g_phyIIEnable );
							if( g_phyIIEnable )
							{
								PickBoneName( "Bone", g_phyIIBone, g_phyIIBoneID );
								IMGUIEditVec3( "Origin", g_phyIIPos, -10000, 10000 );
								IMGUIEditVec3( "Direction", g_phyIIDir, -100, 100 );
								if( ImGui::Button( "Set origin/direction from camera" ) )
								{
									g_phyIIPos = g_EdScene->camera.position;
									g_phyIIDir = g_EdScene->camera.direction;
								}
								IMGUIEditFloat( "Strength", g_phyIIStrength, 0, 1000 );
								IMGUIEditFloat( "Attenuation", g_phyIIAtten, 0, 10 );
								IMGUIEditFloat( "Radius", g_phyIIRadius, 0, 100 );
							}
						});
					});
					
					if( ImGui::Button( "Start" ) )
					{
						if( g_AnimCharInst->m_anRagdoll.m_enabled == false )
						{
							g_AnimCharInst->m_anRagdoll.Initialize( g_AnimCharInst );
							g_AnimCharInst->EnablePhysics();
							if( g_phyIIEnable )
							{
								g_AnimCharInst->m_anRagdoll.ApplyImpulseExt(
									g_phyIIPos,
									g_phyIIDir.Normalized() * g_phyIIStrength,
									g_phyIIAtten,
									g_phyIIRadius,
									g_phyIIBoneID
								);
							}
						}
					}
					ImGui::SameLine();
					if( ImGui::Button( "Stop" ) )
					{
						g_AnimCharInst->DisablePhysics();
					}
					ImGui::SameLine();
					if( ImGui::Button( "Wake up" ) )
					{
						g_AnimCharInst->WakeUp();
					}
				}
				else if( g_mode == MiscProps )
				{
					IMGUI_GROUP( "Display options", true,
					{
						ImGui::Checkbox( "Show bodies", &g_showBodies );
						ImGui::Checkbox( "Show joints", &g_showJoints );
						ImGui::Checkbox( "Show hitboxes", &g_showHitboxes );
						ImGui::Checkbox( "Show attachments", &g_showAttachments );
						ImGui::Checkbox( "Show masks", &g_showMasks );
						ImGui::Checkbox( "Show physics", &g_showPhysics );
						IMGUIEditFloat( "Mask preview tick size", g_maskPreviewTickSize, 0.001f, 1 );
					});
					g_NUIRenderView->EditCameraParams();
				}
			});
			
			//
			// OPEN
			//
			String fn;
#define OPEN_CAPTION "Open character (.chr) file"
			if( needOpen )
				g_NUICharFilePicker->OpenPopup( OPEN_CAPTION );
			if( g_NUICharFilePicker->Popup( OPEN_CAPTION, fn, false ) )
			{
				if( g_AnimChar->Load( fn ) )
				{
					g_AnimCharInst->SetAnimChar( g_AnimChar );
					g_fileName = fn;
					reload_mesh_vertices();
					AfterReload();
				}
				else
				{
					IMGUIError( "Cannot open file: %s", StackPath(fn).str );
				}
			}
			
			//
			// SAVE
			//
			fn = g_fileName;
#define SAVE_CAPTION "Save character (.chr) file"
			if( needSaveAs || ( needSave && g_fileName.size() == 0 ) )
				g_NUICharFilePicker->OpenPopup( SAVE_CAPTION );
			
			bool canSave = needSave && g_fileName.size();
			if( g_NUICharFilePicker->Popup( SAVE_CAPTION, fn, true ) )
				canSave = fn.size();
			if( canSave )
			{
				if( g_AnimChar->Save( fn ) )
				{
					g_fileName = fn;
				}
				else
				{
					IMGUIError( "Cannot save file: %s", StackPath(fn).str );
				}
			}
		}
		IMGUI_MAIN_WINDOW_END;
		
		SGRX_IMGUI_Render();
		SGRX_IMGUI_ClearEvents();
	}
};


extern "C" EXPORT IGame* CreateGame()
{
	return new CharEditor;
}

extern "C" EXPORT void SetBaseGame( void* game )
{
	// g_Game->SetBaseGame( game );
}


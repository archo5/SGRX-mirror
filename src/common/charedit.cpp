

#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <edgui.hpp>
#include <physics.hpp>
#include "edcomui.hpp"


struct EDGUIMainFrame* g_UIFrame;
PhyWorldHandle g_PhyWorld;
SceneHandle g_EdScene;
MeshInstHandle g_FloorMeshInst;
PhyRigidBodyHandle g_FloorBody;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUICharOpenPicker* g_UICharOpenPicker;
struct EDGUICharSavePicker* g_UICharSavePicker;
struct EDGUIBonePicker* g_UIBonePicker;
struct EDGUIBodyType* g_UIBodyType;
struct EDGUIJointType* g_UIJointType;
struct EDGUITransformType* g_UITransformType;
AnimCharacter* g_AnimChar;
AnimMixer::Layer g_AnimMixLayers[2];


inline Quat EA2Q( Vec3 v ){ return Mat4::CreateRotationXYZ( DEG2RAD( v ) ).GetRotationQuaternion(); }
inline Vec3 Q2EA( Quat q ){ return RAD2DEG( Mat4::CreateRotationFromQuat( q ).GetXYZAngles() ); }
inline StringView BodyType2String( uint8_t t )
{
	if( t == AnimCharacter::BodyType_Sphere ) return "Sphere";
	if( t == AnimCharacter::BodyType_Capsule ) return "Capsule";
	if( t == AnimCharacter::BodyType_Box ) return "Box";
	return "None";
}
inline uint8_t String2BodyType( const StringView& s )
{
	if( s == "Sphere" ) return AnimCharacter::BodyType_Sphere;
	if( s == "Capsule" ) return AnimCharacter::BodyType_Capsule;
	if( s == "Box" ) return AnimCharacter::BodyType_Box;
	return AnimCharacter::BodyType_None;
}
inline StringView JointType2String( uint8_t t )
{
	if( t == AnimCharacter::JointType_Hinge ) return "Hinge";
	if( t == AnimCharacter::JointType_ConeTwist ) return "Cone Twist";
	return "None";
}
inline uint8_t String2JointType( const StringView& s )
{
	if( s == "Hinge" ) return AnimCharacter::JointType_Hinge;
	if( s == "Cone Twist" ) return AnimCharacter::JointType_ConeTwist;
	return AnimCharacter::BodyType_None;
}
inline StringView TransformType2String( uint8_t t )
{
	if( t == AnimCharacter::TransformType_UndoParent ) return "UndoParent";
	if( t == AnimCharacter::TransformType_Move ) return "Move";
	if( t == AnimCharacter::TransformType_Rotate ) return "Rotate";
	return "None";
}
inline uint8_t String2TransformType( const StringView& s )
{
	if( s == "UndoParent" ) return AnimCharacter::TransformType_UndoParent;
	if( s == "Move" ) return AnimCharacter::TransformType_Move;
	if( s == "Rotate" ) return AnimCharacter::TransformType_Rotate;
	return AnimCharacter::TransformType_None;
}



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
		SGRX_IMesh* M = g_AnimChar->m_cachedMesh;
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
	SGRX_MeshBone* meshbones = g_AnimChar->m_cachedMesh->m_bones;
	AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ bid ];
	AMBone B = { BI.bone_id, BI.name, (uint8_t) thres };
	
	calc_bone_volume_info( B );
	
	Mat4 world_to_local = BI.bone_id < 0 ? Mat4::Identity : meshbones[ BI.bone_id ].invSkinOffset;
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
		int pbone = g_AnimChar->FindParentBone( bid );
		while( pbone >= 0 )
		{
			Vec3 bs = g_AnimChar->bones[ pbone ].body.size.Abs();
			if( bs.x < minbs || bs.y < minbs || bs.z < minbs )
				pbone = g_AnimChar->FindParentBone( pbone );
			else
				break;
		}
		Vec3 bs = g_AnimChar->bones[ bid ].body.size.Abs();
		Vec3 pbs = g_AnimChar->bones[ pbone ].body.size.Abs();
		if( pbone < 0 || bs.x < minbs || bs.y < minbs || bs.z < minbs ||
			pbs.x < minbs || pbs.y < minbs || pbs.z < minbs )
		{
			BI.joint.type = AnimCharacter::JointType_None;
			BI.joint.parent_name = "";
			BI.joint.parent_id = -1;
		}
		else
		{
			int pbone_m = g_AnimChar->bones[ pbone ].bone_id;
			
			BI.joint.type = AnimCharacter::JointType_ConeTwist;
			BI.joint.parent_name = g_AnimChar->bones[ pbone ].name;
			BI.joint.parent_id = pbone;
			
			Mat4 world_to_parent = meshbones[ pbone_m ].invSkinOffset;
			
			// calculate own position
			Vec3 bone_origin = meshbones[ BI.bone_id ].skinOffset.TransformPos( V3(0) );
			
			// calculate child bone count / avg. position
			int numch = 0;
			Vec3 avg_child_origin = V3(0);
			for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			{
				if( g_AnimChar->FindParentBone( i ) == bid )
				{
					int cmbid = g_AnimChar->bones[ i ].bone_id;
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



static void YesNoText( bool yes )
{
	uint32_t origcol = GR2D_GetBatchRenderer().m_proto.color;
	if( yes )
	{
		GR2D_SetColor( 0.1f, 1, 0 );
		GR2D_DrawTextLine( "YES" );
	}
	else
	{
		GR2D_SetColor( 1, 0.1f, 0 );
		GR2D_DrawTextLine( "NO" );
	}
	GR2D_GetBatchRenderer().Colu( origcol );
}


struct EDGUICharPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUICharPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading chars";
		m_options.clear();
		FS_IterateDirectory( "chars", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		if( isdir == false && name.ends_with( ".chr" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
		}
		return true;
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_PROPCHANGE && e->target == &m_confirm )
		{
			if( m_confirm.m_value == 1 )
			{
				EDGUIRsrcPicker::_OnPickResource();
			}
			return 1;
		}
		return EDGUIRsrcPicker::OnEvent( e );
	}
	
	EDGUIQuestion m_confirm;
};

struct EDGUICharOpenPicker : EDGUICharPicker
{
	EDGUICharOpenPicker()
	{
		caption = "Pick a character to open";
		m_confirm.caption = "Do you really want to open the character? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUICharSavePicker : EDGUICharPicker
{
	EDGUICharSavePicker()
	{
		caption = "Pick a character to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the character?";
	}
	virtual void _OnPickResource()
	{
		if( m_options.find_first_at( m_pickedOption ) == NOT_FOUND )
			EDGUIRsrcPicker::_OnPickResource();
		else
		{
			m_confirm.Open( this );
			m_frame->Add( &m_confirm );
		}
	}
	virtual void _OnConfirm()
	{
		_OnPickResource();
	}
};


struct EDGUIBonePicker : EDGUIRsrcPicker
{
	EDGUIBonePicker()
	{
		caption = "Pick a bone to use";
	}
	void Reload()
	{
		LOG << "Reloading bones";
		m_options.clear();
		m_options.push_back( "" );
		if( g_AnimChar == NULL || !g_AnimChar->m_cachedMesh )
		{
			LOG << "No mesh to load bones from";
			return;
		}
		SGRX_IMesh* M = g_AnimChar->m_cachedMesh;
		for( int i = 0; i < M->m_numBones; ++i )
		{
			m_options.push_back( M->m_bones[ i ].name );
		}
		_Search( m_searchString );
	}
};


struct EDGUIBodyType : EDGUIRsrcPicker
{
	EDGUIBodyType()
	{
		caption = "Pick a body type";
		m_options.push_back( "None" );
		m_options.push_back( "Sphere" );
		m_options.push_back( "Capsule" );
		m_options.push_back( "Box" );
		_Search( m_searchString );
	}
};


struct EDGUIJointType : EDGUIRsrcPicker
{
	EDGUIJointType()
	{
		caption = "Pick a joint type";
		m_options.push_back( "None" );
		m_options.push_back( "Hinge" );
		m_options.push_back( "Cone Twist" );
		_Search( m_searchString );
	}
};


struct EDGUITransformType : EDGUIRsrcPicker
{
	EDGUITransformType()
	{
		caption = "Pick a transform type";
		m_options.push_back( "None" );
		m_options.push_back( "UndoParent" );
		m_options.push_back( "Move" );
		m_options.push_back( "Rotate" );
		_Search( m_searchString );
	}
};



void CE_UpdateParamList();

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
			iid = g_AnimChar->_FindBone( boneName );
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
		
		int bid = xfdata.boneID = g_AnimChar->_FindBone( boneName );
		if( bid < 0 )
			return false;
		if( iid < 0 )
			iid = bid;
		xfdata.itemID = iid;
		
		SGRX_MeshInstance* MI = g_AnimChar->m_cachedMeshInst;
		xfdata.boneToWorld = g_AnimChar->m_cachedMesh->m_bones[ bid ].skinOffset
			* MI->skin_matrices[ bid ] * MI->matrix;
		xfdata.worldToBone = Mat4::Identity;
		xfdata.boneToWorld.InvertTo( xfdata.worldToBone );
		
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
		CE_UpdateParamList();
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



enum CE_GUI_ITEMS
{
	CE_GUI_BASE = 10000,
	CE_GUI_BONEPROPS,
	CE_GUI_BONELISTPROPS,
	CE_GUI_ATCHPROPS,
	CE_GUI_ATCHLISTPROPS,
	CE_GUI_LAYERTFPROPS,
	CE_GUI_LAYERPROPS,
	CE_GUI_LAYERLISTPROPS,
	CE_GUI_MASKCMDPROPS,
	CE_GUI_MASKPROPS,
	CE_GUI_MASKLISTPROPS,
	CE_GUI_CHARPROPS,
};

struct EDGUIBoneProps : EDGUILayoutRow
{
	EDGUIBoneProps() :
		m_group( true, "Bone properties" ),
		m_group_hbox( true, "Hitbox" ),
		m_group_body( true, "Body" ),
		m_group_joint( true, "Joint" ),
		m_group_recalc( true, "Recalculate shapes" ),
		
		m_hbox_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_hbox_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_hbox_extents( V3(0.1f), 2, V3(0), V3(100) ),
		m_hbox_multiplier( 1, 2, 0, 100.0f ),
		
		m_body_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_body_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_body_type( g_UIBodyType, "None" ),
		m_body_size( V3(0.1f), 2, V3(0), V3(100) ),
		
		m_joint_type( g_UIJointType, "None" ),
		m_joint_parent( g_UIBonePicker, "" ),
		m_joint_self_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_joint_self_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_joint_prnt_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_joint_prnt_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_joint_turnlim1( 0, 2, -360, 360 ),
		m_joint_turnlim2( 0, 2, -360, 360 ),
		m_joint_twistlim( 0, 2, -360, 360 ),
		
		m_recalc_minbs( 0.05f, 2, 0, 10 ),
		m_recalc_thres( 96, 0, 255 ),
		m_bid( -1 )
	{
		type = CE_GUI_BONEPROPS;
		
		m_hbox_rotangles.caption = "Rotation (angles)";
		m_hbox_offset.caption = "Offset";
		m_hbox_extents.caption = "Extents";
		m_hbox_multiplier.caption = "Multiplier";
		m_btn_hbox_xform.caption = "Select hitbox for XForm";
		
		m_body_rotangles.caption = "Rotation (angles)";
		m_body_offset.caption = "Offset";
		m_body_type.caption = "Body type";
		m_body_size.caption = "Body size";
		m_btn_body_xform.caption = "Select body for XForm";
		
		m_joint_type.caption = "Joint type";
		m_joint_parent.caption = "Parent";
		m_joint_self_rotangles.caption = "Local rotation";
		m_joint_self_offset.caption = "Local position";
		m_joint_prnt_rotangles.caption = "Parent rotation";
		m_joint_prnt_offset.caption = "Parent position";
		m_joint_turnlim1.caption = "Turn limit 1 (Min/X)";
		m_joint_turnlim2.caption = "Turn limit 2 (Max/Y)";
		m_joint_twistlim.caption = "Twist limit (Z)";
		m_btn_joint_self_xform.caption = "Select local joint frame for XForm";
		m_btn_joint_prnt_xform.caption = "Select parent joint frame for XForm";
		m_recalc_minbs.caption = "Min. body size";
		m_btn_recalc_joint.caption = "Regenerate joint data";
		
		m_recalc_thres.caption = "Threshold";
		m_btn_recalc_body.caption = "Recalculate body";
		m_btn_recalc_hitbox.caption = "Recalculate hitbox";
		m_btn_recalc_both.caption = "Recalculate both";
		
		m_group_hbox.Add( &m_hbox_rotangles );
		m_group_hbox.Add( &m_hbox_offset );
		m_group_hbox.Add( &m_hbox_extents );
		m_group_hbox.Add( &m_hbox_multiplier );
		m_group_hbox.Add( &m_btn_hbox_xform );
		
		m_group_body.Add( &m_body_rotangles );
		m_group_body.Add( &m_body_offset );
		m_group_body.Add( &m_body_type );
		m_group_body.Add( &m_body_size );
		m_group_body.Add( &m_btn_body_xform );
		
		m_group_joint.Add( &m_joint_type );
		m_group_joint.Add( &m_joint_parent );
		m_group_joint.Add( &m_joint_self_rotangles );
		m_group_joint.Add( &m_joint_self_offset );
		m_group_joint.Add( &m_joint_prnt_rotangles );
		m_group_joint.Add( &m_joint_prnt_offset );
		m_group_joint.Add( &m_joint_turnlim1 );
		m_group_joint.Add( &m_joint_turnlim2 );
		m_group_joint.Add( &m_joint_twistlim );
		m_group_joint.Add( &m_btn_joint_self_xform );
		m_group_joint.Add( &m_btn_joint_prnt_xform );
		m_group_joint.Add( &m_recalc_minbs );
		m_group_joint.Add( &m_btn_recalc_joint );
		
		m_group_recalc.Add( &m_recalc_thres );
		m_group_recalc.Add( &m_btn_recalc_body );
		m_group_recalc.Add( &m_btn_recalc_hitbox );
		m_group_recalc.Add( &m_btn_recalc_both );
		
		m_group.Add( &m_group_hbox );
		m_group.Add( &m_group_body );
		m_group.Add( &m_group_joint );
		m_group.Add( &m_group_recalc );
		Add( &m_group );
	}
	
	void Prepare( int which )
	{
		m_bid = which;
		AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ m_bid ];
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "Bone props: %s", StackString<240>( BI.name ).str );
		m_group.caption = bfr;
		m_group.SetOpen( true ); // update caption
		
		m_hbox_rotangles.SetValue( Q2EA( BI.hitbox.rotation ) );
		m_hbox_offset.SetValue( BI.hitbox.position );
		m_hbox_extents.SetValue( BI.hitbox.extents );
		m_hbox_multiplier.SetValue( BI.hitbox.multiplier );
		
		m_body_rotangles.SetValue( Q2EA( BI.body.rotation ) );
		m_body_offset.SetValue( BI.body.position );
		m_body_type.SetValue( BodyType2String( BI.body.type ) );
		m_body_size.SetValue( BI.body.size );
		
		m_joint_type.SetValue( JointType2String( BI.joint.type ) );
		m_joint_parent.SetValue( BI.joint.parent_name );
		m_joint_self_rotangles.SetValue( Q2EA( BI.joint.self_rotation ) );
		m_joint_self_offset.SetValue( BI.joint.self_position );
		m_joint_prnt_rotangles.SetValue( Q2EA( BI.joint.prnt_rotation ) );
		m_joint_prnt_offset.SetValue( BI.joint.prnt_position );
		m_joint_turnlim1.SetValue( BI.joint.turn_limit_1 );
		m_joint_turnlim2.SetValue( BI.joint.turn_limit_2 );
		m_joint_twistlim.SetValue( BI.joint.twist_limit );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_bid >= 0 )
		{
			AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ m_bid ];
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btn_hbox_xform || e->target == &m_btn_body_xform )
				{
					g_XFormState.name = g_AnimChar->bones[ m_bid ].name;
					g_XFormState.type = e->target == &m_btn_hbox_xform ? TT_BoneHitbox : TT_BoneBody;
				}
				if( e->target == &m_btn_joint_self_xform || e->target == &m_btn_joint_prnt_xform )
				{
					g_XFormState.name = g_AnimChar->bones[ m_bid ].name;
					g_XFormState.type = e->target == &m_btn_joint_self_xform ?
						TT_BoneJointSelfFrame : TT_BoneJointParentFrame;
				}
				if( e->target == &m_btn_recalc_joint )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, m_recalc_minbs.m_value, 4 );
					Prepare( m_bid );
				}
				if( e->target == &m_btn_recalc_body )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, m_recalc_minbs.m_value, 1 );
					Prepare( m_bid );
				}
				if( e->target == &m_btn_recalc_hitbox )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, m_recalc_minbs.m_value, 2 );
					Prepare( m_bid );
				}
				if( e->target == &m_btn_recalc_both )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, m_recalc_minbs.m_value, 1|2 );
					Prepare( m_bid );
				}
				break;
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_hbox_rotangles ) BI.hitbox.rotation = EA2Q( m_hbox_rotangles.m_value );
				else if( e->target == &m_hbox_offset ) BI.hitbox.position = m_hbox_offset.m_value;
				else if( e->target == &m_hbox_extents ) BI.hitbox.extents = m_hbox_extents.m_value;
				else if( e->target == &m_hbox_multiplier ) BI.hitbox.multiplier = m_hbox_multiplier.m_value;
				
				else if( e->target == &m_body_rotangles ) BI.body.rotation = EA2Q( m_body_rotangles.m_value );
				else if( e->target == &m_body_offset ) BI.body.position = m_body_offset.m_value;
				else if( e->target == &m_body_type ) BI.body.type = String2BodyType( m_body_type.m_value );
				else if( e->target == &m_body_size ) BI.body.size = m_body_size.m_value;
				
				else if( e->target == &m_joint_type ) BI.joint.type = String2JointType( m_joint_type.m_value );
				else if( e->target == &m_joint_parent )
				{
					BI.joint.parent_name = m_joint_parent.m_value;
					BI.joint.parent_id = g_AnimChar->_FindBone( BI.joint.parent_name );
				}
				else if( e->target == &m_joint_self_rotangles ) BI.joint.self_rotation = EA2Q( m_joint_self_rotangles.m_value );
				else if( e->target == &m_joint_self_offset ) BI.joint.self_position = m_joint_self_offset.m_value;
				else if( e->target == &m_joint_prnt_rotangles ) BI.joint.prnt_rotation = EA2Q( m_joint_prnt_rotangles.m_value );
				else if( e->target == &m_joint_prnt_offset ) BI.joint.prnt_position = m_joint_prnt_offset.m_value;
				else if( e->target == &m_joint_turnlim1 ) BI.joint.turn_limit_1 = m_joint_turnlim1.m_value;
				else if( e->target == &m_joint_turnlim2 ) BI.joint.turn_limit_2 = m_joint_turnlim2.m_value;
				else if( e->target == &m_joint_twistlim ) BI.joint.twist_limit = m_joint_twistlim.m_value;
				
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIGroup m_group_hbox;
	EDGUIGroup m_group_body;
	EDGUIGroup m_group_joint;
	EDGUIGroup m_group_recalc;
	
	EDGUIPropVec3 m_hbox_rotangles;
	EDGUIPropVec3 m_hbox_offset;
	EDGUIPropVec3 m_hbox_extents;
	EDGUIPropFloat m_hbox_multiplier;
	EDGUIButton m_btn_hbox_xform;
	
	EDGUIPropVec3 m_body_rotangles;
	EDGUIPropVec3 m_body_offset;
	EDGUIPropRsrc m_body_type;
	EDGUIPropVec3 m_body_size;
	EDGUIButton m_btn_body_xform;
	
	EDGUIPropRsrc m_joint_type;
	EDGUIPropRsrc m_joint_parent;
	EDGUIPropVec3 m_joint_self_rotangles;
	EDGUIPropVec3 m_joint_self_offset;
	EDGUIPropVec3 m_joint_prnt_rotangles;
	EDGUIPropVec3 m_joint_prnt_offset;
	EDGUIPropFloat m_joint_turnlim1;
	EDGUIPropFloat m_joint_turnlim2;
	EDGUIPropFloat m_joint_twistlim;
	EDGUIButton m_btn_joint_self_xform;
	EDGUIButton m_btn_joint_prnt_xform;
	EDGUIPropFloat m_recalc_minbs;
	EDGUIButton m_btn_recalc_joint;
	
	EDGUIPropInt m_recalc_thres;
	EDGUIButton m_btn_recalc_body;
	EDGUIButton m_btn_recalc_hitbox;
	EDGUIButton m_btn_recalc_both;
	int m_bid;
};


void FC_EditBone( int which );
struct EDGUIBoneListProps : EDGUILayoutRow
{
	EDGUIBoneListProps() :
		m_group( true, "Bones" ),
		m_group_recalc( false, "Recalculate shapes" ),
		m_recalc_minbs( 0.05f, 2, 0, 10 ),
		m_recalc_thres( 96, 0, 255 )
	{
		type = CE_GUI_BONELISTPROPS;
		
		m_recalc_minbs.caption = "Min. body size";
		m_recalc_thres.caption = "Threshold";
		m_btn_recalc_body.caption = "Recalculate body";
		m_btn_recalc_hitbox.caption = "Recalculate hitbox";
		m_btn_recalc_both.caption = "Recalculate both";
		m_btn_recalc_joint.caption = "Recalculate joint";
		
		m_group_recalc.Add( &m_recalc_minbs );
		m_group_recalc.Add( &m_recalc_thres );
		m_group_recalc.Add( &m_btn_recalc_body );
		m_group_recalc.Add( &m_btn_recalc_hitbox );
		m_group_recalc.Add( &m_btn_recalc_both );
		m_group_recalc.Add( &m_btn_recalc_joint );
		
		Add( &m_group_recalc );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_group.Clear();
		m_boneButtons.clear();
		m_boneButtons.resize( g_AnimChar->bones.size() );
		for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
		{
			m_boneButtons[ i ].caption = g_AnimChar->bones[ i ].name;
			int pb = g_AnimChar->FindParentBone( i );
			if( pb >= 0 )
			{
				m_boneButtons[ i ].caption.append( "    (parent: " );
				m_boneButtons[ i ].caption.append( g_AnimChar->bones[ pb ].name );
				m_boneButtons[ i ].caption.append( ")" );
			}
			m_group.Add( &m_boneButtons[ i ] );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btn_recalc_body ) _Recalc(1);
			else if( e->target == &m_btn_recalc_hitbox ) _Recalc(2);
			else if( e->target == &m_btn_recalc_both ) _Recalc(1|2);
			else if( e->target == &m_btn_recalc_joint ) _Recalc(4);
			else if( e->target >= (EDGUIItem*) &m_boneButtons[0] &&
				e->target <= (EDGUIItem*) &m_boneButtons.last() )
			{
				FC_EditBone( e->target - (EDGUIItem*) &m_boneButtons[0] );
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	void _Recalc( int mask )
	{
		for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
			calc_char_bone_info( i, m_recalc_thres.m_value, m_recalc_minbs.m_value, mask );
	}
	
	EDGUIGroup m_group;
	Array< EDGUIButton > m_boneButtons;
	EDGUIGroup m_group_recalc;
	EDGUIPropFloat m_recalc_minbs;
	EDGUIPropInt m_recalc_thres;
	EDGUIButton m_btn_recalc_body;
	EDGUIButton m_btn_recalc_hitbox;
	EDGUIButton m_btn_recalc_both;
	EDGUIButton m_btn_recalc_joint;
};


struct EDGUIAttachmentProps : EDGUILayoutRow
{
	EDGUIAttachmentProps() :
		m_group( true, "Attachment" ),
		m_bone( g_UIBonePicker, "" ),
		m_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_aid( -1 )
	{
		type = CE_GUI_ATCHPROPS;
		
		m_name.caption = "Name";
		m_bone.caption = "Bone";
		m_rotangles.caption = "Rotation (angles)";
		m_offset.caption = "Offset";
		m_btn_atch_xform.caption = "Select attachment for XForm";
		
		m_group.Add( &m_name );
		m_group.Add( &m_bone );
		m_group.Add( &m_rotangles );
		m_group.Add( &m_offset );
		m_group.Add( &m_btn_atch_xform );
		
		Add( &m_group );
	}
	
	void Prepare( int which )
	{
		m_aid = which;
		AnimCharacter::Attachment& A = g_AnimChar->attachments[ m_aid ];
		
		m_name.SetValue( A.name );
		m_bone.SetValue( A.bone );
		m_rotangles.SetValue( Q2EA( A.rotation ) );
		m_offset.SetValue( A.position );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_aid >= 0 )
		{
			AnimCharacter::Attachment& A = g_AnimChar->attachments[ m_aid ];
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btn_atch_xform )
				{
					g_XFormState.name = g_AnimChar->attachments[ m_aid ].name;
					g_XFormState.type = TT_Attachment;
				}
				break;
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_name ) A.name = m_name.m_value;
				else if( e->target == &m_bone )
				{
					A.bone = m_bone.m_value;
					A.bone_id = g_AnimChar->_FindBone( A.bone );
				}
				else if( e->target == &m_rotangles ) A.rotation = EA2Q( m_rotangles.m_value );
				else if( e->target == &m_offset ) A.position = m_offset.m_value;
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropString m_name;
	EDGUIPropRsrc m_bone;
	EDGUIPropVec3 m_rotangles;
	EDGUIPropVec3 m_offset;
	EDGUIButton m_btn_atch_xform;
	int m_aid;
};


void FC_EditAttachment( int which );
struct EDGUIAttachmentListProps : EDGUILayoutRow
{
	EDGUIAttachmentListProps() :
		m_group( true, "Attachments" )
	{
		type = CE_GUI_ATCHLISTPROPS;
		
		m_btnAdd.caption = "Add attachment";
		
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_group.Clear();
		m_atchButtons.clear();
		m_atchButtons.resize( g_AnimChar->attachments.size() );
		for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
		{
			m_atchButtons[ i ].caption = g_AnimChar->attachments[ i ].name;
			m_group.Add( &m_atchButtons[ i ] );
		}
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnAdd )
			{
				g_AnimChar->attachments.push_back( AnimCharacter::Attachment() );
				g_AnimChar->attachments.last().name = "<unnamed>";
				Prepare();
				return 1;
			}
			if( m_atchButtons.size() &&
				e->target >= (EDGUIItem*) &m_atchButtons[0] &&
				e->target <= (EDGUIItem*) &m_atchButtons.last() )
			{
				FC_EditAttachment( e->target - (EDGUIItem*) &m_atchButtons[0] );
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	Array< EDGUIButton > m_atchButtons;
};


void FC_EditLayerTransform( int lid, int tid );
void FC_EditLayer( int which );

struct EDGUILayerTransformProps : EDGUILayoutRow
{
	EDGUILayerTransformProps() :
		m_group( true, "<unset>" ),
		m_bone( g_UIBonePicker ),
		m_type( g_UITransformType ),
		m_offaxis( V3(0,0,1), 2, V3(-100), V3(100) ),
		m_angle( 0, 2, -9999, 9999 ),
		m_base( 0, 2, -100, 100 ),
		m_lid( -1 ),
		m_tid( -1 )
	{
		type = CE_GUI_LAYERTFPROPS;
		
		m_btnBack.caption = "Back to layer <which?>";
		m_bone.caption = "Bone";
		m_type.caption = "Type";
		m_offaxis.caption = "Offset/axis";
		m_angle.caption = "Angle";
		m_base.caption = "Base factor";
		
		m_group.Add( &m_bone );
		m_group.Add( &m_type );
		m_group.Add( &m_offaxis );
		m_group.Add( &m_angle );
		m_group.Add( &m_base );
		
		Add( &m_btnBack );
		Add( &m_group );
	}
	
	void Prepare( int lid, int tid )
	{
		m_lid = lid;
		m_tid = tid;
		
		AnimCharacter::LayerTransform& LT = g_AnimChar->layers[ lid ].transforms[ tid ];
		StackString<200> layername( g_AnimChar->layers[ lid ].name );
		
		char bfr[ 256 ];
		
		sgrx_snprintf( bfr, 256, "Back to layer: %s", layername.str );
		m_btnBack.caption = bfr;
		
		sgrx_snprintf( bfr, 256, "Layer %s transform %d", layername.str, tid );
		m_group.caption = bfr;
		m_group.SetOpen( true );
		
		m_bone.SetValue( LT.bone );
		m_type.SetValue( TransformType2String( LT.type ) );
		m_offaxis.SetValue( LT.posaxis );
		m_angle.SetValue( LT.angle );
		m_base.SetValue( LT.base );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_lid >= 0 && m_tid >= 0 )
		{
			AnimCharacter::LayerTransform& LT = g_AnimChar->layers[ m_lid ].transforms[ m_tid ];
			
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnBack )
				{
					FC_EditLayer( m_lid );
					return 1;
				}
				break;
				
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_bone )
				{
					LT.bone = m_bone.m_value;
					LT.bone_id = g_AnimChar->_FindBone( LT.bone );
				}
				else if( e->target == &m_type ) LT.type = String2TransformType( m_type.m_value );
				else if( e->target == &m_offaxis ) LT.posaxis = m_offaxis.m_value;
				else if( e->target == &m_angle ) LT.angle = m_angle.m_value;
				else if( e->target == &m_base ) LT.base = m_base.m_value;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIButton m_btnBack;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_bone;
	EDGUIPropRsrc m_type;
	EDGUIPropVec3 m_offaxis;
	EDGUIPropFloat m_angle;
	EDGUIPropFloat m_base;
	int m_lid;
	int m_tid;
};


struct EDGUILayerProps : EDGUILayoutRow
{
	EDGUILayerProps() :
		m_testFactor( 1.0f, 2, -100, 100 ),
		m_group( true, "Transforms" ),
		m_lid( -1 )
	{
		type = CE_GUI_LAYERPROPS;
		
		m_name.caption = "Name";
		m_testFactor.caption = "Test factor";
		m_btnAdd.caption = "Add transform";
		
		m_ltfButtons.Add( &m_editButton );
		m_group.Add( &m_ltfButtons );
		
		Add( &m_name );
		Add( &m_testFactor );
		Add( &m_btnAdd );
		Add( &m_group );
	}
	~EDGUILayerProps()
	{
		m_lid = -1;
	}
	
	void Prepare( int lid )
	{
		m_lid = lid;
		AnimCharacter::Layer& L = g_AnimChar->layers[ m_lid ];
		
		m_name.SetValue( L.name );
		m_testFactor.SetValue( L.amount );
		
		m_ltfButtons.m_options.clear();
		m_ltfButtons.m_options.resize( L.transforms.size() );
		for( size_t i = 0; i < L.transforms.size(); ++i )
		{
			char bfr[ 256 ];
			AnimCharacter::LayerTransform& LT = L.transforms[ i ];
			StackString<200> bname( LT.bone );
			switch( LT.type )
			{
			case AnimCharacter::TransformType_None:
				sgrx_snprintf( bfr, 256, "#%d, disabled", (int) i );
				break;
			case AnimCharacter::TransformType_UndoParent:
				sgrx_snprintf( bfr, 256, "#%d, undo parent transform @ %s", (int) i, bname.str );
				break;
			case AnimCharacter::TransformType_Move:
				{
					Vec3& O = LT.posaxis;
					sgrx_snprintf( bfr, 256, "#%d, move by %g;%g;%g @ %s",
						(int) i, O.x, O.y, O.z, bname.str );
				}
				break;
			case AnimCharacter::TransformType_Rotate:
				{
					Vec3& A = LT.posaxis;
					sgrx_snprintf( bfr, 256, "#%d, rotate by %g degrees around %g;%g;%g @ %s",
						(int) i, LT.angle, A.x, A.y, A.z, bname.str );
				}
				break;
			default:
				sgrx_snprintf( bfr, 256, "#%d, unknown? (type=%d)", (int) i, (int) LT.type );
				break;
			}
			m_ltfButtons.m_options[ i ] = bfr;
		}
		m_ltfButtons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_lid >= 0 )
		{
			AnimCharacter::Layer& L = g_AnimChar->layers[ m_lid ];
			
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnAdd )
				{
					L.transforms.push_back( AnimCharacter::LayerTransform() );
					Prepare( m_lid );
					return 1;
				}
				if( e->target == &m_editButton )
				{
					FC_EditLayerTransform( m_lid, m_editButton.id2 );
					return 1;
				}
				if( e->target == &m_editButton.m_up )
				{
					size_t i = m_editButton.id2;
					if( i > 0 )
						TSWAP( L.transforms[ i ], L.transforms[ i - 1 ] );
					Prepare( m_lid );
					return 1;
				}
				if( e->target == &m_editButton.m_dn )
				{
					size_t i = m_editButton.id2;
					if( i < L.transforms.size() - 1 )
						TSWAP( L.transforms[ i ], L.transforms[ i + 1 ] );
					Prepare( m_lid );
					return 1;
				}
				if( e->target == &m_editButton.m_del )
				{
					L.transforms.erase( m_editButton.id2 );
					Prepare( m_lid );
					return 1;
				}
				break;
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_name )
				{
					L.name = m_name.m_value;
				}
				else if( e->target == &m_testFactor )
				{
					L.amount = m_testFactor.m_value;
				}
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIPropString m_name;
	EDGUIPropFloat m_testFactor;
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_ltfButtons;
	EDGUIListItemButton m_editButton;
	int m_lid;
};


struct EDGUILayerListProps : EDGUILayoutRow
{
	EDGUILayerListProps() :
		m_group( true, "Layers" )
	{
		type = CE_GUI_LAYERLISTPROPS;
		
		m_btnAdd.caption = "Add layer";
		
		m_layerButtons.Add( &m_editButton );
		m_group.Add( &m_layerButtons );
		
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_layerButtons.m_options.clear();
		m_layerButtons.m_options.resize( g_AnimChar->layers.size() );
		for( size_t i = 0; i < g_AnimChar->layers.size(); ++i )
		{
			m_layerButtons.m_options[ i ] = g_AnimChar->layers[ i ].name;
		}
		m_layerButtons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnAdd )
			{
				g_AnimChar->layers.push_back( AnimCharacter::Layer() );
				g_AnimChar->layers.last().name = "<unnamed>";
				g_AnimChar->layers.last().amount = 1;
				Prepare();
				return 1;
			}
			if( e->target == &m_editButton )
			{
				FC_EditLayer( m_editButton.id2 );
				return 1;
			}
			if( e->target == &m_editButton.m_up )
			{
				size_t i = m_editButton.id2;
				if( i > 0 )
					TSWAP( g_AnimChar->layers[ i ], g_AnimChar->layers[ i - 1 ] );
				Prepare();
				return 1;
			}
			if( e->target == &m_editButton.m_dn )
			{
				size_t i = m_editButton.id2;
				if( i < g_AnimChar->layers.size() - 1 )
					TSWAP( g_AnimChar->layers[ i ], g_AnimChar->layers[ i + 1 ] );
				Prepare();
				return 1;
			}
			if( e->target == &m_editButton.m_del )
			{
				size_t i = m_editButton.id2;
				g_AnimChar->layers.erase( i );
				Prepare();
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_layerButtons;
	EDGUIListItemButton m_editButton;
};


/////////////////
//  M A S K S  //
/////////////////


void FC_EditMaskCmd( int mid, int cid );
void FC_EditMask( int which );

struct EDGUIMaskCmdProps : EDGUILayoutRow
{
	EDGUIMaskCmdProps() :
		m_group( true, "<unset>" ),
		m_bone( g_UIBonePicker ),
		m_weight( 0, 3, 0, 1 ),
		m_mid( -1 ),
		m_cid( -1 )
	{
		type = CE_GUI_MASKCMDPROPS;
		
		m_btnBack.caption = "Back to mask <which?>";
		m_bone.caption = "Bone";
		m_weight.caption = "Weight";
		m_children.caption = "Incl. children?";
		
		m_group.Add( &m_bone );
		m_group.Add( &m_weight );
		m_group.Add( &m_children );
		
		Add( &m_btnBack );
		Add( &m_group );
	}
	
	void Prepare( int mid, int cid )
	{
		m_mid = mid;
		m_cid = cid;
		
		AnimCharacter::MaskCmd& MC = g_AnimChar->masks[ mid ].cmds[ cid ];
		StackString<200> maskname( g_AnimChar->masks[ mid ].name );
		
		char bfr[ 256 ];
		
		sgrx_snprintf( bfr, 256, "Back to mask: %s", maskname.str );
		m_btnBack.caption = bfr;
		
		sgrx_snprintf( bfr, 256, "Mask %s command %d", maskname.str, cid );
		m_group.caption = bfr;
		m_group.SetOpen( true );
		
		m_bone.SetValue( MC.bone );
		m_weight.SetValue( MC.weight );
		m_children.SetValue( MC.children );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_mid >= 0 && m_cid >= 0 )
		{
			AnimCharacter::MaskCmd& MC = g_AnimChar->masks[ m_mid ].cmds[ m_cid ];
			
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnBack )
				{
					FC_EditMask( m_mid );
					return 1;
				}
				break;
				
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_bone ) MC.bone = m_bone.m_value;
				else if( e->target == &m_weight ) MC.weight = m_weight.m_value;
				else if( e->target == &m_children ) MC.children = m_children.m_value;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIButton m_btnBack;
	EDGUIGroup m_group;
	EDGUIPropRsrc m_bone;
	EDGUIPropFloat m_weight;
	EDGUIPropBool m_children;
	int m_mid;
	int m_cid;
};


struct EDGUIMaskProps : EDGUILayoutRow
{
	EDGUIMaskProps() :
		m_group( true, "Commands" ),
		m_previewSize( 0.04f, 3, 0.001f, 1 ),
		m_mid( -1 )
	{
		type = CE_GUI_MASKPROPS;
		
		m_previewSize.caption = "Preview bone tick size";
		m_name.caption = "Name";
		m_btnAdd.caption = "Add command";
		
		m_mcmdButtons.Add( &m_editButton );
		m_group.Add( &m_mcmdButtons );
		
		Add( &m_previewSize );
		Add( &m_name );
		Add( &m_btnAdd );
		Add( &m_group );
	}
	~EDGUIMaskProps()
	{
		m_mid = -1;
	}
	
	void Prepare( int mid )
	{
		m_mid = mid;
		AnimCharacter::Mask& M = g_AnimChar->masks[ m_mid ];
		
		m_name.SetValue( M.name );
		
		m_mcmdButtons.m_options.clear();
		m_mcmdButtons.m_options.resize( M.cmds.size() );
		for( size_t i = 0; i < M.cmds.size(); ++i )
		{
			char bfr[ 256 ];
			AnimCharacter::MaskCmd& MC = M.cmds[ i ];
			StackString<200> bname( MC.bone );
			sgrx_snprintf( bfr, 256, "#%d, %f @ %s%s", (int) i,
				MC.weight, bname.str, MC.children ? ", incl. children" : "" );
			m_mcmdButtons.m_options[ i ] = bfr;
		}
		m_mcmdButtons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_mid >= 0 )
		{
			AnimCharacter::Mask& M = g_AnimChar->masks[ m_mid ];
			
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btnAdd )
				{
					M.cmds.push_back( AnimCharacter::MaskCmd() );
					Prepare( m_mid );
					return 1;
				}
				if( e->target == &m_editButton )
				{
					FC_EditMaskCmd( m_mid, m_editButton.id2 );
					return 1;
				}
				if( e->target == &m_editButton.m_up )
				{
					size_t i = m_editButton.id2;
					if( i > 0 )
						TSWAP( M.cmds[ i ], M.cmds[ i - 1 ] );
					Prepare( m_mid );
					return 1;
				}
				if( e->target == &m_editButton.m_dn )
				{
					size_t i = m_editButton.id2;
					if( i < M.cmds.size() - 1 )
						TSWAP( M.cmds[ i ], M.cmds[ i + 1 ] );
					Prepare( m_mid );
					return 1;
				}
				if( e->target == &m_editButton.m_del )
				{
					M.cmds.erase( m_editButton.id2 );
					Prepare( m_mid );
					return 1;
				}
				break;
			case EDGUI_EVENT_PROPEDIT:
				if( e->target == &m_name )
				{
					M.name = m_name.m_value;
				}
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIPropString m_name;
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_mcmdButtons;
	EDGUIListItemButton m_editButton;
	EDGUIPropFloat m_previewSize;
	int m_mid;
};


struct EDGUIMaskListProps : EDGUILayoutRow
{
	EDGUIMaskListProps() :
		m_group( true, "Masks" ),
		m_editButton( false )
	{
		type = CE_GUI_MASKLISTPROPS;
		
		m_btnAdd.caption = "Add mask";
		
		m_maskButtons.Add( &m_editButton );
		m_group.Add( &m_maskButtons );
		
		Add( &m_btnAdd );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_maskButtons.m_options.clear();
		m_maskButtons.m_options.resize( g_AnimChar->masks.size() );
		for( size_t i = 0; i < g_AnimChar->masks.size(); ++i )
		{
			m_maskButtons.m_options[ i ] = g_AnimChar->masks[ i ].name;
		}
		m_maskButtons.UpdateOptions();
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btnAdd )
			{
				g_AnimChar->masks.push_back( AnimCharacter::Mask() );
				g_AnimChar->masks.last().name = "<unnamed>";
				Prepare();
				return 1;
			}
			if( e->target == &m_editButton )
			{
				FC_EditMask( m_editButton.id2 );
				return 1;
			}
			if( e->target == &m_editButton.m_del )
			{
				size_t i = m_editButton.id2;
				g_AnimChar->masks.erase( i );
				Prepare();
				return 1;
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIButton m_btnAdd;
	EDGUIBtnList m_maskButtons;
	EDGUIListItemButton m_editButton;
};


struct EDGUICharProps : EDGUILayoutRow
{
	EDGUICharProps() :
		m_group( true, "Character properties" ),
		m_mesh( g_UIMeshPicker )
	{
		type = CE_GUI_CHARPROPS;
		tyname = "charprops";
		
		m_mesh.caption = "Mesh";
		
		m_group.Add( &m_mesh );
		Add( &m_group );
	}
	
	void Prepare()
	{
		m_mesh.SetValue( g_AnimChar->mesh );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_mesh )
			{
				LOG << "Picked MESH: " << m_mesh.m_value;
				g_AnimChar->mesh = m_mesh.m_value;
				g_AnimChar->_OnRenderUpdate();
				SGRX_IMesh* M = g_AnimChar->m_cachedMesh;
				if( M )
				{
					for( int i = 0; i < M->m_numBones; ++i )
					{
						StringView name = M->m_bones[ i ].name;
						size_t j = 0;
						for( ; j < g_AnimChar->bones.size(); ++j )
						{
							if( g_AnimChar->bones[ j ].name == name )
								break;
						}
						if( j == g_AnimChar->bones.size() )
						{
							AnimCharacter::BoneInfo B;
							B.name = name;
							g_AnimChar->bones.push_back( B );
						}
					}
				}
				reload_mesh_vertices();
				g_AnimChar->RecalcBoneIDs();
				lmm_prepmeshinst( g_AnimChar->m_cachedMeshInst );
				g_UIBonePicker->Reload();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIPropRsrc m_mesh;
};


struct FMVertex
{
	Vec3 pos;
	Vec3 nrm;
	Vec2 tex;
};

static void floor_mesh_update( float size, float height )
{
	FMVertex verts[4] =
	{
		{ { -size, -size, 0 }, {0,0,1}, { +size, -size } },
		{ { +size, -size, 0 }, {0,0,1}, { -size, -size } },
		{ { +size, +size, 0 }, {0,0,1}, { -size, +size } },
		{ { -size, +size, 0 }, {0,0,1}, { +size, +size } },
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


struct EDGUIRagdollTestProps : EDGUILayoutRow
{
	EDGUIRagdollTestProps() :
		m_floor_size( 2, 2, 0.01f, 100.0f ),
		m_floor_height( 0, 2, -100.0f, 100.0f )
	{
		tyname = "ragdolltestprops";
		
		m_floor_size.caption = "Floor size";
		m_floor_height.caption = "Floor height";
		m_btn_start.caption = "Start";
		m_btn_stop.caption = "Stop";
		m_btn_wakeup.caption = "Wake up";
		
		Add( &m_floor_size );
		Add( &m_floor_height );
		Add( &m_btn_start );
		Add( &m_btn_stop );
		Add( &m_btn_wakeup );
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
			if( e->target == &m_floor_size || e->target == &m_floor_height )
			{
				floor_mesh_update( m_floor_size.m_value, m_floor_height.m_value );
			}
			break;
			
		case EDGUI_EVENT_BTNCLICK:
			if( e->target == &m_btn_start )
			{
				if( g_AnimChar->m_anRagdoll.m_enabled == false )
				{
					g_AnimChar->m_anRagdoll.Initialize( g_AnimChar );
					g_AnimChar->EnablePhysics();
				}
			}
			if( e->target == &m_btn_stop )
			{
				g_AnimChar->DisablePhysics();
			}
			if( e->target == &m_btn_wakeup )
			{
				g_AnimChar->WakeUp();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIPropFloat m_floor_size;
	EDGUIPropFloat m_floor_height;
	EDGUIButton m_btn_start;
	EDGUIButton m_btn_stop;
	EDGUIButton m_btn_wakeup;
};


struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_showBodies( true ),
		m_showJoints( true ),
		m_showHitboxes( true ),
		m_showAttachments( true ),
		m_showMasks( true ),
		m_showPhysics( true ),
		m_UIMenuSplit( true, 26, 0 ),
		m_UIParamSplit( false, 0, 0.6f ),
		m_UIRenderView( g_EdScene, this )
	{
		tyname = "mainframe";
		
		Add( &m_UIMenuSplit );
		m_UIMenuSplit.SetFirstPane( &m_UIMenuButtons );
		m_UIMenuSplit.SetSecondPane( &m_UIParamSplit );
		m_UIParamSplit.SetFirstPane( &m_UIRenderView );
		m_UIParamSplit.SetSecondPane( &m_UIParamScroll );
		m_UIParamScroll.Add( &m_UIParamList );
		
		// menu
		m_MB_Cat0.caption = "File:";
		m_MBNew.caption = "New";
		m_MBOpen.caption = "Open";
		m_MBSave.caption = "Save";
		m_MBSaveAs.caption = "Save As";
		m_MB_Cat1.caption = "Edit:";
		m_MBEditChar.caption = "Edit character";
		m_MBEditBones.caption = "Edit bones";
		m_MBEditAttachments.caption = "Edit attachments";
		m_MBEditLayers.caption = "Edit layers";
		m_MBEditMasks.caption = "Edit masks";
		m_MBRagdollTest.caption = "Ragdoll test";
		m_UIMenuButtons.Add( &m_MB_Cat0 );
		m_UIMenuButtons.Add( &m_MBNew );
		m_UIMenuButtons.Add( &m_MBOpen );
		m_UIMenuButtons.Add( &m_MBSave );
		m_UIMenuButtons.Add( &m_MBSaveAs );
		m_UIMenuButtons.Add( &m_MB_Cat1 );
		m_UIMenuButtons.Add( &m_MBEditChar );
		m_UIMenuButtons.Add( &m_MBEditBones );
		m_UIMenuButtons.Add( &m_MBEditAttachments );
		m_UIMenuButtons.Add( &m_MBEditLayers );
		m_UIMenuButtons.Add( &m_MBEditMasks );
		m_UIMenuButtons.Add( &m_MBRagdollTest );
	}
	
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_BTNCLICK:
			if(0);
			
			else if( e->target == &m_MBNew ) CH_New();
			else if( e->target == &m_MBOpen ) CH_Open();
			else if( e->target == &m_MBSave ) CH_Save();
			else if( e->target == &m_MBSaveAs ) CH_SaveAs();
			
			else if( e->target == &m_MBEditChar )
			{
				ClearParamList();
				m_charProps.Prepare();
				AddToParamList( &m_charProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditBones )
			{
				ClearParamList();
				m_boneListProps.Prepare();
				AddToParamList( &m_boneListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditAttachments )
			{
				ClearParamList();
				m_atchListProps.Prepare();
				AddToParamList( &m_atchListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditLayers )
			{
				ClearParamList();
				m_layerListProps.Prepare();
				AddToParamList( &m_layerListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBEditMasks )
			{
				ClearParamList();
				m_maskListProps.Prepare();
				AddToParamList( &m_maskListProps );
				SetActiveMode( e->target );
			}
			else if( e->target == &m_MBRagdollTest )
			{
				ClearParamList();
				AddToParamList( &m_ragdollTestProps );
				SetActiveMode( e->target );
			}
			
			return 1;
			
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target == g_UICharOpenPicker )
			{
				CH_Real_Open( g_UICharOpenPicker->GetValue() );
			}
			if( e->target == g_UICharSavePicker )
			{
				CH_Real_Save( g_UICharSavePicker->GetValue() );
			}
			return 1;
		}
		return EDGUIFrame::OnEvent( e );
	}
	
	void SetActiveMode( EDGUIItem* btn )
	{
		m_MBEditChar.SetHighlight( &m_MBEditChar == btn );
		m_MBEditBones.SetHighlight( &m_MBEditBones == btn );
		m_MBEditAttachments.SetHighlight( &m_MBEditAttachments == btn );
		m_MBEditLayers.SetHighlight( &m_MBEditLayers == btn );
		m_MBEditMasks.SetHighlight( &m_MBEditMasks == btn );
		m_MBRagdollTest.SetHighlight( &m_MBRagdollTest == btn );
	}
	
	bool ViewEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PAINT:
			{
				int rx0 = m_UIRenderView.x0;
				int ry0 = m_UIRenderView.y0;
				int rx1 = m_UIRenderView.x1;
				int ry1 = m_UIRenderView.y1;
				
				if( m_showAttachments && PushScissorRect( rx0, ry0, rx1, ry1 ) )
				{
					GR2D_GetBatchRenderer().Reset().Col( 1 );
					
					for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
					{
						Mat4 wm;
						if( g_AnimChar->GetAttachmentMatrix( i, wm ) )
						{
							bool infront = true;
							Vec3 spos = g_EdScene->camera.WorldToScreen( wm.TransformPos( V3(0,0,0) ), &infront );
							if( infront )
							{
								int tx = TLERP( rx0, rx1, spos.x );
								int ty = TLERP( ry0, ry1, spos.y );
								GR2D_DrawTextLine( tx, ty, g_AnimChar->attachments[ i ].name );
							}
						}
					}
					PopScissorRect();
				}
				
				GR2D_GetBatchRenderer().Reset().Col( 0, 0.5f ).Quad( rx0, ry0, rx1, ry0 + 16 );
				
				GR2D_SetTextCursor( rx0, ry0 );
				
				GR2D_SetColor( 1, 1 );
				GR2D_DrawTextLine( "Render items: [Bodies (1): " );
				YesNoText( m_showBodies );
				GR2D_DrawTextLine( ", Joints (2): " );
				YesNoText( m_showJoints );
				GR2D_DrawTextLine( ", Hitboxes (3): " );
				YesNoText( m_showHitboxes );
				GR2D_DrawTextLine( ", Attachments (4): " );
				YesNoText( m_showAttachments );
				GR2D_DrawTextLine( ", Masks (5): " );
				YesNoText( m_showMasks );
				GR2D_DrawTextLine( ", Physics (6): " );
				YesNoText( m_showPhysics );
				GR2D_DrawTextLine( "]" );
			}
			break;
		case EDGUI_EVENT_KEYDOWN:
			if( e->key.repeat == false )
			{
				if( e->key.engkey == SDLK_1 )
					m_showBodies = !m_showBodies;
				if( e->key.engkey == SDLK_2 )
					m_showJoints = !m_showJoints;
				if( e->key.engkey == SDLK_3 )
					m_showHitboxes = !m_showHitboxes;
				if( e->key.engkey == SDLK_4 )
					m_showAttachments = !m_showAttachments;
				if( e->key.engkey == SDLK_5 )
					m_showMasks = !m_showMasks;
				if( e->key.engkey == SDLK_6 )
					m_showPhysics = !m_showPhysics;
				if( e->key.engkey == SDLK_g )
					g_XFormState.OnMoveKey();
				if( e->key.engkey == SDLK_r )
					g_XFormState.OnRotateKey();
				if( e->key.engkey == SDLK_RETURN )
					g_XFormState.OnConfirmKey();
				if( e->key.engkey == SDLK_ESCAPE )
					g_XFormState.OnCancelKey();
			}
			break;
		case EDGUI_EVENT_BTNDOWN:
			if( e->mouse.button == EDGUI_MB_LEFT )
				g_XFormState.OnConfirmKey();
			if( e->mouse.button == EDGUI_MB_RIGHT )
				g_XFormState.OnCancelKey();
			break;
		}
		return true;
	}
	
	void DebugDraw()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		
		if( g_AnimChar && g_AnimChar->m_cachedMesh && g_AnimChar->m_cachedMeshInst )
		{
			if( m_showBodies )
			{
				br.Reset();
				for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
				{
					Mat4 wm;
					if( g_AnimChar->GetBodyMatrix( i, wm ) )
					{
						Vec3 size = g_AnimChar->bones[ i ].body.size;
						if( (int) i == m_boneProps.m_bid )
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
			
			if( m_showJoints )
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
					if( g_AnimChar->GetJointMatrix( i, true, wm ) )
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
					
					if( g_AnimChar->GetJointMatrix( i, false, wm ) )
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
			
			if( m_showHitboxes )
			{
				SceneRaycastCallback_Closest srcc;
				Vec3 p0 = g_UIFrame->m_UIRenderView.crpos;
				Vec3 p1 = p0 + g_UIFrame->m_UIRenderView.crdir * 1000;
				g_AnimChar->RaycastAll( p0, p1, &srcc );
				
				br.Reset();
				for( size_t i = 0; i < g_AnimChar->bones.size(); ++i )
				{
					bool hit = srcc.m_hit && g_AnimChar->bones[ i ].bone_id == srcc.m_closest.boneID;
					Mat4 wm;
					Vec3 ext;
					if( g_AnimChar->GetHitboxOBB( i, wm, ext ) )
					{
						if( (int) i == m_boneProps.m_bid )
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
			if( m_showAttachments )
			{
				br.Reset();
				br.SetPrimitiveType( PT_Lines );
				for( size_t i = 0; i < g_AnimChar->attachments.size(); ++i )
				{
					Mat4 wm;
					if( g_AnimChar->GetAttachmentMatrix( i, wm ) )
					{
						float a, d = 0.8f;
						if( (int) i == m_atchProps.m_aid )
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
			if( m_showMasks && g_AnimChar->m_cachedMesh && g_AnimChar->m_cachedMeshInst )
			{
				br.Reset();
				
				for( size_t i = 0; i < g_AnimChar->masks.size(); ++i )
				{
					if( (int) i != m_maskProps.m_mid )
						continue; // displaying more than one at a time would lead to excessive noise
					
					float maskdata[ SGRX_MAX_MESH_BONES ] = {0};
					
					AnimCharacter::Mask& M = g_AnimChar->masks[ i ];
					for( size_t j = 0; j < M.cmds.size(); ++j )
					{
						AnimCharacter::MaskCmd& MC = M.cmds[ j ];
						int bone_id = g_AnimChar->_FindBone( MC.bone );
						if( bone_id >= 0 )
						{
							maskdata[ bone_id ] = MC.weight;
							if( MC.children )
							{
								for( int b = 0; b < g_AnimChar->m_cachedMesh->m_numBones; ++b )
								{
									if( g_AnimChar->m_cachedMesh->IsBoneUnder( b, bone_id ) )
										maskdata[ b ] = MC.weight;
								}
							}
						}
					}
					
					for( int bone_id = 0; bone_id < g_AnimChar->m_cachedMesh->m_numBones; ++bone_id )
					{
						Mat4 bmtx = g_AnimChar->m_cachedMeshInst->matrix;
						if( g_AnimChar->m_cachedMeshInst->IsSkinned() )
							bmtx = g_AnimChar->m_cachedMeshInst->skin_matrices[ bone_id ] * bmtx;
						bmtx = g_AnimChar->m_cachedMesh->m_bones[ bone_id ].skinOffset * bmtx;
						
						Vec3 pos = bmtx.TransformPos( V3(0) );
						br.Col( 0.1f + ( 1 - maskdata[ bone_id ] ) * 0.8f, 0.1f + maskdata[ bone_id ] * 0.8f, 0.1f );
						br.Tick( pos, m_maskProps.m_previewSize.m_value );
					}
				}
			}
			
			// draw physics
			if( m_showPhysics )
			{
				g_PhyWorld->DebugDraw();
			}
			
			// draw bones
			if( 1 )
			{
				br.Reset();
				SGRX_IMesh* mesh = g_AnimChar->m_cachedMeshInst->GetMesh();
				for( int i = 0; i < mesh->m_numBones; ++i )
				{
					float sxt = ( 1 - float(i) / mesh->m_numBones );
					br.Axis( mesh->m_bones[ i ].skinOffset, 0.1f + sxt * 0.1f );
				}
			}
		}
	}
	
	void AddToParamList( EDGUIItem* item )
	{
		m_UIParamList.Add( item );
	}
	void ClearParamList()
	{
		while( m_UIParamList.m_subitems.size() )
			m_UIParamList.Remove( m_UIParamList.m_subitems.last() );
	}
	void AutoUpdateParamList()
	{
		for( size_t i = 0; i < m_UIParamList.m_subitems.size(); ++i )
		{
			EDGUIItem* item = m_UIParamList.m_subitems[ i ];
			switch( item->type )
			{
			case CE_GUI_BONEPROPS: m_boneProps.Prepare( m_boneProps.m_bid ); break;
			case CE_GUI_BONELISTPROPS: m_boneListProps.Prepare(); break;
			case CE_GUI_ATCHPROPS: m_atchProps.Prepare( m_atchProps.m_aid ); break;
			case CE_GUI_ATCHLISTPROPS: m_atchListProps.Prepare(); break;
			case CE_GUI_LAYERTFPROPS: m_layerXfProps.Prepare( m_layerXfProps.m_lid, m_layerXfProps.m_tid ); break;
			case CE_GUI_LAYERPROPS: m_layerProps.Prepare( m_layerProps.m_lid ); break;
			case CE_GUI_LAYERLISTPROPS: m_layerListProps.Prepare(); break;
			case CE_GUI_MASKCMDPROPS: m_maskCmdProps.Prepare( m_maskCmdProps.m_mid, m_maskCmdProps.m_cid ); break;
			case CE_GUI_MASKPROPS: m_maskProps.Prepare( m_maskProps.m_mid ); break;
			case CE_GUI_MASKLISTPROPS: m_maskListProps.Prepare(); break;
			case CE_GUI_CHARPROPS: m_charProps.Prepare(); break;
			default: break;
			}
		}
	}
	
	void EditBone( int which )
	{
		ClearParamList();
		m_boneProps.Prepare( which );
		AddToParamList( &m_boneProps );
	}
	void EditAttachment( int which )
	{
		ClearParamList();
		m_atchProps.Prepare( which );
		AddToParamList( &m_atchProps );
	}
	void EditLayerTransform( int lid, int tid )
	{
		ClearParamList();
		m_layerXfProps.Prepare( lid, tid );
		AddToParamList( &m_layerXfProps );
	}
	void EditLayer( int which )
	{
		ClearParamList();
		m_layerProps.Prepare( which );
		AddToParamList( &m_layerProps );
	}
	void EditMaskCmd( int mid, int cid )
	{
		ClearParamList();
		m_maskCmdProps.Prepare( mid, cid );
		AddToParamList( &m_maskCmdProps );
	}
	void EditMask( int which )
	{
		ClearParamList();
		m_maskProps.Prepare( which );
		AddToParamList( &m_maskProps );
	}
	
	void ResetEditorState()
	{
		reload_mesh_vertices();
		if( g_AnimChar->m_cachedMeshInst )
			lmm_prepmeshinst( g_AnimChar->m_cachedMeshInst );
		g_UIBonePicker->Reload();
		
		ClearParamList();
		m_charProps.Prepare();
		AddToParamList( &m_charProps );
		SetActiveMode( &m_MBEditChar );
	}
	void CH_New()
	{
		m_fileName = "";
		delete g_AnimChar;
		g_AnimChar = new AnimCharacter( g_EdScene, g_PhyWorld );
		ResetEditorState();
	}
	void CH_Open()
	{
		g_UICharOpenPicker->Reload();
		g_UICharOpenPicker->Open( this, "" );
		m_frame->Add( g_UICharOpenPicker );
	}
	void CH_Save()
	{
		if( m_fileName.size() )
		{
			CH_Real_Save( m_fileName );
		}
		else
		{
			CH_SaveAs();
		}
	}
	void CH_SaveAs()
	{
		g_UICharSavePicker->Reload();
		g_UICharSavePicker->Open( this, "" );
		m_frame->Add( g_UICharSavePicker );
	}
	void CH_Real_Open( const StringView& str )
	{
		LOG << "Trying to open character: " << str;
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "chars/%.*s.chr", TMIN( (int) str.size(), 200 ), str.data() );
		if( !g_AnimChar->Load( bfr ) )
		{
			LOG_ERROR << "FAILED TO LOAD CHAR FILE: " << bfr;
			return;
		}
		ResetEditorState();
		m_fileName = str;
	}
	void CH_Real_Save( const StringView& str )
	{
		LOG << "Trying to save character: " << str;
		char bfr[ 256 ];
		sgrx_snprintf( bfr, sizeof(bfr), "chars/%.*s.chr", TMIN( (int) str.size(), 200 ), str.data() );
		if( !g_AnimChar->Save( bfr ) )
		{
			LOG_ERROR << "FAILED TO SAVE CHAR FILE: " << bfr;
			return;
		}
		m_fileName = str;
	}
	
	String m_fileName;
	
	bool m_showBodies;
	bool m_showJoints;
	bool m_showHitboxes;
	bool m_showAttachments;
	bool m_showMasks;
	bool m_showPhysics;
	
	// property blocks
	EDGUIBoneProps m_boneProps;
	EDGUIBoneListProps m_boneListProps;
	EDGUIAttachmentProps m_atchProps;
	EDGUIAttachmentListProps m_atchListProps;
	EDGUILayerTransformProps m_layerXfProps;
	EDGUILayerProps m_layerProps;
	EDGUILayerListProps m_layerListProps;
	EDGUIMaskCmdProps m_maskCmdProps;
	EDGUIMaskProps m_maskProps;
	EDGUIMaskListProps m_maskListProps;
	EDGUICharProps m_charProps;
	EDGUIRagdollTestProps m_ragdollTestProps;
	
	// core layout
	EDGUILayoutSplitPane m_UIMenuSplit;
	EDGUILayoutSplitPane m_UIParamSplit;
	EDGUILayoutColumn m_UIMenuButtons;
	EDGUIVScroll m_UIParamScroll;
	EDGUILayoutRow m_UIParamList;
	EDGUIRenderView m_UIRenderView;
	
	// menu
	EDGUILabel m_MB_Cat0;
	EDGUIButton m_MBNew;
	EDGUIButton m_MBOpen;
	EDGUIButton m_MBSave;
	EDGUIButton m_MBSaveAs;
	EDGUILabel m_MB_Cat1;
	EDGUIButton m_MBEditChar;
	EDGUIButton m_MBEditBones;
	EDGUIButton m_MBEditAttachments;
	EDGUIButton m_MBEditLayers;
	EDGUIButton m_MBEditMasks;
	EDGUIButton m_MBRagdollTest;
};

void FC_EditBone( int which ){ g_UIFrame->EditBone( which ); }
void FC_EditAttachment( int which ){ g_UIFrame->EditAttachment( which ); }
void FC_EditLayerTransform( int lid, int tid ){ g_UIFrame->EditLayerTransform( lid, tid ); }
void FC_EditLayer( int which ){ g_UIFrame->EditLayer( which ); }
void FC_EditMaskCmd( int mid, int cid ){ g_UIFrame->EditMaskCmd( mid, cid ); }
void FC_EditMask( int which ){ g_UIFrame->EditMask( which ); }
void CE_UpdateParamList(){ g_UIFrame->AutoUpdateParamList(); }




struct CSEditor : IGame
{
	CSEditor() : phySlow(false){}
	bool OnInitialize()
	{
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_SetFont( "core", 12 );
		
		g_UIMeshPicker = new EDGUIMeshPicker( true );
		g_UICharOpenPicker = new EDGUICharOpenPicker;
		g_UICharSavePicker = new EDGUICharSavePicker;
		g_UIBonePicker = new EDGUIBonePicker;
		g_UIBodyType = new EDGUIBodyType;
		g_UIJointType = new EDGUIJointType;
		g_UITransformType = new EDGUITransformType;
		
		// core layout
		g_PhyWorld = PHY_CreateWorld();
		g_PhyWorld->SetGravity( V3( 0, 0, -9.81f ) );
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_EdScene->skyTexture = GR_GetTexture( "textures/sky/overcast1.dds" );
		g_AnimChar = new AnimCharacter( g_EdScene, g_PhyWorld );
		g_AnimMixLayers[ 0 ].anim = &g_AnimChar->m_layerAnimator;
		g_AnimMixLayers[ 0 ].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
		g_AnimMixLayers[ 1 ].anim = &g_AnimChar->m_anRagdoll;
		g_AnimMixLayers[ 1 ].tflags = AnimMixer::TF_Absolute_Pos | AnimMixer::TF_Absolute_Rot;
		g_AnimChar->m_anMixer.layers = g_AnimMixLayers;
		g_AnimChar->m_anMixer.layerCount = sizeof(g_AnimMixLayers) / sizeof(g_AnimMixLayers[0]);
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
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
		
		// TEST
#if 0
		g_AnimChar->m_anDeformer.AddModelForce( V3(-0.5f,0.5f,1), V3(1,-1,0), 0.7f, 1, 0.5f );
#endif
		
		// param area
		g_UIFrame->ResetEditorState();
		
		return true;
	}
	void OnDestroy()
	{
		delete g_UITransformType;
		g_UITransformType = NULL;
		delete g_UIJointType;
		g_UIJointType = NULL;
		delete g_UIBodyType;
		g_UIBodyType = NULL;
		delete g_UIBonePicker;
		g_UIBonePicker = NULL;
		delete g_UICharSavePicker;
		g_UICharSavePicker = NULL;
		delete g_UICharOpenPicker;
		g_UICharOpenPicker = NULL;
		delete g_UIMeshPicker;
		g_UIMeshPicker = NULL;
		delete g_UIFrame;
		g_UIFrame = NULL;
		delete g_AnimChar;
		g_FloorMeshInst = NULL;
		g_AnimChar = NULL;
		g_EdScene = NULL;
		g_PhyWorld = NULL;
	}
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB ) phySlow = true;
		if( e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_TAB ) phySlow = false;
		g_UIFrame->EngineEvent( &e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		g_XFormState.OnUpdate(
			g_UIFrame->m_UIRenderView.CPToNormalized( Game_GetCursorPos() ),
			g_UIFrame->m_UIRenderView.GetViewSize() );
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_AnimChar->RecalcLayerState();
		g_AnimChar->FixedTick( dt );
		for( int i = 0; i < (phySlow?1:10); ++i )
			g_PhyWorld->Step( dt / 10 );
		g_AnimChar->PreRender( 1 );
		g_UIFrame->Draw();
#if 0
		//float fac = sinf( g_AnimChar->m_anDeformer.forces[ 0 ].lifetime * M_PI ) * 0.5f + 0.5f;
		g_AnimChar->m_anDeformer.forces[ 0 ].amount = 1;//fac * 0.5f;
		g_AnimChar->m_anDeformer.forces[ 0 ].pos = g_EdScene->camera.position;
		g_AnimChar->m_anDeformer.forces[ 0 ].dir = g_EdScene->camera.direction;
#endif
	}
	
	bool phySlow;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}


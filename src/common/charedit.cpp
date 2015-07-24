

#include <engine.hpp>
#include <enganim.hpp>
#include <engext.hpp>
#include <edgui.hpp>
#include "edcomui.hpp"


struct EDGUIMainFrame* g_UIFrame;
SceneHandle g_EdScene;
struct EDGUIMeshPicker* g_UIMeshPicker;
struct EDGUICharOpenPicker* g_UICharOpenPicker;
struct EDGUICharSavePicker* g_UICharSavePicker;
struct EDGUIBonePicker* g_UIBonePicker;
struct EDGUIBodyType* g_UIBodyType;
struct EDGUIJointType* g_UIJointType;
struct EDGUITransformType* g_UITransformType;
AnimCharacter* g_AnimChar;
AnimMixer::Layer g_AnimMixLayers[1];


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

void calc_char_bone_info( int bid, int thres, int mask )
{
	if( g_AnimChar == NULL || bid < 0 || bid >= (int) g_AnimChar->bones.size() )
	{
		LOG_ERROR << "Cannot calculate bone info for bone " << bid << " at this time";
		return;
	}
	AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ bid ];
	AMBone B = { BI.bone_id, BI.name, (uint8_t) thres };
	
	calc_bone_volume_info( B );
	
	Mat4 world_to_local = BI.bone_id < 0 ? Mat4::Identity :
		g_AnimChar->m_cachedMesh->m_bones[ BI.bone_id ].invSkinOffset;
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


#define LI_SUBBTN_WIDTH 30
struct EDGUIListItemButton : EDGUIButton
{
	EDGUIListItemButton( bool ordered = true )
	{
		tyname = "btn_item";
		m_up.tyname = "btn_up";
		m_dn.tyname = "btn_dn";
		m_del.tyname = "btn_del";
		
		m_up.caption = "[up]";
		m_dn.caption = "[dn]";
		m_del.caption = "[x]";
		
		m_del.SetHighlight( true );
		
		if( ordered )
		{
			Add( &m_up );
			Add( &m_dn );
		}
		Add( &m_del );
	}
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_LAYOUT:
			EDGUIButton::OnEvent( e );
			SetSubitemLayout( &m_up, x1 - LI_SUBBTN_WIDTH * 3, y0, x1 - LI_SUBBTN_WIDTH * 2, y1 );
			SetSubitemLayout( &m_dn, x1 - LI_SUBBTN_WIDTH * 2, y0, x1 - LI_SUBBTN_WIDTH * 1, y1 );
			SetSubitemLayout( &m_del, x1 - LI_SUBBTN_WIDTH * 1, y0, x1 - LI_SUBBTN_WIDTH * 0, y1 );
			return 0;
			
		case EDGUI_EVENT_PAINT:
			if( backColor )
			{
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( float(x0), float(y0), float(x1), float(y1) );
			}
			if( textColor && caption.size() )
			{
				GR2D_GetBatchRenderer().Colu( textColor );
				GR2D_DrawTextLine( x0 + 2, round(( y0 + y1 ) / 2.0f), caption, HALIGN_LEFT, VALIGN_CENTER );
			}
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				m_subitems[ i ]->OnEvent( e );
			}
			return 1;
			
		}
		return EDGUIButton::OnEvent( e );
	}
	
	EDGUIButton m_up;
	EDGUIButton m_dn;
	EDGUIButton m_del;
};


struct EDGUIBoneProps : EDGUILayoutRow
{
	EDGUIBoneProps() :
		m_group( true, "Bone properties" ),
		m_group_hbox( true, "Hitbox" ),
		m_group_body( true, "Body" ),
		m_group_recalc( true, "Recalculate shapes" ),
		m_hbox_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_hbox_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_hbox_extents( V3(0.1f), 2, V3(0), V3(100) ),
		m_hbox_multiplier( 1, 2, 0, 100.0f ),
		m_body_rotangles( V3(0), 2, V3(-360), V3(360) ),
		m_body_offset( V3(0), 2, V3(-8192), V3(8192) ),
		m_body_type( g_UIBodyType, "None" ),
		m_body_size( V3(0.1f), 2, V3(0), V3(100) ),
		m_recalc_thres( 96, 0, 255 ),
		m_bid( -1 )
	{
		m_hbox_rotangles.caption = "Rotation (angles)";
		m_hbox_offset.caption = "Offset";
		m_hbox_extents.caption = "Extents";
		m_hbox_multiplier.caption = "Multiplier";
		
		m_body_rotangles.caption = "Rotation (angles)";
		m_body_offset.caption = "Offset";
		m_body_type.caption = "Body types";
		m_body_size.caption = "Body size";
		
		m_recalc_thres.caption = "Threshold";
		m_btn_recalc_body.caption = "Recalculate body";
		m_btn_recalc_hitbox.caption = "Recalculate hitbox";
		m_btn_recalc_both.caption = "Recalculate both";
		
		m_group_hbox.Add( &m_hbox_rotangles );
		m_group_hbox.Add( &m_hbox_offset );
		m_group_hbox.Add( &m_hbox_extents );
		m_group_hbox.Add( &m_hbox_multiplier );
		
		m_group_body.Add( &m_body_rotangles );
		m_group_body.Add( &m_body_offset );
		m_group_body.Add( &m_body_type );
		m_group_body.Add( &m_body_size );
		
		m_group_recalc.Add( &m_recalc_thres );
		m_group_recalc.Add( &m_btn_recalc_body );
		m_group_recalc.Add( &m_btn_recalc_hitbox );
		m_group_recalc.Add( &m_btn_recalc_both );
		
		m_group.Add( &m_group_hbox );
		m_group.Add( &m_group_body );
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
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( m_bid >= 0 )
		{
			AnimCharacter::BoneInfo& BI = g_AnimChar->bones[ m_bid ];
			switch( e->type )
			{
			case EDGUI_EVENT_BTNCLICK:
				if( e->target == &m_btn_recalc_body )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, 1 );
					Prepare( m_bid );
				}
				if( e->target == &m_btn_recalc_hitbox )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, 2 );
					Prepare( m_bid );
				}
				if( e->target == &m_btn_recalc_both )
				{
					calc_char_bone_info( m_bid, m_recalc_thres.m_value, 1|2 );
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
				break;
			}
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	EDGUIGroup m_group;
	EDGUIGroup m_group_hbox;
	EDGUIGroup m_group_body;
	EDGUIGroup m_group_recalc;
	EDGUIPropVec3 m_hbox_rotangles;
	EDGUIPropVec3 m_hbox_offset;
	EDGUIPropVec3 m_hbox_extents;
	EDGUIPropFloat m_hbox_multiplier;
	EDGUIPropVec3 m_body_rotangles;
	EDGUIPropVec3 m_body_offset;
	EDGUIPropRsrc m_body_type;
	EDGUIPropVec3 m_body_size;
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
		m_recalc_thres( 96, 0, 255 )
	{
		m_recalc_thres.caption = "Threshold";
		m_btn_recalc_body.caption = "Recalculate body";
		m_btn_recalc_hitbox.caption = "Recalculate hitbox";
		m_btn_recalc_both.caption = "Recalculate both";
		
		m_group_recalc.Add( &m_recalc_thres );
		m_group_recalc.Add( &m_btn_recalc_body );
		m_group_recalc.Add( &m_btn_recalc_hitbox );
		m_group_recalc.Add( &m_btn_recalc_both );
		
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
			calc_char_bone_info( i, m_recalc_thres.m_value, mask );
	}
	
	EDGUIGroup m_group;
	Array< EDGUIButton > m_boneButtons;
	EDGUIGroup m_group_recalc;
	EDGUIPropInt m_recalc_thres;
	EDGUIButton m_btn_recalc_body;
	EDGUIButton m_btn_recalc_hitbox;
	EDGUIButton m_btn_recalc_both;
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
		m_name.caption = "Name";
		m_bone.caption = "Bone";
		m_rotangles.caption = "Rotation (angles)";
		m_offset.caption = "Offset";
		
		m_group.Add( &m_name );
		m_group.Add( &m_bone );
		m_group.Add( &m_rotangles );
		m_group.Add( &m_offset );
		
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
	int m_aid;
};


void FC_EditAttachment( int which );
struct EDGUIAttachmentListProps : EDGUILayoutRow
{
	EDGUIAttachmentListProps() :
		m_group( true, "Attachments" )
	{
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
		m_lid( -1 ),
		m_tid( -1 )
	{
		m_btnBack.caption = "Back to layer <which?>";
		m_bone.caption = "Bone";
		m_type.caption = "Type";
		m_offaxis.caption = "Offset/axis";
		m_angle.caption = "Angle";
		
		m_group.Add( &m_bone );
		m_group.Add( &m_type );
		m_group.Add( &m_offaxis );
		m_group.Add( &m_angle );
		
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
				g_AnimChar->OnRenderUpdate();
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


struct EDGUIMainFrame : EDGUIFrame, EDGUIRenderView::FrameInterface
{
	EDGUIMainFrame() :
		m_showBodies( true ),
		m_showHitboxes( true ),
		m_showAttachments( true ),
		m_showMasks( true ),
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
				GR2D_DrawTextLine( ", Hitboxes (2): " );
				YesNoText( m_showHitboxes );
				GR2D_DrawTextLine( ", Attachments (3): " );
				YesNoText( m_showAttachments );
				GR2D_DrawTextLine( ", Masks (4): " );
				YesNoText( m_showMasks );
				GR2D_DrawTextLine( "]" );
			}
			break;
		case EDGUI_EVENT_KEYDOWN:
			if( e->key.repeat == false && e->key.engkey == SDLK_1 )
				m_showBodies = !m_showBodies;
			if( e->key.repeat == false && e->key.engkey == SDLK_2 )
				m_showHitboxes = !m_showHitboxes;
			if( e->key.repeat == false && e->key.engkey == SDLK_3 )
				m_showAttachments = !m_showAttachments;
			if( e->key.repeat == false && e->key.engkey == SDLK_4 )
				m_showMasks = !m_showMasks;
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
								wm.TransformNormal( V3(0,0,1) ).Normalized(), size.z, 32 );
							break;
						case AnimCharacter::BodyType_Box:
							br.AABB( -size, size, wm );
							break;
						}
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
					
					float maskdata[ MAX_MESH_BONES ] = {0};
					
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
						if( g_AnimChar->m_cachedMeshInst->skin_matrices.size() )
							bmtx = g_AnimChar->m_cachedMeshInst->skin_matrices[ bone_id ] * bmtx;
						bmtx = g_AnimChar->m_cachedMesh->m_bones[ bone_id ].skinOffset * bmtx;
						
						Vec3 pos = bmtx.TransformPos( V3(0) );
						br.Col( 0.1f + ( 1 - maskdata[ bone_id ] ) * 0.8f, 0.1f + maskdata[ bone_id ] * 0.8f, 0.1f );
						br.Tick( pos, m_maskProps.m_previewSize.m_value );
					}
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
		g_AnimChar = new AnimCharacter;
		g_AnimChar->AddToScene( g_EdScene );
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
	bool m_showHitboxes;
	bool m_showAttachments;
	bool m_showMasks;
	
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
};

void FC_EditBone( int which ){ g_UIFrame->EditBone( which ); }
void FC_EditAttachment( int which ){ g_UIFrame->EditAttachment( which ); }
void FC_EditLayerTransform( int lid, int tid ){ g_UIFrame->EditLayerTransform( lid, tid ); }
void FC_EditLayer( int which ){ g_UIFrame->EditLayer( which ); }
void FC_EditMaskCmd( int mid, int cid ){ g_UIFrame->EditMaskCmd( mid, cid ); }
void FC_EditMask( int which ){ g_UIFrame->EditMask( which ); }




SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_SCREEN, RPF_ENABLED, 1, 0, 0, "ps_ss_fog" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 0, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 0, 0, "base" },
};

struct CSEditor : IGame
{
	bool OnInitialize()
	{
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
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
		g_EdScene = GR_CreateScene();
		g_EdScene->camera.position = V3(3);
		g_EdScene->camera.UpdateMatrices();
		g_EdScene->skyTexture = GR_GetTexture( "textures/sky/overcast1.dds" );
		g_AnimChar = new AnimCharacter;
		g_AnimMixLayers[ 0 ].anim = &g_AnimChar->m_layerAnimator;
		g_AnimMixLayers[ 0 ].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
		g_AnimChar->m_anMixer.layers = g_AnimMixLayers;
		g_AnimChar->m_anMixer.layerCount = sizeof(g_AnimMixLayers) / sizeof(g_AnimMixLayers[0]);
		g_AnimChar->AddToScene( g_EdScene );
		g_UIFrame = new EDGUIMainFrame();
		g_UIFrame->Resize( GR_GetWidth(), GR_GetHeight() );
		
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
		g_AnimChar = NULL;
		g_EdScene = NULL;
	}
	void OnEvent( const Event& e )
	{
		g_UIFrame->EngineEvent( &e );
	}
	void OnTick( float dt, uint32_t gametime )
	{
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		g_UIFrame->m_UIRenderView.UpdateCamera( dt );
		g_AnimChar->RecalcLayerState();
		g_AnimChar->FixedTick( dt );
		g_AnimChar->PreRender( 1 );
		g_UIFrame->Draw();
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}


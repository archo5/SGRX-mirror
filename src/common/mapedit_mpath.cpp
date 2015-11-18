

#include "mapedit.hpp"



EdObject* EdMeshPath::Clone()
{
	EdMeshPath* mpc = new EdMeshPath( *this );
	for( int i = 0; i < MAX_MESHPATH_PARTS; ++i )
	{
		mpc->m_parts[ i ].surface_id = 0;
	}
	return mpc;
}

bool EdMeshPath::RayIntersect( const Vec3& rpos, const Vec3& rdir, float outdst[1] ) const
{
	return RaySphereIntersect( rpos, rdir, m_position, 0.2f, outdst );
}

void EdMeshPath::RegenerateMesh()
{
}

Vec3 EdMeshPath::FindCenter() const
{
	Vec3 c = V3(0);
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		c += m_points[ i ].pos;
	}
	if( m_points.size() != 0 )
		c /= m_points.size();
	return c + m_position;
}

int EdMeshPath::GetOnlySelectedVertex() const
{
	int sel = -1;
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		if( m_points[ i ].sel )
		{
			if( sel == -1 )
				sel = i;
			else
				return -1;
		}
	}
	return sel;
}

void EdMeshPath::ScaleVertices( const Vec3& f )
{
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		m_points[ i ].pos *= f;
	}
}

void EdMeshPath::MoveSelectedVertices( const Vec3& t )
{
	for( size_t i = 0; i < m_points.size(); ++i )
	{
		if( m_points[ i ].sel )
		{
			m_points[ i ].pos += t;
		}
	}
}


EDGUIMeshPathPointProps::EDGUIMeshPathPointProps() :
	m_out( NULL ),
	m_pid( 0 ),
	m_pos( V3(0), 2, V3(-8192), V3(8192) ),
	m_smooth( false )
{
	tyname = "meshpathpointprops";
	m_group.caption = "Mesh path point properties";
	m_pos.caption = "Offset";
	m_smooth.caption = "Smooth?";
	
	m_group.Add( &m_pos );
	m_group.Add( &m_smooth );
	
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUIMeshPathPointProps::Prepare( EdMeshPath* mpath, int pid )
{
	m_out = mpath;
	m_pid = pid;
	
	char bfr[ 32 ];
	sgrx_snprintf( bfr, sizeof(bfr), "Point #%d", pid );
	m_group.caption = bfr;
	m_group.SetOpen( true );
	
	m_pos.SetValue( mpath->m_points[ m_pid ].pos );
	m_smooth.SetValue( mpath->m_points[ m_pid ].smooth );
}

int EDGUIMeshPathPointProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( m_out && ( e->target == &m_pos || e->target == &m_smooth ) )
		{
			EdMeshPathPoint& P = m_out->m_points[ m_pid ];
			if( e->target == &m_pos ) P.pos = m_pos.m_value;
			if( e->target == &m_smooth ) P.smooth = m_smooth.m_value;
			m_out->RegenerateMesh();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}


EDGUIMeshPathPartProps::EDGUIMeshPathPartProps() :
	m_out( NULL ),
	m_pid( 0 ),
	m_tex( g_UISurfMtlPicker, "null" ),
	m_off( V2(0), 2, V2(0), V2(1) ),
	m_scaleasp( V2(1), 2, V2(0.01f), V2(100) ),
	m_angle( 0, 1, 0, 360 )
{
	tyname = "meshpathpartprops";
	m_group.caption = "Part properties";
	m_tex.caption = "Texture";
	m_off.caption = "Offset";
	m_scaleasp.caption = "Scale/Aspect";
	m_angle.caption = "Angle";
	
	m_group.Add( &m_tex );
	m_group.Add( &m_off );
	m_group.Add( &m_scaleasp );
	m_group.Add( &m_angle );
	m_group.SetOpen( true );
	Add( &m_group );
}

void EDGUIMeshPathPartProps::Prepare( EdMeshPath* mpath, int pid )
{
	m_out = mpath;
	m_pid = pid;
	EdMeshPathPart& MP = mpath->m_parts[ pid ];
	
	char bfr[ 32 ];
	sgrx_snprintf( bfr, sizeof(bfr), "Layer #%d", pid );
	LoadParams( MP, bfr );
}

void EDGUIMeshPathPartProps::LoadParams( EdMeshPathPart& MP, const char* name )
{
	m_group.caption = name;
	m_group.SetOpen( true );
	
	m_tex.SetValue( MP.texname );
	m_off.SetValue( V2( MP.xoff, MP.yoff ) );
	m_scaleasp.SetValue( V2( MP.scale, MP.aspect ) );
	m_angle.SetValue( MP.angle );
}

void EDGUIMeshPathPartProps::BounceBack( EdMeshPathPart& MP )
{
	MP.texname = m_tex.m_value;
	MP.xoff = m_off.m_value.x;
	MP.yoff = m_off.m_value.y;
	MP.scale = m_scaleasp.m_value.x;
	MP.aspect = m_scaleasp.m_value.y;
	MP.angle = m_angle.m_value;
}

int EDGUIMeshPathPartProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( m_out && (
			e->target == &m_tex ||
			e->target == &m_off ||
			e->target == &m_scaleasp ||
			e->target == &m_angle
		) )
		{
			if( e->target == &m_tex )
			{
				m_out->m_parts[ m_pid ].texname = m_tex.m_value;
			}
			else if( e->target == &m_off )
			{
				m_out->m_parts[ m_pid ].xoff = m_off.m_value.x;
				m_out->m_parts[ m_pid ].yoff = m_off.m_value.y;
			}
			else if( e->target == &m_scaleasp )
			{
				m_out->m_parts[ m_pid ].scale = m_scaleasp.m_value.x;
				m_out->m_parts[ m_pid ].aspect = m_scaleasp.m_value.y;
			}
			else if( e->target == &m_angle )
			{
				m_out->m_parts[ m_pid ].angle = m_angle.m_value;
			}
			m_out->RegenerateMesh();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}


EDGUIMeshPathProps::EDGUIMeshPathProps() :
	m_out( NULL ),
	m_group( true, "Mesh path properties" ),
	m_meshName( g_UIMeshPicker ),
	m_pos( V3(0), 2, V3(-8192), V3(8192) ),
	m_blkGroup( NULL ),
	m_isSolid( false ),
	m_lmquality( 1, 2, 0.01f, 100.0f )
{
	tyname = "meshpartprops";
	m_meshName.caption = "Mesh";
	m_pos.caption = "Position";
	m_blkGroup.caption = "Group";
	m_isSolid.caption = "Is solid?";
	m_lmquality.caption = "Lightmap quality";
}

void EDGUIMeshPathProps::Prepare( EdMeshPath* mpath )
{
	m_out = mpath;
	m_blkGroup.m_rsrcPicker = &g_EdWorld->m_groupMgr.m_grpPicker;
	m_isSolid.SetValue( mpath->m_isSolid );
	
	Clear();
	
	Add( &m_group );
	m_blkGroup.SetValue( g_EdWorld->m_groupMgr.GetPath( mpath->group ) );
	
	m_group.Add( &m_meshName );
	m_group.Add( &m_pos );
	m_group.Add( &m_blkGroup );
	m_group.Add( &m_isSolid );
	m_group.Add( &m_lmquality );
	
	m_meshName.SetValue( mpath->m_meshName );
	m_pos.SetValue( mpath->m_position );
	m_lmquality.SetValue( mpath->m_lmquality );
	
	for( size_t i = 0; i < MAX_MESHPATH_PARTS; ++i )
	{
		m_partProps[ i ].Prepare( mpath, i );
		m_group.Add( &m_partProps[ i ] );
	}
}

int EDGUIMeshPathProps::OnEvent( EDGUIEvent* e )
{
	switch( e->type )
	{
	case EDGUI_EVENT_PROPEDIT:
		if( e->target == &m_pos || e->target == &m_blkGroup )
		{
			if( e->target == &m_pos )
			{
				m_out->m_position = m_pos.m_value;
			}
			else if( e->target == &m_blkGroup )
			{
				EdGroup* grp = g_EdWorld->m_groupMgr.FindGroupByPath( m_blkGroup.m_value );
				if( grp )
					m_out->group = grp->m_id;
			}
			m_out->RegenerateMesh();
		}
		else if( e->target == &m_meshName )
		{
			m_out->m_meshName = m_meshName.m_value;
			m_out->RegenerateMesh();
		}
		else if( e->target == &m_isSolid )
		{
			m_out->m_isSolid = m_isSolid.m_value;
		}
		else if( e->target == &m_lmquality )
		{
			m_out->m_lmquality = m_lmquality.m_value;
			m_out->RegenerateMesh();
		}
		break;
	}
	return EDGUILayoutRow::OnEvent( e );
}



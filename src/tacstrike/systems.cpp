

#define USE_HASHTABLE
#include "level.hpp"



Entity::Entity() : m_typeName("<unknown>")
{
}

Entity::~Entity()
{
	g_GameLevel->UnmapEntityByName( this );
}



void InfoEmissionSystem::UpdateEmitter( Entity* e, const Data& data )
{
	m_emissionData[ e ] = data;
}

void InfoEmissionSystem::RemoveEmitter( Entity* e )
{
	m_emissionData.unset( e );
}

bool InfoEmissionSystem::QueryAny( const Vec3& pos, float rad, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		float dst = rad + D.radius;
		if( ( D.pos - pos ).LengthSq() < dst * dst )
			return true;
	}
	return false;
}

bool InfoEmissionSystem::QueryBB( const Mat4& mtx, uint32_t types )
{
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		Vec3 tp = mtx.TransformPos( D.pos );
		if( tp.x >= -1 && tp.x <= 1 &&
			tp.y >= -1 && tp.y <= 1 &&
			tp.z >= -1 && tp.z <= 1 )
			return true;
	}
	return false;
}

Entity* InfoEmissionSystem::QueryOneRay( const Vec3& from, const Vec3& to, uint32_t types )
{
	Vec3 dir = to - from;
	float maxdist = dir.Length();
	dir /= maxdist;
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		
		float outdst[1];
		if( RaySphereIntersect( from, dir, D.pos, D.radius, outdst ) && outdst[0] < maxdist )
			return m_emissionData.item( i ).key;
	}
	return NULL;
}


MessagingSystem::MessagingSystem()
{
	m_tx_icon_info = GR_GetTexture( "ui/icon_info.png" );
	m_tx_icon_warning = GR_GetTexture( "ui/icon_warning.png" );
	m_tx_icon_cont = GR_GetTexture( "ui/icon_cont.png" );
}

void MessagingSystem::AddMessage( MSMessage::Type type, const StringView& sv, float tmlength )
{
	m_messages.push_back( MSMessage() );
	MSMessage& msg = m_messages.last();
	msg.type = type;
	msg.text = sv;
	msg.tmlength = tmlength;
	msg.position = 0;
}

void MessagingSystem::Tick( float dt )
{
	for( size_t i = m_messages.size(); i > 0; )
	{
		i--;
		MSMessage& msg = m_messages[ i ];
		msg.position += dt;
		if( msg.position > msg.tmlength + 2 )
		{
			m_messages.erase( i );
		}
	}
}

void MessagingSystem::DrawUI()
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "fonts/lato-regular.ttf", 16 );
	
	float y = 0;
	
	for( size_t i = 0; i < m_messages.size(); ++i )
	{
		MSMessage& msg = m_messages[ i ];
		float q = smoothlerp_range( msg.position, 0, 1, 1 + msg.tmlength, 2 + msg.tmlength );
		y += 10 - ( 1 - q ) * 50;
		
		br.Reset();
		br.Col( 0, 0.5f * q );
		br.Quad( 100, y, 450, y + 40 );
		br.Col( 1 );
		switch( msg.type )
		{
		case MSMessage::Info: br.SetTexture( m_tx_icon_info ); break;
		case MSMessage::Warning: br.SetTexture( m_tx_icon_warning ); break;
		case MSMessage::Continued:
		default:
			br.SetTexture( m_tx_icon_cont );
			break;
		}
		br.Quad( 110, y+10, 130, y+30 );
		br.UnsetTexture();
		br.Col( 1, 0.9f * q );
		GR2D_DrawTextLine( 150, y + 20, msg.text, HALIGN_LEFT, VALIGN_CENTER );
		
		y += 40;
	}
}


ObjectiveSystem::ObjectiveSystem() :
	m_alpha(0)
{
	m_tx_icon_open = GR_GetTexture( "ui/obj_open.png" );
	m_tx_icon_done = GR_GetTexture( "ui/obj_done.png" );
	m_tx_icon_failed = GR_GetTexture( "ui/obj_failed.png" );
}

int ObjectiveSystem::AddObjective( const StringView& sv, OSObjective::State state )
{
	int out = m_objectives.size();
	m_objectives.push_back( OSObjective() );
	OSObjective& obj = m_objectives.last();
	obj.text = sv;
	obj.state = state;
	return out;
}

void ObjectiveSystem::Tick( float dt )
{
	float tgt = SHOW_OBJECTIVES.value ? 1 : 0;
	float diff = tgt - m_alpha;
	m_alpha += sign( diff ) * TMIN( diff > 0 ? dt * 3 : dt, fabsf( diff ) );
}

void ObjectiveSystem::DrawUI()
{
	int W = GR_GetWidth();
	int H = GR_GetHeight();
	BatchRenderer& br = GR2D_GetBatchRenderer();
	GR2D_SetFont( "fonts/lato-regular.ttf", 16 );
	
	br.Reset();
	br.Col( 0, 0.5f * m_alpha );
	br.Quad( 0, 0, W, H );
	
	br.Col( 1, m_alpha );
	GR2D_DrawTextLine( 200, 10, "Objectives:" );
	
	float y = 40;
	
	for( size_t i = 0; i < m_objectives.size(); ++i )
	{
		OSObjective& obj = m_objectives[ i ];
		if( obj.state == OSObjective::Hidden )
			continue;
		
		switch( obj.state )
		{
		default:
		case OSObjective::Open: br.SetTexture( m_tx_icon_open ); break;
		case OSObjective::Done: br.SetTexture( m_tx_icon_done ); break;
		case OSObjective::Failed: br.SetTexture( m_tx_icon_failed ); break;
		}
		if( obj.state != OSObjective::Cancelled )
		{
			br.Quad( 210, y, 240, y + 30 );
		}
		
		GR2D_DrawTextLine( 250, y+15, obj.text, HALIGN_LEFT, VALIGN_CENTER );
		if( obj.state == OSObjective::Cancelled )
		{
			br.Reset();
			br.Col( 1, m_alpha );
			br.Quad( 250, y + 17, 250 + GR2D_GetTextLength( obj.text ), y + 19 );
		}
		y += 30;
	}
}


void DamageSystem::Init( SceneHandle scene )
{
	DecalMapPartInfo dmpi[] = { V4(0,0,1,1), V3(0.2f) };
	m_bulletDecalSys.Init( GR_GetTexture( "textures/fx/decals.png" ), GR_GetTexture( "textures/fx/projfalloff2.png" ), dmpi, 1 );
	m_bulletDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bulletDecalMesh = scene->CreateMeshInstance();
	m_bulletDecalMesh->decal = true;
	m_bulletDecalMesh->mesh = m_bulletDecalSys.m_mesh;
}

void DamageSystem::Free()
{
	m_bulletDecalSys.Free();
}

void DamageSystem::Tick( float deltaTime )
{
	UNUSED( deltaTime );
	m_bulletDecalSys.Upload();
}

void DamageSystem::AddBulletDamage( const StringView& type, SGRX_IMesh* targetMesh, int partID,
	const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale )
{
	// TODO handle type
	int decalID = 0;
	
	DecalProjectionInfo projInfo =
	{
		pos, dir, fabsf( Vec3Dot( dir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
		0, scale, 1, 0.5f, scale, 0.5f, false
	};
	if( partID < 0 )
		m_bulletDecalSys.AddDecal( decalID, targetMesh, worldMatrix, &projInfo );
	else
		m_bulletDecalSys.AddDecal( decalID, targetMesh, partID, worldMatrix, &projInfo );
}

void DamageSystem::Clear()
{
	m_bulletDecalSys.ClearAllDecals();
}


BulletSystem::BulletSystem( DamageSystem* dmgsys ) :
	m_damageSystem( dmgsys )
{
}

void BulletSystem::Tick( SGRX_Scene* scene, float deltaTime )
{
	for( size_t i = 0; i < m_bullets.size(); ++i )
	{
		Bullet& B = m_bullets[i];
		
		B.timeleft -= deltaTime;
		if( B.timeleft <= 0 )
		{
			m_bullets.erase( i-- );
			break;
		}
		Vec3 p1 = B.position;
		Vec3 p2 = p1 + B.velocity * deltaTime;
		
		SceneRaycastCallback_Sorting cb( &m_tmpStore );
		scene->RaycastAll( p1, p2, &cb, 0xffffffff );
		
		// sorted list of raycast hits
		bool remb = false;
		if( m_tmpStore.size() )
		{
			for( size_t hitid = 0; hitid < m_tmpStore.size(); ++hitid )
			{
				SceneRaycastInfo& HIT = m_tmpStore[ hitid ];
				float entryIfL0 = Vec3Dot( B.dir, HIT.normal );
				
				// apply damage to hit point
				Vec3 hitpoint = TLERP( p1, p2, HIT.factor );
				m_damageSystem->AddBulletDamage( "TODO",
					HIT.meshinst->mesh, -1, HIT.meshinst->matrix,
					hitpoint, B.dir, HIT.normal );
				
				// handling wall penetration
				B.numSolidRefs += entryIfL0 < 0 ? 1 : -1;
				if( B.numSolidRefs == 1 )
				{
					// entry into solid
					B.intersectStart = hitpoint;
				}
				else if( B.numSolidRefs == 0 )
				{
					// genuine exit, calculate penetration
					float dist = ( hitpoint - B.intersectStart ).Length();
					float q = dist * 50;
					float speedScale = q < 1 ? 1 : 1 / q;
					speedScale = ( speedScale - 1 ) * 1.15f + 1;
					if( speedScale < 0 )
					{
						remb = true;
						break;
					}
					B.velocity * speedScale;
				}
				else if( B.numSolidRefs < 0 )
				{
					// fake exit, abort all
					remb = true;
					break;
				}
			}
		}
		
		if( remb )
			m_bullets.erase( i-- );
		else
			B.position = p2;
	}
}

void BulletSystem::Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg };
	m_bullets.push_back( B );
}

void BulletSystem::Clear()
{
	m_bullets.clear();
}



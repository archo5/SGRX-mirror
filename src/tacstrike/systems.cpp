

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

bool InfoEmissionSystem::QuerySphereAny( const Vec3& pos, float rad, uint32_t types )
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

bool InfoEmissionSystem::QuerySphereAll( IESProcessor* proc, const Vec3& pos, float rad, uint32_t types )
{
	bool ret = false;
	for( size_t i = 0; i < m_emissionData.size(); ++i )
	{
		const Data& D = m_emissionData.item( i ).value;
		if( !( D.types & types ) )
			continue;
		
		float dst = rad + D.radius;
		if( ( D.pos - pos ).LengthSq() >= dst * dst )
			continue;
		
		ret = true;
		if( proc->Process( m_emissionData.item( i ).key, D ) == false )
			return true;
	}
	return ret;
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

int ObjectiveSystem::AddObjective(
	const StringView& title,
	OSObjective::State state,
	const StringView& desc,
	bool required,
	Vec3* location )
{
	int out = m_objectives.size();
	m_objectives.push_back( OSObjective() );
	OSObjective& obj = m_objectives.last();
	obj.title = title;
	obj.state = state;
	obj.desc = desc;
	obj.required = required;
	obj.hasLocation = location != NULL;
	obj.location = location ? *location : V3(0);
	return out;
}

OSObjStats ObjectiveSystem::GetStats()
{
	OSObjStats out = { m_objectives.size(), 0, 0, 0, 0, 0 };
	for( size_t i = 0; i < m_objectives.size(); ++i )
	{
		if( m_objectives[ i ].state == OSObjective::Hidden ) out.numHidden++;
		if( m_objectives[ i ].state == OSObjective::Open ) out.numOpen++;
		if( m_objectives[ i ].state == OSObjective::Done ) out.numDone++;
		if( m_objectives[ i ].state == OSObjective::Failed ) out.numFailed++;
		if( m_objectives[ i ].state == OSObjective::Cancelled ) out.numCancelled++;
	}
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
		
		GR2D_DrawTextLine( 250, y+15, obj.title, HALIGN_LEFT, VALIGN_CENTER );
		if( obj.state == OSObjective::Cancelled )
		{
			br.Reset();
			br.Col( 1, m_alpha );
			br.Quad( 250, y + 17, 250 + GR2D_GetTextLength( obj.title ), y + 19 );
		}
		y += 30;
	}
}


FlareSystem::FlareSystem()
{
	m_ps_flare = GR_GetPixelShader( "flare" );
	m_tex_flare = GR_GetTexture( "textures/fx/flare.png" );
}

void FlareSystem::Clear()
{
	m_flares.clear();
}

void FlareSystem::UpdateFlare( void* handle, const FSFlare& flare )
{
	m_flares.set( handle, flare );
}

bool FlareSystem::RemoveFlare( void* handle )
{
	return m_flares.unset( handle );
}

void FlareSystem::Draw( SGRX_Camera& cam )
{
	GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
	
	float W = GR_GetWidth();
	float H = GR_GetHeight();
	float sz = TMIN( W, H ) * 0.2f;
	BatchRenderer& br = GR2D_GetBatchRenderer().Reset().SetShader( m_ps_flare ).SetTexture( m_tex_flare );
	br.ShaderData.push_back( V4( W, H, 1.0f / W, 1.0f / H ) );
	br.ShaderData.push_back( V4(1) );
	for( size_t i = 0; i < m_flares.size(); ++i )
	{
		FSFlare& FD = m_flares.item( i ).value;
		if( FD.enabled == false || FD.size <= 0 )
			continue;
		br.ShaderData[1] = V4( FD.color, 0.1f / ( ( FD.pos - cam.position ).Length() + 1 ) );
		Vec3 screenpos = cam.WorldToScreen( FD.pos );
		if( Vec3Dot( FD.pos, cam.direction ) < Vec3Dot( cam.position, cam.direction ) )
			continue;
		SceneRaycastCallback_Any srcb;
		g_GameLevel->m_scene->RaycastAll( cam.position, FD.pos, &srcb );
		if( srcb.m_hit )
			continue;
	//	LOG << screenpos.z;
		float dx = cos(0.1f)*0.5f*sz * FD.size;
		float dy = sin(0.1f)*0.5f*sz * FD.size;
		br.TurnedBox( screenpos.x * W, screenpos.y * H, dx, dy );
		br.Flush();
	}
}


const char* DamageSystem::Init( SceneHandle scene, SGRX_LightSampler* sampler )
{
	static char errbfr[ 350 ];
	
	String mtlconfig;
	if( FS_LoadTextFile( "data/damage.dat", mtlconfig ) == false )
		return( "Failed to load data/damage.dat" );
	
	// defaults
	String decal_base_tex = "textures/fx/decals.png";
	String decal_falloff_tex = "textures/fx/projfalloff2.png";
	MtlHandle cur_mtl;
	
	ConfigReader cfgrd( mtlconfig );
	StringView key, value;
	while( cfgrd.Read( key, value ) )
	{
		if( key == "decal_base_tex" ){ decal_base_tex = value; continue; }
		if( key == "decal_falloff_tex" ){ decal_falloff_tex = value; continue; }
		if( key == "material" )
		{
			cur_mtl = new Material;
			cur_mtl->match = value;
			m_bulletDecalMaterials.push_back( cur_mtl );
			continue;
		}
		if( key.part( 0, 4 ) == "mtl_" )
		{
			// error handling
			if( cur_mtl == NULL )
				return "mtl_ command has no material";
		}
		if( key == "mtl_decal" )
		{
			int id = m_bulletDecalInfo.size();
			bool suc = false;
			Vec4 tex_aabb = String_ParseVec4( value.until( "|" ), &suc );
			if( suc == false )
				return "mtl_decal - cannot parse coord rect";
			
			float decal_size = String_ParseFloat( value.after( "|" ), &suc );
			if( suc == false )
				return "mtl_decal - cannot parse size";
			
			DecalMapPartInfo dmpi = { tex_aabb, V3(decal_size) };
			m_bulletDecalInfo.push_back( dmpi );
			cur_mtl->decalIDs.push_back( id );
			
			continue;
		}
		if( key == "mtl_particles" )
		{
			if( cur_mtl->particles.Load( value ) == false )
			{
				sgrx_snprintf( errbfr, 350, "Failed to load particle system '%.*s' while parsing "
					"data/damage.dat", TMIN( 250, (int) value.size() ), value.data() );
				return( errbfr );
			}
			cur_mtl->particles.m_lightSampler = sampler;
			cur_mtl->particles.AddToScene( scene );
			continue;
		}
		if( key == "mtl_sound" )
		{
			cur_mtl->sound = value;
			// TODO validate
		}
	}
	
	m_bulletDecalSys.m_lightSampler = sampler;
	m_bulletDecalSys.Init(
		GR_GetTexture( decal_base_tex ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bulletDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bulletDecalMesh = scene->CreateMeshInstance();
	m_bulletDecalMesh->decal = true;
	m_bulletDecalMesh->sortidx = 202;
	m_bulletDecalMesh->mesh = m_bulletDecalSys.m_mesh;
	m_bulletDecalMesh->textures[0] = GR_GetTexture( "textures/white.png" );
	
	m_bloodDecalSys.m_lightSampler = sampler;
	m_bloodDecalSys.Init(
		GR_GetTexture( "textures/particles/blood.png" ),
		GR_GetTexture( decal_falloff_tex ) );
	m_bloodDecalSys.SetSize( 48 * 1024 * 10 ); // random size
	m_bloodDecalMesh = scene->CreateMeshInstance();
	m_bloodDecalMesh->decal = true;
	m_bloodDecalMesh->sortidx = 201;
	m_bloodDecalMesh->mesh = m_bloodDecalSys.m_mesh;
	m_bloodDecalMesh->textures[0] = GR_GetTexture( "textures/white.png" );
	
	LOG << LOG_DATE << "  Damage system initialized successfully";
	return NULL;
}

void DamageSystem::Free()
{
	m_bulletDecalSys.Free();
	m_bulletDecalMesh = NULL;
	m_bloodDecalSys.Free();
	m_bloodDecalMesh = NULL;
}

void DamageSystem::Tick( float deltaTime )
{
	UNUSED( deltaTime );
	m_bulletDecalSys.Upload();
	m_bloodDecalSys.Upload();
	for( size_t i = 0; i < m_bulletDecalMaterials.size(); ++i )
	{
		Material* mtl = m_bulletDecalMaterials[ i ];
		mtl->particles.Tick( deltaTime );
		mtl->particles.PreRender();
	}
}

void DamageSystem::AddBulletDamage( SGRX_DecalSystem* dmgDecalSysOverride,
	const StringView& type, SGRX_IMesh* targetMesh, int partID,
	const Mat4& worldMatrix, const Vec3& pos, const Vec3& dir, const Vec3& nrm, float scale )
{
	int decalID = -1;
	for( size_t i = 0; i < m_bulletDecalMaterials.size(); ++i )
	{
		Material* mtl = m_bulletDecalMaterials[ i ];
		if( mtl->CheckMatch( type ) )
		{
			// decal
			if( mtl->decalIDs.size() )
				decalID = mtl->decalIDs[ rand() % mtl->decalIDs.size() ];
			
			// particles
			Mat4 tf = Mat4::CreateRotationZ( randf() * M_PI * 2 )
				* Mat4::CreateRotationBetweenVectors( V3(0,0,1), nrm )
				* Mat4::CreateTranslation( pos );
			mtl->particles.SetTransform( tf );
			mtl->particles.Trigger();
			
			// sound
			// TODO
			
			break;
		}
	}
	
	if( decalID != -1 && targetMesh )
	{
		DecalProjectionInfo projInfo =
		{
			pos, dir, fabsf( Vec3Dot( dir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
			0, scale, 1, 0.5f, scale, 0.5f, false,
			m_bulletDecalInfo[ decalID ]
		};
		if( dmgDecalSysOverride == NULL )
			dmgDecalSysOverride = &m_bulletDecalSys;
		if( partID < 0 )
			dmgDecalSysOverride->AddDecal( projInfo, targetMesh, worldMatrix );
		else
			dmgDecalSysOverride->AddDecal( projInfo, targetMesh, partID, worldMatrix );
	}
}

struct DmgSys_GenBlood : IProcessor
{
	void Process( void* data )
	{
		SGRX_CAST( SGRX_MeshInstance*, MI, data );
		if( MI->mesh == NULL ||
			MI->raycastOverride ||
			MI->skin_matrices.size() ||
			MI->decal )
			return;
		DS->m_bloodDecalSys.AddDecal( projInfo, MI->mesh, MI->matrix );
	}
	
	DamageSystem* DS;
	DecalProjectionInfo projInfo;
};

void DamageSystem::AddBlood( Vec3 pos, Vec3 dir )
{
	int decalID = 0;
	if( decalID != -1 )
	{
		DecalProjectionInfo projInfo =
		{
			pos, dir, fabsf( Vec3Dot( dir, V3(0,0,1) ) ) > 0.99f ? V3(0,1,0) : V3(0,0,1),
			45.0f, 1, 1, 0.5f, 1, 0, true,
			{ V4(0,0,1,1), V3(1,1,2) }
		};
		SGRX_Camera cam;
		m_bloodDecalSys.GenerateCamera( projInfo, &cam );
		DmgSys_GenBlood gb;
		{
			gb.DS = this;
			gb.projInfo = projInfo;
		}
		m_bloodDecalMesh->_scene->GatherMeshes( cam, &gb );
	}
}

void DamageSystem::Clear()
{
	m_bulletDecalSys.ClearAllDecals();
	m_bloodDecalSys.ClearAllDecals();
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
		#ifdef TSGAME
		g_DebugLines.DrawLine( p1, p2, m_tmpStore.size() ? COLOR_RGB(255,0,0) : COLOR_RGB(255,255,0) );
		#endif
		
		// sorted list of raycast hits
		bool remb = false;
		if( m_tmpStore.size() )
		{
			for( size_t hitid = 0; hitid < m_tmpStore.size(); ++hitid )
			{
				SceneRaycastInfo& HIT = m_tmpStore[ hitid ];
				if( HIT.meshinst->enabled == false )
					continue;
				
				float entryIfL0 = Vec3Dot( B.dir, HIT.normal );
				MeshInstInfo* mii = (MeshInstInfo*) HIT.meshinst->userData;
				if( mii && mii->ownerType == B.ownerType )
					continue;
				
				StringView decalType = "TODO";
				if( mii && mii->typeOverride )
					decalType = mii->typeOverride;
				
				// apply damage to hit point
				Vec3 hitpoint = TLERP( p1, p2, HIT.factor );
				SGRX_DecalSystem* dmgDecalSys = mii ? mii->dmgDecalSysOverride : NULL;
				m_damageSystem->AddBulletDamage( dmgDecalSys, decalType,
					HIT.meshinst->skin_matrices.size() ? NULL : HIT.meshinst->mesh,
					-1, HIT.meshinst->matrix, hitpoint, B.dir, HIT.normal );
				
				// blood?
				if( decalType == "*human*" )
				{
					m_damageSystem->AddBlood( hitpoint, B.dir );
				}
				
				// send event
				if( mii )
				{
					mii->OnEvent( HIT.meshinst, MIEVT_BulletHit, B.velocity.Length() );
				}
				
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

void BulletSystem::Add( const Vec3& pos, const Vec3& vel, float timeleft, float dmg, uint32_t ownerType )
{
	Bullet B = { pos, vel, vel.Normalized(), timeleft, dmg, ownerType };
	m_bullets.push_back( B );
}

void BulletSystem::Clear()
{
	m_bullets.clear();
}



bool AIDBSystem::CanHearSound( Vec3 pos, int i )
{
	AISound& S = m_sounds[ i ];
	return ( pos - S.position ).Length() < S.radius;
}

void AIDBSystem::Load( ByteArray& data )
{
	m_pathfinder.Load( data );
}

void AIDBSystem::AddSound( Vec3 pos, float rad, float timeout, AISoundType type )
{
	AISound S = { pos, rad, timeout, type };
	m_sounds.push_back( S );
}

void AIDBSystem::Tick( float deltaTime )
{
	for( size_t i = 0; i < m_sounds.size(); ++i )
	{
		AISound& S = m_sounds[ i ];
		S.timeout -= deltaTime;
		if( S.timeout <= 0 )
		{
			m_sounds.uerase( i-- );
		}
	}
}



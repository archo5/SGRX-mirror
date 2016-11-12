

#include "particlesys.hpp"



SGRX_ScenePSRaycast::SGRX_ScenePSRaycast() : layers(0xffffffff)
{
}

void SGRX_ScenePSRaycast::Raycast( DATA_IN* rays, DATA_OUT* isects, size_t count )
{
	SceneRaycastInfo info;
	for( size_t i = 0; i < count; ++i )
	{
		DATA_IN& R = rays[ i ];
		DATA_OUT& IS = isects[ i ];
		if( scene->RaycastOne( R.p1, R.p2, &info, layers ) )
		{
			IS.factor = info.factor;
			IS.normal = info.normal;
			DoFX( TLERP( R.p1, R.p2, info.factor ), info.normal, R.isect_fx );
		}
		else
			IS.factor = -1;
	}
}

void SGRX_ScenePSRaycast::DoFX( const Vec3& pos, const Vec3& nrm, uint32_t fx )
{
}


void ParticleSystem::Emitter::Tick( ParticleSystem* PS, float dt )
{
	if( state_SpawnCurrCount < state_SpawnTotalCount )
	{
		state_SpawnCurrTime = clamp( state_SpawnCurrTime + dt, 0, state_SpawnTotalTime );
		int currcount = state_SpawnCurrTime / state_SpawnTotalTime * state_SpawnTotalCount;
		if( state_SpawnCurrCount < currcount )
		{
			Generate( PS, currcount - state_SpawnCurrCount );
			state_SpawnCurrCount = currcount;
		}
	}
	
	int isectlim = isect_Limit;
	
	Vec3 accel = PS->gravity * tick_GravityMult; // ??? * PS->globalScale;
	for( size_t i = 0; i < particles_Position.size(); ++i )
	{
		Vec2& LT = particles_Lifetime[ i ];
		LT.x += LT.y * dt;
		if( LT.x >= 1 )
		{
particle_remove:
			// remove particle
			particles_Position.uerase( i );
			particles_Velocity.uerase( i );
			particles_Lifetime.uerase( i );
			particles_RandSizeAngVel.uerase( i );
			particles_RandColor.uerase( i );
			particles_Group.uerase( i );
			i--;
			continue;
		}
		Vec3& P = particles_Position[ i ];
		Vec3& V = particles_Velocity[ i ];
		V += accel * dt;
		
		if( isectlim > 0 )
		{
			Vec3 P1 = P + V * dt;
			
			// TODO optimize
			SGRX_ScenePSRaycast rcsys;
			rcsys.scene = PS->m_scene;
			SGRX_IPSRaycast* PSRC = PS->m_psRaycast ? PS->m_psRaycast : &rcsys;
			
			uint32_t fxid = isect_FXChance >= randf() ? isect_FXID : 0;
			SGRX_IPSRaycast::DATA_IN din = { P, P1, fxid };
			SGRX_IPSRaycast::DATA_OUT dout;
			PSRC->Raycast( &din, &dout, 1 );
			if( dout.factor <= 0 )
				P = P1;
			else
			{
				if( isect_Remove )
					goto particle_remove;
				
				Vec3 NV = Vec3Dot( V, dout.normal ) * dout.normal;
				Vec3 TV = V - NV;
				V = -NV * isect_Bounce + TV * ( 1 - isect_Friction );
				P = TLERP( P, P1, dout.factor ) + V * dt * ( 1 - dout.factor );
			}
			isectlim--;
		}
		else
		{
			P += V * dt;
		}
	}
	
	state_lastDelta = dt;
}


static FINLINE Vec3 _ps_rotate( const Vec3& v, const Vec3& axis, float angle )
{
	// http://en.wikipedia.org/wiki/Axis–angle_representation#Rotating_a_vector
	
	float cos_a = cosf( angle );
	float sin_a = sinf( angle );
	Vec3 cross = Vec3Cross( axis, v );
	float dot = Vec3Dot( axis, v );
	
	return cos_a * v + sin_a * cross + ( 1 - cos_a ) * dot * axis;
}

static FINLINE Vec3 _ps_diverge( const Vec3& dir, float dvg )
{
	float baseangle = randf() * M_PI * 2;
	float rotangle = randf() * M_PI * dvg;
	Vec3 axis = { cosf( baseangle ), sinf( baseangle ), 0 };
	
	return _ps_rotate( dir, axis, rotangle );
}

static FINLINE int randi( int x )
{
	if( !x )
		return 0;
	return rand() % x;
}


void ParticleSystem::Emitter::Generate( ParticleSystem* PS, int count )
{
	if( count == 0 )
		return;
	
	Vec3 velMicroDir = create_VelMicroDir.Normalized();
	Vec3 velMacroDir = create_VelMacroDir.Normalized();
	
	int clusterleft = 0;
	float clusterdist = 0;
	Vec3 clusteraxis = {0,0,0};
	float clusterangle = 0;
	
	int allocpos = 0;
	
	for( int i = 0; i < count; ++i )
	{
		if( clusterleft <= 0 )
		{
			clusterleft = create_VelCluster + randi( create_VelClusterExt );
			clusterdist = create_VelMacroDistExt.x + create_VelMacroDistExt.y * randf();
			Vec3 clusterdir = _ps_diverge( velMacroDir, create_VelMacroDvg );
			clusteraxis = Vec3Cross( V3(0,0,1), clusterdir ).Normalized();
			clusterangle = acos( clamp( Vec3Dot( V3(0,0,1), clusterdir ), -1, 1 ) );
		}
		clusterleft--;
		
		// Lifetime
		float LT = create_LifetimeExt.x + create_LifetimeExt.y * randf();
		if( LT <= 0 )
			continue;
		Vec2 LTV = { 0, 1.0f / LT };
		
		// Position
		Vec3 P = create_Pos;
		P += V3( create_PosBox.x * randf11(), create_PosBox.y * randf11(), create_PosBox.z * randf11() );
		float pos_ang = randf() * M_PI * 2, pos_zang = randf() * M_PI, pos_len = randf() * create_PosRadius;
		float cos_zang = cos( pos_zang ), sin_zang = sin( pos_zang );
		P += V3( cos( pos_ang ) * sin_zang, sin( pos_ang ) * sin_zang, cos_zang ) * pos_len;
		
		// Velocity
		Vec3 V = _ps_diverge( velMicroDir, create_VelMicroDvg );
		V = _ps_rotate( V, clusteraxis, clusterangle );
		V *= clusterdist + create_VelMicroDistExt.x + create_VelMicroDistExt.y * randf();
		
		P *= PS->globalScale;
		V *= PS->globalScale;
		
		// absolute positioning
		if( absolute )
		{
			P = PS->m_transform.TransformPos( P );
		//	V /= PS->m_transform.GetScale();
			V = PS->m_transform.TransformNormal( V );
		}
		
		// size, angle
		Vec3 randSAV =
		{
			randf(),
			create_AngleDirDvg.x + create_AngleDirDvg.y * randf11(),
			create_AngleVelDvg.x + create_AngleVelDvg.y * randf11()
		};
		// color [HSV], opacity
		Vec4 randHSVO = { randf(), randf(), randf(), randf() };
		
		if( particles_Position.size() < (size_t) spawn_MaxCount )
		{
			particles_Position.push_back( P );
			particles_Velocity.push_back( V );
			particles_Lifetime.push_back( LTV );
			particles_RandSizeAngVel.push_back( randSAV );
			particles_RandColor.push_back( randHSVO );
			particles_Group.push_back( PS->m_nextGroup );
		}
		else
		{
			int i = ( allocpos++ ) % spawn_MaxCount;
			particles_Position[ i ] = P;
			particles_Velocity[ i ] = V;
			particles_Lifetime[ i ] = LTV;
			particles_RandSizeAngVel[ i ] = randSAV;
			particles_RandColor[ i ] = randHSVO;
			particles_Group[ i ] = PS->m_nextGroup;
		}
	}
}

void ParticleSystem::Emitter::Trigger( ParticleSystem* PS )
{
	state_SpawnTotalCount = spawn_Count + randi( spawn_CountExt );
	state_SpawnCurrCount = 0;
	state_SpawnTotalTime = spawn_TimeExt.x + spawn_TimeExt.y * randf();
	state_SpawnCurrTime = 0;
	if( state_SpawnTotalTime == 0 )
	{
		Generate( PS, state_SpawnTotalCount );
		state_SpawnCurrCount += state_SpawnTotalCount;
	}
}


void ParticleSystem::Emitter::PreRender( ParticleSystem* PS, ps_prerender_info& info )
{
	const Vec3 S_X = info.basis[0];
	const Vec3 S_Y = info.basis[1];
	const Vec3 S_Z = info.basis[2];
	
	size_t bv = PS->m_vertices.size();
	
	for( size_t i = 0; i < particles_Position.size(); ++i )
	{
		// step 1: extract data
		Vec3 POS = particles_Position[ i ];
		Vec3 VEL = particles_Velocity[ i ];
		Vec2 LFT = particles_Lifetime[ i ];
		Vec3 SAV = particles_RandSizeAngVel[ i ];
		Vec4 RCO = particles_RandColor[ i ];
		uint16_t GRP = particles_Group[ i ];
		
		if( absolute == false )
		{
			POS = info.transform.TransformPos( POS );
			VEL = info.transform.TransformNormal( VEL );
		}
		
		// step 2: fill in the missing data
		float q = LFT.x;
		float t = LFT.x / LFT.y;
		float ANG = SAV.y + ( SAV.z + tick_AngleAcc * t ) * t;
		float SIZ = curve_Size.GetValue( q, SAV.x ) * PS->globalScale;
		Vec4 COL = V4
		(
			HSV(
				V3(
					curve_ColorHue.GetValue( q, RCO.x ),
					curve_ColorSat.GetValue( q, RCO.y ),
					curve_ColorVal.GetValue( q, RCO.z ) * 2
				)
			),
			curve_Opacity.GetValue( q, RCO.w )
		);
		
		if( render_Additive == false )
		{
			Vec3 color = PS->m_groups[ GRP ].color * 0.5f;
			COL.x *= color.x;
			COL.y *= color.y;
			COL.z *= color.z;
		}
		
		// step 3: generate vertex/index data
		if( render_Stretch )
		{
			uint16_t cv = PS->m_vertices.size() - bv;
			uint16_t idcs[18] =
			{
				cv+0, cv+2, cv+3, cv+3, cv+1, cv+0,
				cv+2, cv+4, cv+5, cv+5, cv+3, cv+2,
				cv+4, cv+6, cv+7, cv+7, cv+5, cv+4,
			};
			PS->m_indices.append( idcs, 18 );
			
			Vec3 P_2 = POS - VEL * state_lastDelta;
			
			Vec2 proj1 = info.viewProj.TransformPos( POS ).ToVec2();
			Vec2 proj2 = info.viewProj.TransformPos( P_2 ).ToVec2();
			Vec2 dir2d = ( proj2 - proj1 ).Normalized();
			
			// invert Y
			Vec3 D_X = ( S_X * dir2d.x - S_Y * dir2d.y ).Normalized();
			Vec3 D_Y = Vec3Cross( S_Z, D_X ).Normalized();
			
			Vertex verts[8] =
			{
				{ P_2 + ( -D_X -D_Y ) * SIZ, COL,   0,   0, 0 },
				{ P_2 + ( -D_X +D_Y ) * SIZ, COL,   0, 255, 0 },
				{ P_2 + (      -D_Y ) * SIZ, COL, 127,   0, 0 },
				{ P_2 + (      +D_Y ) * SIZ, COL, 127, 255, 0 },
				{ POS + (      -D_Y ) * SIZ, COL, 127,   0, 0 },
				{ POS + (      +D_Y ) * SIZ, COL, 127, 255, 0 },
				{ POS + ( +D_X -D_Y ) * SIZ, COL, 255,   0, 0 },
				{ POS + ( +D_X +D_Y ) * SIZ, COL, 255, 255, 0 },
			};
			PS->m_vertices.append( verts, 8 );
		}
		else
		{
			uint16_t cv = PS->m_vertices.size() - bv;
			uint16_t idcs[6] = { cv, cv+1, cv+2, cv+2, cv+3, cv };
			PS->m_indices.append( idcs, 6 );
			
			float ang = M_PI + ANG;
			Vec3 RSX = _ps_rotate( S_X, S_Z, ang );
			Vec3 RSY = _ps_rotate( S_Y, S_Z, ang );
			
			Vertex verts[4] =
			{
				{ POS + ( -RSX -RSY ) * SIZ, COL,   0,   0, 0 },
				{ POS + ( +RSX -RSY ) * SIZ, COL, 255,   0, 0 },
				{ POS + ( +RSX +RSY ) * SIZ, COL, 255, 255, 0 },
				{ POS + ( -RSX +RSY ) * SIZ, COL,   0, 255, 0 },
			};
			PS->m_vertices.append( verts, 4 );
		}
	}
}


ParticleSystem::ParticleSystem() :
	gravity(V3(0,0,-10)), maxGroupCount(10), globalScale(1),
	looping(true), retriggerTimeExt(V2(1,0.1f)),
	m_isPlaying(false), m_retriggerTime(0), m_nextGroup(0),
	m_transform(Mat4::Identity), m_lightSampler(NULL), m_psRaycast(NULL)
{
	RegisterHandler( EID_ScenePreRender );
}

bool ParticleSystem::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( ba );
	Serialize( br, false );
	return !br.error;
}

bool ParticleSystem::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw, false );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void ParticleSystem::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_ScenePreRender )
	{
		SGRX_CAST( SGRX_Scene*, scene, edata.GetUserData() );
		if( scene == m_scene )
		{
			_PreRender();
		}
	}
}

void ParticleSystem::OnRenderUpdate()
{
	if( !m_scene )
		return;
	
	if( !m_vdecl )
		m_vdecl = GR_GetVertexDecl( PARTICLE_VDECL );
	
	m_meshInsts.resize( emitters.size() );
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		Emitter& E = emitters[ i ];
		
		if( !m_meshInsts[ i ] )
			m_meshInsts[ i ] = m_scene->CreateMeshInstance();
		if( !m_meshInsts[ i ]->GetMesh() )
			m_meshInsts[ i ]->SetMesh( GR_CreateMesh() );
		
	//	m_meshInsts[ i ]->matrix = Mat4::Identity; // E.absolute ? Mat4::Identity : m_transform;
		SGRX_Material mtl;
		mtl.blendMode = E.render_Additive ? SGRX_MtlBlend_Additive : SGRX_MtlBlend_Basic;
		mtl.flags = E.render_Additive * SGRX_MtlFlag_Unlit;
		for( int t = 0; t < NUM_PARTICLE_TEXTURES; ++t )
			mtl.textures[ t ] = E.render_Textures[ t ];
		mtl.shader = String_Concat( E.render_Shader, ":PARTICLE" );
		m_meshInsts[ i ]->materials.assign( &mtl, 1 );
		m_meshInsts[ i ]->OnUpdate();
		
		SGRX_MeshPart MP = { 0, 0, 0, 0 };
		m_meshInsts[ i ]->GetMesh()->SetPartData( &MP, 1 );
	}
}

void ParticleSystem::AddToScene( SceneHandle sh )
{
	if( !m_vdecl )
		m_vdecl = GR_GetVertexDecl( PARTICLE_VDECL );
	
	m_scene = sh;
	
	OnRenderUpdate();
}

void ParticleSystem::SetTransform( const Mat4& mtx )
{
	m_transform = mtx;
}

bool ParticleSystem::Tick( float dt )
{
	bool retval = false;
	
	float prevrt = m_retriggerTime;
	m_retriggerTime -= dt;
	if( m_retriggerTime <= 0 && prevrt > 0 )
	{
		Trigger();
		m_retriggerTime = looping ? ( retriggerTimeExt.x + retriggerTimeExt.y * randf() ) : 0;
		retval = true;
	}
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		emitters[ i ].Tick( this, dt );
	}
	return retval;
}

void ParticleSystem::_PreRender()
{
	if( !m_scene )
		return;
	
	const Mat4& invmtx = m_scene->camera.mInvView;
	ps_prerender_info info =
	{
		m_scene,
		m_transform,
		m_scene->camera.mView * m_scene->camera.mProj,
		{
			invmtx.TransformNormal( V3(1,0,0) ),
			invmtx.TransformNormal( V3(0,1,0) ),
			invmtx.TransformNormal( V3(0,0,1) )
		}
	};
	
	if( m_meshInsts.size() != emitters.size() )
		OnRenderUpdate();
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		MeshHandle mesh = m_meshInsts[ i ]->GetMesh();
		
		m_vertices.clear();
		m_indices.clear();
		
		SGRX_MeshPart& MP = mesh->m_meshParts[ 0 ];
		MP.vertexOffset = 0;
		MP.indexOffset = 0;
		
		emitters[ i ].PreRender( this, info );
		
		MP.vertexCount = m_vertices.size();
		MP.indexCount = m_indices.size();
		
		if( m_vertices.size() )
		{
			if( m_vertices.size_bytes() > mesh->m_vertexDataSize )
				mesh->SetVertexData( m_vertices.data(), m_vertices.size_bytes(), m_vdecl );
			else
				mesh->UpdateVertexData( m_vertices.data(), m_vertices.size_bytes() );
			mesh->SetAABBFromVertexData( m_vertices.data(), m_vertices.size_bytes(), m_vdecl );
		}
		
		if( m_indices.size() )
		{
			if( m_indices.size_bytes() > mesh->m_indexDataSize )
				mesh->SetIndexData( m_indices.data(), m_indices.size_bytes(), false );
			else
				mesh->UpdateIndexData( m_indices.data(), m_indices.size_bytes() );
		}
	}
}

void ParticleSystem::Trigger()
{
	if( m_groups.size() < (size_t) maxGroupCount )
		m_groups.resize( maxGroupCount );
	m_nextGroup++;
	m_nextGroup %= maxGroupCount;
	// TODO: pick more accurate position
	( m_lightSampler ? m_lightSampler : &GR_GetDummyLightSampler() )->SampleLight(
		m_transform.TransformPos( V3(0) ), m_groups[ m_nextGroup ].color );
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		emitters[ i ].Trigger( this );
	}
}

void ParticleSystem::Play()
{
	if( !m_isPlaying )
	{
		m_isPlaying = true;
		m_retriggerTime = 0.00001f;
	}
}

void ParticleSystem::Stop()
{
	if( m_isPlaying )
	{
		m_isPlaying = false;
		m_retriggerTime = 0;
	}
}


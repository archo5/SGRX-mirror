

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#include "level.hpp"


GameLevel* g_GameLevel = NULL;
bool g_Paused = false;
PhyWorldHandle g_PhyWorld;
SoundSystemHandle g_SoundSys;

Command MOVE_LEFT( "move_left" );
Command MOVE_RIGHT( "move_right" );
Command MOVE_UP( "move_up" );
Command MOVE_DOWN( "move_down" );
Command SHOOT( "shoot" );
Command RELOAD( "reload" );
Command SLOW_WALK( "slow_walk" );
Command SPRINT( "sprint" );
Command CROUCH( "crouch" );
Command SHOW_OBJECTIVES( "show_objectives" );


SGRX_RenderPass g_RenderPasses_Main[] =
{
	{ RPT_SHADOWS, RPF_ENABLED, 1, 0, 0, "shadow" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_SOLID | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 2, "ext_s4" },
	{ RPT_OBJECT, RPF_DECALS | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_PROJECTORS, RPF_ENABLED, 1, 0, 0, "projector" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_STATIC | RPF_ENABLED, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_OBJ_DYNAMIC | RPF_ENABLED | RPF_CALC_DIRAMB, 1, 4, 0, "base" },
	{ RPT_OBJECT, RPF_MTL_TRANSPARENT | RPF_LIGHTOVERLAY | RPF_ENABLED, 100, 0, 2, "ext_s4" },
};



void Game_SetPaused( bool paused )
{
	g_Paused = paused;
}

bool Game_IsPaused()
{
	return g_Paused;
}


struct SplashScreen : IScreen
{
	float m_timer;
	TextureHandle m_tx_crage;
	TextureHandle m_tx_back;
	
	SplashScreen() : m_timer(0)
	{
	}
	
	void OnStart()
	{
		m_timer = 0;
		m_tx_crage = GR_GetTexture( "ui/crage_logo.png" );
		m_tx_back = GR_GetTexture( "ui/flare_bg.png" );
	}
	void OnEnd()
	{
		m_tx_crage = NULL;
		m_tx_back = NULL;
	}
	
	bool OnEvent( const Event& e )
	{
		if( m_timer > 0.5 && ( e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN ) )
		{
			m_timer = 5;
		}
		return true;
	}
	bool Draw( float delta )
	{
		m_timer += delta;
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.UnsetTexture()
			.Col( 0, 1 )
			.Quad( 0, 0, GR_GetWidth(), GR_GetHeight() );
		
		float vis_crage = smoothlerp_range( m_timer, 0, 1, 3, 5 );
		if( vis_crage )
		{
			// background
			float maxw = TMAX( GR_GetWidth(), GR_GetHeight() ) + 50;
			br.SetTexture( m_tx_back ).Col( 1, vis_crage ).QuadWH( 0, -m_timer * 10, maxw, maxw );
			
			// logo
			const TextureInfo& texinfo = m_tx_crage.GetInfo();
			float scale = GR_GetWidth() / 1024.0f;
			br.SetTexture( m_tx_crage ).Box( GR_GetWidth() / 2.0f, GR_GetHeight() / 2.1f, texinfo.width * scale, texinfo.height * scale );
		}
		
		return m_timer > 5;
	}
}
g_SplashScreen;


struct TmpPlayer : Entity
{
	TmpPlayer() :
		m_pos( V3(0,3,0) ),
		m_dir( V3(-1,0,0) ),
		m_aimdir( V3(-1,0,0) ),
		m_ivPos( m_pos )
	{
		meshInst = g_GameLevel->m_scene->CreateMeshInstance();
		meshInst->mesh = GR_GetMesh( "meshes/tstest.ssm" );
		meshInst->skin_matrices.resize( meshInst->mesh->m_numBones );
		meshInst->dynamic = 1;
		
		SGRX_PhyRigidBodyInfo rbinfo;
		rbinfo.shape = g_PhyWorld->CreateCapsuleShape( 0.3f, 1.2f );
		rbinfo.position = m_pos + V3(0,0,0.9f);
		rbinfo.mass = 100;
		rbinfo.inertia = V3(0);
		rbinfo.canSleep = 0;
		rigidBody = g_PhyWorld->CreateRigidBody( rbinfo );
		
		anLayers[0].anim = &anMainPlayer;
		anLayers[1].anim = &anTopPlayer;
		anLayers[2].anim = &anRootTurner;
		anLayers[2].tflags = AnimMixer::TF_Absolute_Rot | AnimMixer::TF_Additive;
		anLayers[3].anim = &anRagdoll;
		anLayers[3].tflags = AnimMixer::TF_Absolute_Pos | AnimMixer::TF_Absolute_Rot;
		anLayers[3].factor = 0;
		anMixer.layers = anLayers;
		anMixer.layerCount = sizeof(anLayers) / sizeof(anLayers[0]);
		anEnd.animSource = &anMixer;
		anEnd.PrepareForMesh( meshInst->mesh );
		anTopPlayer.ClearBlendFactors( 0.0f );
	//	anTopPlayer.SetBlendFactors( meshInst->mesh, "Bip01 Spine1", 1.0f );
		anRootTurner.ClearFactors( 0.0f );
		anRootTurner.SetFactors( meshInst->mesh, "Bip01 Spine", 1.0f, false );
		anRootTurner.SetFactors( meshInst->mesh, "Bip01 Spine1", 1.0f, false );
		anMixer.mesh = meshInst->mesh;
		
		anMainPlayer.Play( GR_GetAnim( "standing_idle" ) );
		anTopPlayer.Play( GR_GetAnim( "run" ) );
		
		ByteArray ba;
		FS_LoadBinaryFile( "test.rdd", ba );
		ByteReader br( &ba );
		br << skinfo;
		anRagdoll.Initialize( g_PhyWorld, meshInst->mesh, &skinfo );
	}
	
	void FixedTick( float dt )
	{
		m_pos = rigidBody->GetPosition() + V3(0,0,-0.8f);
		
		float isd;
		Vec3 raypos, raydir, isp;
		if( g_GameLevel->m_scene->camera.GetCursorRay( Game_GetCursorPos().x / GR_GetWidth(), Game_GetCursorPos().y / GR_GetHeight(), raypos, raydir ) )
		{
			if( RayPlaneIntersect( raypos, raydir, V4(0,0,1,1), &isd ) )
			{
				isp = raypos + raydir * isd;
				m_aimdir = ( isp - (m_pos + V3(0,0,1)) ).Normalized();
			}
		}
		
		Vec2 md = { MOVE_LEFT.value - MOVE_RIGHT.value, MOVE_DOWN.value - MOVE_UP.value };
		md.Normalize();
		Vec3 dir =
		{
			md.x * dt * 4,
			md.y * dt * 4,
			0
		};
		rigidBody->SetLinearVelocity( dir / dt );
	//	m_pos += dir;
		
		const char* animname = "run";
		if( Vec2Dot( md, m_aimdir.ToVec2() ) < 0 )
		{
			md = -md;
			animname = "run_bw";
		}
		Vec3 ddir = { md.x, md.y, 0 };
		float q = clamp( dt * 10 / ( 0.001f + Vec3Angle( m_dir, ddir ) ), 0, 1 );
	//	LOG << "SLERP " << m_dir << ", " << ddir << ", " << q;
		m_dir = Vec3Slerp( m_dir, ddir, q ).Normalized();
		if( Vec3Dot( m_dir, m_aimdir ) < 0.01f )
			m_dir = Vec3Slerp( m_dir, m_aimdir, ( acos( clamp( Vec3Dot( m_dir, m_aimdir ), -1, 1 ) ) - acos(0.01f) ) / M_PI ).Normalized();
	//	LOG << m_dir;
		
		Vec2 rundir = m_dir.ToVec2();
#if 0 // NEED LERP
		Vec2 aimdir = m_aimdir.ToVec2();
		if( Vec2Dot( rundir, aimdir ) < cos(M_PI/4) )
		{
			if( Vec2Dot( rundir, aimdir ) < -cos(M_PI/4) )
			{
				rundir = -rundir;
				animname = "run_bw";
			}
			else
			{
				Vec2 rtdir = rundir.Perp();
				if( Vec2Dot( rtdir, aimdir ) > 0 )
				{
					rundir = rundir.Perp();
					animname = "strafe_left";
				}
				else
				{
					rundir = rundir.Perp2();
					animname = "strafe_right";
				}
			}
		}
#else
#endif
		
		Quat mainrot = Quat::CreateAxisAngle( 0, 0, 1, atan2( rundir.y, rundir.x ) - M_PI / 2 );
		Quat toprot = Quat::CreateAxisAngle( 0, 0, 1, atan2( m_aimdir.y, m_aimdir.x ) - M_PI / 2 );
		toprot = toprot * mainrot.Inverted();
		for( size_t i = 0; i < anRootTurner.names.size(); ++i )
		{
			if( anRootTurner.names[ i ] == StringView("Bip01 Spine") )
				anRootTurner.rotation[ i ] = mainrot;
			else if( anRootTurner.names[ i ] == StringView("Bip01 Spine1") )
				anRootTurner.rotation[ i ] = toprot;
		}
		
		anMainPlayer.Play( GR_GetAnim( md.Length() ? animname : "standing_idle" ), false, 0.2f );
		anEnd.Advance( dt );
		anRagdoll.AdvanceTransforms( &anEnd );
		m_ivPos.Advance( m_pos );
		
		if( SHOOT.value )
		{
			anRagdoll.EnablePhysics( Mat4::CreateTranslation( m_pos ) );
		}
		else
		{
			anRagdoll.DisablePhysics();
		}
		
		InfoEmissionSystem::Data D = { m_pos, 0.5f, IEST_HeatSource | IEST_Player };
		g_GameLevel->m_infoEmitters.UpdateEmitter( this, D );
	}
	void Tick( float dt, float bf )
	{
		Vec3 POS = m_ivPos.Get( bf );
		
		meshInst->matrix = Mat4::CreateTranslation( POS );
		
		Vec3 camdir = V3(0,-3,-3);// V3(0,0,-8);
		g_GameLevel->m_scene->camera.position = POS - camdir;
		g_GameLevel->m_scene->camera.direction = camdir.Normalized();
		g_GameLevel->m_scene->camera.updir = V3(0,-1,0);
		g_GameLevel->m_scene->camera.aspect = 1024.0f / 576.0f;
		g_GameLevel->m_scene->camera.angle = 90; // 30;
		g_GameLevel->m_scene->camera.UpdateMatrices();
		
		g_GameLevel->LightMesh( meshInst, &spos );
		
		anEnd.Interpolate( bf );
		GR_ApplyAnimator( &anEnd, meshInst );
	}
	void DebugDraw()
	{
		#if 0
		BatchRenderer& br = GR2D_GetBatchRenderer().Reset();
		
		Mat4 matrices[ MAX_MESH_BONES ];
		GR_ApplyAnimator( &anEnd, meshInst->mesh, matrices, anEnd.names.size(), false, &meshInst->matrix );
		for( size_t i = 0; i < skinfo.hitboxes.size(); ++i )
		{
			const SkeletonInfo::HitBox& hb = skinfo.hitboxes[ i ];
			Mat4 tf = Mat4::Identity;
			for( size_t j = 0; j < anEnd.names.size(); ++j )
			{
				if( anEnd.names[ j ] == hb.name )
				{
					tf = matrices[ j ];
					break;
				}
			}
			
			br.Col( 0.1f, 0.9f, 0.1f, 0.5f );
		//	br.AABB( V3(-0.02f), V3(0.02f), tf );
			br.AABB( -hb.extents, hb.extents, Mat4::CreateSRT( V3(1), hb.rotation, hb.position ) * tf );
			br.Col( 0.2f, 0.7f, 0.9f, 0.8f );
			br.SetPrimitiveType( PT_Lines );
			br.Pos( tf.TransformPos( hb.position ) ).Pos( tf.TransformPos( hb.position + hb.zdir * 0.1f ) );
			br.Col( 0.2f, 0.4f, 0.9f, 0.8f );
			Mat4 tfx = Mat4::CreateSRT( V3(1), hb.rotation, hb.position ) * tf;
			br.Pos( tfx.TransformPos( V3(0) ) ).Pos( tfx.TransformPos( V3(0,0,0.1f) ) );
		}
		#endif
	}
	
	SkeletonInfo skinfo;
	int32_t spos;
	MeshInstHandle meshInst;
	
	PhyRigidBodyHandle rigidBody;
	
	AnimPlayer anMainPlayer;
	AnimPlayer anTopPlayer;
	Animator anRootTurner;
	AnimMixer anMixer;
	AnimMixer::Layer anLayers[4];
	AnimInterp anEnd;
	AnimRagdoll anRagdoll;
	
	Vec3 m_pos;
	Vec3 m_dir;
	Vec3 m_aimdir;
	
	IVState< Vec3 > m_ivPos;
};


struct TmpTable
{
	TmpTable( const Vec3& pos = V3(0), const Quat& rot = Quat::Identity ) :
		state_Pos( pos ), state_Rot( rot )
	{
		meshInst = g_GameLevel->m_scene->CreateMeshInstance();
		meshInst->mesh = GR_GetMesh( "meshes/test_table.ssm" );
		meshInst->dynamic = 1;
		
		SGRX_PhyRigidBodyInfo rbinfo;
		
		rbinfo.mass = 0.0f;
		rbinfo.shape = g_PhyWorld->CreateAABBShape( V3(-100,-100,-100), V3(100,100,0) );
	//	rigidBody2 = g_PhyWorld->CreateRigidBody( rbinfo );
		
		rbinfo.shape = g_PhyWorld->CreateAABBShape( meshInst->mesh->m_boundsMin, meshInst->mesh->m_boundsMax );
		rbinfo.shape->SetScale( V3(0.5f) );
		rbinfo.mass = 50.0f;
		rbinfo.inertia = rbinfo.shape->CalcInertia( rbinfo.mass );
		rbinfo.position = pos;
		rbinfo.rotation = rot;
		rigidBody = g_PhyWorld->CreateRigidBody( rbinfo );
	}
	~TmpTable()
	{
	}
	void FixedTick( float dt )
	{
		state_Pos.Advance( rigidBody->GetPosition() );
		state_Rot.Advance( rigidBody->GetRotation() );
	}
	void Tick( float dt, float bf )
	{
		Vec3 P = state_Pos.Get( bf );
		Quat R = state_Rot.Get( bf );
		
		meshInst->matrix = Mat4::CreateSRT( V3(0.5f), R, P );
		LightTree::Sample SMP = { P };
		g_GameLevel->m_ltSamples.Interpolate( SMP );
		meshInst->constants[10] = V4( SMP.color[0], 1 );
		meshInst->constants[11] = V4( SMP.color[1], 1 );
		meshInst->constants[12] = V4( SMP.color[2], 1 );
		meshInst->constants[13] = V4( SMP.color[3], 1 );
		meshInst->constants[14] = V4( SMP.color[4], 1 );
		meshInst->constants[15] = V4( SMP.color[5], 1 );
	}
	
	PhyRigidBodyHandle rigidBody;
	PhyRigidBodyHandle rigidBody2;
	MeshInstHandle meshInst;
	
	IVState<Vec3> state_Pos;
	IVState<Quat> state_Rot;
};


#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)

struct TACStrikeGame : IGame, SGRX_DebugDraw
{
	TACStrikeGame() : m_accum( 0.0f )
	{
	}
	
	void OnInitialize()
	{
		g_SoundSys = SND_CreateSystem();
		
		g_PhyWorld = PHY_CreateWorld();
		g_PhyWorld->SetGravity( V3( 0, 0, -9.81f ) );
		
		GR_SetRenderPasses( g_RenderPasses_Main, SGRX_ARRAY_SIZE( g_RenderPasses_Main ) );
		
		GR_GetVertexDecl( "pf3tf2" );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );
		Game_RegisterAction( &MOVE_UP );
		Game_RegisterAction( &MOVE_DOWN );
		Game_RegisterAction( &SHOOT );
		Game_RegisterAction( &RELOAD );
		Game_RegisterAction( &SLOW_WALK );
		Game_RegisterAction( &SPRINT );
		Game_RegisterAction( &CROUCH );
		
		Game_BindKeyToAction( SDLK_a, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_d, &MOVE_RIGHT );
		Game_BindKeyToAction( SDLK_w, &MOVE_UP );
		Game_BindKeyToAction( SDLK_s, &MOVE_DOWN );
		Game_BindKeyToAction( SDLK_g, &SHOOT );
		Game_BindKeyToAction( SDLK_r, &RELOAD );
	//	Game_BindKeyToAction( SDLK_LCTRL, &SLOW_WALK );
		Game_BindKeyToAction( SDLK_LSHIFT, &SPRINT );
		Game_BindKeyToAction( SDLK_LCTRL, &CROUCH );
		
		g_GameLevel = new GameLevel();
		
		Game_AddOverlayScreen( &g_SplashScreen );
		
		g_GameLevel->Load( "ver3" );
		
		GR_LoadAnims( "meshes/animtest.ssm.anm", "my_" );
		GR_LoadAnims( "meshes/tstest.ssm.anm" );
//		GR_LoadAnims( "meshes/charmodel2.ssm.anm" );
		myanim = GR_GetAnim( "run" );
	//	myanim = GR_GetAnim( "my_jiggle" );
		
	//	myplayer = new TmpPlayer;
		PS = new ParticleSystem;
		PS->emitters.push_back( ParticleSystem::Emitter() );
		ParticleSystem::Emitter& E = PS->emitters[0];
		E.curve_ColorVal.valueRange = V2(1);
		E.curve_Opacity.valueRange = V2(1);
		E.curve_Size.valueRange = V2(0.02f);
		E.create_VelMacroDir = V3(0,0,1);
		E.create_VelMacroDvg = 0.05f;
		E.create_VelMacroDistExt = V2( 10, 0.1f );
		E.render_Stretch = true;
		E.render_Textures[0] = GR_GetTexture( "textures/particles/spark_fast.png" );
		PS->Save( "pssavetest" );
		PS->Load( "pssavetest" );
		PS->OnRenderUpdate();
		PS->AddToScene( g_GameLevel->m_scene );
		PS->SetTransform( Mat4::CreateTranslation( 0, 1, 1 ) );
		PS->Play();
		
		mylight1 = g_GameLevel->m_scene->CreateLight();
		mylight1->type = LIGHT_SPOT;
		mylight1->position = V3(2,9,8);
		mylight1->direction = V3( -1, -2, -4 ).Normalized();
		mylight1->updir = V3(0,-1,0);
		mylight1->color = HSV(V3(0.01f,0.9f,1.9f));
		mylight1->range = 50;
		mylight1->power = 2;
		mylight1->angle = 30;
		mylight1->aspect = 1;
		mylight1->cookieTexture = GR_GetTexture( "textures/cookies/default.png" );
		mylight1->shadowTexture = GR_CreateRenderTexture( 128, 128, RT_FORMAT_DEPTH );
		mylight1->hasShadows = true;
		mylight1->UpdateTransform();
		
		mytable = new TmpTable( V3(2,2,7), Quat::CreateAxisAngle( V3(1,0,0), 1.5f) );
		
		g_GameLevel->StartLevel();
	}
	void OnDestroy()
	{
		delete mytable;
		mytable = NULL;
		
		mylight1 = NULL;
		delete PS;
		PS = NULL;
	//	delete myplayer;
	//	myplayer = NULL;
		delete g_GameLevel;
		g_GameLevel = NULL;
		
		g_PhyWorld = NULL;
		g_SoundSys = NULL;
	}
	
	void Game_FixedTick( float dt )
	{
		int ITERS = 10;
		if( !g_GameLevel->m_paused )
		{
			for( int i = 0; i < ITERS; ++i )
				g_PhyWorld->Step( dt / ITERS );
		}
		g_GameLevel->FixedTick( dt );
		
	//	myplayer->FixedTick( dt );
		mytable->FixedTick( dt );
	}
	void Game_Tick( float dt, float bf )
	{
		g_GameLevel->Tick( dt, bf );
		
		//
		// TEST
		//
		PS->Tick( dt );
		PS->PreRender();
	//	myplayer->Tick( dt, bf );
		mytable->Tick( dt, bf );
		
		Vec3 CP, CD;
		if( SHOOT.value && g_GameLevel->m_scene->camera.GetCursorRay(
			Game_GetCursorPos().x / GR_GetWidth(), Game_GetCursorPos().y / GR_GetHeight(), CP, CD ) )
		{
			g_GameLevel->m_bulletSystem.Add( CP, CD * 10, 1, 1 );
		}
	}
	void Game_Render()
	{
	//	g_GameLevel->Draw();
		SGRX_RenderScene rsinfo( V4( g_GameLevel->m_levelTime ), g_GameLevel->m_scene );
		rsinfo.debugdraw = this;
		rsinfo.postdraw = g_GameLevel;
		GR_RenderScene( rsinfo );
		
		g_GameLevel->Draw2D();
		
		//
		// TEST
		//
		GR2D_SetColor( 1 );
		char bfr[ 1204 ];
		sprintf( bfr, "meshes: %d, draw calls: %d", (int) GR_GetRenderStats().numVisMeshes, (int) GR_GetRenderStats().numMDrawCalls );
	//	GR2D_DrawTextLine( 128, 128, bfr );
		
	//	BatchRenderer& br = GR2D_GetBatchRenderer();
	//	const LightTree& LT = g_GameLevel->m_ltSamples;
	//		GR2D_SetFont( "fonts/lato-regular.ttf", 12 );
	//	for( size_t i = 0; i < LT.m_tris.size(); i += 3 )
	//	{
	//		Vec3 p0 = LT.m_samples[ LT.m_tris[ i+0 ] ].pos;
	//		Vec3 p1 = LT.m_samples[ LT.m_tris[ i+1 ] ].pos;
	//		Vec3 p2 = LT.m_samples[ LT.m_tris[ i+2 ] ].pos;
	//		Vec3 pos = g_GameLevel->m_scene->camera.WorldToScreen( (p0+p1+p2)/3 );
	//		char dstbfr[ 2000 ];
	//		sprintf( dstbfr, "%g", PointTriangleDistance( myplayer->m_pos, p0, p1, p2 ) );
	//		GR2D_DrawTextLine( pos.x*1024, pos.y*576, dstbfr );
	//	}
	//	br.Flush();
	}
	
	void OnTick( float dt, uint32_t gametime )
	{
		g_SoundSys->Update();
		
		if( dt > MAX_TICK_SIZE )
			dt = MAX_TICK_SIZE;
		
		m_accum += dt;
		while( m_accum >= 0 )
		{
			Game_FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		Game_Tick( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		Game_Render();
	}
	void DebugDraw()
	{
//		g_PhyWorld->DebugDraw();
	//	SGRX_MeshBone* bones = animtest->mesh->m_bones;
	//	for( int i = 0; i < animtest->mesh->m_numBones; ++i )
	//		DrawTick( bones[ i ].skinOffset );
		
	//	BatchRenderer& br = GR2D_GetBatchRenderer();
	//	br.Reset().SetPrimitiveType( PT_Triangles );
	//	const LightTree& LT = g_GameLevel->m_ltSamples;
	//	for( size_t i = 0; i < LT.m_tris.size(); i += 3 )
	//	{
	//		float q = myplayer->spos == i / 3 ? 0.5f : 0;
	//		br.Col( 1,q,q, 0.5f ).Pos( LT.m_samples[ LT.m_tris[ i+0 ] ].pos );
	//		br.Col( q,1,q, 0.5f ).Pos( LT.m_samples[ LT.m_tris[ i+1 ] ].pos );
	//		br.Col( q,q,1, 0.5f ).Pos( LT.m_samples[ LT.m_tris[ i+2 ] ].pos );
	//	}
	//	br.SetPrimitiveType( PT_Lines );
	//	br.Col( 0.5f, 0.5f );
	//	for( size_t i = 0; i < LT.m_triadj.size() / 2; ++i )
	//	{
	//		Vec3 posA = ( LT.m_samples[ LT.m_tris[ i*3+0 ] ].pos + LT.m_samples[ LT.m_tris[ i*3+1 ] ].pos + LT.m_samples[ LT.m_tris[ i*3+2 ] ].pos ) / 3;
	//		for( size_t a = LT.m_triadj[ i*2 ]; a < LT.m_triadj[ i*2 ] + LT.m_triadj[ i*2+1 ]; ++a )
	//		{
	//			size_t j = LT.m_adjdata[ a ];
	//			if( j > i )
	//			{
	//				Vec3 posB = ( LT.m_samples[ LT.m_tris[ j*3+0 ] ].pos + LT.m_samples[ LT.m_tris[ j*3+1 ] ].pos + LT.m_samples[ LT.m_tris[ j*3+2 ] ].pos ) / 3;
	//				br.Pos( posA ).Pos( posB );
	//			}
	//		}
	//	}
	//	br.Flush();
//		myplayer->DebugDraw();
	}
	void DrawTick( const Mat4& mtx )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.UnsetTexture().SetPrimitiveType( PT_Lines ).Flush();
		br.Col( 1,0,0 ).Pos( mtx.TransformPos( V3(0,0,0) ) ).Pos( mtx.TransformPos( V3(1,0,0)*0.1f ) );
		br.Col( 0,1,0 ).Pos( mtx.TransformPos( V3(0,0,0) ) ).Pos( mtx.TransformPos( V3(0,1,0)*0.1f ) );
		br.Col( 0,0,1 ).Pos( mtx.TransformPos( V3(0,0,0) ) ).Pos( mtx.TransformPos( V3(0,0,1)*1.1f ) );
		br.Flush();
	}
	
	float m_accum;
	
	AnimHandle myanim;
	LightHandle mylight1;
//	TmpPlayer* myplayer;
	TmpTable* mytable;
	ParticleSystem* PS;
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}


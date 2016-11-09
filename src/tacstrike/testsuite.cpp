

#include <engine.hpp>
#include <engext.hpp>
#include <gamegui.hpp>


InputState MOVE_LEFT( "move_left" );
InputState MOVE_RIGHT( "move_right" );


struct ITest
{
	virtual StringView GetName() const { return "<unnamed test>"; }
	virtual void Do( float dt, float bf ){}
	virtual void FixedTick( float dt ){}
	virtual void OnInitialize(){}
	virtual void OnDestroy(){}
	virtual void OnEvent( const Event& e ){}
	
	void DrawFontTextures()
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		Array< TextureHandle > fontTextures;
		int count = DBG_GetFontTextures( fontTextures );
		
		int w = GR_GetWidth(), h = GR_GetHeight();
		float x0 = w / 2.0f;
		float iw = w / 2.0f / fontTextures.size();
		for( size_t i = 0; i < fontTextures.size(); ++i )
		{
			br.SetTexture( fontTextures[ i ] );
			br.Quad( x0 + iw * i, h/2, x0 + iw * (i+1), h/2 + iw );
		}
		
		char bfr[64];
		sgrx_snprintf( bfr, 64, "glyph count: %d", count );
		GR2D_SetFont( "system_outlined", 7 );
		GR2D_DrawTextLine( w/2, h/2+iw, bfr );
	}
};



struct Test_Intro : ITest
{
	virtual StringView GetName() const { return "Intro"; }
	void Do( float, float )
	{
		GR2D_SetWorldMatrix( Mat4::Identity );
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		GR2D_SetFont( "core", 24 );
		GR2D_DrawTextLine( GR_GetWidth()/2, GR_GetHeight()/2,
			"Press F2/F3 to change tests", HALIGN_CENTER, VALIGN_CENTER );
	}
}
g_TestIntro;



struct Test_PixelPerfectRendering : ITest
{
	Test_PixelPerfectRendering() : m_time( 0 ){}
	virtual StringView GetName() const { return "Pixel-perfect rendering"; }
	void Do( float dt, float )
	{
		m_time += dt;
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, 1, 1 ) );
		TextureHandle cliptex = GR_GetTexture( "textures/clip.png:nolerp" );
		GR_PreserveResource( cliptex );
		GR2D_GetBatchRenderer().Reset().SetTexture( cliptex ).Quad( 0, 0, 1, 1 );
		
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::CreateScale( 2, 2, 1 ) * Mat4::CreateTranslation( 0, 50, 0 ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::CreateScale( 4, 4, 1 ) * Mat4::CreateTranslation( 0, 100, 0 ) );
		DrawSomeText();
		GR2D_SetWorldMatrix( Mat4::Identity );
		
		DrawFontTextures();
	}
	void DrawSomeText()
	{
		TextureHandle cliptex = GR_GetTexture( "textures/clip.png:nolerp" );
		GR_PreserveResource( cliptex );
		int size = 24 + 12 * sinf(m_time);
		GR2D_SetFont( "core", size );
		
		GR2D_GetBatchRenderer().Reset().SetTexture( cliptex ).Quad( 0, 0, 16, 16 );
		GR2D_DrawTextLine( round(20 * (sinf(m_time)*0.5f+0.5f)),
			round(20 * (cosf(m_time)*0.5f+0.5f)), "abcdefghijklmnopqrstuvwxyz" );
		GR2D_DrawTextLine( round(20 * (sinf(m_time)*0.5f+0.5f)),
			round(20 * (cosf(m_time)*0.5f+0.5f)) + size, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	}
	float m_time;
}
g_TestPixelPerfectRendering;



struct Test_AdvancedText : ITest
{
	Test_AdvancedText(){}
	virtual StringView GetName() const { return "Advanced text rendering"; }
	void Do( float, float )
	{
		double t0 = sgrx_hqtime();
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		GR2D_SetFont( "core", 12 );
		for( int y = 0; y <= 2; ++y )
		{
			for( int x = 0; x <= 2; ++x )
			{
				int xo = y * 300 + x * 100;
				DT( 10 + xo, 10, 90 + xo, 90, "text automatically split into several lines", x, y );
			}
		}
		for( int y = 0; y <= 2; ++y )
		{
			for( int x = 0; x <= 2; ++x )
			{
				int xo = y * 300 + x * 100;
				DT( 10 + xo, 110, 90 + xo, 190, "text automatically split into several lines and cropped because it is just too long", x, y );
			}
		}
		for( int y = 0; y <= 2; ++y )
		{
			for( int x = 0; x <= 2; ++x )
			{
				int xo = y * 300 + x * 100;
				DTP( 10 + xo, 210, 90 + xo, 290, "tagged #f(mono)text#f(core) automatically #{#c(255,127,0)split#} into #{#c(0,127,255,0.5)several#} lines", x, y );
			}
		}
		double t1 = sgrx_hqtime();
		char bfr[ 64 ];
		sgrx_snprintf( bfr, 64, "Time to process text: %g ms", ( t1 - t0 ) * 1000 );
		GR2D_DrawTextLine( 0, GR_GetHeight() - 12, bfr );
		
		DrawFontTextures();
	}
	void DT( int x0, int y0, int x1, int y1, StringView text, int ha = HALIGN_LEFT, int va = VALIGN_TOP )
	{
		GR2D_GetBatchRenderer().Reset().AARectOutline( x0-4, y0-4, x1+4, y1+4, 4 );
		GR2D_DrawTextRect( x0, y0, x1, y1, text, ha, va, false );
	}
	void DTP( int x0, int y0, int x1, int y1, StringView text, int ha = HALIGN_LEFT, int va = VALIGN_TOP )
	{
		GR2D_GetBatchRenderer().Reset().AARectOutline( x0-4, y0-4, x1+4, y1+4, 4 );
		GR2D_DrawTextRect( x0, y0, x1, y1, text, ha, va, true );
	}
}
g_TestAdvancedText;



struct Test_BruteForce : ITest
{
	Test_BruteForce(){}
	virtual StringView GetName() const { return "Brute force batch rendering"; }
	void OnInitialize()
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
	}
	void Do( float, float )
	{
		TextureHandle cliptex = GR_GetTexture( "textures/clip.png:nolerp" );
		GR_PreserveResource( cliptex );
		
		TextureHandle nulltex = GR_GetTexture( "textures/null.png:nolerp" );
		GR_PreserveResource( nulltex );
		
		SGRX_ITexture* textures[2] = { cliptex, nulltex };
		
		int i = 0;
		GR2D_SetViewMatrix( Mat4::CreateUI( 0, 0, GR_GetWidth(), GR_GetHeight() ) );
		GR2D_GetBatchRenderer().Reset();
		for( int y = 0; y < GR_GetHeight(); y += 16 )
		{
			for( int x = 0; x < GR_GetWidth(); x += 16 )
			{
				int c = i++ % 2;
				GR2D_GetBatchRenderer().SetTexture( textures[c] ).QuadWH( x, y, 16, 16 );
			}
		}
	//	LOG << "rendered " << i << " tiles (16x16)";
	}
}
g_TestBruteForce;



struct Test_GameUI : ITest
{
	virtual StringView GetName() const { return "Game GUI test"; }
	Test_GameUI() : m_guiSys(NULL)
	{
	}
	
	void OnInitialize()
	{
		m_scriptCtx = new ScriptContext;
		m_guiSys = new GameUISystem( m_scriptCtx );
		m_guiSys->Load( "ui/test.sgs" );
	}
	void OnDestroy()
	{
		SAFE_DELETE( m_guiSys );
		delete m_scriptCtx;
	}
	void OnEvent( const Event& e )
	{
		m_guiSys->EngineEvent( e );
	}
	void Do( float dt, float )
	{
		m_scriptCtx->Push( dt );
		m_scriptCtx->GlobalCall( "process_threads", 1 );
		m_guiSys->Draw( dt );
	}
	
	ScriptContext* m_scriptCtx;
	GameUISystem* m_guiSys;
}
g_TestGameUI;



struct Lighting1 : SGRX_LightSampler
{
	void SampleLight( const Vec3& pos, Vec3 outcolors[6] )
	{
		outcolors[0] = V3(0.3f,0.2f,0.2f) * 0.2f;
		outcolors[1] = V3(0.5f,0.4f,0.4f) * 0.2f;
		outcolors[2] = V3(0.7f,0.6f,0.6f) * 0.2f;
		outcolors[3] = V3(0.3f,0.4f,0.3f) * 0.2f;
		outcolors[4] = V3(0.5f,0.6f,0.5f) * 0.2f;
		outcolors[5] = V3(0.7f,0.8f,0.7f) * 0.2f;
	}
}
g_Lighting1;

struct Test_3DRendering : ITest
{
	virtual StringView GetName() const { return "3D rendering test"; }
	Test_3DRendering()
	{
	}
	
	void OnInitialize()
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
		
		m_scene = GR_CreateScene();
		m_scene->camera.position = V3(-10,-10,10);
		m_scene->camera.direction = V3(10,10,-5).Normalized();
		m_scene->camera.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
		m_scene->camera.angle = 45;
		m_scene->camera.UpdateMatrices();
		
		MeshHandle mesh = GR_GetMesh( "sys:cube" );
		
		for( int y = 0; y < 50; ++y )
		{
			for( int x = 0; x < 50; ++x )
			{
				MeshInstHandle mih = m_scene->CreateMeshInstance();
				mih->SetMesh( mesh );
				mih->matrix = Mat4::CreateTranslation( x * 3, y * 3, 0 );
				mih->SetLightingMode( SGRX_LM_Dynamic );
				g_Lighting1.LightMesh( mih );
				m_meshes.push_back( mih );
			}
		}
	}
	void OnDestroy()
	{
		m_meshes.clear();
		m_scene = NULL;
	}
	void OnEvent( const Event& e )
	{
	}
	void Do( float dt, float )
	{
		SGRX_RenderScene rs( V4(0), m_scene );
		GR_RenderScene( rs );
	}
	
	SceneHandle m_scene;
	Array< MeshInstHandle > m_meshes;
}
g_Test3DRendering;



struct Test_Characters : ITest
{
	virtual StringView GetName() const { return "Character animation test"; }
	Test_Characters()
	{
	}
	
	void OnInitialize()
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
		
		m_phyWorld = PHY_CreateWorld();
		m_scene = GR_CreateScene();
		m_scene->camera.position = V3(-10,-10,10);
		m_scene->camera.direction = V3(10,10,-6).Normalized();
		m_scene->camera.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
		m_scene->camera.angle = 45;
		m_scene->camera.UpdateMatrices();
		
		int xe = 12, ye = 12;
		m_chars.resize( xe * ye );
		for( int y = 0; y < ye; ++y )
		{
			for( int x = 0; x < xe; ++x )
			{
				int i = x + y * xe;
				m_chars[ i ] = new AnimCharInst( m_scene, m_phyWorld );
				m_chars[ i ]->SetAnimChar( "chars/tstest.chr" );
				g_Lighting1.LightMesh( m_chars[ i ]->m_cachedMeshInst );
				m_chars[ i ]->SetTransform( Mat4::CreateTranslation( x * 2, y * 2, 0 ) );
			}
		}
	}
	void OnDestroy()
	{
		for( size_t i = 0; i < m_chars.size(); ++i )
			delete m_chars[ i ];
		m_chars.clear();
		m_scene = NULL;
		m_phyWorld = NULL;
	}
	void OnEvent( const Event& e )
	{
	}
	void FixedTick( float dt )
	{
		for( size_t i = 0; i < m_chars.size(); ++i )
		{
			m_chars[ i ]->FixedTick( dt );
		}
	}
	void Do( float dt, float bf )
	{
		float t = sgrx_hqtime();
		for( size_t i = 0; i < m_chars.size(); ++i )
		{
			m_chars[ i ]->SetFloat( "run", sin(t+i) * 0.5f + 0.5f );
			m_chars[ i ]->SetFloat( "aim", cos(t*1.2f+i) + 1 );
			m_chars[ i ]->PreRender( bf );
		}
		SGRX_RenderScene rs( V4(0), m_scene );
		GR_RenderScene( rs );
	}
	
	PhyWorldHandle m_phyWorld;
	SceneHandle m_scene;
	Array< AnimCharInst* > m_chars;
}
g_TestCharacters;



struct Test_Decals : ITest
{
	virtual StringView GetName() const { return "Decal test"; }
	Test_Decals()
	{
		m_accum = 0.1f;
	}
	
	void OnInitialize()
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
		
		m_scene = GR_CreateScene();
		m_scene->camera.position = V3(-10,-10,10);
		m_scene->camera.direction = V3(10,10,-10).Normalized();
		m_scene->camera.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
		m_scene->camera.angle = 90;
		m_scene->camera.UpdateMatrices();
		
		m_decalSys.Init( m_scene,
			GR_GetTexture( "textures/grid.png" ),
			GR_GetTexture( "textures/fx/projfalloff2.png" )
		);
		m_decalSys.SetSize( 1024 * 1024 ); // 1 MB for decals
		m_decalSys.m_lightSampler = &g_Lighting1;
		
		MeshHandle mesh = GR_GetMesh( "sys:plane" );
		MeshInstHandle mih = m_scene->CreateMeshInstance();
		mih->SetMesh( mesh );
		mih->matrix = Mat4::CreateScale( 10, 10, 1 );
		mih->SetLightingMode( SGRX_LM_Dynamic );
		g_Lighting1.LightMesh( mih );
		m_meshes.push_back( mih );
		
		for( int y = -1; y <= 1; ++y )
		{
			for( int x = -1; x <= 1; ++x )
			{
				mesh = GR_GetMesh( "meshes/chars/tstest.ssm" );
				mih = m_scene->CreateMeshInstance();
				mih->SetMesh( mesh );
				mih->matrix =
					Mat4::CreateScale( V3(3) ) *
					Mat4::CreateRotationZ( DEG2RAD( 115.0f ) ) *
					Mat4::CreateTranslation( V3(x,y,0) * 5 );
				mih->SetLightingMode( SGRX_LM_Dynamic );
				g_Lighting1.LightMesh( mih );
				m_meshes.push_back( mih );
			}
		}
	}
	void OnDestroy()
	{
		m_decalSys.Free();
		m_meshes.clear();
		m_scene = NULL;
	}
	void OnEvent( const Event& e )
	{
		if( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_c )
			m_decalSys.ClearAllDecals();
	}
	void Do( float dt, float bf )
	{
		m_accum -= dt;
		while( m_accum < 0.1f )
		{
			m_accum += 0.1f;
			
			DecalProjectionInfo dpi =
			{
				V3( randf11() * 10, randf11() * 10, 0 ),
				V3( 0, 0, -1 ),
				V3( 0, -1, 0 ),
				90.0f, // fov
				2.0f, // ortho scale
				1.0f, // aspect
				1.0f, // aamix
				1.0f, // distance scale
				0.9f, // pushback
				rand() % 2, // perspective
				{
					V4(0,0,1,1),
					V3(2),
				}
			};
			for( size_t i = 0; i < m_meshes.size(); ++i )
			{
				SGRX_MeshInstance* mi = m_meshes[ i ];
				m_decalSys.AddDecal( dpi, mi->GetMesh(), mi->matrix );
			}
		}
		
		m_decalSys.Upload();
		SGRX_RenderScene rs( V4(0), m_scene );
		GR_RenderScene( rs );
	}
	
	float m_accum;
	SceneHandle m_scene;
	Array< MeshInstHandle > m_meshes;
	SGRX_DecalSystem m_decalSys;
}
g_TestDecals;



struct Test_Projectors : ITest
{
	virtual StringView GetName() const { return "Projector test"; }
	Test_Projectors()
	{
		m_time = 0;
	}
	void OnInitialize()
	{
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.vsync = false;
		GR_SetVideoMode( rs );
		
		m_scene = GR_CreateScene();
		m_scene->cullScene = &m_cullScene;
		m_scene->debugDrawFlags = SGRX_SceneDbgDraw_ActiveLights;
		m_scene->camera.position = V3(-10,-10,10);
		m_scene->camera.direction = V3(10,10,-10).Normalized();
		m_scene->camera.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
		m_scene->camera.angle = 90;
		m_scene->camera.UpdateMatrices();
		
		MeshHandle mesh = GR_GetMesh( "sys:plane" );
		MeshInstHandle mih = m_scene->CreateMeshInstance();
		mih->SetMesh( mesh );
		mih->matrix = Mat4::CreateScale( 10, 10, 1 );
		mih->SetLightingMode( SGRX_LM_Dynamic );
		g_Lighting1.LightMesh( mih );
		m_meshes.push_back( mih );
		
		for( int y = -1; y <= 1; ++y )
		{
			for( int x = -1; x <= 1; ++x )
			{
				mesh = GR_GetMesh( "meshes/chars/tstest.ssm" );
				mih = m_scene->CreateMeshInstance();
				mih->SetMesh( mesh );
				mih->matrix =
					Mat4::CreateScale( V3(3) ) *
					Mat4::CreateRotationZ( DEG2RAD( 115.0f ) ) *
					Mat4::CreateTranslation( V3(x,y,0) * 5 );
				mih->SetLightingMode( SGRX_LM_Dynamic );
				g_Lighting1.LightMesh( mih );
				m_meshes.push_back( mih );
			}
		}
		
		for( int i = 0; i < 8; ++i )
		{
			LightHandle proj = m_scene->CreateLight();
			proj->type = LIGHT_PROJ;
			proj->position = V3(0,0,1);
			proj->direction = V3(0,0,-1);
			proj->updir = V3(0,1,0);
			proj->angle = 60;
			proj->range = 7.0f;
			proj->UpdateTransform();
			proj->projectionMaterial.textures[0] = GR_GetTexture( "textures/fx/blobshadow.png" );//GR_GetTexture( "textures/unit.png" );
			proj->projectionMaterial.textures[1] = GR_GetTexture( "textures/fx/projfalloff2.png" );
			m_projs.push_back( proj );
		}
	}
	void OnDestroy()
	{
		m_meshes.clear();
		m_projs.clear();
		m_scene = NULL;
	}
	void Do( float dt, float bf )
	{
		m_time += dt;
		for( size_t i = 0; i < m_projs.size(); ++i )
		{
			float t = m_time + float(i) / m_projs.size() * M_PI * 2;
			float q = t;
			m_projs[ i ]->position = V3( cos( q ) * 6, sin( q ) * 6, 5 );
		}
		
		SGRX_RenderScene rs( V4(0), m_scene );
		GR_RenderScene( rs );
	}
	
	float m_time;
	SGRX_DefaultCullScene m_cullScene;
	SceneHandle m_scene;
	Array< LightHandle > m_projs;
	Array< MeshInstHandle > m_meshes;
}
g_TestProjectors;


struct PosLightSampler : SGRX_LightSampler
{
	int x, y;
	void SampleLight( const Vec3& pos, Vec3 outcolors[6] )
	{
		for( int i = 0; i < 6; ++i )
			outcolors[i] = V3(x*0.5f+0.5f,y*0.5f+0.5f,1) * 0.2f;
	}
};

void GetCubemapVectors( Vec3 outfwd[6], Vec3 outup[6] )
{
	// order: +X, -X, +Y, -Y, +Z, -Z
	outfwd[0] = V3(+1,0,0); outup[0] = V3(0,1,0);
	outfwd[1] = V3(-1,0,0); outup[1] = V3(0,1,0);
	outfwd[2] = V3(0,+1,0); outup[2] = V3(0,0,-1);
	outfwd[3] = V3(0,-1,0); outup[3] = V3(0,0,1);
	outfwd[4] = V3(0,0,+1); outup[4] = V3(0,1,0);
	outfwd[5] = V3(0,0,-1); outup[5] = V3(0,1,0);
	
	// fix-up for shader conventions
//	for( int i = 0; i < 6; ++i )
//	{
//		TSWAP( outfwd[ i ].y, outfwd[ i ].z );
//		TSWAP( outup[ i ].y, outup[ i ].z );
//	}
	
//	for(int i = 0; i < 6;++i)
//	{
//		if( i == 4) continue;
//		outup[i] = outfwd[ i ];
//	}
}
struct Test_SceneCubemap : ITest
{
	virtual StringView GetName() const { return "Scene cubemap test"; }
	Test_SceneCubemap()
	{
		m_time = 0;
	}
	void OnInitialize()
	{
		m_scene = GR_CreateScene();
		m_scene->camera.position = V3(-10,-10,10);
		m_scene->camera.direction = V3(10,10,-10).Normalized();
		m_scene->camera.aspect = safe_fdiv( GR_GetWidth(), GR_GetHeight() );
		m_scene->camera.angle = 90;
		m_scene->camera.UpdateMatrices();
		
		MeshInstHandle mih = m_scene->CreateMeshInstance();
		mih->SetMesh( GR_GetMesh( "sys:plane" ) );
		mih->matrix = Mat4::CreateScale( 10, 10, 1 );
		mih->SetLightingMode( SGRX_LM_Dynamic );
		g_Lighting1.LightMesh( mih );
		m_meshes.push_back( mih );
		
		for( int z = -1; z <= 1; ++z )
		{
			for( int y = -1; y <= 1; ++y )
			{
				for( int x = -1; x <= 1; ++x )
				{
					if( x == 0 && y == 0 && z == 0 )
						continue;
					
					PosLightSampler pls;
					pls.x = x;
					pls.y = y;
					
					mih = m_scene->CreateMeshInstance();
					if( z == 0 )
					{
						mih->SetMesh( GR_GetMesh( "meshes/chars/tstest.ssm" ) );
						mih->matrix =
							Mat4::CreateScale( V3(3) ) *
							Mat4::CreateRotationZ( DEG2RAD( 115.0f ) ) *
							Mat4::CreateTranslation( V3(x,y,0) * 5 );
					}
					else
					{
						mih->SetMesh( GR_GetMesh( "sys:sphere" ) );
						mih->matrix =
							Mat4::CreateScale( V3(0.1f) ) *
							Mat4::CreateTranslation( V3(x*1,y*1,z+1) );
					}
					mih->SetLightingMode( SGRX_LM_Dynamic );
					pls.LightMesh( mih );
					m_meshes.push_back( mih );
				}
			}
		}
		
		m_cubemap = m_scene->CreateCubemap( 128, V3( 0, 0, 1 ) );
		
		mih = m_scene->CreateMeshInstance();
		mih->SetMesh( GR_GetMesh( "sys:sphere" ) );
		mih->matrix = Mat4::CreateTranslation( 0, 0, 1 );
		mih->SetLightingMode( SGRX_LM_Dynamic );
		SGRX_Material& mtl = mih->GetMaterial(0);
		mtl.shader = "bumpspecglossm";
		mtl.textures[0] = GR_GetTexture( "sys:black2d" );
		mtl.textures[1] = GR_GetTexture( "sys:normal2d" );
		mtl.textures[2] = GR_GetTexture( "textures/specglossgrid.png" );// "sys:white2d" );
		mih->OnUpdate();
		mih->SetMITexture( 2, m_cubemap );
		g_Lighting1.LightMesh( mih );
		m_meshes.push_back( mih );
	}
	void OnDestroy()
	{
		m_cubemap = NULL;
		m_meshes.clear();
		m_scene = NULL;
	}
	void Do( float dt, float bf )
	{
		m_time += dt;
		
		Vec3 tgt = V3(0,0,1);
		Vec3 dir = V3( cosf( m_time ) * 2, sinf( m_time ) * 2, 1 );
		m_scene->camera.position = tgt + dir;
		m_scene->camera.direction = -dir;
		m_scene->camera.znear = 0.1f;
		m_scene->camera.UpdateMatrices();
		
	//	m_scene->CreateCubemap( 128, V3( 0, 0, 1 ) );
		
		SGRX_RenderScene rs( V4(0), m_scene );
		GR_RenderScene( rs );
	}
	
	SceneHandle m_scene;
	TextureHandle m_cubemap;
	Array< MeshInstHandle > m_meshes;
	float m_time;
}
g_TestSceneCubemap;





size_t g_CurTest = 0;
ITest* g_Tests[] =
{
	&g_TestIntro,
	&g_TestPixelPerfectRendering,
	&g_TestAdvancedText,
	&g_TestBruteForce,
	&g_TestGameUI,
	&g_Test3DRendering,
	&g_TestCharacters,
	&g_TestDecals,
	&g_TestProjectors,
	&g_TestSceneCubemap,
};
RenderSettings g_rs;
#define TESTCOUNT (sizeof(g_Tests)/sizeof(g_Tests[0]))

static void InitTest()
{
	ITest* T = g_Tests[ g_CurTest ];
	GR_GetVideoMode( g_rs );
	T->OnInitialize();
	char bfr[ 4096 ];
	sgrx_snprintf( bfr, 4096, "Test #%d: %s", (g_CurTest+1), StackString<4096>(T->GetName()).str );
	Window_SetTitle( bfr );
}
static void FreeTest()
{
	ITest* T = g_Tests[ g_CurTest ];
	T->OnDestroy();
	GR_SetVideoMode( g_rs );
}
static void SetTest( size_t i )
{
	if( i < 0 ) i = 0;
	if( i >= TESTCOUNT ) i = TESTCOUNT - 1;
	FreeTest();
	g_CurTest = i;
	InitTest();
}


struct TestSuite : IGame
{
	TestSuite() : m_accum( 0.0f ),
		m_lastTime( 0 ), m_frameCount( 0 )
	{
	}
	
	bool OnConfigure( int argc, char* argv[] )
	{
#if 0
		RenderSettings rs;
		GR_GetVideoMode( rs );
		rs.width = 800;
		rs.height = 600;
		//rs.fullscreen = FULLSCREEN_WINDOWED;
		GR_SetVideoMode( rs );
#endif
		return true;
	}
	
	bool OnInitialize()
	{
		puts( "\n === subsystem tests ===\n" );
		ShaderDefTest();
		puts( "\n === initial testing finished ===\n" );
		
		Game_FileSystems().insert( 0, new BasicFileSystem( "../data-test" ) );
		
		GR2D_LoadFont( "core", "fonts/lato-regular.ttf" );
		GR2D_LoadFont( "mono", "fonts/dejavu-sans-mono-regular.ttf" );
		
		Game_RegisterAction( &MOVE_LEFT );
		Game_RegisterAction( &MOVE_RIGHT );

		Game_BindKeyToAction( SDLK_F2, &MOVE_LEFT );
		Game_BindKeyToAction( SDLK_F3, &MOVE_RIGHT );
		
		InitTest();
		return true;
	}
	void OnDestroy()
	{
		FreeTest();
	}
	void OnTick( float dt, uint32_t gametime )
	{
#define MAX_TICK_SIZE (1.0f/15.0f)
#define FIXED_TICK_SIZE (1.0f/30.0f)
		m_accum += dt;
		while( m_accum >= 0 )
		{
			g_Tests[ g_CurTest ]->FixedTick( FIXED_TICK_SIZE );
			m_accum -= FIXED_TICK_SIZE;
		}
		
		if( MOVE_LEFT.IsPressed() )
		{
			SetTest( ( g_CurTest + TESTCOUNT - 1 ) % TESTCOUNT );
		}
		if( MOVE_RIGHT.IsPressed() )
		{
			SetTest( ( g_CurTest + 1 ) % TESTCOUNT );
		}
		g_Tests[ g_CurTest ]->Do( dt, ( m_accum + FIXED_TICK_SIZE ) / FIXED_TICK_SIZE );
		
		m_frameCount++;
		if( gametime >= m_lastTime + 1000 )
		{
			char bfr[ 4096 ];
			ITest* T = g_Tests[ g_CurTest ];
			sgrx_snprintf( bfr, 4096, "Test #%d: %s | ms: %f | FPS: %u",
				(g_CurTest+1), StackString<4096>(T->GetName()).str,
				1000.0f / m_frameCount, unsigned(m_frameCount) );
			Window_SetTitle( bfr );
			m_lastTime = gametime;
			m_frameCount = 0;
		}
	}
	void OnEvent( const Event& e )
	{
		g_Tests[ g_CurTest ]->OnEvent( e );
	}
	
	float m_accum;
	uint32_t m_lastTime;
	uint32_t m_frameCount;
	
	
	void ShaderDefTest()
	{
		puts( "- shader definition" );
		
		SGRX_XShaderDef def;
		ASSERT( def.LoadText( "[[pass]] one\n[[endpass]]\n" ) );
		ASSERT( def.passes.size() == 1 );
		ASSERT( def.passes[ 0 ].name == "one" );
		ASSERT( def.passes[ 0 ].order == 0 );
		ASSERT( def.passes[ 0 ].render_state.wireFill == false );
		
		ASSERT( def.LoadText( "[[pass]] two\nOrder 15\nWireFill True\n[[endpass]]\n" ) );
		ASSERT( def.passes.size() == 1 );
		ASSERT( def.passes[ 0 ].name == "two" );
		ASSERT( def.passes[ 0 ].order == 15 );
		ASSERT( def.passes[ 0 ].render_state.wireFill == true );
		
		ASSERT( def.LoadText( "[[pass]] one\n"
			"WireFill True\n"
			"Enabled False\n"
			"[[endpass]]\n"
			"[[pass]] two\n"
			"Inherit one\n"
			"[[endpass]]\n" ) );
		ASSERT( def.passes.size() == 1 );
		ASSERT( def.passes[ 0 ].name == "two" );
		ASSERT( def.passes[ 0 ].render_state.wireFill == true );
		
		ASSERT( def.LoadText( "\n[[shader]]\n\n something \n[[endshader]]" ) );
		ASSERT( def.passes.size() == 0 );
		// all newlines preceding shader must go to it
		ASSERT( def.shader == "\n\n\n something \n" );
		
		puts( "--> OK" );
	}
}
g_Game;


extern "C" EXPORT IGame* CreateGame()
{
	return &g_Game;
}



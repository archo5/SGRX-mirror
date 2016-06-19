

#pragma once
#include "engine.hpp"



enum SetFactorMode
{
	SFM_Set,
	SFM_Add,
	SFM_AddDist,
};

ENGINE_EXPORT void GR_ClearFactors( const ArrayView< float >& out, float factor );
ENGINE_EXPORT void GR_SetFactors( const ArrayView< float >& out, const MeshHandle& mesh, const StringView& name, float factor, bool ch = true, uint8_t mode = SFM_Set );
ENGINE_EXPORT void GR_FindBones( int* subbones, int& numsb, const MeshHandle& mesh, const StringView& name, bool ch );



struct IF_GCC(ENGINE_EXPORT) SGRX_Animation : SGRX_RCRsrc
{
	struct Track
	{
		String name;
		uint32_t offset;
		uint16_t posFrames;
		uint16_t rotFrames;
		uint16_t sclFrames;
		uint16_t flags; // reserved, use 0
		
		template< class T > void Serialize( T& arch )
		{
			arch.smallString( name );
			arch << offset;
			arch << posFrames;
			arch << rotFrames;
			arch << sclFrames;
			arch << flags;
		}
	};
	struct Marker
	{
		char name[ MAX_ANIM_MARKER_NAME_LENGTH ]; // engine.hpp
		uint16_t frame;
		
		StringView GetName()
		{
			return StringView( name, sgrx_snlen( name, MAX_ANIM_MARKER_NAME_LENGTH ) );
		}
		
		template< class T > void Serialize( T& arch )
		{
			arch.memory( name, MAX_ANIM_MARKER_NAME_LENGTH );
			arch << frame;
		}
	};
	
	ENGINE_EXPORT SGRX_Animation();
	ENGINE_EXPORT ~SGRX_Animation();
	
	ENGINE_EXPORT int FindTrackID( StringView name );
	ENGINE_EXPORT void GetState( int track, float framePos, Vec3& outpos, Quat& outrot, Vec3& outscl );
	ENGINE_EXPORT bool CheckMarker( const StringView& name, float fp0, float fp1 );
	
	ENGINE_EXPORT void ClearTracks();
	ENGINE_EXPORT void AddTrack( StringView name, Vec3SAV pos, QuatSAV rot, Vec3SAV scl );
	
	template< class T > void Serialize( T& arch )
	{
		uint32_t anim_chunk = arch.beginChunk( "ANIM" );
		arch << speed;
		arch << frameCount;
		arch << data;
		arch << tracks;
		arch << markers;
		arch.endChunk( anim_chunk );
	}
	
	FINLINE float GetAnimTime(){ return safe_fdiv( frameCount - 1, speed ); }
	
	Array< float > data;
	Array< Track > tracks;
	Array< Marker > markers;
	float speed;
	uint16_t frameCount;
};

struct AnimHandle : Handle< SGRX_Animation >
{
	AnimHandle() : Handle(){}
	AnimHandle( const AnimHandle& h ) : Handle( h ){}
	AnimHandle( SGRX_Animation* anm ) : Handle( anm ){}
};

struct SGRX_AnimBundle
{
	Array< AnimHandle > anims;
	
	ENGINE_EXPORT void Serialize( ByteReader& arch );
	ENGINE_EXPORT void Serialize( ByteWriter& arch );
};

// Track-track mapping for animators to find the matching track indices
typedef HashTable< AnimHandle, int* > AnimCache;

struct AnimInfo
{
	uint32_t frameID; // to prevent repetition of work
	Mat4 rootXF;
};
#define ANIMATOR_ADVANCE_FRAME_CHECK( info ) \
	if( (info)->frameID == m_frameID ) return; \
	m_frameID = (info)->frameID;

struct AnimTrackXForm
{
	Vec3 pos;
	Quat rot;
	Vec3 scl;
	
	FINLINE void Reset()
	{
		pos = V3(0);
		rot = Quat::Identity;
		scl = V3(1);
	}
	FINLINE void SetAdd( const AnimTrackXForm& a, const AnimTrackXForm& b )
	{
		pos = a.pos + b.pos;
		rot = a.rot * b.rot;
		scl = a.scl * b.scl;
	}
	FINLINE void SetLerp( const AnimTrackXForm& a, const AnimTrackXForm& b, float s )
	{
		pos = TLERP( a.pos, b.pos, s );
		rot = TLERP( a.rot, b.rot, s );
		scl = TLERP( a.scl, b.scl, s );
	}
	FINLINE void SetMatrix( const Mat4& m )
	{
		pos = m.GetTranslation();
		rot = m.GetRotationQuaternion();
		scl = m.GetScale();
	}
	FINLINE Mat4 GetSRT() const
	{
		return Mat4::CreateSRT( scl, rot, pos );
	}
};
struct AnimTrackFrame : AnimTrackXForm
{
	float fq;
	
	FINLINE void Reset( float f = 0 )
	{
		AnimTrackXForm::Reset();
		fq = f;
	}
};
typedef ArrayView< AnimTrackFrame > ATFrameView;

struct IF_GCC(ENGINE_EXPORT) Animator
{
	Animator() : m_frameID( 0 ){}
	virtual void Prepare(){}
	virtual void Advance( float deltaTime, AnimInfo* info ){}
	
	MeshHandle m_mesh;
	ATFrameView m_pose;
	uint32_t m_frameID;
};

struct IF_GCC(ENGINE_EXPORT) AnimMask : Animator
{
	AnimMask() : animSource( NULL ){}
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	Animator* animSource;
	Array< float > blendFactors;
};

enum AnimBlendMode
{
	ABM_Normal = 0,
	ABM_Additive = 1,
};

struct IF_GCC(ENGINE_EXPORT) AnimBlend : Animator
{
	AnimBlend() : animSourceA( NULL ), animSourceB( NULL ), blendFactor( 1 ), blendMode( ABM_Normal ){}
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	Animator* animSourceA;
	Animator* animSourceB;
	float blendFactor;
	uint8_t blendMode;
};

#define ANIM_RELABS_INVERT 0x01
#define ANIM_RELABS_POSOFF 0x02
struct IF_GCC(ENGINE_EXPORT) AnimRelAbs : Animator
{
	AnimRelAbs() : animSource( NULL ), flags( 0 ){}
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	Array< Mat4 > m_tmpMtx;
	
	Animator* animSource;
	uint8_t flags;
};

struct IF_GCC(ENGINE_EXPORT) AnimMixer : Animator
{
	enum TransformFlags
	{
		TF_None = 0,
		TF_Absolute_Rot = 0x01,
		TF_Absolute_Pos = 0x02,
		TF_Absolute_Scale = 0x04,
		TF_Absolute_All = (TF_Absolute_Rot|TF_Absolute_Pos|TF_Absolute_Scale),
		TF_Additive = 0x10,
		TF_IgnoreMeshXF = 0x20,
	};
	
	struct Layer
	{
		Layer() : anim(NULL), factor(1), tflags(0){}
		
		Animator* anim;
		float factor;
		int tflags;
	};
	
	ENGINE_EXPORT AnimMixer();
	ENGINE_EXPORT ~AnimMixer();
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	Array< Mat4 > m_staging;
	
	Layer* layers; // INTERFACE
	int layerCount; // INTERFACE
};

struct IF_GCC(ENGINE_EXPORT) AnimPlayer : Animator
{
	struct Anim
	{
		AnimHandle anim;
		int* trackIDs;
		float at;
		float fade_at;
		float prev_fade_at;
		float fadetime;
		float speed;
		bool once;
	};
	
	ENGINE_EXPORT AnimPlayer();
	ENGINE_EXPORT ~AnimPlayer();
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	ENGINE_EXPORT void Play( const AnimHandle& anim, bool once = false, float fadetime = 0.5f );
	ENGINE_EXPORT void Stop();
	ENGINE_EXPORT bool CheckMarker( const StringView& name );
	FINLINE void SetLastAnimSpeed( float s ){ if( m_currentAnims.size() ) m_currentAnims.last().speed = s; }
	ENGINE_EXPORT float GetLastAnimBlendFactor() const;
	
	ENGINE_EXPORT int* _getTrackIds( const AnimHandle& anim );
	ENGINE_EXPORT void _clearAnimCache();
	
	AnimCache m_animCache;
	Array< Anim > m_currentAnims;
	Array< float > m_blendFactors;
};

struct IF_GCC(ENGINE_EXPORT) AnimInterp : Animator
{
	ENGINE_EXPORT AnimInterp();
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	ENGINE_EXPORT void Transfer();
	ENGINE_EXPORT void Interpolate( float deltaTime );
	
	Array< AnimTrackXForm > m_prev_pose;
	
	Animator* animSource; // INTERFACE
};

struct IF_GCC(ENGINE_EXPORT) AnimDeformer : Animator
{
	struct Force
	{
		int boneID; // -1 for world
		Vec3 pos; // in bone/world space
		Vec3 dir; // normalized in bone/world space
		float amount; // force = dir * amount
		float radius; // distf = 1 - min( 1, dist / radius )
		float power; // pow( distf, ->power<- )
		float lifetime; // how long since it's been created
	};
	
	ENGINE_EXPORT AnimDeformer();
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	ENGINE_EXPORT void AddLocalForce( const Vec3& pos, const Vec3& dir,
		float rad, float power = 1, float amount = 0.0f );
	ENGINE_EXPORT void AddModelForce( const Vec3& pos, const Vec3& dir,
		float rad, float power = 1, float amount = 0.0f );
	ENGINE_EXPORT int _FindClosestBone( const Vec3& pos );
	ENGINE_EXPORT void _UpdatePoseInfo();
	
	int GetParentBoneID( int i ) const
	{
		if( i < 0 || i >= m_mesh->m_numBones )
			return -1;
		return m_mesh->m_bones[ i ].parent_id;
	}
	
	Array< Mat4 > m_skinOffsets;
	Array< Mat4 > m_invSkinOffsets;
	Array< Vec3 > m_bonePositions;
	
	// INTERFACE
	Array< Force > forces;
	Animator* animSource;
	int numIterations;
	int numConstraintIterations;
};



ENGINE_EXPORT bool GR_ReadAnimBundle( const StringView& path, SGRX_AnimBundle& out );
ENGINE_EXPORT bool GR_EnumAnimBundle( const StringView& path, Array< RCString >& out );
ENGINE_EXPORT AnimHandle GR_GetAnim( const StringView& name );
ENGINE_EXPORT bool GR_ApplyAnimator( const Animator* animator, Mat4* out, size_t outsz, bool applyinv = true, Mat4* base = NULL );
ENGINE_EXPORT bool GR_ApplyAnimator( const Animator* animator, SGRX_MeshInstance* meshinst );



struct IF_GCC(ENGINE_EXPORT) SGRX_IPSRaycast
{
	struct DATA_IN
	{
		Vec3 p1;
		Vec3 p2;
		uint32_t isect_fx;
	};
	struct DATA_OUT
	{
		Vec3 normal;
		float factor;
	};
	
	ENGINE_EXPORT virtual void Raycast( DATA_IN* rays, DATA_OUT* isects, size_t count ) = 0;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_ScenePSRaycast : SGRX_IPSRaycast
{
	ENGINE_EXPORT SGRX_ScenePSRaycast();
	ENGINE_EXPORT virtual void Raycast( DATA_IN* rays, DATA_OUT* isects, size_t count );
	ENGINE_EXPORT virtual void DoFX( const Vec3& pos, const Vec3& nrm, uint32_t fx ); // empty
	
	SceneHandle scene;
	uint32_t layers;
};

#define PARTICLESYSTEM_VERSION 5
// 1: initial version
// 2: added group count
// 3: added global scale
// 4: added gravity multiplier
// 5: added intersection attribs [limit, friction, bounce, fx, remove]
// 6: added intersection FX chance

#define PARTICLE_VDECL "pf3cf40b4"
#define NUM_PARTICLE_TEXTURES 4

struct IF_GCC(ENGINE_EXPORT) ParticleSystem : SGRX_RefCounted, SGRX_IEventHandler
{
	struct Vertex
	{
		Vec3 pos;
		Vec4 color;
		uint8_t u;
		uint8_t v;
		uint16_t pad;
	};
	
	struct ps_prerender_info
	{
		SceneHandle scene;
		Mat4 transform;
		Mat4 viewProj;
		Vec3 basis[3];
	};
	
	struct Group
	{
		Vec3 color;
	};
	
	struct Emitter
	{
		struct Curve
		{
			Curve() : randomValDvg(V2(0)), valueRange(V2(0)){}
			
			Array< Vec2 > values;
			Vec2 randomValDvg;
			Vec2 valueRange;
			
			template< class T > void Serialize( T& arch )
			{
				arch << values;
				arch << randomValDvg;
				arch << valueRange;
			}
			
			FINLINE float GetValue( float t, float r )
			{
				if( values.size() == 0 )
					return valueRange.x;
				size_t end = values.size() - 1;
				if( t < 0 ) t = 0;
				size_t i = t * end;
				if( i >= end ) i = end;
				size_t i1 = i + 1;
				if( i1 >= end ) i1 = end;
				float q = t * end - i;
				Vec2 interp = TLERP( values[ i ], values[ i1 ], q );
				return TLERP( valueRange.x, valueRange.y, interp.x + interp.y * r );
			}
		};
		
		Array< Vec3 > particles_Position;
		Array< Vec3 > particles_Velocity;
		Array< Vec2 > particles_Lifetime; // 0-1, increment
		Array< Vec3 > particles_RandSizeAngVel;
		Array< Vec4 > particles_RandColor; // HSV, opacity
		Array< uint16_t > particles_Group;
		
		Curve curve_Size;
		Curve curve_ColorHue;
		Curve curve_ColorSat;
		Curve curve_ColorVal;
		Curve curve_Opacity;
		
		int spawn_MaxCount;
		int spawn_Count;
		int spawn_CountExt;
		Vec2 spawn_TimeExt;
		
		Vec3 create_Pos;
		Vec3 create_PosBox;
		float create_PosRadius;
		Vec3 create_VelMicroDir;
		float create_VelMicroDvg;
		Vec2 create_VelMicroDistExt;
		Vec3 create_VelMacroDir;
		float create_VelMacroDvg;
		Vec2 create_VelMacroDistExt;
		int create_VelCluster;
		int create_VelClusterExt;
		Vec2 create_LifetimeExt;
		Vec2 create_AngleDirDvg;
		Vec2 create_AngleVelDvg;
		
		float tick_AngleAcc;
		float tick_GravityMult;
		bool absolute;
		
		int isect_Limit;
		float isect_Friction;
		float isect_Bounce;
		uint32_t isect_FXID;
		float isect_FXChance;
		bool isect_Remove;
		
		TextureHandle render_Textures[ NUM_PARTICLE_TEXTURES ];
		String render_Shader;
		bool render_Additive;
		bool render_Stretch;
		
		int state_SpawnTotalCount;
		int state_SpawnCurrCount;
		float state_SpawnTotalTime;
		float state_SpawnCurrTime;
		float state_lastDelta;
		
		Emitter() :
			spawn_MaxCount(100), spawn_Count(10), spawn_CountExt(2), spawn_TimeExt(V2(1,0.2f)),
			create_Pos(V3(0)), create_PosBox(V3(0)), create_PosRadius(0),
			create_VelMicroDir(V3(0,0,1)), create_VelMicroDvg(0), create_VelMicroDistExt(V2(0)),
			create_VelMacroDir(V3(0,0,1)), create_VelMacroDvg(0), create_VelMacroDistExt(V2(1,0.1f)),
			create_VelCluster(1), create_VelClusterExt(0),
			create_LifetimeExt(V2(3,0.1f)), create_AngleDirDvg(V2(0,(float)M_PI)), create_AngleVelDvg(V2(0)),
			tick_AngleAcc(0), tick_GravityMult(1), absolute(false),
			isect_Limit(0), isect_Friction(0), isect_Bounce(1), isect_FXID(0), isect_FXChance(1), isect_Remove(false),
			render_Shader("particle"), render_Additive(false), render_Stretch(false),
			state_SpawnTotalCount(0), state_SpawnCurrCount(0), state_SpawnTotalTime(0), state_SpawnCurrTime(0),
			state_lastDelta(0)
		{}
		
		template< class T > void Serialize( T& arch, bool incl_state )
		{
			arch.marker( "EMITTER" );
			
			arch << curve_Size;
			arch << curve_ColorHue;
			arch << curve_ColorSat;
			arch << curve_ColorVal;
			arch << curve_Opacity;
			
			arch << spawn_MaxCount;
			arch << spawn_Count;
			arch << spawn_CountExt;
			arch << spawn_TimeExt;
			
			arch << create_Pos;
			arch << create_PosBox;
			arch << create_PosRadius;
			arch << create_VelMicroDir;
			arch << create_VelMicroDvg;
			arch << create_VelMicroDistExt;
			arch << create_VelMacroDir;
			arch << create_VelMacroDvg;
			arch << create_VelMacroDistExt;
			arch << create_VelCluster;
			arch << create_VelClusterExt;
			arch << create_LifetimeExt;
			arch << create_AngleDirDvg;
			arch << create_AngleVelDvg;
			
			arch << tick_AngleAcc;
			arch( tick_GravityMult, arch.version >= 4, 1 );
			arch << absolute;
			
			arch( isect_Limit, arch.version >= 5, 0 );
			arch( isect_Friction, arch.version >= 5, 0.0f );
			arch( isect_Bounce, arch.version >= 5, 1.0f );
			arch( isect_FXID, arch.version >= 5, 0 );
			arch( isect_FXChance, arch.version >= 6, 1.0f );
			arch( isect_Remove, arch.version >= 5, false );
			
			for( int i = 0; i < NUM_PARTICLE_TEXTURES; ++i )
			{
				if( T::IsReader )
				{
					bool hastex = false;
					arch << hastex;
					if( hastex )
					{
						String texname;
						arch << texname;
						render_Textures[ i ] = GR_GetTexture( texname );
					}
				}
				else
				{
					bool hastex = !!render_Textures[ i ];
					arch << hastex;
					if( hastex )
						arch << render_Textures[ i ]->m_key;
				}
			}
			arch << render_Shader;
			arch << render_Additive;
			arch << render_Stretch;
			
			if( incl_state )
			{
				arch << state_SpawnTotalCount;
				arch << state_SpawnCurrCount;
				arch << state_SpawnTotalTime;
				arch << state_SpawnCurrTime;
				arch << state_lastDelta;
				
				arch << particles_Position;
				arch << particles_Velocity;
				arch << particles_Lifetime;
				arch << particles_RandSizeAngVel;
				arch << particles_RandColor;
			}
			else if( T::IsReader )
			{
				// clear state on read data
				state_SpawnTotalCount = 0;
				state_SpawnCurrCount = 0;
				state_SpawnTotalTime = 0;
				state_SpawnCurrTime = 0;
				state_lastDelta = 0;
				
				particles_Position.clear();
				particles_Velocity.clear();
				particles_Lifetime.clear();
				particles_RandSizeAngVel.clear();
				particles_RandColor.clear();
				particles_Group.clear();
			}
		}
		
		void Tick( ParticleSystem* PS, float dt );
		void Generate( ParticleSystem* PS, int count );
		void Trigger( ParticleSystem* PS );
		
		void PreRender( ParticleSystem* PS, ps_prerender_info& info );
	};
	
	Array< Emitter > emitters;
	Vec3 gravity;
	uint16_t maxGroupCount;
	float globalScale;
	bool looping;
	Vec2 retriggerTimeExt;
	
	bool m_isPlaying;
	float m_retriggerTime;
	uint16_t m_nextGroup;
	Mat4 m_transform;
	SGRX_LightSampler* m_lightSampler;
	SGRX_IPSRaycast* m_psRaycast;
	
	Array< Group > m_groups;
	Array< Vertex > m_vertices;
	Array< uint16_t > m_indices;
	SceneHandle m_scene;
	VertexDeclHandle m_vdecl;
	
	Array< MeshInstHandle > m_meshInsts;
	
	ParticleSystem();
	
	template< class T > void Serialize( T& arch, bool incl_state )
	{
		arch.marker( "SGRX_PARTSYS" );
		
		SerializeVersionHelper<T> svh( arch, PARTICLESYSTEM_VERSION );
		
		uint32_t emcnt = 0;
		if( T::IsReader )
		{
			svh << emcnt;
			emitters.resize( emcnt );
		}
		else
		{
			emcnt = emitters.size();
			svh << emcnt;
		}
		for( uint32_t i = 0; i < emcnt; ++i )
		{
			emitters[ i ].Serialize( svh, incl_state );
		}
		
		svh << gravity;
		svh( maxGroupCount, svh.version >= 2, 10 );
		svh( globalScale, svh.version >= 3, 1 );
		svh << looping;
		svh << retriggerTimeExt;
		
		if( incl_state )
		{
			svh << m_isPlaying;
			svh << m_retriggerTime;
			svh << m_transform;
		}
		else if( T::IsReader )
		{
			m_isPlaying = false;
			m_retriggerTime = 0;
			m_transform = Mat4::Identity;
		}
		
		if( T::IsReader )
			OnRenderUpdate();
	}
	ENGINE_EXPORT bool Load( const StringView& sv );
	ENGINE_EXPORT bool Save( const StringView& sv );
	
	ENGINE_EXPORT void HandleEvent( SGRX_EventID eid, const EventData& edata );
	
	ENGINE_EXPORT void OnRenderUpdate();
	ENGINE_EXPORT void AddToScene( SceneHandle sh );
	ENGINE_EXPORT void SetTransform( const Mat4& mtx );
	
	ENGINE_EXPORT bool Tick( float dt );
	ENGINE_EXPORT void _PreRender();
	
	ENGINE_EXPORT void Trigger();
	ENGINE_EXPORT void Play();
	ENGINE_EXPORT void Stop();
};
typedef Handle< ParticleSystem > PartSysHandle;




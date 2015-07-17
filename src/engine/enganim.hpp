

#pragma once
#include "engine.hpp"



ENGINE_EXPORT void GR_ClearFactors( Array< float >& out, float factor );
ENGINE_EXPORT void GR_SetFactors( Array< float >& out, const MeshHandle& mesh, const StringView& name, float factor, bool ch = true );
ENGINE_EXPORT void GR_FindBones( int* subbones, int& numsb, const MeshHandle& mesh, const StringView& name, bool ch );


struct IF_GCC(ENGINE_EXPORT) SGRX_Animation
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	ENGINE_EXPORT ~SGRX_Animation();
	
	ENGINE_EXPORT Vec3* GetPosition( int track );
	ENGINE_EXPORT Quat* GetRotation( int track );
	ENGINE_EXPORT Vec3* GetScale( int track );
	
	ENGINE_EXPORT void GetState( int track, float framePos, Vec3& outpos, Quat& outrot, Vec3& outscl );
	
	String name;
	int frameCount;
	float speed;
	size_t nameSize;
	Array< float > data;
	Array< String > trackNames;
	
	int32_t _refcount;
};

struct AnimHandle : Handle< SGRX_Animation >
{
	AnimHandle() : Handle(){}
	AnimHandle( const AnimHandle& h ) : Handle( h ){}
	AnimHandle( SGRX_Animation* anm ) : Handle( anm ){}
};

// Track-track mapping for animators to find the matching track indices
typedef HashTable< AnimHandle, int* > AnimCache;

struct IF_GCC(ENGINE_EXPORT) Animator
{
	ENGINE_EXPORT virtual void Prepare( String* new_names, int count );
	ENGINE_EXPORT bool PrepareForMesh( const MeshHandle& mesh );
	virtual void Advance( float deltaTime ){}
	
	void ClearFactors( float f ){ GR_ClearFactors( factor, f ); }
	void SetFactors( const MeshHandle& mesh, const StringView& name, float f, bool ch = true ){ GR_SetFactors( factor, mesh, name, f, ch ); }
	ENGINE_EXPORT virtual Array< float >& GetBlendFactorArray();
	
	Array< String > names;
	Array< Vec3 > position;
	Array< Quat > rotation;
	Array< Vec3 > scale;
	Array< float > factor;
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
	ENGINE_EXPORT virtual void Prepare( String* new_names, int count );
	ENGINE_EXPORT virtual void Advance( float deltaTime );
	
	Array< Mat4 > m_staging;
	MeshHandle mesh;
	
	Layer* layers;
	int layerCount;
};

struct IF_GCC(ENGINE_EXPORT) AnimPlayer : Animator
{
	struct Anim
	{
		AnimHandle anim;
		int* trackIDs;
		float at;
		float fade_at;
		float fadetime;
		bool once;
	};
	
	ENGINE_EXPORT AnimPlayer();
	ENGINE_EXPORT ~AnimPlayer();
	ENGINE_EXPORT virtual void Prepare( String* new_names, int count );
	ENGINE_EXPORT virtual void Advance( float deltaTime );
	
	ENGINE_EXPORT void Play( const AnimHandle& anim, bool once = false, float fadetime = 0.5f );
	
	ENGINE_EXPORT int* _getTrackIds( const AnimHandle& anim );
	ENGINE_EXPORT void _clearAnimCache();
	
	AnimCache animCache;
	Array< Anim > currentAnims;
	Array< float > blendFactor;
	
	void ClearBlendFactors( float f ){ GR_ClearFactors( blendFactor, f ); }
	void SetBlendFactors( const MeshHandle& mesh, const StringView& name, float f, bool ch = true ){ GR_SetFactors( blendFactor, mesh, name, f, ch ); }
	ENGINE_EXPORT virtual Array< float >& GetBlendFactorArray();
};

struct IF_GCC(ENGINE_EXPORT) AnimInterp : Animator
{
	ENGINE_EXPORT AnimInterp();
	ENGINE_EXPORT virtual void Prepare( String* new_names, int count );
	ENGINE_EXPORT virtual void Advance( float deltaTime );
	ENGINE_EXPORT void Transfer();
	ENGINE_EXPORT void Interpolate( float deltaTime );
	
	Array< Vec3 > prev_position;
	Array< Quat > prev_rotation;
	Array< Vec3 > prev_scale;
	Animator* animSource;
};



struct IF_GCC(ENGINE_EXPORT) AnimCharacter : IMeshRaycast
{
	enum BodyType
	{
		BodyType_None = 0,
		BodyType_Sphere = 1,
		BodyType_Capsule = 2,
		BodyType_Box = 3,
	};
	enum JointType
	{
		JointType_None = 0,
		JointType_Hinge = 1,
	};
	enum TransformType
	{
		TransformType_None = 0,
		TransformType_UndoParent = 1,
		TransformType_Move = 2,
		TransformType_Rotate = 3,
	};
	
	struct HitBox
	{
		Quat rotation;
		Vec3 position;
		Vec3 extents;
		float multiplier;
		
		HitBox() : rotation( Quat::Identity ), position( V3(0) ),
			extents( V3(0.1f) ), multiplier( 1 ){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "HITBOX" );
			arch( rotation );
			arch( position );
			arch( extents );
			arch( multiplier );
		}
	};
	struct Body
	{
		Quat rotation;
		Vec3 position;
		uint8_t type; // BodyType
		Vec3 size; // x = radius, z = capsule height
		
		Body() : rotation( Quat::Identity ), position( V3(0) ),
			type( BodyType_None ), size( V3(0.1f) ){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "BODY" );
			arch( type );
			arch( rotation );
			arch( position );
			arch( size );
		}
	};
	struct Joint
	{
		String parent_name;
		uint8_t type; // JointType
		Vec3 local_offset1;
		Vec3 local_offset2;
		
		Joint() : type( JointType_None ), local_offset1( V3(0) ),
			local_offset2( V3(0) ){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "JOINT" );
			arch( type );
			arch( parent_name );
			arch( local_offset1 );
			arch( local_offset2 );
		}
	};
	
	struct BoneInfo
	{
		String name;
		HitBox hitbox;
		Body body;
		Joint joint;
		
		int bone_id;
		
		BoneInfo() : bone_id(-1){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "BONEINFO" );
			arch( name );
			arch( hitbox );
			arch( body );
		//	arch( joint );
		}
	};
	
	struct Attachment
	{
		String name;
		String bone;
		Quat rotation;
		Vec3 position;
		
		int bone_id;
		
		Attachment() : rotation( Quat::Identity ), position( V3(0) ), bone_id(-1){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "ATTACHMENT" );
			arch( name );
			arch( bone );
			arch( rotation );
			arch( position );
		}
	};
	
	struct LayerTransform
	{
		String bone;
		uint8_t type; // TransformType
		Vec3 posaxis; // offset for 'move', axis for 'rotate'
		float angle; // only for rotation
		
		int bone_id;
		
		LayerTransform() : type( TransformType_None ), posaxis( V3(0,0,1) ),
			angle( 0 ), bone_id(-1){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "LAYERXF" );
			arch( type );
			arch( bone );
			arch( posaxis );
			arch( angle );
		}
	};
	struct Layer
	{
		String name;
		Array< LayerTransform > transforms;
		
		float amount;
		
		Layer() : amount( 0 ){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "LAYER" );
			arch( name );
			arch( transforms );
		}
	};
	
	struct MaskCmd
	{
		String bone;
		float weight;
		bool children;
		
		MaskCmd() : weight( 0 ), children( true ){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "MASKCMD" );
			arch( bone );
			arch( weight );
			arch( children );
		}
	};
	struct Mask
	{
		String name;
		Array< MaskCmd > cmds;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "MASK" );
			arch( name );
			arch( cmds );
		}
	};
	
	template< class T > void Serialize( T& basearch )
	{
		basearch.marker( "SGRXCHAR" );
		
		// 1: initial
		// 2: added masks
		SerializeVersionHelper<T> arch( basearch, 2 );
		
		arch( mesh );
		arch( bones );
		arch( attachments );
		arch( layers );
		arch( masks, arch.version >= 2 );
	}
	
	ENGINE_EXPORT AnimCharacter();
	
	ENGINE_EXPORT bool Load( const StringView& sv );
	ENGINE_EXPORT bool Save( const StringView& sv );
	
	ENGINE_EXPORT void OnRenderUpdate();
	ENGINE_EXPORT void AddToScene( SceneHandle sh );
	ENGINE_EXPORT void SetTransform( const Mat4& mtx );
	
	ENGINE_EXPORT void FixedTick( float deltaTime );
	ENGINE_EXPORT void PreRender( float blendFactor );
	ENGINE_EXPORT void RecalcLayerState();
	
	ENGINE_EXPORT int _FindBone( const StringView& name );
	ENGINE_EXPORT void RecalcBoneIDs();
	ENGINE_EXPORT bool GetBodyMatrix( int which, Mat4& outwm );
	ENGINE_EXPORT bool GetHitboxOBB( int which, Mat4& outwm, Vec3& outext );
	ENGINE_EXPORT bool GetAttachmentMatrix( int which, Mat4& outwm );
	ENGINE_EXPORT bool ApplyMask( const StringView& name, Animator* tgt );
	ENGINE_EXPORT int FindAttachment( const StringView& name );
	ENGINE_EXPORT void SortEnsureAttachments( const StringView* atchnames, int count );
	
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, struct SGRX_MeshInstance* cbmi = NULL );
	
	String mesh;
	Array< BoneInfo > bones;
	Array< Attachment > attachments;
	Array< Layer > layers;
	Array< Mask > masks;
	
	SceneHandle m_scene;
	MeshHandle m_cachedMesh;
	MeshInstHandle m_cachedMeshInst;
	Animator m_layerAnimator;
	AnimMixer m_anMixer;
	AnimInterp m_anEnd;
};



struct SkeletonInfo
{
	enum BodyType
	{
		BodyType_Capsule = 1
	};
	
	struct HitBox
	{
		String name;
		Quat rotation;
		Vec3 position;
		Vec3 xdir, ydir, zdir;
		Vec3 extents;
		float multiplier;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name );
			arch( rotation );
			arch( position );
			arch( xdir ); arch( ydir ); arch( zdir );
			arch( extents );
			arch( multiplier );
		}
	};
	
	struct Body
	{
		String name;
		Quat rotation;
		Vec3 position;
		uint8_t type;
		float capsule_radius;
		float capsule_height;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name );
			arch( rotation );
			arch( position );
			arch( type );
			if( type == BodyType_Capsule )
			{
				arch( capsule_radius );
				arch( capsule_height );
			}
		}
	};
	
	struct Joint
	{
		String name1;
		String name2;
		Vec3 local_offset1;
		Vec3 local_offset2;
		uint8_t type;
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch( name1 );
			arch( name2 );
			arch( local_offset1 );
			arch( local_offset2 );
			arch( type );
		}
	};
	
	Array< HitBox > hitboxes;
	Array< Body > bodies;
	Array< Joint > joints;
	
	template< class T > void Serialize( T& arch )
	{
		arch.marker( "SGRXSKRI" );
		SerializeVersionHelper<T> vh( arch, 1 );
		vh( hitboxes );
		vh( bodies );
		vh( joints );
	}
};



ENGINE_EXPORT int GR_LoadAnims( const StringView& path, const StringView& prefix = StringView() );
ENGINE_EXPORT AnimHandle GR_GetAnim( const StringView& name );
ENGINE_EXPORT bool GR_ApplyAnimator( const Animator* animator, MeshHandle mh, Mat4* out, size_t outsz, bool applyinv = true, Mat4* base = NULL );
ENGINE_EXPORT bool GR_ApplyAnimator( const Animator* animator, MeshInstHandle mih );



#define PARTICLESYSTEM_VERSION 2
// 1: initial version
// 2: added group count

#define PARTICLE_VDECL "pf3cf40b4"
#define NUM_PARTICLE_TEXTURES 4

struct IF_GCC(ENGINE_EXPORT) ParticleSystem : SGRX_DummyLightSampler
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
		bool absolute;
		
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
			tick_AngleAcc(0), absolute(false),
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
			arch << absolute;
			
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
		
		void Tick( ParticleSystem* PS, float dt, const Vec3& accel, const Mat4& mtx );
		void Generate( int count, const Mat4& mtx, uint16_t group );
		void Trigger( const Mat4& mtx, uint16_t group );
		
		void PreRender( ParticleSystem* PS, ps_prerender_info& info );
	};
	
	Array< Emitter > emitters;
	Vec3 gravity;
	uint16_t maxGroupCount;
	bool looping;
	Vec2 retriggerTimeExt;
	
	bool m_isPlaying;
	float m_retriggerTime;
	uint16_t m_nextGroup;
	Mat4 m_transform;
	SGRX_LightSampler* m_lightSampler;
	
	Array< Group > m_groups;
	Array< Vertex > m_vertices;
	Array< uint16_t > m_indices;
	SceneHandle m_scene;
	VertexDeclHandle m_vdecl;
	
	Array< MeshInstHandle > m_meshInsts;
	
	ParticleSystem() :
		gravity(V3(0,0,-10)), maxGroupCount(10), looping(true),
		retriggerTimeExt(V2(1,0.1f)),
		m_isPlaying(false), m_retriggerTime(0), m_nextGroup(0),
		m_transform(Mat4::Identity), m_lightSampler(NULL)
	{}
	
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
	
	ENGINE_EXPORT void OnRenderUpdate();
	ENGINE_EXPORT void AddToScene( SceneHandle sh );
	ENGINE_EXPORT void SetTransform( const Mat4& mtx );
	
	ENGINE_EXPORT bool Tick( float dt );
	ENGINE_EXPORT void PreRender();
	
	ENGINE_EXPORT void Trigger();
	ENGINE_EXPORT void Play();
	ENGINE_EXPORT void Stop();
};



struct DecalMapPartInfo
{
	Vec4 bbox; // left, top, right, bottom / x0,y0,x1,y1
	Vec3 size; // width / height / depth
};

struct DecalProjectionInfo
{
//	DecalProjectionInfo() : pos(V3(0)), dir(V3(0,1,0)), up(V3(0,0,1)),
//		fovAngleDeg(90), orthoScale(1), aspectMult(1), aamix(0.5f),
//		distanceScale(1.0f), perspective(false)
//	{}
//	
	Vec3 pos;
	Vec3 dir;
	Vec3 up;
	float fovAngleDeg; // perspective only
	float orthoScale; // ortho only
	float aspectMult; // perspective only
	float aamix; // perspective only
	float distanceScale;
	float pushBack;
	bool perspective;
};

struct IF_GCC(ENGINE_EXPORT) DecalSystem
{
	ENGINE_EXPORT DecalSystem();
	ENGINE_EXPORT ~DecalSystem();
	
	ENGINE_EXPORT void Init( TextureHandle texDecal, TextureHandle texFalloff, DecalMapPartInfo* decalCoords, int numDecals );
	ENGINE_EXPORT void Free();
	ENGINE_EXPORT void SetSize( uint32_t vbSize );
	
	ENGINE_EXPORT void Upload();
	
	ENGINE_EXPORT void AddDecal( int decalID, SGRX_IMesh* targetMesh, const Mat4& worldMatrix, DecalProjectionInfo* projInfo );
	ENGINE_EXPORT void AddDecal( int decalID, SGRX_IMesh* targetMesh, int partID, const Mat4& worldMatrix, DecalProjectionInfo* projInfo );
	ENGINE_EXPORT void ClearAllDecals();
	ENGINE_EXPORT void GenerateCamera( int decalID, DecalProjectionInfo& projInfo, SGRX_Camera* out );
	
	ENGINE_EXPORT void _ScaleDecalTexcoords( size_t vbfrom, int decalID );
	ENGINE_EXPORT void _GenDecalMatrix( int decalID, DecalProjectionInfo* projInfo, Mat4* outVPM, float* out_invzn2zf );
	
	SGRX_LightSampler* m_lightSampler;
	Array< DecalMapPartInfo > m_decalBounds;
	VertexDeclHandle m_vertexDecl;
	MaterialHandle m_material;
	MeshHandle m_mesh;
	ByteArray m_vertexData;
	UInt32Array m_indexData;
	Array< uint32_t > m_decals;
	uint32_t m_vbSize;
};



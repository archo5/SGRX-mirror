

#pragma once
#include "engine.hpp"



EXPORT void GR_ClearFactors( Array< float >& out, float factor );
EXPORT void GR_SetFactors( Array< float >& out, const MeshHandle& mesh, const StringView& name, float factor );
EXPORT void GR_FindBones( int* subbones, int& numsb, const MeshHandle& mesh, const StringView& name, bool ch );


struct EXPORT SGRX_Animation
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	~SGRX_Animation();
	
	Vec3* GetPosition( int track );
	Quat* GetRotation( int track );
	Vec3* GetScale( int track );
	
	void GetState( int track, float framePos, Vec3& outpos, Quat& outrot, Vec3& outscl );
	
	String name;
	int frameCount;
	float speed;
	size_t nameSize;
	Array< float > data;
	Array< String > trackNames;
	
	int32_t _refcount;
};

struct EXPORT AnimHandle : Handle< SGRX_Animation >
{
	AnimHandle() : Handle(){}
	AnimHandle( const AnimHandle& h ) : Handle( h ){}
	AnimHandle( SGRX_Animation* anm ) : Handle( anm ){}
};

// Track-track mapping for animators to find the matching track indices
typedef HashTable< AnimHandle, int* > AnimCache;

struct EXPORT Animator
{
	virtual void Prepare( String* new_names, int count );
	bool PrepareForMesh( const MeshHandle& mesh );
	virtual void Advance( float deltaTime ){}
	
	void ClearFactors( float f ){ GR_ClearFactors( factor, f ); }
	void SetFactors( const MeshHandle& mesh, const StringView& name, float f ){ GR_SetFactors( factor, mesh, name, f ); }
	
	Array< String > names;
	Array< Vec3 > position;
	Array< Quat > rotation;
	Array< Vec3 > scale;
	Array< float > factor;
};

struct EXPORT AnimMixer : Animator
{
	struct Layer
	{
		Layer() : anim(NULL), factor(1){}
		
		Animator* anim;
		float factor;
	};
	
	AnimMixer();
	~AnimMixer();
	virtual void Prepare( String* names, int count );
	virtual void Advance( float deltaTime );
	
	Layer* layers;
	int layerCount;
};

struct EXPORT AnimPlayer : Animator
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
	
	AnimPlayer();
	~AnimPlayer();
	virtual void Prepare( String* names, int count );
	virtual void Advance( float deltaTime );
	
	void Play( const AnimHandle& anim, bool once = false, float fadetime = 0.5f );
	
	int* _getTrackIds( const AnimHandle& anim );
	void _clearAnimCache();
	
	AnimCache animCache;
	Array< Anim > currentAnims;
	Array< float > blendFactor;
	
	void ClearBlendFactors( float f ){ GR_ClearFactors( blendFactor, f ); }
	void SetBlendFactors( const MeshHandle& mesh, const StringView& name, float f ){ GR_SetFactors( blendFactor, mesh, name, f ); }
};



EXPORT int GR_LoadAnims( const StringView& path, const StringView& prefix = StringView() );
EXPORT AnimHandle GR_GetAnim( const StringView& name );
EXPORT bool GR_ApplyAnimator( const Animator* animator, MeshInstHandle mih );



#define PARTICLE_VDECL "pf3cf40b4"

struct EXPORT ParticleSystem
{
	struct Vertex
	{
		Vec3 pos;
		Vec4 color;
		uint8_t u;
		uint8_t v;
		uint16_t pad;
	};
	
	struct Emitter
	{
		struct Curve
		{
			Curve() : randomValDvg(V2(0)), valueRange(V2(0)){}
			
			Array< Vec2 > values;
			Vec2 randomValDvg;
			Vec2 valueRange;
			
			FINLINE float GetValue( float t, float r )
			{
				if( values.size() == 0 )
					return valueRange.x;
				size_t end = values.size() - 1;
				size_t i = t * end;
				if( i < 0 ) i = 0;
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
		Array< Vec2 > particles_RandSizeAngle;
		Array< Vec4 > particles_RandColor; // HSV, opacity
		
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
		
		// calculated at rendering time using x = x0 + ( v + a * t ) * t
		Vec2 tick_AngleVelAcc;
		bool absolute;
		
		TextureHandle render_Textures[ NUM_MATERIAL_TEXTURES ];
		String render_Shader;
		bool render_Additive;
		bool render_Stretch;
		
		int state_SpawnTotalCount;
		int state_SpawnCurrCount;
		float state_SpawnTotalTime;
		float state_SpawnCurrTime;
		
		Emitter() :
			spawn_MaxCount(100), spawn_Count(10), spawn_CountExt(2), spawn_TimeExt(V2(1,0.2f)),
			create_Pos(V3(0)), create_PosBox(V3(0)), create_PosRadius(0),
			create_VelMicroDir(V3(0,0,1)), create_VelMicroDvg(0), create_VelMicroDistExt(V2(1,0.1f)),
			create_VelMacroDir(V3(0,0,1)), create_VelMacroDvg(0), create_VelMacroDistExt(V2(1,0.1f)),
			create_VelCluster(1), create_VelClusterExt(0),
			create_LifetimeExt(V2(10,0.1f)), create_AngleDirDvg(V2(0,M_PI)),
			tick_AngleVelAcc(V2(0)), absolute(false),
			render_Shader("particle"), render_Additive(false), render_Stretch(false),
			state_SpawnTotalCount(0), state_SpawnCurrCount(0), state_SpawnTotalTime(0), state_SpawnCurrTime(0)
		{}
		
		void Tick( float dt, const Vec3& accel, const Mat4& mtx );
		void Generate( int count, const Mat4& mtx );
		void Trigger( const Mat4& mtx );
		
		void PreRender( Array< Vertex >& vertices, Array< uint16_t >& indices, const Mat4& transform, const Vec3 axes[2] );
	};
	
	Array< Emitter > emitters;
	Vec3 gravity;
	Mat4 transform;
	bool looping;
	Vec2 retriggerTimeExt;
	
	bool m_isPlaying;
	float m_retriggerTime;
	
	Array< Vertex > m_vertices;
	Array< uint16_t > m_indices;
	SceneHandle m_scene;
	VertexDeclHandle m_vdecl;
	MeshHandle m_mesh;
	MeshInstHandle m_meshInst;
	
	ParticleSystem() :
		gravity(V3(0,0,-10)), transform(Mat4::Identity), looping(true), retriggerTimeExt(V2(1,0.1f)),
		m_isPlaying(false), m_retriggerTime(0)
	{}
	
	void OnRenderUpdate();
	void AddToScene( SceneHandle sh );
	void SetTransform( const Mat4& mtx );
	
	void Tick( float dt );
	void PreRender();
	
	void Trigger();
	void Play();
	void Stop();
};



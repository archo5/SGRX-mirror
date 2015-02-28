

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



struct EXPORT ParticleSystem
{
	struct Emitter
	{
		struct Curve
		{
			enum Mode { Off = 0, Up = 1, Down = 2, Bidi = 3 };
			
			Array< Vec2 > values;
			int mode;
		};
		
		Array< Vec3 > particles_Position;
		Array< Vec3 > particles_Velocity;
		Array< Vec2 > particles_Lifetime;
		Array< Vec2 > particles_RandSizeAngle;
		Array< Vec4 > particles_RandColor;
		
		Curve curve_Size;
		Curve curve_ColorHue;
		Curve curve_ColorSat;
		Curve curve_ColorVal;
		Curve curve_Opacity;
		
		int spawn_MaxCount;
		int spawn_Count;
		int spawn_CountExt;
		Vec2 spawn_TimeExt;
		int spawn_CurrCount;
		float spawn_CurrTime;
		
		Vec3 create_PosBox;
		float create_PosRadius;
		Vec3 create_VelMicroDir;
		float create_VelMicroDvg;
		Vec2 create_VelMicroDistExt;
		Vec3 create_VelMacroDir;
		float create_VelMacroDvg;
		Vec2 create_VelMacroDistExt;
		int create_VelClusterMin;
		int create_VelClusterMax;
		Vec2 create_LifetimeExt;
		Vec2 create_AngleDirDvg;
		
		Vec2 tick_AngleVelAcc;
		
		TextureHandle render_Texture;
		ShaderHandle render_Shader;
		Array< Vec4 > render_ShaderConsts;
		bool render_Additive;
		bool render_Stretch;
		
		int state_SpawnCount;
		float state_SpawnTime;
		
		void Tick( float dt );
		void Generate( int count );
	};
	
	MeshHandle m_mesh;
	MeshInstHandle m_meshInstances;
	
	void Tick( float dt );
	void PreRender();
	void Trigger();
	void Play();
	void Stop();
};



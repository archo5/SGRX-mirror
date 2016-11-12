

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
typedef HashTable< StringView, AnimHandle > AnimHashTable;

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

#define ANIM_PLAY_LOOP 0
#define ANIM_PLAY_ONCE 1
#define ANIM_PLAY_CLAMP 2

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
		uint8_t playMode;
	};
	
	ENGINE_EXPORT AnimPlayer();
	ENGINE_EXPORT ~AnimPlayer();
	ENGINE_EXPORT virtual void Prepare();
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	ENGINE_EXPORT void Play( const AnimHandle& anim, uint8_t playMode = ANIM_PLAY_LOOP, float fadetime = 0.5f );
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



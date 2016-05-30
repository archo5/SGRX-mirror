

#pragma once
#include "enganim.hpp"
#include "physics.hpp"



#define OPERAND_NONE 0xffff
struct MEOperation
{
	uint16_t type;
	uint16_t op1;
	uint16_t op2;
};
struct MECompileResult
{
	MECompileResult(){}
	MECompileResult( StringView up, StringView e ) : unparsed( up ), error( e ){}
	operator bool () const { return error; }
	
	StringView unparsed;
	StringView error;
};
struct MEVariableInterface
{
	// return OPERAND_NONE on failure
	virtual uint16_t GetID( StringView name ) const = 0;
	// GetValue must handle the >= count case
	virtual double GetValue( uint16_t i ) const = 0;
};
struct MathEquation
{
	Array< double > consts;
	Array< MEOperation > ops;
	
	MECompileResult Compile( StringView script, const MEVariableInterface* vars );
	double Eval( const MEVariableInterface* vars );
	
	struct MEPTRes _AllocOper();
	struct MEPTRes _AllocConst( double val );
	void _Optimize( MEOperation& O );
	struct MEPTRes _ParseTokens(
		ArrayView< struct METoken > tokenlist,
		StringView parentfirst,
		const MEVariableInterface* vars
	);
	void _Clean();
	double _Op1( const MEOperation& O, const MEVariableInterface* vars );
	double _Op2( const MEOperation& O, const MEVariableInterface* vars );
	double _DoOp( uint16_t op, const MEVariableInterface* vars );
};



struct IF_GCC(ENGINE_EXPORT) AnimRagdoll : Animator
{
	struct Body
	{
		Vec3 relPos;
		Quat relRot;
		PhyRigidBodyHandle bodyHandle;
		PhyJointHandle jointHandle;
		Vec3 prevPos;
		Vec3 currPos;
		Quat prevRot;
		Quat currRot;
	};
	
	ENGINE_EXPORT AnimRagdoll( PhyWorldHandle phyWorld );
	ENGINE_EXPORT void Initialize( struct AnimCharacter* chinfo );
	ENGINE_EXPORT virtual bool Prepare( const MeshHandle& mesh );
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	ENGINE_EXPORT void SetBoneTransforms( int bone_id, const Vec3& prev_pos, const Vec3& curr_pos, const Quat& prev_rot, const Quat& curr_rot );
	ENGINE_EXPORT void AdvanceTransforms( Animator* anim );
	ENGINE_EXPORT void EnablePhysics( const Mat4& worldMatrix );
	ENGINE_EXPORT void DisablePhysics();
	ENGINE_EXPORT void WakeUp();
	ENGINE_EXPORT void ApplyImpulse( Vec3 origin, Vec3 imp, size_t bone = NOT_FOUND );
	ENGINE_EXPORT void ApplyImpulseExt( Vec3 origin, Vec3 imp, float atten, float radius, size_t bone = NOT_FOUND );
	
	bool m_enabled;
	float m_lastTickSize;
	PhyWorldHandle m_phyWorld;
	MeshHandle m_mesh;
	Array< Body > m_bones;
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
		JointType_ConeTwist = 2,
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
		Vec3 self_position;
		Quat self_rotation;
		Vec3 prnt_position;
		Quat prnt_rotation;
		float turn_limit_1; // Hinge(min), ConeTwist(1)
		float turn_limit_2; // Hinge(max), ConeTwist(2)
		float twist_limit; // ConeTwist
		
		int parent_id;
		
		Joint() : type( JointType_None ),
			self_position( V3(0) ), self_rotation( Quat::Identity ),
			prnt_position( V3(0) ), prnt_rotation( Quat::Identity ),
			turn_limit_1( 0 ), turn_limit_2( 0 ), twist_limit( 0 ),
			parent_id(-1)
		{}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "JOINT" );
			arch( type );
			arch( parent_name );
			arch( self_position );
			arch( self_rotation );
			arch( prnt_position );
			arch( prnt_rotation );
			arch( turn_limit_1 );
			arch( turn_limit_2 );
			arch( twist_limit );
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
			arch( joint, arch.version >= 3 );
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
		float base; // additional unconditional offset
		
		int bone_id;
		
		LayerTransform() : type( TransformType_None ), posaxis( V3(0,0,1) ),
			angle( 0 ), base(0), bone_id(-1){}
		
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch.marker( "LAYERXF" );
			arch( type );
			arch( bone );
			arch( posaxis );
			arch( angle );
			arch( base, arch.version >= 4, 0 );
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
	
	struct Variable : SGRX_RefCounted
	{
		String name;
		float value;
		
		Variable() : value(0){}
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch << name;
			arch << value;
		}
		template< class T > static Variable* UnserializeCreate( T& arch )
		{
			return new Variable;
		}
	};
	struct State
	{
		SGRX_GUID guid;
		String name;
		String anim;
		bool loop;
		float speed;
		
		State() : loop(true), speed(1){}
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch << guid;
			arch << name;
			arch << anim;
			arch << loop;
			arch << speed;
		}
	};
	struct Transition
	{
		String expr;
		MathEquation compiled_expr;
		SGRX_GUID source; // NULL GUID = transition from any state (bidi not sup.)
		SGRX_GUID target;
		bool bidi;
		
		Transition() : bidi(false){}
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch << expr;
			arch << source;
			arch << target;
			arch << bidi;
		}
	};
	enum NodeType
	{
		NT_Unknown = 0,
		NT_Player = 1,
	};
	struct Node
	{
		virtual ~Node(){}
		uint8_t type; // NodeType
		SGRX_GUID guid;
	};
	struct PlayerNode : Node
	{
		Array< Handle< State > > states;
		Array< Handle< Transition > > transitions;
		
		AnimPlayer player;
		HashTable< SGRX_GUID, size_t > transition_lookup; /* GUID -> ID array offset */
		Array< size_t > transition_lookup_ids; /* ID count, IDs, ...
		... NULL GUID is first set of entries, always present */
		
		void RehashTransitions();
		template< class T > void Serialize( SerializeVersionHelper<T>& arch )
		{
			arch << states;
			arch << transitions;
			if( T::IsReader )
				RehashTransitions();
		}
	};
	
	template< class T > void Serialize( T& basearch )
	{
		basearch.marker( "SGRXCHAR" );
		
		// 1: initial
		// 2: added masks
		// 3: added joints
		// 4: base offset
		// 5: added variables
		SerializeVersionHelper<T> arch( basearch, 5 );
		
		arch( mesh );
		arch( bones );
		arch( attachments );
		arch( layers );
		arch( masks, arch.version >= 2 );
		arch( variables, arch.version >= 5 );
	}
	
	ENGINE_EXPORT AnimCharacter( SceneHandle sh, PhyWorldHandle phyWorld );
	
	ENGINE_EXPORT bool Load( const StringView& sv );
	ENGINE_EXPORT bool Save( const StringView& sv );
	
	ENGINE_EXPORT void _OnRenderUpdate();
	ENGINE_EXPORT void SetTransform( const Mat4& mtx );
	
	ENGINE_EXPORT void FixedTick( float deltaTime );
	ENGINE_EXPORT void PreRender( float blendFactor );
	ENGINE_EXPORT void RecalcLayerState();
	
	ENGINE_EXPORT void EnablePhysics();
	ENGINE_EXPORT void DisablePhysics();
	ENGINE_EXPORT void WakeUp();
	
	ENGINE_EXPORT int _FindBone( const StringView& name );
	ENGINE_EXPORT int FindParentBone( int which );
	ENGINE_EXPORT void RecalcBoneIDs();
	ENGINE_EXPORT bool GetBodyMatrix( int which, Mat4& outwm );
	ENGINE_EXPORT bool GetJointFrameMatrices( int which, Mat4& outself, Mat4& outprnt );
	ENGINE_EXPORT bool GetJointMatrix( int which, bool parent, Mat4& outwm );
	ENGINE_EXPORT void _GetHitboxMatrix( int which, Mat4& outwm );
	ENGINE_EXPORT bool GetHitboxOBB( int which, Mat4& outwm, Vec3& outext );
	ENGINE_EXPORT bool GetAttachmentMatrix( int which, Mat4& outwm, bool worldspace = true ) const;
	ENGINE_EXPORT bool ApplyMask( const StringView& name, Animator* tgt );
	ENGINE_EXPORT int FindAttachment( const StringView& name );
	ENGINE_EXPORT void SortEnsureAttachments( const StringView* atchnames, int count );
	
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, struct SGRX_MeshInstance* cbmi = NULL );
	ENGINE_EXPORT void MRC_DebugDraw( SGRX_MeshInstance* mi );
	
	FINLINE Mat4 GetAttachmentMatrix( int which, bool worldspace = true )
	{
		Mat4 out = Mat4::Identity;
		GetAttachmentMatrix( which, out, worldspace );
		return out;
	}
	FINLINE Vec3 GetAttachmentPos( int which, Vec3 p = V3(0) )
	{
		return GetAttachmentMatrix( which ).TransformPos( p );
	}
	FINLINE Vec3 GetLocalAttachmentPos( int which, Vec3 p = V3(0) )
	{
		return GetAttachmentMatrix( which, false ).TransformPos( p );
	}
	
	String mesh;
	Array< BoneInfo > bones;
	Array< Attachment > attachments;
	Array< Layer > layers;
	Array< Mask > masks;
	Array< Handle< Variable > > variables;
	
	SceneHandle m_scene;
	MeshHandle m_cachedMesh;
	MeshInstHandle m_cachedMeshInst;
	Animator m_layerAnimator;
	AnimMixer m_anMixer;
	AnimDeformer m_anDeformer;
	AnimRagdoll m_anRagdoll;
	AnimInterp m_anEnd;
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
	DecalMapPartInfo decalInfo;
};

struct IF_GCC(ENGINE_EXPORT) SGRX_DecalSystem : SGRX_RefCounted
{
	ENGINE_EXPORT SGRX_DecalSystem();
	ENGINE_EXPORT ~SGRX_DecalSystem();
	
	ENGINE_EXPORT void Init( SceneHandle scene, TextureHandle texDecal, TextureHandle texFalloff );
	ENGINE_EXPORT void Free();
	ENGINE_EXPORT void SetSize( uint32_t vbSize );
	ENGINE_EXPORT void SetDynamic( bool dynamic );
	
	ENGINE_EXPORT void Upload();
	
	ENGINE_EXPORT void AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, const Mat4& worldMatrix );
	ENGINE_EXPORT void AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, int partID, const Mat4& worldMatrix );
	ENGINE_EXPORT void ClearAllDecals();
	ENGINE_EXPORT void GenerateCamera( const DecalProjectionInfo& projInfo, SGRX_Camera* out );
	
	ENGINE_EXPORT void _ScaleDecalTexcoords( const DecalProjectionInfo& projInfo, size_t vbfrom );
	ENGINE_EXPORT void _InvTransformDecals( size_t vbfrom );
	ENGINE_EXPORT void _GenDecalMatrix( const DecalProjectionInfo& projInfo, Mat4* outVPM, float* out_invzn2zf );
	
	SGRX_LightSampler* m_lightSampler;
	
	VertexDeclHandle m_vertexDecl;
	MeshInstHandle m_meshInst;
	SGRX_Material m_material;
	MeshHandle m_mesh;
	ByteArray m_vertexData;
	UInt32Array m_indexData;
	Array< uint32_t > m_decals;
	uint32_t m_vbSize;
};
typedef Handle< SGRX_DecalSystem > DecalSysHandle;



struct IF_GCC(ENGINE_EXPORT) SGRX_HelpTextRenderer
{
	struct Text
	{
		size_t text_start;
		size_t text_size;
		StringView font;
		Vec4 color;
		Vec2 pos;
		int fontSize;
		int width;
		int lineNum;
		int padding;
		bool button;
	};
	struct FontInfo
	{
		StringView name;
		float sizeFactor;
	};
	
	ENGINE_EXPORT SGRX_HelpTextRenderer();
	ENGINE_EXPORT void RenderText( StringView text );
	ENGINE_EXPORT virtual void AddInputText( ActionInput input );
	ENGINE_EXPORT virtual void AddActionInputText( StringView action );
	ENGINE_EXPORT virtual void DrawTextItem( Text& item );
	
	// interface helpers
	void SetNamedFont( StringView key, StringView name, float factor = 1 )
	{
		FontInfo fi = { name, factor };
		namedFonts.set( key, fi );
	}
	StringView GetText( Text& text )
	{
		return StringView( m_strings ).part( text.text_start, text.text_size );
	}
	
	// internals
	ENGINE_EXPORT void SetColor( StringView name );
	ENGINE_EXPORT void SetFont( StringView name );
	ENGINE_EXPORT void AddText( StringView text );
	void AddText( StringView from, StringView to ){ AddText( from.part( 0, to.data() - from.data() ) ); }
	
	// settings
	HashTable< StringView, Vec4 > namedColors;
	HashTable< StringView, FontInfo > namedFonts;
	int fontSize;
	Vec2 centerPos;
	float lineHeightFactor;
	TextureHandle buttonTex;
	int buttonTexBorder;
	int buttonBorder;
	float opacity;
	
	// cached data
	int m_lineCount;
	int m_curLine;
	String m_strings;
	Vec4 m_curColor;
	FontInfo m_curFont;
	Array< Text > m_textCache;
};




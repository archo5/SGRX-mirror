

#pragma once
#include "enganim.hpp"
#include "physics.hpp"



#define ME_OPERAND_NONE 0xffff
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
	// return ME_OPERAND_NONE on failure
	virtual uint16_t MEGetID( StringView name ) const = 0;
	// MEGetValue must handle the >= count case
	virtual double MEGetValue( uint16_t i ) const = 0;
};
struct MathEquation
{
	Array< double > consts;
	Array< MEOperation > ops;
	
	ENGINE_EXPORT void Clear();
	ENGINE_EXPORT MECompileResult Compile( StringView script, const MEVariableInterface* vars );
	ENGINE_EXPORT double Eval( const MEVariableInterface* vars ) const;
	ENGINE_EXPORT void Dump() const;
	
	ENGINE_EXPORT struct MEPTRes _AllocOper();
	ENGINE_EXPORT struct MEPTRes _AllocConst( double val );
	ENGINE_EXPORT void _Optimize( MEOperation& O );
	ENGINE_EXPORT struct MEPTRes _ParseTokens(
		ArrayView< struct METoken > tokenlist,
		StringView parentfirst,
		const MEVariableInterface* vars
	);
	ENGINE_EXPORT void _Clean();
	ENGINE_EXPORT double _Op1( const MEOperation& O, const MEVariableInterface* vars ) const;
	ENGINE_EXPORT double _Op2( const MEOperation& O, const MEVariableInterface* vars ) const;
	ENGINE_EXPORT double _DoOp( uint16_t op, const MEVariableInterface* vars ) const;
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
	ENGINE_EXPORT virtual void Prepare();
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
	Array< Body > m_bones;
};

struct IF_GCC(ENGINE_EXPORT) AnimRotator : Animator
{
	AnimRotator() : animSource( NULL ), angle( 0 ){}
	ENGINE_EXPORT virtual void Advance( float deltaTime, AnimInfo* info );
	
	Animator* animSource;
	float angle;
};



struct IF_GCC(ENGINE_EXPORT) AnimCharacter : IMeshRaycast, MEVariableInterface
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		
		template< class T > void Serialize( T& arch )
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
		uint8_t mode;
		
		MaskCmd() : weight( 0 ), children( true ), mode( SFM_Set ){}
		
		template< class T > void Serialize( T& arch )
		{
			arch.marker( "MASKCMD" );
			arch( bone );
			arch( weight );
			arch( children );
			arch( mode, arch.version >= 6, SFM_Set );
		}
	};
	struct Mask
	{
		String name;
		Array< MaskCmd > cmds;
		
		template< class T > void Serialize( T& arch )
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
		template< class T > void Serialize( T& arch )
		{
			arch << name;
			arch << value;
		}
		template< class T > static Variable* UnserializeCreate( T& arch )
		{
			return new Variable;
		}
	};
	struct ValExpr
	{
		String expr;
		MathEquation compiled_expr;
		ENGINE_EXPORT MECompileResult Recompile( const MEVariableInterface* vars );
		double Eval( const MEVariableInterface* vars ) const { return compiled_expr.Eval( vars ); }
		template< class T > void Serialize( T& arch )
		{
			arch << expr;
		}
	};
	struct Alias : SGRX_RefCounted
	{
		String name;
		ValExpr expr;
		double value;
		
		Alias() : value(0){}
		template< class T > void Serialize( T& arch )
		{
			arch << name;
			arch << expr;
		}
		template< class T > static Alias* UnserializeCreate( T& arch )
		{
			return new Alias;
		}
	};
	struct State : SGRX_RefCounted
	{
		SGRX_GUID guid;
		String name;
		String anim;
		bool loop;
		ValExpr speed;
		float fade_time;
		// editor data
		Vec2 editor_pos;
		
		State() : loop(true), fade_time(0.5f){ speed.expr = "1"; speed.Recompile(NULL); }
		StringView GetName()
		{
			if( name.size() ) return name;
			StringView out = SV(anim).after( ":" );
			if( out ) return out;
			if( anim.size() ) return anim;
			return "<unnamed>";
		}
		void Init( Vec2 ep )
		{
			editor_pos = ep;
			guid = SGRX_GUID::Generate();
		}
		template< class T > void Serialize( T& arch )
		{
			arch.marker( "STATE" );
			arch << guid;
			arch << name;
			arch << anim;
			arch << loop;
			arch << speed;
			arch << fade_time;
			arch << editor_pos;
		}
		template< class T > static State* UnserializeCreate( T& arch )
		{
			return new State;
		}
	};
	struct Transition : SGRX_RefCounted
	{
		ValExpr expr;
		SGRX_GUID source; // NULL GUID = transition from any state (bidi not sup.)
		SGRX_GUID target;
		bool bidi;
		
		Transition() : bidi(false){}
		template< class T > void Serialize( T& arch )
		{
			arch.marker( "TRANSITION" );
			arch << expr;
			arch << source;
			arch << target;
			arch << bidi;
		}
		template< class T > static Transition* UnserializeCreate( T& arch )
		{
			return new Transition;
		}
	};
	enum NodeType
	{
		NT_Unknown = 0,
		NT_Player = 1,
		NT_Blend = 2,
		NT_Ragdoll = 3,
		NT_Mask = 4,
		NT_RelAbs = 5,
		NT_Layers = 6,
		NT_Rotator = 7,
	};
	typedef SerializeVersionHelper<ByteReader> SVHBR;
	typedef SerializeVersionHelper<ByteWriter> SVHBW;
	#define IMPL_VIRTUAL_SERIALIZE \
		virtual void Serialize( SVHBR& arch ){ SerializeT( arch ); } \
		virtual void Serialize( SVHBW& arch ){ SerializeT( arch ); }
	struct IF_GCC(ENGINE_EXPORT) Node : SGRX_RefCounted
	{
		virtual ~Node(){}
		const char* GetName()
		{
			if( type == NT_Player ) return "Player";
			if( type == NT_Blend ) return "Blend";
			if( type == NT_Ragdoll ) return "Ragdoll";
			if( type == NT_Mask ) return "Mask";
			if( type == NT_RelAbs ) return "Rel <-> Abs";
			if( type == NT_Layers ) return "Layers";
			if( type == NT_Rotator ) return "Rotator";
			return "<Unknown>";
		}
		virtual int GetInputLinkCount(){ return 0; }
		virtual SGRX_GUID* GetInputLink( int i ){ return NULL; }
		virtual Animator** GetInputSource( int i ){ return NULL; }
		virtual bool OwnsAnimator(){ return type != NT_Ragdoll && type != NT_Layers; }
		virtual Animator* GetAnimator( AnimCharacter* ch ) = 0;
		virtual void Advance( float dt, const MEVariableInterface* vars ){};
		Node( uint8_t t ) : type(t), editor_pos(V2(0)){}
		virtual void Init( Vec2 ep )
		{
			editor_pos = ep;
			guid = SGRX_GUID::Generate();
		}
		template< class T > static Node* UnserializeCreate( T& arch );
		template< class T > void SerializeT( T& arch )
		{
			if( T::IsWriter )
			{
				arch.marker( "NODE" );
				arch << type;
			}
			arch << guid;
			arch << editor_pos;
		}
		IMPL_VIRTUAL_SERIALIZE;
		
		uint8_t type; // NodeType
		SGRX_GUID guid;
		Vec2 editor_pos;
	};
	struct IF_GCC(ENGINE_EXPORT) RagdollNode : Node
	{
		RagdollNode() : Node( NT_Ragdoll ){}
		ENGINE_EXPORT virtual Animator* GetAnimator( AnimCharacter* ch );
	};
	struct IF_GCC(ENGINE_EXPORT) LayersNode : Node
	{
		LayersNode() : Node( NT_Layers ){}
		ENGINE_EXPORT virtual Animator* GetAnimator( AnimCharacter* ch );
	};
	struct IF_GCC(ENGINE_EXPORT) PlayerNode : Node
	{
		Array< Handle< State > > states;
		Array< Handle< Transition > > transitions;
		Handle< State > starting_state;
		
		Handle< State > current_state;
		AnimPlayer player_anim;
		HashTable< SGRX_GUID, State* > state_lookup;
		HashTable< SGRX_GUID, size_t > transition_lookup; /* GUID -> ID array offset */
		Array< size_t > transition_lookup_ids; /* ID count, IDs, ...
		... NULL GUID is first set of entries, always present */
		
		PlayerNode() : Node( NT_Player ){ RehashTransitions(); }
		virtual Animator* GetAnimator( AnimCharacter* ){ return &player_anim; }
		virtual void Advance( float dt, const MEVariableInterface* vars )
		{
			if( current_state )
				player_anim.SetLastAnimSpeed( current_state->speed.Eval( vars ) );
		}
		ENGINE_EXPORT void StartCurrentState();
		ENGINE_EXPORT void UpdateState( const MEVariableInterface* vars );
		ENGINE_EXPORT void RehashStates();
		ENGINE_EXPORT void RehashTransitions();
		template< class T > void SerializeT( T& arch )
		{
			Node::Serialize( arch );
			arch << states;
			arch << transitions;
			
			uint32_t starting_id = states.find_first_at( starting_state );
			arch << starting_id;
			starting_state = size_t(starting_id) < states.size() ? states[ starting_id ] : NULL;
			
			if( T::IsReader )
			{
				RehashStates();
				RehashTransitions();
				current_state = starting_state;
			}
		}
		IMPL_VIRTUAL_SERIALIZE;
	};
	struct IF_GCC(ENGINE_EXPORT) MaskNode : Node
	{
		SGRX_GUID src;
		String mask_name;
		
		AnimMask mask_anim;
		
		MaskNode() : Node( NT_Mask ){}
		virtual int GetInputLinkCount(){ return 1; }
		virtual SGRX_GUID* GetInputLink( int i ){ return i ? NULL : &src; }
		virtual Animator** GetInputSource( int i ){ return i ? NULL : &mask_anim.animSource; }
		virtual Animator* GetAnimator( AnimCharacter* ){ return &mask_anim; }
		template< class T > void SerializeT( T& arch )
		{
			Node::Serialize( arch );
			arch << src;
			arch << mask_name;
		}
		IMPL_VIRTUAL_SERIALIZE;
	};
	struct IF_GCC(ENGINE_EXPORT) BlendNode : Node
	{
		SGRX_GUID A;
		SGRX_GUID B;
		ValExpr factor;
		uint8_t mode;
		
		AnimBlend blend_anim;
		
		BlendNode() : Node( NT_Blend ), mode( ABM_Normal ){
			factor.expr = "1";
			factor.Recompile( NULL ); }
		virtual int GetInputLinkCount(){ return 2; }
		virtual SGRX_GUID* GetInputLink( int i ){
			if( i == 0 ) return &A;
			if( i == 1 ) return &B;
			return NULL; }
		virtual Animator** GetInputSource( int i ){
			if( i == 0 ) return &blend_anim.animSourceA;
			if( i == 1 ) return &blend_anim.animSourceB;
			return NULL; }
		virtual Animator* GetAnimator( AnimCharacter* ){ return &blend_anim; }
		virtual void Advance( float dt, const MEVariableInterface* vars )
		{
			blend_anim.blendFactor = factor.Eval( vars );
			blend_anim.blendMode = mode;
		}
		template< class T > void SerializeT( T& arch )
		{
			Node::Serialize( arch );
			arch << A;
			arch << B;
			arch << factor;
			arch << mode;
		}
		IMPL_VIRTUAL_SERIALIZE;
	};
	struct IF_GCC(ENGINE_EXPORT) RelAbsNode : Node
	{
		SGRX_GUID src;
		uint8_t flags;
		
		AnimRelAbs relabs_anim;
		
		RelAbsNode() : Node( NT_RelAbs ), flags( 0 ){}
		virtual int GetInputLinkCount(){ return 1; }
		virtual SGRX_GUID* GetInputLink( int i ){
			if( i == 0 ) return &src;
			return NULL; }
		virtual Animator** GetInputSource( int i ){
			if( i == 0 ) return &relabs_anim.animSource;
			return NULL; }
		virtual Animator* GetAnimator( AnimCharacter* ){ return &relabs_anim; }
		virtual void Advance( float dt, const MEVariableInterface* vars )
		{
			relabs_anim.flags = flags;
		}
		template< class T > void SerializeT( T& arch )
		{
			Node::Serialize( arch );
			arch << src;
			arch << flags;
		}
		IMPL_VIRTUAL_SERIALIZE;
	};
	struct IF_GCC(ENGINE_EXPORT) RotatorNode : Node
	{
		SGRX_GUID src;
		ValExpr angle;
		
		AnimRotator rotator_anim;
		
		RotatorNode() : Node( NT_Rotator ){ angle.expr = "0"; angle.Recompile( NULL ); }
		virtual int GetInputLinkCount(){ return 1; }
		virtual SGRX_GUID* GetInputLink( int i ){
			if( i == 0 ) return &src;
			return NULL; }
		virtual Animator** GetInputSource( int i ){
			if( i == 0 ) return &rotator_anim.animSource;
			return NULL; }
		virtual Animator* GetAnimator( AnimCharacter* ){ return &rotator_anim; }
		virtual void Advance( float dt, const MEVariableInterface* vars )
		{
			rotator_anim.angle = angle.Eval( vars );
		}
		template< class T > void SerializeT( T& arch )
		{
			Node::Serialize( arch );
			arch << src;
			arch << angle;
		}
		IMPL_VIRTUAL_SERIALIZE;
	};
	
	template< class T > void Serialize( T& basearch )
	{
		basearch.marker( "SGRXCHAR" );
		
		// 1: initial
		// 2: added masks
		// 3: added joints
		// 4: base offset
		// 5: added nodes, variables, aliases
		// 6: added mask mode
		SerializeVersionHelper<T> arch( basearch, 6 );
		
		arch( mesh );
		arch( bones );
		arch( attachments );
		arch( layers );
		arch( masks, arch.version >= 2 );
		if( arch.version >= 5 )
		{
			arch << nodes;
			arch << variables;
			arch << aliases;
			
			uint32_t output_id = nodes.find_first_at( output_node );
			arch << output_id;
			output_node = size_t(output_id) < nodes.size() ? nodes[ output_id ] : NULL;
		}
		else
		{
			nodes.clear();
			variables.clear();
			aliases.clear();
			output_node = NULL;
		}
	}
	
	ENGINE_EXPORT AnimCharacter( SceneHandle sh, PhyWorldHandle phyWorld );
	
	ENGINE_EXPORT bool Load( const StringView& sv );
	ENGINE_EXPORT bool Save( const StringView& sv );
	
	ENGINE_EXPORT void _OnRenderUpdate();
	ENGINE_EXPORT void _UnlinkNode( Node* node );
	ENGINE_EXPORT void _Prepare();
	ENGINE_EXPORT void _EquipAnimator( Animator* anim, int which );
	ENGINE_EXPORT void ResetStates();
	ENGINE_EXPORT void SetTransform( const Mat4& mtx );
	
	ENGINE_EXPORT void FixedTick( float deltaTime, bool changeStates = true );
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
	ENGINE_EXPORT bool ApplyMask( const StringView& name, AnimMask* tgt );
	ENGINE_EXPORT int FindAttachment( const StringView& name );
	ENGINE_EXPORT void SortEnsureAttachments( const StringView* atchnames, int count );
	
	ENGINE_EXPORT void RaycastAll( const Vec3& from, const Vec3& to, struct SceneRaycastCallback* cb, struct SGRX_MeshInstance* cbmi = NULL );
	ENGINE_EXPORT void MRC_DebugDraw( SGRX_MeshInstance* mi );
	ENGINE_EXPORT uint16_t MEGetID( StringView name ) const;
	ENGINE_EXPORT double MEGetValue( uint16_t i ) const;
	
	ENGINE_EXPORT void _RehashNodes();
	ENGINE_EXPORT void _ReindexVariables();
	ENGINE_EXPORT void _LinkAnimNodes();
	ENGINE_EXPORT void _SetVar( StringView name, float val );
	FINLINE void SetBool( StringView name, bool val ){ _SetVar( name, val ); }
	FINLINE void SetInt( StringView name, int val ){ _SetVar( name, val ); }
	FINLINE void SetFloat( StringView name, float val ){ _SetVar( name, val ); }
	
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
	Array< Handle< Node > > nodes;
	Array< Handle< Variable > > variables;
	Array< Handle< Alias > > aliases;
	Handle< Node > output_node;
	
	SceneHandle m_scene;
	MeshHandle m_cachedMesh;
	MeshInstHandle m_cachedMeshInst;
	HashTable< StringView, uint16_t > m_variable_index;
	HashTable< SGRX_GUID, Node* > m_node_map;
	Array< AnimTrackFrame > m_node_frames;
	uint32_t m_frameID;
	Animator m_anLayers;
	AnimDeformer m_anDeformer;
	AnimRagdoll m_anRagdoll;
	AnimInterp m_anEnd;
};

template< class T > AnimCharacter::Node* AnimCharacter::Node::UnserializeCreate( T& arch )
{
	uint8_t type;
	arch.marker( "NODE" );
	arch << type;
	if( type == NT_Player ) return new PlayerNode;
	if( type == NT_Blend ) return new BlendNode;
	if( type == NT_Ragdoll ) return new RagdollNode;
	if( type == NT_Mask ) return new MaskNode;
	if( type == NT_RelAbs ) return new RelAbsNode;
	if( type == NT_Layers ) return new LayersNode;
	if( type == NT_Rotator ) return new RotatorNode;
	return NULL;
}



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






#include "engext.hpp"



//////////////////////
//  R A G D O L L
////////////////////

AnimRagdoll::AnimRagdoll( PhyWorldHandle phyWorld ) :
	m_enabled( false ),
	m_lastTickSize( 0 ),
	m_phyWorld( phyWorld )
{
	ASSERT( phyWorld && "physics world handle must be valid" );
}

void AnimRagdoll::Initialize( AnimCharacter* chinfo )
{
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.enabled = false;
	rbinfo.friction = 0.8f;
	rbinfo.restitution = 0.02f;
	rbinfo.linearDamping = 0.1f;
	rbinfo.angularDamping = 0.1f;
	
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		Body B = { V3(0), Quat::Identity,
			NULL, NULL, V3(0), V3(0), Quat::Identity, Quat::Identity };
		m_bones[ i ] = B;
	}
	
	if( m_factors.size() != chinfo->bones.size() )
	{
		// ragdoll not used
		return;
	}
	
	for( size_t bid = 0; bid < chinfo->bones.size(); ++bid )
	{
		AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		Body* TB = &m_bones[ BI.bone_id ];
		
	//	LOG << SB.name << " > " << SB.capsule_radius << "|" << SB.capsule_height;
		if( BI.body.type == AnimCharacter::BodyType_Sphere )
		{
			rbinfo.shape = m_phyWorld->CreateSphereShape( BI.body.size.x );
		}
		else if( BI.body.type == AnimCharacter::BodyType_Capsule )
		{
			rbinfo.shape = m_phyWorld->CreateCapsuleShape(
				BI.body.size.x, ( BI.body.size.z - BI.body.size.x ) * 2 );
		}
		else if( BI.body.type == AnimCharacter::BodyType_Box )
		{
			rbinfo.shape = m_phyWorld->CreateBoxShape( BI.body.size );
		}
		else continue;
		rbinfo.mass = 2;//4.0f / 3.0f * M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_radius + M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_height;
		
		rbinfo.inertia = rbinfo.shape->CalcInertia( rbinfo.mass );
		
		TB->relPos = BI.body.position;
		TB->relRot = BI.body.rotation;
		TB->bodyHandle = m_phyWorld->CreateRigidBody( rbinfo );
	}
	
	for( size_t bid = 0; bid < chinfo->bones.size(); ++bid )
	{
		AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		if( BI.joint.parent_id >= 0 && BI.joint.type != AnimCharacter::JointType_None )
		{
			AnimCharacter::BoneInfo& PBI = chinfo->bones[ BI.joint.parent_id ];
			Mat4 jsm, jpm;
			if( m_bones[ BI.bone_id ].bodyHandle &&
				m_bones[ PBI.bone_id ].bodyHandle &&
				chinfo->GetJointFrameMatrices( bid, jsm, jpm ) )
			{
				if( BI.joint.type == AnimCharacter::JointType_Hinge )
				{
					SGRX_PhyHingeJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ BI.bone_id ].bodyHandle;
					jinfo.bodyB = m_bones[ PBI.bone_id ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					m_bones[ BI.bone_id ].jointHandle =
						m_phyWorld->CreateHingeJoint( jinfo );
				}
				else if( BI.joint.type == AnimCharacter::JointType_ConeTwist )
				{
					SGRX_PhyConeTwistJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ BI.bone_id ].bodyHandle;
					jinfo.bodyB = m_bones[ PBI.bone_id ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					jinfo.coneLimitX = DEG2RAD( BI.joint.turn_limit_1 );
					jinfo.coneLimitY = DEG2RAD( BI.joint.turn_limit_2 );
					jinfo.twistLimit = DEG2RAD( BI.joint.twist_limit );
					m_bones[ BI.bone_id ].jointHandle =
						m_phyWorld->CreateConeTwistJoint( jinfo );
				}
			}
		}
	}
}

bool AnimRagdoll::Prepare( const MeshHandle& mesh )
{
	if( Animator::Prepare( mesh ) == false )
		return false;
	
	m_mesh = mesh;
	m_bones.resize( m_factors.size() );
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		Body B = { V3(0), Quat::Identity,
			NULL, NULL, V3(0), V3(0), Quat::Identity, Quat::Identity };
		m_bones[ i ] = B;
		m_positions[ i ] = V3( 0 );
		m_rotations[ i ] = Quat::Identity;
		m_scales[ i ] = V3( 1 );
		m_factors[ i ] = 0;
	}
	
	return true;
}

void AnimRagdoll::Advance( float deltaTime, AnimInfo* info )
{
	m_lastTickSize = deltaTime;
	
	ASSERT( m_bones.size() == m_factors.size() );
	for( size_t i = 0; i < m_factors.size(); ++i )
		m_factors[ i ] = m_enabled && m_bones[ i ].bodyHandle;
	
	if( m_enabled == false )
		return;
	
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
		{
			Vec3 pos = B.bodyHandle->GetPosition();
			Quat rot = B.bodyHandle->GetRotation();
			Quat nrot = rot * B.relRot.Inverted();
			m_positions[ i ] = pos - Mat4::CreateRotationFromQuat(nrot).TransformNormal( B.relPos );
			m_rotations[ i ] = nrot;
		}
	}
}

void AnimRagdoll::SetBoneTransforms( int bone_id, const Vec3& prev_pos, const Vec3& curr_pos, const Quat& prev_rot, const Quat& curr_rot )
{
	ASSERT( bone_id >= 0 && bone_id < (int) m_factors.size() );
	Body& B = m_bones[ bone_id ];
	B.prevPos = prev_pos;
	B.currPos = curr_pos;
	B.prevRot = prev_rot;
	B.currRot = curr_rot;
}

void AnimRagdoll::AdvanceTransforms( Animator* anim )
{
	if( m_factors.size() != anim->m_factors.size() )
		return;
	
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		Body& B = m_bones[ i ];
		B.prevPos = B.currPos;
		B.prevRot = B.currRot;
		B.currPos = anim->m_positions[ i ];
		B.currRot = anim->m_rotations[ i ];
	}
}

void AnimRagdoll::EnablePhysics( const Mat4& worldMatrix )
{
	if( m_enabled )
		return;
	m_enabled = true;
	
	Mat4 prev_boneToWorldMatrices[ SGRX_MAX_MESH_BONES ];
	Mat4 curr_boneToWorldMatrices[ SGRX_MAX_MESH_BONES ];
	SGRX_MeshBone* MB = m_mesh->m_bones;
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		Mat4& prev_M = prev_boneToWorldMatrices[ i ];
		Mat4& curr_M = curr_boneToWorldMatrices[ i ];
		prev_M = Mat4::CreateSRT( V3(1), B.prevRot, B.prevPos ) * MB[ i ].boneOffset;
		curr_M = Mat4::CreateSRT( V3(1), B.currRot, B.currPos ) * MB[ i ].boneOffset;
		if( MB[ i ].parent_id >= 0 )
		{
			prev_M = prev_M * prev_boneToWorldMatrices[ MB[ i ].parent_id ];
			curr_M = curr_M * curr_boneToWorldMatrices[ MB[ i ].parent_id ];
		}
		else
		{
			prev_M = prev_M * worldMatrix;
			curr_M = curr_M * worldMatrix;
		}
	}
	
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
		{
			// calculate world transforms
			Mat4 rtf = Mat4::CreateSRT( V3(1), B.relRot, B.relPos );
			Mat4 prev_wtf = rtf * prev_boneToWorldMatrices[ i ];
			Mat4 curr_wtf = rtf * curr_boneToWorldMatrices[ i ];
			
			Vec3 prev_wpos = prev_wtf.GetTranslation();
			Vec3 curr_wpos = curr_wtf.GetTranslation();
			Quat prev_wrot = prev_wtf.GetRotationQuaternion();
			Quat curr_wrot = curr_wtf.GetRotationQuaternion();
			
			B.bodyHandle->SetPosition( curr_wpos );
			B.bodyHandle->SetRotation( curr_wrot );
			if( m_lastTickSize > SMALL_FLOAT )
			{
				float speed = 1.0f / m_lastTickSize;
				B.bodyHandle->SetLinearVelocity( ( curr_wpos - prev_wpos ) * speed );
				B.bodyHandle->SetAngularVelocity( CalcAngularVelocity( prev_wrot, curr_wrot ) * speed );
			}
			B.bodyHandle->SetEnabled( true );
		}
		if( B.jointHandle )
		{
			B.jointHandle->SetEnabled( true );
		}
	}
}

void AnimRagdoll::DisablePhysics()
{
	if( m_enabled == false )
		return;
	m_enabled = false;
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.jointHandle )
			B.jointHandle->SetEnabled( false );
		if( B.bodyHandle )
			B.bodyHandle->SetEnabled( false );
	}
}

void AnimRagdoll::WakeUp()
{
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
			B.bodyHandle->WakeUp();
	}
}



AnimCharacter::AnimCharacter( SceneHandle sh, PhyWorldHandle phyWorld ) :
	m_scene( sh ),
	m_anRagdoll( phyWorld )
{
	ASSERT( sh && "scene handle must be valid" );
	
	m_anDeformer.animSource = &m_anMixer;
	m_anEnd.animSource = &m_anDeformer;
	m_cachedMeshInst = m_scene->CreateMeshInstance();
	m_cachedMeshInst->raycastOverride = this;
}

bool AnimCharacter::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( ba );
	Serialize( br );
	if( br.error )
		return false;
	
	_OnRenderUpdate();
	m_cachedMeshInst->raycastOverride = m_cachedMesh->m_numBones ? this : NULL;
	return true;
}

bool AnimCharacter::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void AnimCharacter::_OnRenderUpdate()
{
	m_cachedMesh = GR_GetMesh( mesh );
	m_cachedMeshInst->SetMesh( m_cachedMesh );
	m_cachedMeshInst->skin_matrices.resize( m_cachedMesh ? m_cachedMesh->m_numBones : 0 );
	RecalcBoneIDs();
	m_anEnd.Prepare( m_cachedMesh );
	if( m_cachedMesh && (int) m_layerAnimator.m_factors.size() != m_cachedMesh->m_numBones )
		m_layerAnimator.Prepare( m_cachedMesh );
	m_layerAnimator.ClearFactors( 1.0f );
	m_anRagdoll.Initialize( this );
}

void AnimCharacter::SetTransform( const Mat4& mtx )
{
	if( m_cachedMeshInst )
		m_cachedMeshInst->matrix = mtx;
}

void AnimCharacter::FixedTick( float deltaTime )
{
	AnimInfo info = { m_cachedMeshInst->matrix };
	m_anEnd.Advance( deltaTime, &info );
	m_anRagdoll.AdvanceTransforms( &m_anEnd );
}

void AnimCharacter::PreRender( float blendFactor )
{
	m_anEnd.Interpolate( blendFactor );
	GR_ApplyAnimator( &m_anEnd, m_cachedMeshInst );
}

void AnimCharacter::RecalcLayerState()
{
	if( m_cachedMesh == NULL )
		return;
	
	TMEMSET( m_layerAnimator.m_positions.data(), m_layerAnimator.m_positions.size(), V3(0) );
	TMEMSET( m_layerAnimator.m_rotations.data(), m_layerAnimator.m_rotations.size(), Quat::Identity );
	for( size_t i = 0; i < layers.size(); ++i )
	{
		Layer& L = layers[ i ];
		for( size_t j = 0; j < L.transforms.size(); ++j )
		{
			LayerTransform& LT = L.transforms[ j ];
			if( LT.bone_id < 0 )
				continue;
			switch( LT.type )
			{
			case TransformType_UndoParent:
				{
					int parent_id = m_cachedMesh->m_bones[ LT.bone_id ].parent_id;
					if( parent_id >= 0 )
					{
						m_layerAnimator.m_positions[ LT.bone_id ] -= m_layerAnimator.m_positions[ parent_id ];
						m_layerAnimator.m_rotations[ LT.bone_id ] =
							m_layerAnimator.m_rotations[ LT.bone_id ] * m_layerAnimator.m_rotations[ parent_id ].Inverted();
					}
				}
				break;
			case TransformType_Move:
				m_layerAnimator.m_positions[ LT.bone_id ] += LT.posaxis * ( LT.base + L.amount );
				break;
			case TransformType_Rotate:
				m_layerAnimator.m_rotations[ LT.bone_id ] = m_layerAnimator.m_rotations[ LT.bone_id ]
					* Quat::CreateAxisAngle( LT.posaxis.Normalized(), DEG2RAD( LT.angle ) * ( LT.base + L.amount ) );
				break;
			}
		}
	}
}

void AnimCharacter::EnablePhysics()
{
	m_anRagdoll.EnablePhysics( m_cachedMeshInst->matrix );
}

void AnimCharacter::DisablePhysics()
{
	m_anRagdoll.DisablePhysics();
}

void AnimCharacter::WakeUp()
{
	m_anRagdoll.WakeUp();
}

int AnimCharacter::_FindBone( const StringView& name )
{
	if( !m_cachedMesh )
		return -1;
	int bid = 0;
	for( ; bid < m_cachedMesh->m_numBones; ++bid )
	{
		if( m_cachedMesh->m_bones[ bid ].name == name )
			break;
	}
	return bid < m_cachedMesh->m_numBones ? bid : -1;
}

int AnimCharacter::FindParentBone( int which )
{
	if( m_cachedMesh == NULL )
		return -1;
	if( which < 0 || which >= (int) bones.size() )
		return -1;
	
	int parent_id = m_cachedMesh->m_bones[ bones[ which ].bone_id ].parent_id;
	for( size_t i = 0; i < bones.size(); ++i )
	{
		if( bones[ i ].bone_id == parent_id )
			return i;
	}
	return -1;
}

void AnimCharacter::RecalcBoneIDs()
{
	for( size_t i = 0; i < bones.size(); ++i )
	{
		BoneInfo& BI = bones[ i ];
		BI.bone_id = _FindBone( BI.name );
		BI.joint.parent_id = _FindBone( BI.joint.parent_name );
	}
	for( size_t i = 0; i < attachments.size(); ++i )
	{
		Attachment& AT = attachments[ i ];
		AT.bone_id = _FindBone( AT.bone );
	}
	for( size_t i = 0; i < layers.size(); ++i )
	{
		Layer& LY = layers[ i ];
		for( size_t j = 0; j < LY.transforms.size(); ++j )
		{
			LayerTransform& LT = LY.transforms[ j ];
			LT.bone_id = _FindBone( LT.bone );
		}
	}
}

bool AnimCharacter::GetBodyMatrix( int which, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	
	outwm = m_cachedMeshInst->matrix;
	if( BI.bone_id >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ BI.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ BI.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ) * outwm;
	return true;
}

bool AnimCharacter::GetJointFrameMatrices( int which, Mat4& outself, Mat4& outprnt )
{
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	int pb = BI.joint.parent_id;
	if( pb < 0 || pb >= (int) bones.size() )
		return false;
	BoneInfo& PBI = bones[ pb ];
	
	// calc inverse body matrices
	Mat4 bm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ),
		pbm = Mat4::CreateRotationFromQuat( PBI.body.rotation ) *
		Mat4::CreateTranslation( PBI.body.position );
	Mat4 ibm = Mat4::Identity, ipbm = Mat4::Identity;
	bm.InvertTo( ibm );
	pbm.InvertTo( ipbm );
	
	// calc joint matrices
	outself = Mat4::CreateRotationFromQuat( BI.joint.self_rotation ) *
		Mat4::CreateTranslation( BI.joint.self_position ) * ibm;
	outprnt = Mat4::CreateRotationFromQuat( BI.joint.prnt_rotation ) *
		Mat4::CreateTranslation( BI.joint.prnt_position ) * ipbm;
	return true;
}

bool AnimCharacter::GetJointMatrix( int which, bool parent, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	int bid = BI.bone_id;
	if( parent )
	{
		int pb = BI.joint.parent_id;
		if( pb < 0 || pb >= (int) bones.size() )
			return false;
		bid = bones[ pb ].bone_id;
	}
	
	outwm = m_cachedMeshInst->matrix;
	if( bid >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ bid ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ bid ].skinOffset * outwm;
	}
	if( parent )
	{
		outwm = Mat4::CreateRotationFromQuat( BI.joint.prnt_rotation ) *
			Mat4::CreateTranslation( BI.joint.prnt_position ) * outwm;
	}
	else
	{
		outwm = Mat4::CreateRotationFromQuat( BI.joint.self_rotation ) *
			Mat4::CreateTranslation( BI.joint.self_position ) * outwm;
	}
	return true;
}

void AnimCharacter::_GetHitboxMatrix( int which, Mat4& outwm )
{
	BoneInfo& BI = bones[ which ];
	outwm = Mat4::CreateRotationFromQuat( BI.hitbox.rotation ) *
		Mat4::CreateTranslation( BI.hitbox.position );
	if( BI.bone_id >= 0 )
	{
		outwm = outwm * m_cachedMesh->m_bones[ BI.bone_id ].skinOffset;
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = outwm * m_cachedMeshInst->skin_matrices[ BI.bone_id ];
		}
	}
}

bool AnimCharacter::GetHitboxOBB( int which, Mat4& outwm, Vec3& outext )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	if( BI.hitbox.multiplier == 0 )
		return false; // a way to disable it
	
	_GetHitboxMatrix( which, outwm );
	outwm = outwm * m_cachedMeshInst->matrix;
	outext = BI.hitbox.extents;
	return true;
}

bool AnimCharacter::GetAttachmentMatrix( int which, Mat4& outwm, bool worldspace )
{
	outwm = worldspace ? m_cachedMeshInst->matrix : Mat4::Identity;
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) attachments.size() )
		return false;
	Attachment& AT = attachments[ which ];
	
	if( AT.bone_id >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ AT.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ AT.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( AT.rotation ) *
		Mat4::CreateTranslation( AT.position ) * outwm;
	return true;
}

bool AnimCharacter::ApplyMask( const StringView& name, Animator* tgt )
{
	if( !m_cachedMesh )
		return false;
	
	for( size_t i = 0; i < masks.size(); ++i )
	{
		Mask& M = masks[ i ];
		if( M.name != name )
			continue;
		
		Array< float >& factors = tgt->GetBlendFactorArray();
		GR_ClearFactors( factors, 0 );
		for( size_t j = 0; j < M.cmds.size(); ++j )
		{
			MaskCmd& MC = M.cmds[ j ];
			GR_SetFactors( factors, m_cachedMesh, MC.bone, MC.weight, MC.children );
		}
		return true;
	}
	return false;
}

int AnimCharacter::FindAttachment( const StringView& name )
{
	for( size_t i = 0; i < attachments.size(); ++i )
	{
		if( attachments[ i ].name == name )
			return i;
	}
	return -1;
}

void AnimCharacter::SortEnsureAttachments( const StringView* atchnames, int count )
{
	for( int i = 0; i < count; ++i )
	{
		int aid = -1;
		for( size_t j = i; j < attachments.size(); ++j )
		{
			if( attachments[ i ].name == atchnames[ i ] )
			{
				aid = j;
				break;
			}
		}
		if( aid == i )
			continue; // at the right place already
		if( aid != -1 )
		{
			TMEMSWAP( attachments[ i ], attachments[ aid ] );
		}
		else
		{
			attachments.insert( i, Attachment() );
			attachments[ i ].name = atchnames[ i ];
		}
	}
}

void AnimCharacter::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	UNUSED( cbmi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	for( size_t i = 0; i < bones.size(); ++i )
	{
		Mat4 bxf, inv;
		BoneInfo& BI = bones[ i ];
		if( BI.hitbox.multiplier == 0 )
			continue;
		_GetHitboxMatrix( i, bxf );
		if( bxf.InvertTo( inv ) )
		{
			Vec3 p0 = inv.TransformPos( from );
			Vec3 p1 = inv.TransformPos( to );
			float dst[1];
			if( SegmentAABBIntersect2( p0, p1, -BI.hitbox.extents, BI.hitbox.extents, dst ) )
			{
				Vec3 N = ( from - to ).Normalized();
				SceneRaycastInfo srci = { dst[0], N, 0, 0, -1, -1, BI.bone_id, m_cachedMeshInst };
				cb->AddResult( &srci );
			}
		}
	}
}

void AnimCharacter::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	UNUSED( mi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset().Col( 0.1f, 0.5f, 0.9f );
	for( size_t i = 0; i < bones.size(); ++i )
	{
		Mat4 bxf;
		BoneInfo& BI = bones[ i ];
		if( BI.hitbox.multiplier == 0 )
			continue;
		_GetHitboxMatrix( i, bxf );
		br.AABB( -BI.hitbox.extents, BI.hitbox.extents, bxf * m_cachedMeshInst->matrix );
	}
}



SGRX_DecalSystem::SGRX_DecalSystem() : m_lightSampler(NULL), m_vbSize(0)
{
}

SGRX_DecalSystem::~SGRX_DecalSystem()
{
	Free();
}

void SGRX_DecalSystem::Init( SceneHandle scene, TextureHandle texDecal, TextureHandle texFalloff )
{
	m_vertexDecl = GR_GetVertexDecl( SGRX_VDECL_DECAL );
	m_mesh = GR_CreateMesh();
	
	m_meshInst = scene->CreateMeshInstance();
	m_meshInst->SetMesh( m_mesh );
	SGRX_Material mtl;
	mtl.flags = SGRX_MtlFlag_Decal;
	mtl.blendMode = SGRX_MtlBlend_Basic;
	mtl.shader = "decal";
	mtl.textures[0] = texDecal;
	mtl.textures[1] = texFalloff;
	m_meshInst->materials.assign( &mtl, 1 );
	m_meshInst->SetMITexture( 0, GR_GetTexture( "textures/white.png" ) );
	m_meshInst->SetLightingMode( SGRX_LM_Decal );
}

void SGRX_DecalSystem::Free()
{
	ClearAllDecals();
	m_mesh = NULL;
	m_meshInst = NULL;
	m_vbSize = 0;
}

void SGRX_DecalSystem::SetSize( uint32_t vbSize )
{
	m_vbSize = vbSize;
}

void SGRX_DecalSystem::SetDynamic( bool dynamic )
{
	m_meshInst->SetLightingMode( dynamic ? SGRX_LM_Dynamic : SGRX_LM_Decal );
}

void SGRX_DecalSystem::Upload()
{
	if( m_mesh == NULL )
		return;
	
	// cut excess decals
	size_t vbcutsize = 0, vbsize = m_vertexData.size(), ibcutsize = 0, cutcount = 0;
	while( vbsize > m_vbSize + vbcutsize )
	{
		vbcutsize += m_decals[ cutcount++ ];
		ibcutsize += m_decals[ cutcount++ ];
	}
	if( cutcount )
	{
		m_vertexData.erase( 0, vbcutsize );
		m_indexData.erase( 0, ibcutsize );
		m_decals.erase( 0, cutcount );
		uint32_t iboff = vbcutsize / sizeof(SGRX_Vertex_Decal);
		for( size_t i = 0; i < m_indexData.size(); ++i )
		{
			m_indexData[ i ] -= iboff;
		}
	}
	
	// apply data
	if( m_vertexData.size() )
	{
		m_mesh->SetVertexData( m_vertexData.data(), m_vertexData.size_bytes(), m_vertexDecl );
		m_mesh->SetIndexData( m_indexData.data(), m_indexData.size_bytes(), true );
	}
	SGRX_MeshPart mp = { 0, m_vertexData.size() / sizeof(SGRX_Vertex_Decal), 0, m_indexData.size() };
	m_mesh->SetPartData( &mp, 1 );
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, int partID, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color, partID, 1 );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::ClearAllDecals()
{
	m_vertexData.clear();
	m_indexData.clear();
	m_decals.clear();
}

void SGRX_DecalSystem::GenerateCamera( const DecalProjectionInfo& projInfo, SGRX_Camera* out )
{
	// TODO for now...
	ASSERT( projInfo.perspective );
	
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	float dist = DMPI.size.z * projInfo.distanceScale;
	
	out->position = projInfo.pos - projInfo.dir * projInfo.pushBack * dist;
	out->direction = projInfo.dir;
	out->updir = projInfo.up;
	out->angle = projInfo.fovAngleDeg;
	out->aspect = projInfo.aspectMult;
	out->aamix = projInfo.aamix;
	out->znear = dist * 0.001f;
	out->zfar = dist;
	
	out->UpdateMatrices();
}

void SGRX_DecalSystem::_ScaleDecalTexcoords( const DecalProjectionInfo& projInfo, size_t vbfrom )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->texcoord.x = TLERP( DMPI.bbox.x, DMPI.bbox.z, vdata->texcoord.x );
		vdata->texcoord.y = TLERP( DMPI.bbox.y, DMPI.bbox.w, vdata->texcoord.y );
		vdata++;
	}
}

void SGRX_DecalSystem::_InvTransformDecals( size_t vbfrom )
{
	if( m_meshInst->GetLightingMode() == SGRX_LM_Decal )
		return;
	
	Mat4 inv = Mat4::Identity;
	m_meshInst->matrix.InvertTo( inv );
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->position = inv.TransformPos( vdata->position );
		vdata->normal = inv.TransformNormal( vdata->normal );
		vdata++;
	}
}

void SGRX_DecalSystem::_GenDecalMatrix( const DecalProjectionInfo& projInfo, Mat4* outVPM, float* out_invzn2zf )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	float znear = 0, dist = DMPI.size.z * projInfo.distanceScale;
	Mat4 projMtx, viewMtx = Mat4::CreateLookAt(
		projInfo.pos - projInfo.dir * projInfo.pushBack * dist,
		projInfo.dir, projInfo.up );
	if( projInfo.perspective )
	{
		float aspect = DMPI.size.x / DMPI.size.y * projInfo.aspectMult;
		znear = dist * 0.001f;
		projMtx = Mat4::CreatePerspective( projInfo.fovAngleDeg, aspect, projInfo.aamix, znear, dist );
	}
	else
	{
		Vec2 psz = DMPI.size.ToVec2() * 0.5f * projInfo.orthoScale;
		projMtx = Mat4::CreateOrtho( V3( -psz.x, -psz.y, 0 ), V3( psz.x, psz.y, DMPI.size.z * projInfo.distanceScale ) );
	}
	*outVPM = viewMtx * projMtx;
	*out_invzn2zf = safe_fdiv( 1, dist - znear );
}



SGRX_HelpTextRenderer::SGRX_HelpTextRenderer() :
	fontSize(16), centerPos(V2(100,100)), lineHeightFactor(1.2f),
	buttonTexBorder(40), buttonBorder(4), opacity(1),
	m_lineCount(0), m_curLine(0), m_curColor(V4(1))
{
	m_curFont.sizeFactor = 1;
}

void SGRX_HelpTextRenderer::RenderText( StringView text )
{
	m_curLine = 0;
	m_lineCount = text.count( "\n" );
	SetColor( "" );
	SetFont( "" );
	m_strings.clear();
	m_textCache.clear();
	
	StringView start = text, it = text;
	while( it.size() )
	{
		char c = it.ch();
		if( c == '#' )
		{
			AddText( start, it );
			it.skip( 1 );
			char c2 = it.ch();
			if( c2 == '#' )
			{
				start = it;
				it.skip( 1 ); // skip one #, include another
				continue;
			}
			else if( c2 == 'c' || c2 == 'C' ) // color
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView colorname = it.until( ")" );
				SetColor( colorname );
				it.skip( colorname.size() + 1 );
				start = it;
				continue;
			}
			else if( c2 == 'f' || c2 == 'F' ) // font
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView fontname = it.until( ")" );
				SetFont( fontname );
				it.skip( fontname.size() + 1 );
				start = it;
				continue;
			}
			else if( c2 == 'a' || c2 == 'A' ) // action
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView actionname = it.until( ")" );
				AddActionInputText( actionname );
				it.skip( actionname.size() + 1 );
				start = it;
				continue;
			}
		}
		else if( c == '\n' )
		{
			AddText( start, it );
			m_curLine++;
			it.skip( 1 );
			start = it;
		}
		it.skip( 1 );
	}
	if( start.size() )
		AddText( start );
	
	// calculate h-positions
	size_t si = 0;
	int curline = 0;
	int totalWidth = 0;
	for( size_t i = 0; i <= m_textCache.size(); ++i )
	{
		if( i == m_textCache.size() || curline != m_textCache[ i ].lineNum )
		{
			int ho = 0;
			for( ; si < i; ++si )
			{
				m_textCache[ si ].pos.x += ho - totalWidth / 2;
				ho += m_textCache[ si ].width;
			}
			
			if( i == m_textCache.size() )
				break;
			
			curline = m_textCache[ i ].lineNum;
			totalWidth = 0;
		}
		totalWidth += m_textCache[ i ].width;
	}
	
	// draw text items
	for( size_t i = 0; i < m_textCache.size(); ++i )
		DrawTextItem( m_textCache[ i ] );
	
	m_textCache.clear();
}

void SGRX_HelpTextRenderer::AddInputText( ActionInput input )
{
	StringView text = Game_GetInputName( input );
	AddText( text );
	int pad = buttonBorder;
	m_textCache.last().padding = pad;
	m_textCache.last().width += pad * 2;
	m_textCache.last().button = true;
}

void SGRX_HelpTextRenderer::AddActionInputText( StringView action )
{
	InputState* cmd = Game_FindAction( action );
	ActionInput inputs[2];
	int num = Game_GetActionBindings( cmd, inputs, 2 );
	for( int i = 0; i < num; ++i )
	{
		if( i != 0 )
			AddText( "/" );
		AddInputText( inputs[ i ] );
	}
}

void SGRX_HelpTextRenderer::DrawTextItem( Text& item )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	Vec4 c = item.color;
	Vec2 pos = V2( round( item.pos.x ), round( item.pos.y ) );
	if( item.button )
	{
		int hsize = item.fontSize / 2 + item.padding;
		Vec4 rect = V4( pos.x, pos.y - hsize, pos.x + item.width, pos.y + hsize );
		Vec2 texoff = buttonTex.GetInvSize() * buttonTexBorder;
		Vec4 texbdr = V4( texoff.x, texoff.y, texoff.x, texoff.y );
		br.Reset().Col( 1, opacity )
		  .SetTexture( buttonTex ).Button( rect, V4(buttonBorder), texbdr );
		c = V4( V3(0.1f), 1 );
	}
	
	GR2D_SetColor( c.x, c.y, c.z, c.w * opacity );
	GR2D_SetFont( item.font, item.fontSize );
	GR2D_DrawTextLine( item.pos.x + item.padding, item.pos.y, GetText( item ), HALIGN_LEFT, VALIGN_CENTER );
}

void SGRX_HelpTextRenderer::SetColor( StringView name )
{
	Vec4* col = namedColors.getptr( name );
	m_curColor = col ? *col : V4(1);
}

void SGRX_HelpTextRenderer::SetFont( StringView name )
{
	FontInfo* font = namedFonts.getptr( name );
	if( font )
		m_curFont = *font;
	else
	{
		FontInfo dummy = { "", 1 };
		m_curFont = dummy;
	}
}

void SGRX_HelpTextRenderer::AddText( StringView text )
{
	Text T =
	{
		m_strings.size(),
		text.size(),
		m_curFont.name,
		m_curColor,
		centerPos,
		fontSize * m_curFont.sizeFactor,
		0,
		m_curLine,
		false
	};
	m_strings.append( text );
	T.pos.y += ( m_curLine - ( m_lineCount - 1 ) * 0.5f ) * fontSize * lineHeightFactor;
	GR2D_SetFont( T.font, T.fontSize );
	T.width = GR2D_GetTextLength( text );
	m_textCache.push_back( T );
}




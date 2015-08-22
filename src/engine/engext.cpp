

#include "engext.hpp"



//////////////////////
//  R A G D O L L
////////////////////

#if 0

AnimRagdoll::AnimRagdoll() :
	m_enabled( false ),
	m_lastTickSize( 0 )
{
}

void AnimRagdoll::Initialize( PhyWorldHandle world, MeshHandle mesh, SkeletonInfo* skinfo )
{
	m_mesh = mesh;
	
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.enabled = false;
	rbinfo.friction = 0.8f;
	rbinfo.restitution = 0.02f;
	
	for( size_t bid = 0; bid < skinfo->bodies.size(); ++bid )
	{
		SkeletonInfo::Body& SB = skinfo->bodies[ bid ];
		
		Body* TB = NULL;
		for( size_t i = 0; i < m_bones.size(); ++i )
		{
			if( names[ i ] == SB.name )
			{
				TB = &m_bones[ i ];
				break;
			}
		}
		if( !TB )
			continue;
		
	//	LOG << SB.name << " > " << SB.capsule_radius << "|" << SB.capsule_height;
		rbinfo.shape = world->CreateCapsuleShape( SB.capsule_radius, SB.capsule_height );
		rbinfo.mass = 0;//4.0f / 3.0f * M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_radius + M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_height;
		
		rbinfo.inertia = rbinfo.shape->CalcInertia( rbinfo.mass );
		
		TB->relPos = SB.position;
		TB->relRot = SB.rotation;
		TB->bodyHandle = world->CreateRigidBody( rbinfo );
	}
	
	for( size_t jid = 0; jid < skinfo->joints.size(); ++jid )
	{
		SkeletonInfo::Joint& SJ = skinfo->joints[ jid ];
		UNUSED( SJ ); // TODO
	}
}

void AnimRagdoll::Prepare( String* new_names, int count )
{
	Animator::Prepare( new_names, count );
	m_bones.resize( count );
	for( int i = 0; i < count; ++i )
	{
		Body B = { V3(0), Quat::Identity, NULL, V3(0), V3(0), Quat::Identity, Quat::Identity };
		m_bones[ i ] = B;
		position[ i ] = V3( 0 );
		rotation[ i ] = Quat::Identity;
		scale[ i ] = V3( 1 );
		factor[ i ] = 1;
	}
}

void AnimRagdoll::Advance( float deltaTime )
{
	m_lastTickSize = deltaTime;
	if( m_enabled == false )
		return;
	for( size_t i = 0; i < names.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
		{
			Vec3 pos = B.bodyHandle->GetPosition();
			Quat rot = B.bodyHandle->GetRotation();
			Vec3 invRelPos = -B.relPos;
			Quat invRelRot = B.relRot.Inverted();
			position[ i ] = invRelRot.Transform( pos - invRelPos );
			rotation[ i ] = rot * invRelRot;
		}
	}
}

void AnimRagdoll::SetBoneTransforms( int bone_id, const Vec3& prev_pos, const Vec3& curr_pos, const Quat& prev_rot, const Quat& curr_rot )
{
	ASSERT( bone_id >= 0 && bone_id < (int) names.size() );
	Body& B = m_bones[ bone_id ];
	B.prevPos = prev_pos;
	B.currPos = curr_pos;
	B.prevRot = prev_rot;
	B.currRot = curr_rot;
}

void AnimRagdoll::AdvanceTransforms( Animator* anim )
{
	ASSERT( names.size() == anim->names.size() );
	for( size_t i = 0; i < names.size(); ++i )
	{
		Body& B = m_bones[ i ];
		B.prevPos = B.currPos;
		B.prevRot = B.currRot;
		B.currPos = anim->position[ i ];
		B.currRot = anim->rotation[ i ];
	}
}

void AnimRagdoll::EnablePhysics( const Mat4& worldMatrix )
{
	if( m_enabled )
		return;
	m_enabled = true;
	
	Mat4 prev_boneToWorldMatrices[ MAX_MESH_BONES ];
	Mat4 curr_boneToWorldMatrices[ MAX_MESH_BONES ];
	SGRX_MeshBone* MB = m_mesh->m_bones;
	for( size_t i = 0; i < names.size(); ++i )
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
	}
	for( size_t i = 0; i < m_joints.size(); ++i )
	{
		m_joints[ i ]->SetEnabled( true );
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
		if( B.bodyHandle )
			B.bodyHandle->SetEnabled( false );
	}
	for( size_t i = 0; i < m_joints.size(); ++i )
	{
		m_joints[ i ]->SetEnabled( false );
	}
}

#endif



AnimCharacter::AnimCharacter()
{
	m_anDeformer.animSource = &m_anMixer;
	m_anEnd.animSource = &m_anDeformer;
}

bool AnimCharacter::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( &ba );
	Serialize( br );
	if( br.error )
		return false;
	
	if( m_scene )
		OnRenderUpdate();
	else
	{
		m_cachedMesh = GR_GetMesh( mesh );
		RecalcBoneIDs();
	}
	return true;
}

bool AnimCharacter::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void AnimCharacter::OnRenderUpdate()
{
	if( m_scene == NULL )
		return;
	
	if( m_cachedMeshInst == NULL )
	{
		m_cachedMeshInst = m_scene->CreateMeshInstance();
	}
	m_cachedMesh = GR_GetMesh( mesh );
	m_cachedMeshInst->mesh = m_cachedMesh;
	m_cachedMeshInst->skin_matrices.resize( m_cachedMesh ? m_cachedMesh->m_numBones : 0 );
	RecalcBoneIDs();
	m_anEnd.Prepare( m_cachedMesh );
	if( m_cachedMesh && (int) m_layerAnimator.m_factors.size() != m_cachedMesh->m_numBones )
		m_layerAnimator.Prepare( m_cachedMesh );
	m_layerAnimator.ClearFactors( 1.0f );
}

void AnimCharacter::AddToScene( SceneHandle sh )
{
	m_scene = sh;
	
	OnRenderUpdate();
}

void AnimCharacter::SetTransform( const Mat4& mtx )
{
	if( m_cachedMeshInst )
		m_cachedMeshInst->matrix = mtx;
}

void AnimCharacter::FixedTick( float deltaTime )
{
	m_anEnd.Advance( deltaTime );
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
				m_layerAnimator.m_positions[ LT.bone_id ] += LT.posaxis * L.amount;
				break;
			case TransformType_Rotate:
				m_layerAnimator.m_rotations[ LT.bone_id ] = m_layerAnimator.m_rotations[ LT.bone_id ]
					* Quat::CreateAxisAngle( LT.posaxis.Normalized(), DEG2RAD( LT.angle ) * L.amount );
				break;
			}
		}
	}
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
	if( !m_cachedMesh || !m_cachedMeshInst )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	
	outwm = m_cachedMeshInst->matrix;
	if( BI.bone_id >= 0 )
	{
		if( m_cachedMeshInst->skin_matrices.size() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ BI.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ BI.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ) * outwm;
	return true;
}

bool AnimCharacter::GetHitboxOBB( int which, Mat4& outwm, Vec3& outext )
{
	if( !m_cachedMesh || !m_cachedMeshInst )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	if( BI.hitbox.multiplier == 0 )
		return false; // a way to disable it
	
	outwm = m_cachedMeshInst->matrix;
	if( BI.bone_id >= 0 )
	{
		if( m_cachedMeshInst->skin_matrices.size() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ BI.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ BI.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( BI.hitbox.rotation ) *
		Mat4::CreateTranslation( BI.hitbox.position ) * outwm;
	outext = BI.hitbox.extents;
	return true;
}

bool AnimCharacter::GetAttachmentMatrix( int which, Mat4& outwm )
{
	if( !m_cachedMesh || !m_cachedMeshInst )
		return false;
	if( which < 0 || which >= (int) attachments.size() )
		return false;
	Attachment& AT = attachments[ which ];
	
	outwm = m_cachedMeshInst->matrix;
	if( AT.bone_id >= 0 )
	{
		if( m_cachedMeshInst->skin_matrices.size() )
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
	if( !m_cachedMesh || !m_cachedMeshInst )
		return;
	for( size_t i = 0; i < bones.size(); ++i )
	{
		BoneInfo& BI = bones[ i ];
		Mat4 bxf = Mat4::CreateRotationFromQuat( BI.hitbox.rotation ) *
			Mat4::CreateTranslation( BI.hitbox.position );
		if( BI.bone_id >= 0 )
		{
			bxf = bxf * m_cachedMesh->m_bones[ BI.bone_id ].skinOffset;
			if( m_cachedMeshInst->skin_matrices.size() )
			{
				bxf = bxf * m_cachedMeshInst->skin_matrices[ BI.bone_id ];
			}
		}
		bxf = bxf * m_cachedMeshInst->matrix;
		
		Mat4 inv;
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



SGRX_DecalSystem::SGRX_DecalSystem() : m_lightSampler(NULL), m_ownMatrix(NULL), m_vbSize(0)
{
}

SGRX_DecalSystem::~SGRX_DecalSystem()
{
	Free();
}

void SGRX_DecalSystem::Init( TextureHandle texDecal, TextureHandle texFalloff )
{
	m_vertexDecl = GR_GetVertexDecl( SGRX_VDECL_DECAL );
	m_mesh = GR_CreateMesh();
	m_material = GR_CreateMaterial(); // TODO: pass loaded material when there is something to load
	m_material->blendMode = MBM_BASIC;
	m_material->shader = GR_GetSurfaceShader( "decal" );
	m_material->textures[0] = texDecal;
	m_material->textures[1] = texFalloff;
}

void SGRX_DecalSystem::Free()
{
	ClearAllDecals();
	m_mesh = NULL;
	m_vbSize = 0;
}

void SGRX_DecalSystem::SetSize( uint32_t vbSize )
{
	m_vbSize = vbSize;
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
		m_mesh->SetVertexData( m_vertexData.data(), m_vertexData.size_bytes(), m_vertexDecl, true );
		m_mesh->SetIndexData( m_indexData.data(), m_indexData.size_bytes(), true );
	}
	SGRX_MeshPart mp = { 0, m_vertexData.size() / sizeof(SGRX_Vertex_Decal), 0, m_indexData.size(), m_material };
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
	if( m_ownMatrix == NULL )
		return;
	
	Mat4 inv = Mat4::Identity;
	m_ownMatrix->InvertTo( inv );
	
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




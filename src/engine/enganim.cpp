

#define USE_VEC4
#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "enganim.hpp"
#include "physics.hpp"


SGRX_Animation::~SGRX_Animation()
{
}

Vec3* SGRX_Animation::GetPosition( int track )
{
	return (Vec3*) &data[ track * 10 * frameCount ];
}

Quat* SGRX_Animation::GetRotation( int track )
{
	return (Quat*) &data[ track * 10 * frameCount + 3 * frameCount ];
}

Vec3* SGRX_Animation::GetScale( int track )
{
	return (Vec3*) &data[ track * 10 * frameCount + 7 * frameCount ];
}

void SGRX_Animation::GetState( int track, float framePos, Vec3& outpos, Quat& outrot, Vec3& outscl )
{
	Vec3* pos = GetPosition( track );
	Quat* rot = GetRotation( track );
	Vec3* scl = GetScale( track );
	
	if( framePos < 0 )
		framePos = 0;
	else if( framePos > frameCount )
		framePos = frameCount;
	
	int f0 = floor( framePos );
	int f1 = f0 + 1;
	if( f1 >= frameCount )
		f1 = f0;
	float q = framePos - f0;
	
	outpos = TLERP( pos[ f0 ], pos[ f1 ], q );
	outrot = TLERP( rot[ f0 ], rot[ f1 ], q );
	outscl = TLERP( scl[ f0 ], scl[ f1 ], q );
}

bool SGRX_Animation::CheckMarker( const StringView& name, float fp0, float fp1 )
{
	for( size_t i = 0; i < markers.size(); ++i )
	{
		if( markers[ i ].GetName() == name &&
			fp0 <= float(markers[ i ].frame) &&
			fp1 > float(markers[ i ].frame) )
			return true;
	}
	return false;
}

void Animator::Prepare( String* new_names, int count )
{
	names.assign( new_names, count );
	position.clear();
	rotation.clear();
	scale.clear();
	factor.clear();
	position.resize( count );
	rotation.resize( count );
	scale.resize( count );
	factor.resize( count );
	for( int i = 0; i < count; ++i )
	{
		position[ i ] = V3(0);
		rotation[ i ] = Quat::Identity;
		scale[ i ] = V3(1);
		factor[ i ] = 0;
	}
}

bool Animator::PrepareForMesh( const MeshHandle& mesh )
{
	SGRX_IMesh* M = mesh;
	if( !M )
		return false;
	Array< String > bonenames;
	bonenames.resize( M->m_numBones );
	for( int i = 0; i < M->m_numBones; ++i )
		bonenames[ i ] = M->m_bones[ i ].name;
	Prepare( bonenames.data(), bonenames.size() );
	return true;
}

Array< float >& Animator::GetBlendFactorArray()
{
	return factor;
}


AnimMixer::AnimMixer() : layers(NULL), layerCount(0)
{
}

AnimMixer::~AnimMixer()
{
}

void AnimMixer::Prepare( String* new_names, int count )
{
	Animator::Prepare( new_names, count );
	m_staging.resize( count );
	for( int i = 0; i < layerCount; ++i )
	{
		layers[ i ].anim->Prepare( new_names, count );
	}
}

void AnimMixer::Advance( float deltaTime )
{
	// generate output
	for( size_t i = 0; i < names.size(); ++i )
	{
		position[ i ] = V3(0);
		rotation[ i ] = Quat::Identity;
		scale[ i ] = V3(1);
		factor[ i ] = 0;
	}
	
	for( int layer = 0; layer < layerCount; ++layer )
	{
		Animator* AN = layers[ layer ].anim;
		AN->Advance( deltaTime );
		
		int tflags = layers[ layer ].tflags;
		bool abslayer = ( tflags & TF_Absolute_All ) && mesh && mesh->m_numBones == (int) names.size();
		SGRX_MeshBone* MB = mesh->m_bones;
		
		for( size_t i = 0; i < names.size(); ++i )
		{
			Vec3 P = AN->position[ i ];
			Quat R = AN->rotation[ i ];
			Vec3 S = AN->scale[ i ];
			float q = AN->factor[ i ] * layers[ layer ].factor;
			
			if( abslayer )
			{
				// to matrix
				Mat4 tfm = Mat4::CreateSRT( S, R, P );
				
				// extract diff
				Mat4 orig = MB[ i ].boneOffset;
				if( MB[ i ].parent_id >= 0 )
				{
					orig = orig * m_staging[ MB[ i ].parent_id ];
				}
				Mat4 inv = Mat4::Identity;
				orig.InvertTo( inv );
				Mat4 diff = tflags & TF_Additive ?
					Mat4::CreateSRT( scale[ i ], rotation[ i ], position[ i ] ) * orig * tfm * inv :
					tfm * inv;
		//		if( q )
		//			LOG << i << ":\t" << R;
			//	if( q )
			//		LOG << i << ":\t" << diff.GetRotationQuaternion() << rotation[i];
				
				// convert back to SRP
				P = ( tflags & TF_Absolute_Pos ) ? diff.GetTranslation() : position[ i ];
				R = ( tflags & TF_Absolute_Rot ) ? diff.GetRotationQuaternion() : rotation[ i ];
				S = ( tflags & TF_Absolute_Scale ) ? diff.GetScale() : scale[ i ];
				
				m_staging[ i ] = Mat4::CreateSRT( scale[ i ], rotation[ i ], position[ i ] ) * orig;
			}
			
			if( !factor[ i ] )
			{
				position[ i ] = P;
				rotation[ i ] = R;
				scale[ i ] = S;
				factor[ i ] = q;
			}
			else
			{
				position[ i ] = TLERP( position[ i ], P, q );
				rotation[ i ] = TLERP( rotation[ i ], R, q );
				scale[ i ] = TLERP( scale[ i ], S, q );
				factor[ i ] = TLERP( factor[ i ], 1.0f, q );
			}
		}
	}
}

AnimPlayer::AnimPlayer()
{
}

AnimPlayer::~AnimPlayer()
{
	_clearAnimCache();
}

void AnimPlayer::Prepare( String* new_names, int count )
{
	currentAnims.clear();
	_clearAnimCache();
	Animator::Prepare( new_names, count );
	blendFactor.resize( count );
	for( int i = 0; i < count; ++i )
		blendFactor[ i ] = 1;
}

void AnimPlayer::Advance( float deltaTime )
{
	// process tracks
	for( size_t i = currentAnims.size(); i > 0; )
	{
		--i;
		
		Anim& A = currentAnims[ i ];
		SGRX_Animation* AN = A.anim;
		A.at += deltaTime;
		A.prev_fade_at = A.fade_at;
		A.fade_at += deltaTime;
		
		float animTime = AN->frameCount / AN->speed;
		if( !A.once )
		{
			A.at = fmodf( A.at, animTime );
			// permanent animation faded fully in, no need to keep previous tracks
			if( A.fade_at > A.fadetime )
			{
				currentAnims.erase( 0, i );
				break;
			}
		}
		// temporary animation has finished playback
		else if( A.at >= animTime )
		{
			currentAnims.erase( i );
			continue;
		}
	}
	
	// generate output
	for( size_t i = 0; i < names.size(); ++i )
	{
		position[ i ] = V3(0);
		rotation[ i ] = Quat::Identity;
		scale[ i ] = V3(1);
		factor[ i ] = 0;
	}
	for( size_t an = 0; an < currentAnims.size(); ++an )
	{
		Anim& A = currentAnims[ an ];
		SGRX_Animation* AN = A.anim;
		
		Vec3 P = V3(0), S = V3(1);
		Quat R = Quat::Identity;
		for( size_t i = 0; i < names.size(); ++i )
		{
			int tid = A.trackIDs[ i ];
			if( tid < 0 )
				continue;
			
			AN->GetState( tid, A.at * AN->speed, P, R, S );
			float animTime = AN->frameCount / AN->speed;
			float q = A.once ?
				smoothlerp_range( A.fade_at, 0, A.fadetime, animTime - A.fadetime, animTime ) :
				smoothlerp_oneway( A.fade_at, 0, A.fadetime );
			if( !factor[ i ] )
			{
				position[ i ] = P;
				rotation[ i ] = R;
				scale[ i ] = S;
				factor[ i ] = q;
			}
			else
			{
				position[ i ] = TLERP( position[ i ], P, q );
				rotation[ i ] = TLERP( rotation[ i ], R, q );
				scale[ i ] = TLERP( scale[ i ], S, q );
				factor[ i ] = TLERP( factor[ i ], 1.0f, q );
			}
		}
	}
	for( size_t i = 0; i < names.size(); ++i )
		factor[ i ] *= blendFactor[ i ];
}

void AnimPlayer::Play( const AnimHandle& anim, bool once, float fadetime )
{
	if( !anim )
		return;
	if( !once && currentAnims.size() && currentAnims.last().once == false && currentAnims.last().anim == anim )
		return; // ignore repetitive invocations
	Anim A = { anim, _getTrackIds( anim ), 0, 0, 0, fadetime, once };
	currentAnims.push_back( A );
}

bool AnimPlayer::CheckMarker( const StringView& name )
{
	for( size_t i = 0; i < currentAnims.size(); ++i )
	{
		Anim& A = currentAnims[ i ];
		SGRX_Animation* AN = A.anim;
		float fp0 = A.prev_fade_at * AN->speed;
		float fp1 = A.fade_at * AN->speed;
		if( fp0 == fp1 )
			continue; // prevent 'thrilling' markers
		if( A.once == false )
		{
			float pfp1 = fmodf( fp1, AN->frameCount );
			fp0 += ( pfp1 - fp1 );
			fp1 = pfp1;
			// fp1 is inside [0;frameCount), fp0 may be < 0
			if( fp0 < 0 )
			{
				// check +frameCount range as well
				if( AN->CheckMarker( name, fp0 + AN->frameCount, fp1 + AN->frameCount ) )
					return true;
			}
		}
		if( AN->CheckMarker( name, fp0, fp1 ) )
			return true;
	}
	return false;
}

int* AnimPlayer::_getTrackIds( const AnimHandle& anim )
{
	if( !names.size() )
		return NULL;
	int* ids = animCache.getcopy( anim );
	if( !ids )
	{
		ids = new int [ names.size() ];
		for( size_t i = 0; i < names.size(); ++i )
		{
			ids[ i ] = anim->trackNames.find_first_at( names[ i ] );
		}
		animCache.set( anim, ids );
	}
	return ids;
}

void AnimPlayer::_clearAnimCache()
{
	for( size_t i = 0; i < animCache.size(); ++i )
	{
		delete [] animCache.item( i ).value;
	}
	animCache.clear();
}

Array< float >& AnimPlayer::GetBlendFactorArray()
{
	return blendFactor;
}


AnimInterp::AnimInterp() : animSource( NULL )
{
}

void AnimInterp::Prepare( String* new_names, int count )
{
	Animator::Prepare( new_names, count );
	prev_position.resize( count );
	prev_rotation.resize( count );
	prev_scale.resize( count );
	if( animSource )
		animSource->Prepare( new_names, count );
}

void AnimInterp::Advance( float deltaTime )
{
	Transfer();
	animSource->Advance( deltaTime );
}

void AnimInterp::Transfer()
{
	for( size_t i = 0; i < names.size(); ++i )
	{
		prev_position[ i ] = animSource->position[ i ];
		prev_rotation[ i ] = animSource->rotation[ i ];
		prev_scale[ i ] = animSource->scale[ i ];
	}
}

void AnimInterp::Interpolate( float deltaTime )
{
	for( size_t i = 0; i < names.size(); ++i )
	{
		position[ i ] = TLERP( prev_position[ i ], animSource->position[ i ], deltaTime );
		rotation[ i ] = TLERP( prev_rotation[ i ], animSource->rotation[ i ], deltaTime );
		scale[ i ] = TLERP( prev_scale[ i ], animSource->scale[ i ], deltaTime );
	}
}


void GR_ClearFactors( Array< float >& out, float factor )
{
	TMEMSET( out.data(), out.size(), factor );
}

void GR_SetFactors( Array< float >& out, const MeshHandle& mesh, const StringView& name, float factor, bool ch )
{
	int subbones[ MAX_MESH_BONES ];
	int numsb = 0;
	GR_FindBones( subbones, numsb, mesh, name, ch );
	for( int i = 0; i < numsb; ++i )
	{
		out[ subbones[ i ] ] = factor;
	}
}

void GR_FindBones( int* subbones, int& numsb, const MeshHandle& mesh, const StringView& name, bool ch )
{
	if( !mesh )
		return;
	
	int numbones = mesh->m_numBones;
	SGRX_MeshBone* mbones = mesh->m_bones;
	int b = 0;
	for( ; b < numbones; ++b )
	{
		if( mbones[ b ].name == name )
			break;
	}
	if( b >= numbones )
		return;
	
	int sbstart = numsb;
	subbones[ numsb++ ] = b;
	if( ch )
	{
		for( ; b < numbones; ++b )
		{
			for( int sb = sbstart; sb < numsb; ++sb )
			{
				if( subbones[ sb ] == mbones[ b ].parent_id )
				{
					subbones[ numsb++ ] = b;
					break;
				}
			}
		}
	}
}


bool GR_ApplyAnimator( const Animator* animator, MeshHandle mh, Mat4* out, size_t outsz, bool applyinv, Mat4* base )
{
	SGRX_IMesh* mesh = mh;
	if( !mesh )
		return false;
	if( outsz != animator->position.size() )
		return false;
	if( outsz != (size_t) mesh->m_numBones )
		return false;
	SGRX_MeshBone* MB = mesh->m_bones;
	
	for( size_t i = 0; i < outsz; ++i )
	{
		Mat4& M = out[ i ];
		M = Mat4::CreateSRT( animator->scale[ i ], animator->rotation[ i ], animator->position[ i ] ) * MB[ i ].boneOffset;
		if( MB[ i ].parent_id >= 0 )
			M = M * out[ MB[ i ].parent_id ];
		else if( base )
			M = M * *base;
	}
	if( applyinv )
	{
		for( size_t i = 0; i < outsz; ++i )
		{
			Mat4& M = out[ i ];
			M = MB[ i ].invSkinOffset * M;
		}
	}
	return true;
}

bool GR_ApplyAnimator( const Animator* animator, MeshInstHandle mih )
{
	if( !mih )
		return false;
	return GR_ApplyAnimator( animator, mih->mesh, mih->skin_matrices.data(), mih->skin_matrices.size() );
}



SGRX_ScenePSRaycast::SGRX_ScenePSRaycast() : layers(0xffffffff)
{
}

void SGRX_ScenePSRaycast::Raycast( DATA_IN* rays, DATA_OUT* isects, size_t count )
{
	SceneRaycastInfo info;
	for( size_t i = 0; i < count; ++i )
	{
		DATA_IN& R = rays[ i ];
		DATA_OUT& IS = isects[ i ];
		if( scene->RaycastOne( R.p1, R.p2, &info, layers ) )
		{
			IS.factor = info.factor;
			IS.normal = info.normal;
			DoFX( TLERP( R.p1, R.p2, info.factor ), info.normal, R.isect_fx );
		}
		else
			IS.factor = -1;
	}
}

void SGRX_ScenePSRaycast::DoFX( const Vec3& pos, const Vec3& nrm, uint32_t fx )
{
}


void ParticleSystem::Emitter::Tick( ParticleSystem* PS, float dt )
{
	if( state_SpawnCurrCount < state_SpawnTotalCount )
	{
		state_SpawnCurrTime = clamp( state_SpawnCurrTime + dt, 0, state_SpawnTotalTime );
		int currcount = state_SpawnCurrTime / state_SpawnTotalTime * state_SpawnTotalCount;
		if( state_SpawnCurrCount < currcount )
		{
			Generate( PS, currcount - state_SpawnCurrCount );
			state_SpawnCurrCount = currcount;
		}
	}
	
	int isectlim = isect_Limit;
	
	Vec3 accel = PS->gravity * tick_GravityMult; // ??? * PS->globalScale;
	for( size_t i = 0; i < particles_Position.size(); ++i )
	{
		Vec2& LT = particles_Lifetime[ i ];
		LT.x += LT.y * dt;
		if( LT.x >= 1 )
		{
particle_remove:
			// remove particle
			particles_Position.uerase( i );
			particles_Velocity.uerase( i );
			particles_Lifetime.uerase( i );
			particles_RandSizeAngVel.uerase( i );
			particles_RandColor.uerase( i );
			particles_Group.uerase( i );
			i--;
			continue;
		}
		Vec3& P = particles_Position[ i ];
		Vec3& V = particles_Velocity[ i ];
		V += accel * dt;
		
		if( isectlim > 0 )
		{
			Vec3 P1 = P + V * dt;
			
			// TODO optimize
			SGRX_ScenePSRaycast rcsys;
			rcsys.scene = PS->m_scene;
			SGRX_IPSRaycast* PSRC = PS->m_psRaycast ? PS->m_psRaycast : &rcsys;
			
			uint32_t fxid = isect_FXChance >= randf() ? isect_FXID : 0;
			SGRX_IPSRaycast::DATA_IN din = { P, P1, fxid };
			SGRX_IPSRaycast::DATA_OUT dout;
			PSRC->Raycast( &din, &dout, 1 );
			if( dout.factor <= 0 )
				P = P1;
			else
			{
				if( isect_Remove )
					goto particle_remove;
				
				Vec3 NV = Vec3Dot( V, dout.normal ) * dout.normal;
				Vec3 TV = V - NV;
				V = -NV * isect_Bounce + TV * ( 1 - isect_Friction );
				P = TLERP( P, P1, dout.factor ) + V * dt * ( 1 - dout.factor );
			}
			isectlim--;
		}
		else
		{
			P += V * dt;
		}
	}
	
	state_lastDelta = dt;
}


static FINLINE Vec3 _ps_rotate( const Vec3& v, const Vec3& axis, float angle )
{
	// http://en.wikipedia.org/wiki/Axis–angle_representation#Rotating_a_vector
	
	float cos_a = cosf( angle );
	float sin_a = sinf( angle );
	Vec3 cross = Vec3Cross( axis, v );
	float dot = Vec3Dot( axis, v );
	
	return cos_a * v + sin_a * cross + ( 1 - cos_a ) * dot * axis;
}

static FINLINE Vec3 _ps_diverge( const Vec3& dir, float dvg )
{
	float baseangle = randf() * M_PI * 2;
	float rotangle = randf() * M_PI * dvg;
	Vec3 axis = { cosf( baseangle ), sinf( baseangle ), 0 };
	
	return _ps_rotate( dir, axis, rotangle );
}

static FINLINE int randi( int x )
{
	if( !x )
		return 0;
	return rand() % x;
}


void ParticleSystem::Emitter::Generate( ParticleSystem* PS, int count )
{
	if( count == 0 )
		return;
	
	Vec3 velMicroDir = create_VelMicroDir.Normalized();
	Vec3 velMacroDir = create_VelMacroDir.Normalized();
	
	int clusterleft = 0;
	float clusterdist = 0;
	Vec3 clusteraxis = {0,0,0};
	float clusterangle = 0;
	
	int allocpos = 0;
	
	for( int i = 0; i < count; ++i )
	{
		if( clusterleft <= 0 )
		{
			clusterleft = create_VelCluster + randi( create_VelClusterExt );
			clusterdist = create_VelMacroDistExt.x + create_VelMacroDistExt.y * randf();
			Vec3 clusterdir = _ps_diverge( velMacroDir, create_VelMacroDvg );
			clusteraxis = Vec3Cross( V3(0,0,1), clusterdir ).Normalized();
			clusterangle = acos( clamp( Vec3Dot( V3(0,0,1), clusterdir ), -1, 1 ) );
		}
		clusterleft--;
		
		// Lifetime
		float LT = create_LifetimeExt.x + create_LifetimeExt.y * randf();
		if( LT <= 0 )
			continue;
		Vec2 LTV = { 0, 1.0f / LT };
		
		// Position
		Vec3 P = create_Pos;
		P += V3( create_PosBox.x * randf11(), create_PosBox.y * randf11(), create_PosBox.z * randf11() );
		float pos_ang = randf() * M_PI * 2, pos_zang = randf() * M_PI, pos_len = randf() * create_PosRadius;
		float cos_zang = cos( pos_zang ), sin_zang = sin( pos_zang );
		P += V3( cos( pos_ang ) * sin_zang, sin( pos_ang ) * sin_zang, cos_zang ) * pos_len;
		
		// Velocity
		Vec3 V = _ps_diverge( velMicroDir, create_VelMicroDvg );
		V = _ps_rotate( V, clusteraxis, clusterangle );
		V *= clusterdist + create_VelMicroDistExt.x + create_VelMicroDistExt.y * randf();
		
		P *= PS->globalScale;
		V *= PS->globalScale;
		
		// absolute positioning
		if( absolute )
		{
			P = PS->m_transform.TransformPos( P );
			V /= PS->m_transform.GetScale();
			V = PS->m_transform.TransformNormal( V );
		}
		
		// size, angle
		Vec3 randSAV =
		{
			randf(),
			create_AngleDirDvg.x + create_AngleDirDvg.y * randf11(),
			create_AngleVelDvg.x + create_AngleVelDvg.y * randf11()
		};
		// color [HSV], opacity
		Vec4 randHSVO = { randf(), randf(), randf(), randf() };
		
		if( particles_Position.size() < (size_t) spawn_MaxCount )
		{
			particles_Position.push_back( P );
			particles_Velocity.push_back( V );
			particles_Lifetime.push_back( LTV );
			particles_RandSizeAngVel.push_back( randSAV );
			particles_RandColor.push_back( randHSVO );
			particles_Group.push_back( PS->m_nextGroup );
		}
		else
		{
			int i = ( allocpos++ ) % spawn_MaxCount;
			particles_Position[ i ] = P;
			particles_Velocity[ i ] = V;
			particles_Lifetime[ i ] = LTV;
			particles_RandSizeAngVel[ i ] = randSAV;
			particles_RandColor[ i ] = randHSVO;
			particles_Group[ i ] = PS->m_nextGroup;
		}
	}
}

void ParticleSystem::Emitter::Trigger( ParticleSystem* PS )
{
	state_SpawnTotalCount = spawn_Count + randi( spawn_CountExt );
	state_SpawnCurrCount = 0;
	state_SpawnTotalTime = spawn_TimeExt.x + spawn_TimeExt.y * randf();
	state_SpawnCurrTime = 0;
	if( state_SpawnTotalTime == 0 )
	{
		Generate( PS, state_SpawnTotalCount );
		state_SpawnCurrCount += state_SpawnTotalCount;
	}
}


void ParticleSystem::Emitter::PreRender( ParticleSystem* PS, ps_prerender_info& info )
{
	const Vec3 S_X = info.basis[0];
	const Vec3 S_Y = info.basis[1];
	const Vec3 S_Z = info.basis[2];
	
	size_t bv = PS->m_vertices.size();
	
	for( size_t i = 0; i < particles_Position.size(); ++i )
	{
		// step 1: extract data
		Vec3 POS = particles_Position[ i ];
		Vec3 VEL = particles_Velocity[ i ];
		Vec2 LFT = particles_Lifetime[ i ];
		Vec3 SAV = particles_RandSizeAngVel[ i ];
		Vec4 RCO = particles_RandColor[ i ];
		uint16_t GRP = particles_Group[ i ];
		
		if( absolute == false )
		{
			POS = info.transform.TransformPos( POS );
			VEL = info.transform.TransformNormal( VEL );
		}
		
		// step 2: fill in the missing data
		float q = LFT.x;
		float t = LFT.x / LFT.y;
		float ANG = SAV.y + ( SAV.z + tick_AngleAcc * t ) * t;
		float SIZ = curve_Size.GetValue( q, SAV.x ) * PS->globalScale;
		Vec4 COL = V4
		(
			HSV(
				V3(
					curve_ColorHue.GetValue( q, RCO.x ),
					curve_ColorSat.GetValue( q, RCO.y ),
					curve_ColorVal.GetValue( q, RCO.z )
				)
			),
			curve_Opacity.GetValue( q, RCO.w )
		);
		
		if( render_Additive == false )
		{
			Vec3 color = PS->m_groups[ GRP ].color;
			COL.x *= color.x;
			COL.y *= color.y;
			COL.z *= color.z;
		}
		
		// step 3: generate vertex/index data
		if( render_Stretch )
		{
			uint16_t cv = PS->m_vertices.size() - bv;
			uint16_t idcs[18] =
			{
				cv+0, cv+2, cv+3, cv+3, cv+1, cv+0,
				cv+2, cv+4, cv+5, cv+5, cv+3, cv+2,
				cv+4, cv+6, cv+7, cv+7, cv+5, cv+4,
			};
			PS->m_indices.append( idcs, 18 );
			
			Vec3 P_2 = POS - VEL * state_lastDelta;
			
			Vec2 proj1 = info.viewProj.TransformPos( POS ).ToVec2();
			Vec2 proj2 = info.viewProj.TransformPos( P_2 ).ToVec2();
			Vec2 dir2d = ( proj2 - proj1 ).Normalized();
			
			// invert Y
			Vec3 D_X = ( S_X * dir2d.x - S_Y * dir2d.y ).Normalized();
			Vec3 D_Y = Vec3Cross( S_Z, D_X ).Normalized();
			
			Vertex verts[8] =
			{
				{ P_2 + ( -D_X -D_Y ) * SIZ, COL,   0,   0, 0 },
				{ P_2 + ( -D_X +D_Y ) * SIZ, COL,   0, 255, 0 },
				{ P_2 + (      -D_Y ) * SIZ, COL, 127,   0, 0 },
				{ P_2 + (      +D_Y ) * SIZ, COL, 127, 255, 0 },
				{ POS + (      -D_Y ) * SIZ, COL, 127,   0, 0 },
				{ POS + (      +D_Y ) * SIZ, COL, 127, 255, 0 },
				{ POS + ( +D_X -D_Y ) * SIZ, COL, 255,   0, 0 },
				{ POS + ( +D_X +D_Y ) * SIZ, COL, 255, 255, 0 },
			};
			PS->m_vertices.append( verts, 8 );
		}
		else
		{
			uint16_t cv = PS->m_vertices.size() - bv;
			uint16_t idcs[6] = { cv, cv+1, cv+2, cv+2, cv+3, cv };
			PS->m_indices.append( idcs, 6 );
			
			float ang = M_PI + ANG;
			Vec3 RSX = _ps_rotate( S_X, S_Z, ang );
			Vec3 RSY = _ps_rotate( S_Y, S_Z, ang );
			
			Vertex verts[4] =
			{
				{ POS + ( -RSX -RSY ) * SIZ, COL,   0,   0, 0 },
				{ POS + ( +RSX -RSY ) * SIZ, COL, 255,   0, 0 },
				{ POS + ( +RSX +RSY ) * SIZ, COL, 255, 255, 0 },
				{ POS + ( -RSX +RSY ) * SIZ, COL,   0, 255, 0 },
			};
			PS->m_vertices.append( verts, 4 );
		}
	}
}


bool ParticleSystem::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( &ba );
	Serialize( br, false );
	return !br.error;
}

bool ParticleSystem::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw, false );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void ParticleSystem::OnRenderUpdate()
{
	if( !m_scene )
		return;
	
	if( !m_vdecl )
		m_vdecl = GR_GetVertexDecl( PARTICLE_VDECL );
	
	m_meshInsts.resize( emitters.size() );
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		Emitter& E = emitters[ i ];
		
		if( !m_meshInsts[ i ] )
			m_meshInsts[ i ] = m_scene->CreateMeshInstance();
		if( !m_meshInsts[ i ]->mesh )
			m_meshInsts[ i ]->mesh = GR_CreateMesh();
		
	//	m_meshInsts[ i ]->matrix = Mat4::Identity; // E.absolute ? Mat4::Identity : m_transform;
	//	m_meshInsts[ i ]->transparent = 1;
	//	m_meshInsts[ i ]->additive = E.render_Additive;
	//	m_meshInsts[ i ]->unlit = E.render_Additive;
		
		SGRX_MeshPart MP = { 0, 0, 0, 0 };
		
		MaterialHandle mh = GR_CreateMaterial();
		mh->blendMode = E.render_Additive ? MBM_ADDITIVE : MBM_BASIC;
		mh->flags = E.render_Additive * MFL_UNLIT;
		mh->shader = GR_GetSurfaceShader( String_Concat( E.render_Shader, "+PARTICLE" ) );
		for( int t = 0; t < NUM_PARTICLE_TEXTURES; ++t )
			mh->textures[ t ] = E.render_Textures[ t ];
		MP.material = mh;
		
		m_meshInsts[ i ]->mesh->SetPartData( &MP, 1 );
	}
}

void ParticleSystem::AddToScene( SceneHandle sh )
{
	if( !m_vdecl )
		m_vdecl = GR_GetVertexDecl( PARTICLE_VDECL );
	
	m_scene = sh;
	
	OnRenderUpdate();
}

void ParticleSystem::SetTransform( const Mat4& mtx )
{
	m_transform = mtx;
}

bool ParticleSystem::Tick( float dt )
{
	bool retval = false;
	
	float prevrt = m_retriggerTime;
	m_retriggerTime -= dt;
	if( m_retriggerTime <= 0 && prevrt > 0 )
	{
		Trigger();
		m_retriggerTime = looping ? ( retriggerTimeExt.x + retriggerTimeExt.y * randf() ) : 0;
		retval = true;
	}
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		emitters[ i ].Tick( this, dt );
	}
	return retval;
}

void ParticleSystem::PreRender()
{
	if( !m_scene )
		return;
	
	const Mat4& invmtx = m_scene->camera.mInvView;
	ps_prerender_info info =
	{
		m_scene,
		m_transform,
		m_scene->camera.mView * m_scene->camera.mProj,
		{
			invmtx.TransformNormal( V3(1,0,0) ),
			invmtx.TransformNormal( V3(0,1,0) ),
			invmtx.TransformNormal( V3(0,0,1) )
		}
	};
	
	if( m_meshInsts.size() != emitters.size() )
		OnRenderUpdate();
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		MeshHandle mesh = m_meshInsts[ i ]->mesh;
		
		m_vertices.clear();
		m_indices.clear();
		
		SGRX_MeshPart& MP = mesh->m_meshParts[ 0 ];
		MP.vertexOffset = 0;
		MP.indexOffset = 0;
		
		emitters[ i ].PreRender( this, info );
		
		MP.vertexCount = m_vertices.size();
		MP.indexCount = m_indices.size();
		
		if( m_vertices.size() )
		{
			if( m_vertices.size_bytes() > mesh->m_vertexDataSize )
				mesh->SetVertexData( m_vertices.data(), m_vertices.size_bytes(), m_vdecl, false );
			else
				mesh->UpdateVertexData( m_vertices.data(), m_vertices.size_bytes(), false );
			mesh->SetAABBFromVertexData( m_vertices.data(), m_vertices.size_bytes(), m_vdecl );
		}
		
		if( m_indices.size() )
		{
			if( m_indices.size_bytes() > mesh->m_indexDataSize )
				mesh->SetIndexData( m_indices.data(), m_indices.size_bytes(), false );
			else
				mesh->UpdateIndexData( m_indices.data(), m_indices.size_bytes() );
		}
	}
}

void ParticleSystem::Trigger()
{
	if( m_groups.size() < (size_t) maxGroupCount )
		m_groups.resize( maxGroupCount );
	m_nextGroup++;
	m_nextGroup %= maxGroupCount;
	// TODO: pick more accurate position
	( m_lightSampler ? m_lightSampler : &GR_GetDummyLightSampler() )->SampleLight(
		m_transform.TransformPos( V3(0) ), m_groups[ m_nextGroup ].color );
	
	for( size_t i = 0; i < emitters.size(); ++i )
	{
		emitters[ i ].Trigger( this );
	}
}

void ParticleSystem::Play()
{
	if( !m_isPlaying )
	{
		m_isPlaying = true;
		m_retriggerTime = 0.00001f;
	}
}

void ParticleSystem::Stop()
{
	if( m_isPlaying )
	{
		m_isPlaying = false;
		m_retriggerTime = 0;
	}
}






#include "enganim.hpp"
#include "physics.hpp"


SGRX_Animation::SGRX_Animation() : speed( 0 ), frameCount( 0 )
{
}

SGRX_Animation::~SGRX_Animation()
{
}

int SGRX_Animation::FindTrackID( StringView name )
{
	for( size_t i = 0; i < tracks.size(); ++i )
	{
		if( tracks[ i ].name == name )
			return i;
	}
	return NOT_FOUND;
}

void SGRX_Animation::GetState( int track, float framePos, Vec3& outpos, Quat& outrot, Vec3& outscl )
{
	const Track& T = tracks[ track ];
	
	ASSERT( T.posFrames == 1 || T.posFrames == frameCount );
	ASSERT( T.rotFrames == 1 || T.rotFrames == frameCount );
	ASSERT( T.sclFrames == 1 || T.sclFrames == frameCount );
	
	// calculate data pointers
	float* at = &data[ T.offset ];
	Vec3* pos = (Vec3*) at;
	at += T.posFrames * 3;
	Quat* rot = (Quat*) at;
	at += T.rotFrames * 4;
	Vec3* scl = (Vec3*) at;
	
	// find interpolation frames & factor
	float q = fmodf( framePos, 1.0f );
	int f0 = floor( framePos );
	int f1 = f0 + 1;
	
	int flast = frameCount - 1;
	f0 = TCLAMP( f0, 0, flast );
	f1 = TCLAMP( f1, 0, flast );
	
	// interpolate
	if( T.posFrames == 1 )
		outpos = *pos;
	else
		outpos = TLERP( pos[ f0 ], pos[ f1 ], q );
	
	if( T.rotFrames == 1 )
		outrot = *rot;
	else
		outrot = TLERP( rot[ f0 ], rot[ f1 ], q );
	
	if( T.sclFrames == 1 )
		outscl = *scl;
	else
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

void SGRX_Animation::ClearTracks()
{
	data.clear();
	tracks.clear();
}

void SGRX_Animation::AddTrack( StringView name, Vec3SAV pos, QuatSAV rot, Vec3SAV scl )
{
	ASSERT( pos.size() == 1 || pos.size() == frameCount );
	ASSERT( pos.size() == 1 || pos.size() == frameCount );
	ASSERT( pos.size() == 1 || pos.size() == frameCount );
	
	Track T = { name, data.size(), pos.size(), rot.size(), scl.size(), 0 };
	tracks.push_back( T );
	
	for( size_t i = 0; i < pos.size(); ++i )
	{
		Vec3 tmp = pos[ i ];
		data.append( &tmp.x, 3 );
	}
	for( size_t i = 0; i < rot.size(); ++i )
	{
		Quat tmp = rot[ i ];
		data.append( &tmp.x, 4 );
	}
	for( size_t i = 0; i < scl.size(); ++i )
	{
		Vec3 tmp = scl[ i ];
		data.append( &tmp.x, 3 );
	}
}

void SGRX_AnimBundle::Serialize( ByteReader& arch )
{
	ByteView anbd_chunk = arch.readChunk( "SGRXANBD" );
	ByteReader anr( anbd_chunk );
	
	anims.clear();
	while( !anr.atEnd() && !anr.error )
	{
		AnimHandle anim = new SGRX_Animation;
		anim->Serialize( anr );
		anr.smallString( anim->m_key );
		anims.push_back( anim );
	}
}

void SGRX_AnimBundle::Serialize( ByteWriter& arch )
{
	uint32_t anbd_chunk = arch.beginChunk( "SGRXANBD" );
	
	for( size_t i = 0; i < anims.size(); ++i )
	{
		anims[ i ]->Serialize( arch );
		arch.smallString( anims[ i ]->m_key );
	}
	
	arch.endChunk( anbd_chunk );
}

bool Animator::Prepare( const MeshHandle& mesh )
{
	SGRX_IMesh* M = mesh;
	if( !M )
		return false;
	
	m_mesh = mesh;
	int count = M->m_numBones;
	m_positions.clear();
	m_rotations.clear();
	m_scales.clear();
	m_factors.clear();
	m_positions.resize( count );
	m_rotations.resize( count );
	m_scales.resize( count );
	m_factors.resize( count );
	for( int i = 0; i < count; ++i )
	{
		m_positions[ i ] = V3(0);
		m_rotations[ i ] = Quat::Identity;
		m_scales[ i ] = V3(1);
		m_factors[ i ] = 0;
	}
	
	return true;
}

Array< float >& Animator::GetBlendFactorArray()
{
	return m_factors;
}


AnimMixer::AnimMixer() : layers(NULL), layerCount(0)
{
}

AnimMixer::~AnimMixer()
{
}

bool AnimMixer::Prepare( const MeshHandle& mesh )
{
	if( Animator::Prepare( mesh ) == false )
		return false;
	m_staging.resize( m_factors.size() );
	for( int i = 0; i < layerCount; ++i )
	{
		if( layers[ i ].anim->Prepare( mesh ) == false )
			return false;
	}
	return true;
}

void AnimMixer::Advance( float deltaTime, AnimInfo* info )
{
	// generate output
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		m_positions[ i ] = V3(0);
		m_rotations[ i ] = Quat::Identity;
		m_scales[ i ] = V3(1);
		m_factors[ i ] = 0;
	}
	
	for( int layer = 0; layer < layerCount; ++layer )
	{
		Animator* AN = layers[ layer ].anim;
		AN->Advance( deltaTime, info );
		
		int tflags = layers[ layer ].tflags;
		bool abslayer = ( tflags & TF_Absolute_All ) != 0;
		SGRX_MeshBone* MB = m_mesh->m_bones;
		
		for( size_t i = 0; i < m_factors.size(); ++i )
		{
			Vec3 P = AN->m_positions[ i ];
			Quat R = AN->m_rotations[ i ];
			Vec3 S = AN->m_scales[ i ];
			float q = AN->m_factors[ i ] * layers[ layer ].factor;
			
			Mat4 orig;
			if( abslayer )
			{
				// to matrix
				Mat4 tfm = Mat4::CreateSRT( S, R, P );
				
				// extract diff
				orig = MB[ i ].boneOffset;
				if( MB[ i ].parent_id >= 0 )
				{
					orig = orig * m_staging[ MB[ i ].parent_id ];
				}
				else if( tflags & TF_IgnoreMeshXF )
				{
					orig = orig * info->rootXF;
				}
				Mat4 inv = Mat4::Identity;
				orig.InvertTo( inv );
				Mat4 diff = tflags & TF_Additive ?
					Mat4::CreateSRT( m_scales[ i ], m_rotations[ i ], m_positions[ i ] ) * orig * tfm * inv :
					tfm * inv;
		//		if( q )
		//			LOG << i << ":\t" << R;
			//	if( q )
			//		LOG << i << ":\t" << diff.GetRotationQuaternion() << m_rotations[i];
				
				// convert back to SRP
				P = ( tflags & TF_Absolute_Pos ) ? diff.GetTranslation() : m_positions[ i ];
				R = ( tflags & TF_Absolute_Rot ) ? diff.GetRotationQuaternion() : m_rotations[ i ];
				S = ( tflags & TF_Absolute_Scale ) ? diff.GetScale() : m_scales[ i ];
			}
			
			if( !m_factors[ i ] )
			{
				m_positions[ i ] = P;
				m_rotations[ i ] = R;
				m_scales[ i ] = S;
				m_factors[ i ] = q;
			}
			else
			{
				m_positions[ i ] = TLERP( m_positions[ i ], P, q );
				m_rotations[ i ] = TLERP( m_rotations[ i ], R, q );
				m_scales[ i ] = TLERP( m_scales[ i ], S, q );
				m_factors[ i ] = TLERP( m_factors[ i ], 1.0f, q );
			}
			
			if( abslayer )
			{
				m_staging[ i ] = Mat4::CreateSRT( m_scales[ i ], m_rotations[ i ], m_positions[ i ] ) * orig;
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

bool AnimPlayer::Prepare( const MeshHandle& mesh )
{
	m_currentAnims.clear();
	_clearAnimCache();
	if( Animator::Prepare( mesh ) == false )
		return false;
	m_blendFactors.resize( m_factors.size() );
	for( size_t i = 0; i < m_factors.size(); ++i )
		m_blendFactors[ i ] = 1;
	return true;
}

void AnimPlayer::Advance( float deltaTime, AnimInfo* info )
{
	// process tracks
	for( size_t i = m_currentAnims.size(); i > 0; )
	{
		--i;
		
		Anim& A = m_currentAnims[ i ];
		SGRX_Animation* AN = A.anim;
		A.at += deltaTime;
		A.prev_fade_at = A.fade_at;
		A.fade_at += deltaTime;
		
		float animTime = AN->GetAnimTime();
		if( !A.once )
		{
			A.at = fmodf( A.at, animTime );
			// permanent animation faded fully in, no need to keep previous tracks
			if( A.fade_at > A.fadetime )
			{
				m_currentAnims.erase( 0, i );
				break;
			}
		}
		// temporary animation has finished playback
		else if( A.at >= animTime )
		{
			m_currentAnims.erase( i );
			continue;
		}
	}
	
	// generate output
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		m_positions[ i ] = V3(0);
		m_rotations[ i ] = Quat::Identity;
		m_scales[ i ] = V3(1);
		m_factors[ i ] = 0;
	}
	for( size_t an = 0; an < m_currentAnims.size(); ++an )
	{
		Anim& A = m_currentAnims[ an ];
		SGRX_Animation* AN = A.anim;
		
		Vec3 P = V3(0), S = V3(1);
		Quat R = Quat::Identity;
		for( size_t i = 0; i < m_factors.size(); ++i )
		{
			int tid = A.trackIDs[ i ];
			if( tid < 0 )
				continue;
			
			AN->GetState( tid, A.at * AN->speed, P, R, S );
			float animTime = AN->GetAnimTime();
			float q = A.once ?
				smoothlerp_range( A.fade_at, 0, A.fadetime, animTime - A.fadetime, animTime ) :
				smoothlerp_oneway( A.fade_at, 0, A.fadetime );
			if( !m_factors[ i ] )
			{
				m_positions[ i ] = P;
				m_rotations[ i ] = R;
				m_scales[ i ] = S;
				m_factors[ i ] = q;
			}
			else
			{
				m_positions[ i ] = TLERP( m_positions[ i ], P, q );
				m_rotations[ i ] = TLERP( m_rotations[ i ], R, q );
				m_scales[ i ] = TLERP( m_scales[ i ], S, q );
				m_factors[ i ] = TLERP( m_factors[ i ], 1.0f, q );
			}
		}
	}
	for( size_t i = 0; i < m_factors.size(); ++i )
		m_factors[ i ] *= m_blendFactors[ i ];
}

void AnimPlayer::Play( const AnimHandle& anim, bool once, float fadetime )
{
	if( !anim )
		return;
	if( !once && m_currentAnims.size() && m_currentAnims.last().once == false && m_currentAnims.last().anim == anim )
		return; // ignore repetitive invocations
	Anim A = { anim, _getTrackIds( anim ), 0, 0, 0, fadetime, once };
	m_currentAnims.push_back( A );
}

void AnimPlayer::Stop()
{
	m_currentAnims.clear();
}

bool AnimPlayer::CheckMarker( const StringView& name )
{
	for( size_t i = 0; i < m_currentAnims.size(); ++i )
	{
		Anim& A = m_currentAnims[ i ];
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
	if( !m_factors.size() )
		return NULL;
	int* ids = m_animCache.getcopy( anim );
	if( !ids )
	{
		ids = new int [ m_factors.size() ];
		for( size_t i = 0; i < m_factors.size(); ++i )
		{
			ids[ i ] = anim->FindTrackID( m_mesh->m_bones[ i ].name );
		}
		m_animCache.set( anim, ids );
	}
	return ids;
}

void AnimPlayer::_clearAnimCache()
{
	for( size_t i = 0; i < m_animCache.size(); ++i )
	{
		delete [] m_animCache.item( i ).value;
	}
	m_animCache.clear();
}

Array< float >& AnimPlayer::GetBlendFactorArray()
{
	return m_blendFactors;
}


AnimInterp::AnimInterp() : animSource( NULL )
{
}

bool AnimInterp::Prepare( const MeshHandle& mesh )
{
	if( Animator::Prepare( mesh ) == false )
		return false;
	m_prev_positions.resize( m_factors.size() );
	m_prev_rotations.resize( m_factors.size() );
	m_prev_scales.resize( m_factors.size() );
	if( animSource )
	{
		if( animSource->Prepare( mesh ) == false )
			return false;
	}
	return true;
}

void AnimInterp::Advance( float deltaTime, AnimInfo* info )
{
	Transfer();
	animSource->Advance( deltaTime, info );
}

void AnimInterp::Transfer()
{
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		m_prev_positions[ i ] = animSource->m_positions[ i ];
		m_prev_rotations[ i ] = animSource->m_rotations[ i ];
		m_prev_scales[ i ] = animSource->m_scales[ i ];
	}
}

void AnimInterp::Interpolate( float deltaTime )
{
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		m_positions[ i ] = TLERP( m_prev_positions[ i ], animSource->m_positions[ i ], deltaTime );
		m_rotations[ i ] = TLERP( m_prev_rotations[ i ], animSource->m_rotations[ i ], deltaTime );
		m_scales[ i ] = TLERP( m_prev_scales[ i ], animSource->m_scales[ i ], deltaTime );
	}
}


AnimDeformer::AnimDeformer() : numIterations(4), numConstraintIterations(4)
{
}

bool AnimDeformer::Prepare( const MeshHandle& mesh )
{
	if( Animator::Prepare( mesh ) == false )
		return false;
	_UpdatePoseInfo();
	if( animSource )
	{
		if( animSource->Prepare( mesh ) == false )
			return false;
	}
	return true;
}

void AnimDeformer::Advance( float deltaTime, AnimInfo* info )
{
	animSource->Advance( deltaTime, info );
	for( size_t fid = 0; fid < forces.size(); ++fid )
		forces[ fid ].lifetime += deltaTime;
	
	_UpdatePoseInfo();
	
	int count = TMIN( int(m_factors.size()), SGRX_MAX_MESH_BONES );
	Vec3 posns[ SGRX_MAX_MESH_BONES ];
	Vec3 dirs[ SGRX_MAX_MESH_BONES ];
	float dists[ SGRX_MAX_MESH_BONES ];
	
	// save joint info
	for( int i = 0; i < count; ++i )
	{
		int pid = GetParentBoneID( i );
		posns[ i ] = m_bonePositions[ i ];
		dirs[ i ] = m_bonePositions[ i ];
		if( pid >= 0 )
			dirs[ i ] -= m_bonePositions[ pid ];
		dists[ i ] = dirs[ i ].Length();
	}
	
	// deform
	for( int it = 0; it < numIterations; ++it )
	{
		// apply forces
		for( size_t fid = 0; fid < forces.size(); ++fid )
		{
			Force& F = forces[ fid ];
			Vec3 fpos = F.pos;
			if( F.boneID >= 0 )
			{
				fpos = m_skinOffsets[ F.boneID ].TransformPos( fpos );
			}
			float amount = F.amount / numIterations;
			if( F.dir == V3(0) )
			{
				// omni force
				for( int i = 0; i < count; ++i )
				{
					Vec3 fpush = ( m_bonePositions[ i ] - F.pos ).Normalized();
					float distfac = TMIN( 1.0f, ( F.pos - m_bonePositions[ i ] ).Length() / F.radius );
					distfac = powf( 1 - distfac, F.power );
					m_bonePositions[ i ] += fpush * distfac * amount;
				}
			}
			else
			{
				// directional force
				Vec3 fpos = F.pos;
				Vec3 fdir = F.dir;
				if( F.boneID >= 0 )
				{
					fpos = m_skinOffsets[ F.boneID ].TransformPos( fpos );
					fdir = m_skinOffsets[ F.boneID ].TransformNormal( fdir );
				}
				Vec3 fpush = fdir.Normalized() * amount;
				for( int i = 0; i < count; ++i )
				{
					float distfac = TMIN( 1.0f, ( fpos - m_bonePositions[ i ] ).Length() / F.radius );
					distfac = powf( 1 - distfac, F.power );
					m_bonePositions[ i ] += fpush * distfac;
				}
			}
		}
		
		// solve constraints
		float invcit = 1.0f / numConstraintIterations;
		for( int cit = 0; cit < numConstraintIterations; ++cit )
		{
			for( int i = 0; i < count; ++i )
			{
				int pid = GetParentBoneID( i );
				// constraint to immovable origin is not used
				if( pid >= 0 )
				{
					Vec3 p0 = m_bonePositions[ pid ];
					Vec3 p1 = m_bonePositions[ i ];
					float d = dists[ i ];
					Vec3 diff = p1 - p0;
					float difflen = diff.Length();
					float amount = ( d - difflen ) * 0.5f * invcit;
					Vec3 dn = diff.Normalized() * amount;
					m_bonePositions[ pid ] -= dn;
					m_bonePositions[ i ] += dn;
				}
			}
		}
	}
	
	// produce animation data
#if 1
	for( int i = 0; i < count; ++i )
	{
		m_positions[ i ] = animSource->m_positions[ i ];
		m_rotations[ i ] = animSource->m_rotations[ i ];
		m_scales[ i ] = animSource->m_scales[ i ];
		m_factors[ i ] = animSource->m_factors[ i ];
	}
	if( count > 0 )
	{
		Vec3 off = m_bonePositions[ 0 ] - posns[ 0 ];
		m_positions[ 0 ] += m_invSkinOffsets[ 0 ].TransformNormal( off );
	}
	Mat4 skinXF[ SGRX_MAX_MESH_BONES ];
	for( int i = 0; i < count; ++i )
	{
		skinXF[ i ] = m_mesh->m_bones[ i ].boneOffset;
		if( i > 0 )
		{
			int pid = GetParentBoneID( i );
			skinXF[ i ] = skinXF[ i ] * skinXF[ pid ];
		}
		
		// gather average global target/local original child pos.
		Vec3 avgtgt = V3(0);
		Vec3 avgpos = V3(0);
		int cc = 0;
		for( int ch = i + 1; ch < count; ++ch )
		{
			if( GetParentBoneID( ch ) != i )
				continue;
			avgtgt += m_bonePositions[ ch ];
			avgpos += posns[ ch ];
			cc++;
		}
		
		if( cc == 0 )
			continue;
		
		avgpos /= cc;
		avgtgt /= cc;
		
		Mat4 curxf = Mat4::CreateSRT( m_scales[ i ], m_rotations[ i ], m_positions[ i ] );
		Mat4 inv = Mat4::Identity;
		( curxf * skinXF[ i ] ).InvertTo( inv );
		avgtgt = inv.TransformPos( avgtgt ).Normalized();
		avgpos = m_invSkinOffsets[ i ].TransformPos( avgpos ).Normalized();
		m_rotations[ i ] = Quat::CreateAxisAxis( avgpos, avgtgt ) * m_rotations[ i ];
		
		skinXF[ i ] = Mat4::CreateSRT(
			m_scales[ i ], m_rotations[ i ], m_positions[ i ] ) * skinXF[ i ];
		
		m_factors[ i ] = 1;
	}
#else
	for( int i = 0; i < count; ++i )
	{
		m_positions[ i ] = animSource->m_positions[ i ];
		m_rotations[ i ] = animSource->m_rotations[ i ];
		m_scales[ i ] = animSource->m_scales[ i ];
		m_factors[ i ] = animSource->m_factors[ i ];
		
		int pid = GetParentBoneID( i );
		if( pid >= 0 )
		{
			// rotate bone with axis/angle in bone space
			Vec3 nmdir = m_bonePositions[ i ] - m_bonePositions[ pid ];
			Vec3 omdir = dirs[ i ];
			
			Vec3 tf_nmdir = m_invSkinOffsets[ i ].TransformNormal( nmdir ).Normalized();
			Vec3 tf_omdir = m_invSkinOffsets[ i ].TransformNormal( omdir ).Normalized();
			m_rotations[ i ] = m_rotations[ i ] * Quat::CreateAxisAxis( tf_omdir, tf_nmdir );
			
			// rotate child bones backwards
			for( int ch = i + 1; ch < count; ++ch )
			{
				if( GetParentBoneID( ch ) != i )
					continue;
				tf_nmdir = m_invSkinOffsets[ ch ].TransformNormal( nmdir ).Normalized();
				tf_omdir = m_invSkinOffsets[ ch ].TransformNormal( omdir ).Normalized();
				m_rotations[ ch ] = m_rotations[ ch ] * Quat::CreateAxisAxis( tf_nmdir, tf_omdir );
			}
		}
		else
		{
			// move bone set in world space
			m_positions[ i ] += m_invSkinOffsets[ i ].TransformNormal( m_bonePositions[ i ] - posns[ i ] );
		}
		m_factors[ i ] = 1;
	}
#endif
}

void AnimDeformer::AddLocalForce( const Vec3& pos, const Vec3& dir, float rad, float power, float amount )
{
	int cb = _FindClosestBone( pos );
	Force F = { cb, pos, dir, amount, rad, power, 0 };
	if( cb >= 0 )
	{
		Mat4 iso = m_invSkinOffsets[ cb ];
		F.pos = iso.TransformPos( pos );
		F.dir = iso.TransformNormal( dir );
	}
	forces.push_back( F );
}

void AnimDeformer::AddModelForce( const Vec3& pos, const Vec3& dir, float rad, float power, float amount )
{
	Force F = { -1, pos, dir, amount, rad, power, 0 };
	forces.push_back( F );
}

int AnimDeformer::_FindClosestBone( const Vec3& pos )
{
	int bone = -1;
	float cdist = FLT_MAX;
	for( size_t i = 0; i < m_bonePositions.size(); ++i )
	{
		float nd = ( pos - m_bonePositions[ i ] ).LengthSq();
		if( nd < cdist )
		{
			bone = i;
			cdist = nd;
		}
	}
	return bone;
}

void AnimDeformer::_UpdatePoseInfo()
{
	m_skinOffsets.resize( m_factors.size() );
	m_invSkinOffsets.resize( m_factors.size() );
	m_bonePositions.resize( m_factors.size() );
	
	GR_ApplyAnimator( animSource, m_skinOffsets.data(), m_skinOffsets.size(), false );
	for( size_t i = 0; i < m_factors.size(); ++i )
	{
		Mat4 inv = Mat4::Identity;
		m_skinOffsets[ i ].InvertTo( inv );
		m_invSkinOffsets[ i ] = inv;
		m_bonePositions[ i ] = m_skinOffsets[ i ].TransformPos( V3(0) );
	}
}


void GR_ClearFactors( Array< float >& out, float factor )
{
	TMEMSET( out.data(), out.size(), factor );
}

void GR_SetFactors( Array< float >& out, const MeshHandle& mesh, const StringView& name, float factor, bool ch )
{
	int subbones[ SGRX_MAX_MESH_BONES ];
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


bool GR_ReadAnimBundle( const StringView& path, SGRX_AnimBundle& out )
{
	LOG_FUNCTION;
	
	ByteArray ba;
	if( !FS_LoadBinaryFile( path, ba ) )
	{
		LOG << "Failed to load animation bundle file: " << path;
		return false;
	}
	
	ByteReader br( ba );
	br << out;
	return !br.error;
}

bool GR_ApplyAnimator( const Animator* animator, Mat4* out, size_t outsz, bool applyinv, Mat4* base )
{
	SGRX_IMesh* mesh = animator->m_mesh;
	if( !mesh )
		return false;
	if( outsz != animator->m_positions.size() )
		return false;
	if( outsz != (size_t) mesh->m_numBones )
		return false;
	SGRX_MeshBone* MB = mesh->m_bones;
	
	for( size_t i = 0; i < outsz; ++i )
	{
		Mat4& M = out[ i ];
		M = Mat4::CreateSRT( animator->m_scales[ i ], animator->m_rotations[ i ], animator->m_positions[ i ] ) * MB[ i ].boneOffset;
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

bool GR_ApplyAnimator( const Animator* animator, SGRX_MeshInstance* meshinst )
{
	if( !meshinst || meshinst->GetMesh() != animator->m_mesh )
		return false;
	return GR_ApplyAnimator( animator,
		meshinst->skin_matrices.data(),
		meshinst->skin_matrices.size() );
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
		//	V /= PS->m_transform.GetScale();
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
					curve_ColorVal.GetValue( q, RCO.z ) * 2
				)
			),
			curve_Opacity.GetValue( q, RCO.w )
		);
		
		if( render_Additive == false )
		{
			Vec3 color = PS->m_groups[ GRP ].color * 0.5f;
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


ParticleSystem::ParticleSystem() :
	gravity(V3(0,0,-10)), maxGroupCount(10), globalScale(1),
	looping(true), retriggerTimeExt(V2(1,0.1f)),
	m_isPlaying(false), m_retriggerTime(0), m_nextGroup(0),
	m_transform(Mat4::Identity), m_lightSampler(NULL), m_psRaycast(NULL)
{
	RegisterHandler( EID_ScenePreRender );
}

bool ParticleSystem::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( ba );
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

void ParticleSystem::HandleEvent( SGRX_EventID eid, const EventData& edata )
{
	if( eid == EID_ScenePreRender )
	{
		SGRX_CAST( SGRX_Scene*, scene, edata.GetUserData() );
		if( scene == m_scene )
		{
			_PreRender();
		}
	}
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
		if( !m_meshInsts[ i ]->GetMesh() )
			m_meshInsts[ i ]->SetMesh( GR_CreateMesh() );
		
	//	m_meshInsts[ i ]->matrix = Mat4::Identity; // E.absolute ? Mat4::Identity : m_transform;
		SGRX_Material mtl;
		mtl.blendMode = E.render_Additive ? SGRX_MtlBlend_Additive : SGRX_MtlBlend_Basic;
		mtl.flags = E.render_Additive * SGRX_MtlFlag_Unlit;
		for( int t = 0; t < NUM_PARTICLE_TEXTURES; ++t )
			mtl.textures[ t ] = E.render_Textures[ t ];
		mtl.shader = E.render_Shader;
		m_meshInsts[ i ]->materials.assign( &mtl, 1 );
		m_meshInsts[ i ]->OnUpdate();
		
		SGRX_MeshPart MP = { 0, 0, 0, 0 };
		m_meshInsts[ i ]->GetMesh()->SetPartData( &MP, 1 );
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

void ParticleSystem::_PreRender()
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
		MeshHandle mesh = m_meshInsts[ i ]->GetMesh();
		
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
				mesh->SetVertexData( m_vertices.data(), m_vertices.size_bytes(), m_vdecl );
			else
				mesh->UpdateVertexData( m_vertices.data(), m_vertices.size_bytes() );
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




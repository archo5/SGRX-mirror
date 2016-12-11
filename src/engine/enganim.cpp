

#include "engine_int.hpp"
#include "enganim.hpp"

extern AnimHashTable* g_Anims;


const char* AnimFileParser::Parse( ByteReader& br )
{
	uint32_t numAnims, sectionLength = 0;
	br.marker( "SS3DANIM" );
	if( br.error )
		return "file not SS3DANIM";
	br << numAnims;
	for( uint32_t i = 0; i < numAnims; ++i )
	{
		br << sectionLength;
		
		Anim anim;
		br << anim.nameSize;
		anim.name = (char*) br.at();
		br.padding( anim.nameSize );
		br << anim.frameCount;
		br << anim.speed;
		br << anim.trackCount;
		anim.trackDataOff = trackData.size();
		br << anim.markerCount;
		anim.markerDataOff = markerData.size();
		
		if( br.error )
			return "failed to read animation data";
		
		for( uint8_t t = 0; t < anim.trackCount; ++t )
		{
			br << sectionLength;
			
			Track track;
			br << track.nameSize;
			track.name = (char*) br.at();
			br.padding( track.nameSize );
			
			track.dataPtr = (float*) br.at();
			br.padding( sizeof( float ) * 10 * anim.frameCount );
			
			if( br.error )
				return "failed to read track data";
			
			trackData.push_back( track );
		}
		
		for( uint8_t m = 0; m < anim.markerCount; ++m )
		{
			Marker marker;
			
			br.memory( marker.name, MAX_ANIM_MARKER_NAME_LENGTH );
			br << marker.frame;
			
			if( br.error )
				return "failed to read marker data";
			
			markerData.push_back( marker );
		}
		
		animData.push_back( anim );
	}
	return NULL;
}


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
	
	Track T = { name.str(), data.size(), pos.size(), rot.size(), scl.size(), 0 };
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
	CByteView anbd_chunk = arch.readChunk( "SGRXANBD" );
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


void AnimMask::Prepare()
{
	blendFactors.resize_using( m_mesh.GetBoneCount(), 1 );
}

void AnimMask::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	if( animSource )
	{
		animSource->Advance( deltaTime, info );
		for( size_t i = 0; i < m_pose.size(); ++i )
		{
			m_pose[ i ] = animSource->m_pose[ i ];
			m_pose[ i ].fq *= blendFactors[ i ];
		}
	}
}


void AnimBlend::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	if( animSourceA )
	{
		animSourceA->Advance( deltaTime, info );
		if( animSourceB )
		{
			animSourceB->Advance( deltaTime, info );
			if( blendMode == ABM_Additive )
			{
				for( size_t i = 0; i < m_pose.size(); ++i )
				{
					AnimTrackXForm mul;
					mul.SetAdd(
						animSourceA->m_pose[ i ],
						animSourceB->m_pose[ i ]
					);
					m_pose[ i ].SetLerp(
						animSourceA->m_pose[ i ],
						mul,
						animSourceB->m_pose[ i ].fq * blendFactor
					);
					m_pose[ i ].fq = clamp( animSourceA->m_pose[ i ].fq + animSourceB->m_pose[ i ].fq * blendFactor, 0, 1 );
				}
			}
			else // ABM_Normal
			{
				for( size_t i = 0; i < m_pose.size(); ++i )
				{
					m_pose[ i ].SetLerp(
						animSourceA->m_pose[ i ],
						animSourceB->m_pose[ i ],
						animSourceB->m_pose[ i ].fq * blendFactor
					);
					m_pose[ i ].fq = TLERP( animSourceA->m_pose[ i ].fq, 1.0f, animSourceB->m_pose[ i ].fq * blendFactor );
				}
			}
		}
		else
		{
			for( size_t i = 0; i < m_pose.size(); ++i )
				m_pose[ i ] = animSourceA->m_pose[ i ];
		}
	}
}


void AnimRelAbs::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	if( animSource )
	{
		animSource->Advance( deltaTime, info );
		m_tmpMtx.resize( m_mesh.GetBoneCount() );
		SGRX_MeshBone* MB = m_mesh.GetBonePtr();
		if( !( flags & ANIM_RELABS_INVERT ) )
		{
			// rel -> abs
			for( size_t i = 0; i < m_pose.size(); ++i )
			{
				Mat4& M = m_tmpMtx[ i ];
				M = M4MulAff( animSource->m_pose[ i ].GetSRT(), MB[ i ].boneOffset );
				if( MB[ i ].parent_id >= 0 )
					M = M4MulAff( M, m_tmpMtx[ MB[ i ].parent_id ] );
				else
					M = M4MulAff( M, info->rootXF );
				m_pose[ i ].fq = animSource->m_pose[ i ].fq;
				m_pose[ i ].SetMatrix( M );
			}
		}
		else
		{
			// abs -> rel
			for( size_t i = 0; i < m_pose.size(); ++i )
			{
				Mat4& M = m_tmpMtx[ i ];
				m_pose[ i ].fq = animSource->m_pose[ i ].fq;
				M = animSource->m_pose[ i ].GetSRT();
				Mat4 pM = MB[ i ].boneOffset;
				if( MB[ i ].parent_id >= 0 )
					pM = M4MulAff( pM, m_tmpMtx[ MB[ i ].parent_id ] );
				else
					pM = M4MulAff( pM, info->rootXF );
				m_pose[ i ].SetMatrix( M * pM.Inverted() );
			}
		}
	}
}


AnimMixer::AnimMixer() : layers(NULL), layerCount(0)
{
}

AnimMixer::~AnimMixer()
{
}

void AnimMixer::Prepare()
{
	m_staging.resize( m_pose.size() );
}

void AnimMixer::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	// generate output
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		m_pose[ i ].Reset();
	}
	
	for( int layer = 0; layer < layerCount; ++layer )
	{
		Animator* AN = layers[ layer ].anim;
		AN->Advance( deltaTime, info );
		
		int tflags = layers[ layer ].tflags;
		bool abslayer = ( tflags & TF_Absolute_All ) != 0;
		SGRX_MeshBone* MB = m_mesh->m_bones;
		
		for( size_t i = 0; i < m_pose.size(); ++i )
		{
			Vec3 P = m_pose[ i ].pos;
			Quat R = m_pose[ i ].rot;
			Vec3 S = m_pose[ i ].scl;
			float q = m_pose[ i ].fq * layers[ layer ].factor;
			
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
					m_pose[ i ].GetSRT() * orig * tfm * inv :
					tfm * inv;
		//		if( q )
		//			LOG << i << ":\t" << R;
			//	if( q )
			//		LOG << i << ":\t" << diff.GetRotationQuaternion() << m_rotations[i];
				
				// convert back to SRP
				P = ( tflags & TF_Absolute_Pos ) ? diff.GetTranslation() : m_pose[ i ].pos;
				R = ( tflags & TF_Absolute_Rot ) ? diff.GetRotationQuaternion() : m_pose[ i ].rot;
				S = ( tflags & TF_Absolute_Scale ) ? diff.GetScale() : m_pose[ i ].scl;
			}
			
			if( !m_pose[ i ].fq )
			{
				m_pose[ i ].pos = P;
				m_pose[ i ].rot = R;
				m_pose[ i ].scl = S;
				m_pose[ i ].fq = q;
			}
			else
			{
				m_pose[ i ].pos = TLERP( m_pose[ i ].pos, P, q );
				m_pose[ i ].rot = TLERP( m_pose[ i ].rot, R, q );
				m_pose[ i ].scl = TLERP( m_pose[ i ].scl, S, q );
				m_pose[ i ].fq = TLERP( m_pose[ i ].fq, 1.0f, q );
			}
			
			if( abslayer )
			{
				m_staging[ i ] = Mat4::CreateSRT( m_pose[ i ].scl, m_pose[ i ].rot, m_pose[ i ].pos ) * orig;
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

void AnimPlayer::Prepare()
{
	for( size_t i = 0; i < m_currentAnims.size(); ++i )
		m_currentAnims[ i ].trackIDs = NULL;
	_clearAnimCache();
	m_blendFactors.resize_using( m_pose.size(), 1 );
}

void AnimPlayer::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	// process tracks
	for( size_t i = m_currentAnims.size(); i > 0; )
	{
		--i;
		
		Anim& A = m_currentAnims[ i ];
		SGRX_Animation* AN = A.anim;
		A.prev_at = A.at;
		A.at += deltaTime * A.speed;
		A.fade_at += A.fadeusespeed ? deltaTime * fabsf( A.speed ) : deltaTime;
		
		if( AN == NULL )
		{
			if( A.fade_at > A.fadetime )
			{
				// do not preserve empty ended anim
				m_currentAnims.erase( 0, i + 1 );
				break;
			}
			continue;
		}
		float animTime = AN->GetAnimTime();
		if( A.playMode != ANIM_PLAY_ONCE )
		{
			if( A.playMode == ANIM_PLAY_LOOP )
			{
				// ANIM_PLAY_CLAMP does not need this
				A.at = fmodf( A.at, animTime );
				if( A.at < 0 )
					A.at += animTime;
			}
			else if( A.playMode == ANIM_PLAY_CLAMP )
			{
				A.at = TCLAMP( A.at, 0.0f, animTime );
			}
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
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		m_pose[ i ].Reset();
	}
	for( size_t an = 0; an < m_currentAnims.size(); ++an )
	{
		Anim& A = m_currentAnims[ an ];
		SGRX_Animation* AN = A.anim;
		if( !A.trackIDs && AN )
			A.trackIDs = _getTrackIds( AN );
		
		Vec3 P = V3(0), S = V3(1);
		Quat R = Quat::Identity;
		for( size_t i = 0; i < m_pose.size(); ++i )
		{
			float q, t;
			if( AN )
			{
				int tid = A.trackIDs[ i ];
				if( tid < 0 )
					continue;
				AN->GetState( tid, A.at * AN->speed, P, R, S );
				float animTime = AN->GetAnimTime();
				q = A.playMode == ANIM_PLAY_ONCE ?
					smoothlerp_range( A.fade_at, 0, A.fadetime, animTime - A.fadetime, animTime ) :
					smoothlerp_oneway( A.fade_at, 0, A.fadetime );
				t = 1;
			}
			else
			{
				q = smoothlerp_oneway( A.fade_at, 0, A.fadetime );
				t = 0;
			}
			if( !m_pose[ i ].fq )
			{
				m_pose[ i ].pos = P;
				m_pose[ i ].rot = R;
				m_pose[ i ].scl = S;
				m_pose[ i ].fq = q * t;
			}
			else
			{
				m_pose[ i ].pos = TLERP( m_pose[ i ].pos, P, q );
				m_pose[ i ].rot = TLERP( m_pose[ i ].rot, R, q );
				m_pose[ i ].scl = TLERP( m_pose[ i ].scl, S, q );
				m_pose[ i ].fq = TLERP( m_pose[ i ].fq, t, q );
			}
		}
	}
	for( size_t i = 0; i < m_pose.size(); ++i )
		m_pose[ i ].fq *= m_blendFactors[ i ];
}

void AnimPlayer::Play( const AnimHandle& anim, uint8_t playMode, float fadetime, bool fadeusespeed )
{
	if( playMode != ANIM_PLAY_ONCE &&
		m_currentAnims.size() &&
		m_currentAnims.last().playMode == playMode &&
		m_currentAnims.last().anim == anim )
		return; // ignore repetitive invocations
	Anim A = { anim, NULL, 0, 0, 0, fadetime, fadeusespeed, 1, playMode };
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
		if( !AN )
			continue;
		float fp0 = A.prev_at * AN->speed;
		float fp1 = A.at * AN->speed;
		if( fp0 == fp1 )
			continue; // prevent 'thrilling' markers
		if( A.playMode != ANIM_PLAY_ONCE )
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

float AnimPlayer::GetLastAnimBlendFactor() const
{
	if( m_currentAnims.size() < 1 )
		return 0;
	const Anim& A = m_currentAnims.last();
	SGRX_Animation* AN = A.anim;
	if( !AN )
		return 0;
	float animTime = AN->GetAnimTime();
	return A.playMode == ANIM_PLAY_ONCE ?
		smoothlerp_range( A.fade_at, 0, A.fadetime, animTime - A.fadetime, animTime ) :
		smoothlerp_oneway( A.fade_at, 0, A.fadetime );
}

int* AnimPlayer::_getTrackIds( const AnimHandle& anim )
{
	if( !m_pose.size() || !anim )
		return NULL;
	int* ids = m_animCache.getcopy( anim );
	if( !ids )
	{
		ids = new int [ m_pose.size() ];
		for( size_t i = 0; i < m_pose.size(); ++i )
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


AnimInterp::AnimInterp() : animSource( NULL )
{
}

void AnimInterp::Prepare()
{
	m_prev_pose.resize( m_pose.size() );
}

void AnimInterp::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	if( !animSource )
		return;
	Transfer();
	animSource->Advance( deltaTime, info );
}

void AnimInterp::Transfer()
{
	if( !animSource )
		return;
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		m_prev_pose[ i ] = animSource->m_pose[ i ];
	}
}

void AnimInterp::Interpolate( float deltaTime )
{
	if( !animSource )
		return;
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		m_pose[ i ].SetLerp( m_prev_pose[ i ], animSource->m_pose[ i ], deltaTime );
	}
}


AnimDeformer::AnimDeformer() : numIterations(4), numConstraintIterations(4)
{
}

void AnimDeformer::Prepare()
{
	_UpdatePoseInfo();
}

void AnimDeformer::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	
	if( !animSource )
		return;
	
	animSource->Advance( deltaTime, info );
	for( size_t fid = 0; fid < forces.size(); ++fid )
		forces[ fid ].lifetime += deltaTime;
	
	_UpdatePoseInfo();
	
	int count = TMIN( int(m_pose.size()), SGRX_MAX_MESH_BONES );
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
	for( int i = 0; i < count; ++i )
	{
		m_pose[ i ] = animSource->m_pose[ i ];
	}
	if( count > 0 )
	{
		Vec3 off = m_bonePositions[ 0 ] - posns[ 0 ];
		m_pose[ 0 ].pos += m_invSkinOffsets[ 0 ].TransformNormal( off );
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
		
		Mat4 curxf = m_pose[ i ].GetSRT();
		Mat4 inv = Mat4::Identity;
		( curxf * skinXF[ i ] ).InvertTo( inv );
		avgtgt = inv.TransformPos( avgtgt ).Normalized();
		avgpos = m_invSkinOffsets[ i ].TransformPos( avgpos ).Normalized();
		m_pose[ i ].rot = Quat::CreateAxisAxis( avgpos, avgtgt ) * m_pose[ i ].rot;
		
		skinXF[ i ] = m_pose[ i ].GetSRT() * skinXF[ i ];
		
		m_pose[ i ].fq = 1;
	}
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
	m_skinOffsets.resize( m_pose.size() );
	m_invSkinOffsets.resize( m_pose.size() );
	m_bonePositions.resize( m_pose.size() );
	
	GR_ApplyAnimator( animSource, m_skinOffsets.data(), m_skinOffsets.size(), false );
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		Mat4 inv = Mat4::Identity;
		m_skinOffsets[ i ].InvertTo( inv );
		m_invSkinOffsets[ i ] = inv;
		m_bonePositions[ i ] = m_skinOffsets[ i ].TransformPos( V3(0) );
	}
}


void GR_ClearFactors( const ArrayView< float >& out, float factor )
{
	TMEMSET( out.data(), out.size(), factor );
}

void GR_SetFactors( const ArrayView< float >& out, const MeshHandle& mesh, const StringView& name, float factor, bool ch, uint8_t mode )
{
	int subbones[ SGRX_MAX_MESH_BONES ];
	int numsb = 0;
	GR_FindBones( subbones, numsb, mesh, name, ch );
	if( mode == SFM_AddDist )
	{
		for( int i = 0; i < numsb; ++i )
			out[ subbones[ i ] ] += factor * mesh->BoneDistance( subbones[ i ], subbones[ 0 ] );
	}
	else if( mode == SFM_Add )
	{
		for( int i = 0; i < numsb; ++i )
			out[ subbones[ i ] ] += factor;
	}
	else if( mode == SFM_Set )
	{
		for( int i = 0; i < numsb; ++i )
			out[ subbones[ i ] ] = factor;
	}
	for( int i = 0; i < numsb; ++i )
		out[ subbones[ i ] ] = clamp( out[ subbones[ i ] ], 0, 1 );
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
		LOG_ERROR << LOG_DATE << "  Failed to load anim.bundle file: " << path;
		return false;
	}
	
	ByteReader br( ba );
	br << out;
	if( br.error )
		LOG_ERROR << LOG_DATE << "  Failed to load anim.bundle file: " << path << " - parsing error";
	return !br.error;
}

bool GR_EnumAnimBundle( const StringView& path, Array< RCString >& out, bool rawnames )
{
	LOG_FUNCTION;
	
	ByteArray ba;
	if( !FS_LoadBinaryFile( path, ba ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load anim.bundle file: " << path;
		return false;
	}
	
	ByteReader br( ba );
	CByteView anbd_chunk = br.readChunk( "SGRXANBD" );
	if( br.error )
		return false;
	ByteReader anr( anbd_chunk );
	
	StackPath spath( path );
	String tmpname;
	while( !anr.atEnd() && !anr.error )
	{
		// skip ANIM chunk
		anr.readChunk( "ANIM" );
		// read the name
		anr.smallString( tmpname );
		
		if( rawnames )
			out.push_back( tmpname );
		else
		{
			// compose full key
			char bfr[ 1024 ];
			tmpname.push_back( '\0' );
			sgrx_snprintf( bfr, 1024, "%s:%s", spath.str, tmpname.data() );
			
			out.push_back( bfr );
		}
	}
	return !anr.error;
}

bool GR_ApplyAnimator( const Animator* animator, Mat4* out, size_t outsz, bool applyinv, Mat4* base )
{
	SGRX_IMesh* mesh = animator->m_mesh;
	if( !mesh )
		return false;
	if( outsz != animator->m_pose.size() )
		return false;
	if( outsz != (size_t) mesh->m_numBones )
		return false;
	SGRX_MeshBone* MB = mesh->m_bones;
	
	for( size_t i = 0; i < outsz; ++i )
	{
		Mat4& M = out[ i ];
		M = M4MulAff( animator->m_pose[ i ].GetSRT(), MB[ i ].boneOffset );
		if( MB[ i ].parent_id >= 0 )
			M = M4MulAff( M, out[ MB[ i ].parent_id ] );
		else if( base )
			M = M4MulAff( M, *base );
	}
	if( applyinv )
	{
		for( size_t i = 0; i < outsz; ++i )
		{
			Mat4& M = out[ i ];
			M = M4MulAff( MB[ i ].invSkinOffset, M );
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



static void _LoadAnimBundle( const StringView& path, const StringView& prefix )
{
	LOG_FUNCTION;
	
	SGRX_AnimBundle bundle;
	if( !GR_ReadAnimBundle( path, bundle ) )
		return;
	
	for( size_t i = 0; i < bundle.anims.size(); ++i )
	{
		SGRX_Animation* anim = bundle.anims[ i ];
		
		anim->m_key.insert( 0, prefix );		
		g_Anims->set( anim->m_key, anim );
	}
	
	if( VERBOSE ) LOG << "Loaded " << bundle.anims.size() << " anims from anim.bundle " << path;
}

AnimHandle GR_GetAnim( const StringView& name )
{
	if( !name )
		return NULL;
	AnimHandle out = g_Anims->getcopy( name );
	if( out )
		return out;
	
	StringView bundle = name.until( ":" );
	if( name.size() == bundle.size() )
	{
		LOG_ERROR << LOG_DATE << "  Failed to request animation: " << name
			<< " - invalid name (expected <bundle>:<anim>)";
		return NULL;
	}
	
	_LoadAnimBundle( bundle, name.part( 0, bundle.size() + 1 ) );
	
	return g_Anims->getcopy( name );
}



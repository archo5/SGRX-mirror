

#define USE_QUAT
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "enganim.hpp"


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

AnimMixer::AnimMixer() : layers(NULL), layerCount(0)
{
}

AnimMixer::~AnimMixer()
{
}

void AnimMixer::Prepare( String* names, int count )
{
	Animator::Prepare( names, count );
	for( int i = 0; i < layerCount; ++i )
	{
		layers[ i ].anim->Prepare( names, count );
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
		
		for( int i = 0; i < names.size(); ++i )
		{
			Vec3 P = AN->position[ i ];
			Quat R = AN->rotation[ i ];
			Vec3 S = AN->scale[ i ];
			float q = AN->factor[ i ] * layers[ layer ].factor;
			
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

void AnimPlayer::Prepare( String* names, int count )
{
	currentAnims.clear();
	_clearAnimCache();
	Animator::Prepare( names, count );
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
	Anim A = { anim, _getTrackIds( anim ), 0, 0, fadetime, once };
	currentAnims.push_back( A );
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

void GR_ClearFactors( Array< float >& out, float factor )
{
	TMEMSET( out.data(), out.size(), factor );
}

void GR_SetFactors( Array< float >& out, const MeshHandle& mesh, const StringView& name, float factor )
{
	int subbones[ MAX_MESH_BONES ];
	int numsb = 0;
	GR_FindBones( subbones, numsb, mesh, name, true );
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


bool GR_ApplyAnimator( const Animator* animator, MeshInstHandle mih )
{
	SGRX_MeshInstance* MI = mih;
	if( !MI )
		return false;
	SGRX_IMesh* mesh = MI->mesh;
	if( !mesh )
		return false;
	size_t sz = MI->skin_matrices.size();
	if( sz != animator->position.size() )
		return false;
	if( sz != mesh->m_numBones )
		return false;
	SGRX_MeshBone* MB = mesh->m_bones;
	
	for( size_t i = 0; i < sz; ++i )
	{
		Mat4& M = MI->skin_matrices[ i ];
		M = Mat4::CreateSRT( animator->scale[ i ], animator->rotation[ i ], animator->position[ i ] ) * MB[ i ].boneOffset;
		if( MB[ i ].parent_id >= 0 )
			M = M * MI->skin_matrices[ MB[ i ].parent_id ];
	}
	for( size_t i = 0; i < sz; ++i )
	{
		Mat4& M = MI->skin_matrices[ i ];
		M = MB[ i ].invSkinOffset * M;
	}
	return true;
}




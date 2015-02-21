

#pragma once
#include "engine.hpp"



struct EXPORT SGRX_Animation
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	~SGRX_Animation();
	
	struct Track
	{
		String name;
		Vec3* position;
		Vec3* rotation;
		Vec3* scale;
		int frameCount;
	};
	
	Track* tracks;
	int trackCount;
	float speed;
	
	int32_t _refcount;
};

struct EXPORT AnimHandle : Handle< SGRX_Animation >
{
	AnimHandle() : Handle(){}
	AnimHandle( const AnimHandle& h ) : Handle( h ){}
	AnimHandle( SGRX_Animation* anm ) : Handle( anm ){}
};

typedef HashTable< AnimHandle, SGRX_Animation::Track** > AnimCache;

struct EXPORT Animator
{
	virtual void Prepare( String* new_names, int count );
	virtual void Advance( float deltaTime ){}
	
	Array< String > names;
	Array< Vec3 > position;
	Array< Vec3 > rotation;
	Array< Vec3 > scale;
	Array< float > factor;
};

struct EXPORT AnimMixer : Animator
{
	struct Layer
	{
		Animator* anim;
		float factor;
	};
	
	Layer* layers;
	int layerCount;
};

struct EXPORT AnimPlayer : Animator
{
	struct Anim
	{
		AnimHandle animHandle;
		SGRX_Animation::Track** animTracks;
		float at;
		float fadetime;
		bool once;
	};
	
	~AnimPlayer();
	virtual void Prepare( String* names, int count );
	virtual void Advance( float deltaTime );
	
	void Play( const AnimHandle& anim, bool once = false, float fadetime = 0.5f );
	
	AnimCache animCache;
	Array< Anim > currentAnims;
};




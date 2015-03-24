

#pragma once
#include "utils.hpp"


#ifdef SOUND_BUILDING
#  define SOUND_EXPORT __declspec(dllexport)
#else
#  define SOUND_EXPORT __declspec(dllimport)
#endif



struct SGRX_Sound3DAttribs
{
	Vec3 position;
	Vec3 velocity;
	Vec3 forward;
	Vec3 up;
};


struct SOUND_EXPORT SGRX_ISoundEventInstance
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_ISoundEventInstance() : _refcount(0){}
	virtual ~SGRX_ISoundEventInstance(){}
	
	virtual void Start() = 0;
	virtual void Stop( bool immediate = false ) = 0;
	virtual bool GetPaused() = 0;
	virtual void SetPaused( bool paused ) = 0;
	virtual float GetVolume() = 0;
	virtual void SetVolume( float v ) = 0;
	virtual float GetPitch() = 0;
	virtual void SetPitch( float v ) = 0;
	virtual bool SetParameter( const StringView& name, float value ) = 0;
	virtual void Set3DAttribs( const SGRX_Sound3DAttribs& attribs ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_ISoundEventInstance > SoundEventInstanceHandle;


struct SOUND_EXPORT SGRX_ISoundSystem
{
	FINLINE void Acquire(){ ++_refcount; }
	FINLINE void Release(){ --_refcount; if( _refcount <= 0 ) delete this; }
	SGRX_ISoundSystem() : _refcount(0){}
	virtual ~SGRX_ISoundSystem(){}
	
	virtual void Update() = 0;
	virtual bool Load( const StringView& file, bool async = false ) = 0;
	virtual bool PreloadEventData( const StringView& name ) = 0;
	virtual SoundEventInstanceHandle CreateEventInstance( const StringView& name ) = 0;
	virtual void Set3DAttribs( const SGRX_Sound3DAttribs& attribs ) = 0;
	
	int32_t _refcount;
};
typedef Handle< SGRX_ISoundSystem > SoundSystemHandle;


SOUND_EXPORT SoundSystemHandle SND_CreateSystem( int maxchannels = 32, bool rh3d = true );


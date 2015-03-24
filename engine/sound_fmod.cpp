

#include "sound.hpp"

#include "fmod_studio.h"


inline FMOD_VECTOR V2FV( const Vec3& v )
{
	FMOD_VECTOR out = {0};
	out.x = v.x;
	out.y = v.y;
	out.z = v.z;
	return out;
}


struct FMODSoundEventInstance : SGRX_ISoundEventInstance
{
	FMODSoundEventInstance( FMOD_STUDIO_EVENTINSTANCE* inst ) : m_eventInst( inst )
	{
	}
	~FMODSoundEventInstance()
	{
		FMOD_Studio_EventInstance_Release( m_eventInst );
	}
	
	void Start()
	{
		FMOD_Studio_EventInstance_Start( m_eventInst );
	}
	void Stop( bool immediate )
	{
		FMOD_Studio_EventInstance_Stop( m_eventInst, immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT );
	}
	bool GetPaused()
	{
		FMOD_BOOL out = 0;
		if( FMOD_Studio_EventInstance_GetPaused( m_eventInst, &out ) == FMOD_OK )
			return out != 0;
		return false;
	}
	void SetPaused( bool paused )
	{
		FMOD_Studio_EventInstance_SetPaused( m_eventInst, paused );
	}
	float GetVolume()
	{
		float out = 1;
		if( FMOD_Studio_EventInstance_GetVolume( m_eventInst, &out ) == FMOD_OK )
			return out;
		return 1;
	}
	void SetVolume( float v )
	{
		FMOD_Studio_EventInstance_SetVolume( m_eventInst, v );
	}
	float GetPitch()
	{
		float out = 1;
		if( FMOD_Studio_EventInstance_GetPitch( m_eventInst, &out ) == FMOD_OK )
			return out;
		return 1;
	}
	void SetPitch( float v )
	{
		FMOD_Studio_EventInstance_SetPitch( m_eventInst, v );
	}
	bool SetParameter( const StringView& name, float value )
	{
		return FMOD_Studio_EventInstance_SetParameterValue( m_eventInst, StackString<256>( name ), value ) == FMOD_OK;
	}
	void Set3DAttribs( const SGRX_Sound3DAttribs& attribs )
	{
		FMOD_3D_ATTRIBUTES fmattr = {0};
		fmattr.position = V2FV( attribs.position );
		fmattr.velocity = V2FV( attribs.velocity );
		fmattr.forward = V2FV( attribs.forward );
		fmattr.up = V2FV( attribs.up );
		FMOD_Studio_EventInstance_Set3DAttributes( m_eventInst, &fmattr );
	}
	
	FMOD_STUDIO_EVENTINSTANCE* m_eventInst;
};


struct FMODSoundSystem : SGRX_ISoundSystem
{
	FMODSoundSystem( FMOD_STUDIO_SYSTEM* sys ) : m_sys( sys )
	{
	}
	~FMODSoundSystem()
	{
		FMOD_Studio_System_Release( m_sys );
	}
	
	bool Load( const StringView& file, bool async )
	{
		FMOD_STUDIO_BANK* outbank = NULL;
		int initflags = async ? FMOD_STUDIO_LOAD_BANK_NONBLOCKING : FMOD_STUDIO_LOAD_BANK_NORMAL;
		int ret = FMOD_Studio_System_LoadBankFile( m_sys, StackPath(file), initflags, &outbank );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD bank file: " << file << " (error " << ret << ")";
			return false;
		}
		return true;
	}
	bool PreloadEventData( const StringView& name )
	{
		char bfr[ 256 ] = {0};
		snprintf( bfr, sizeof(bfr), "event:%.*s", TMIN( 240, (int) name.size() ) );
		
		FMOD_STUDIO_EVENTDESCRIPTION* desc = NULL;
		int ret = FMOD_Studio_System_GetEvent( m_sys, bfr, &desc );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to find FMOD event: " << name << " (error " << ret << ")";
			return false;
		}
		
		ret = FMOD_Studio_EventDescription_LoadSampleData( desc );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD sample data for event: " << name << " (error " << ret << ")";
			return false;
		}
		return true;
	}
	SoundEventInstanceHandle CreateEventInstance( const StringView& name )
	{
		char bfr[ 256 ] = {0};
		snprintf( bfr, sizeof(bfr), "event:%.*s", TMIN( 240, (int) name.size() ) );
		
		FMOD_STUDIO_EVENTDESCRIPTION* desc = NULL;
		int ret = FMOD_Studio_System_GetEvent( m_sys, bfr, &desc );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to find FMOD event: " << name << " (error " << ret << ")";
			return NULL;
		}
		
		FMOD_STUDIO_EVENTINSTANCE* inst = NULL;
		ret = FMOD_Studio_EventDescription_CreateInstance( desc, &inst );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to create FMOD event instance: " << name << " (error " << ret << ")";
			return NULL;
		}
		
		return new FMODSoundEventInstance( inst );
	}
	void Set3DAttribs( const SGRX_Sound3DAttribs& attribs )
	{
		FMOD_3D_ATTRIBUTES fmattr = {0};
		fmattr.position = V2FV( attribs.position );
		fmattr.velocity = V2FV( attribs.velocity );
		fmattr.forward = V2FV( attribs.forward );
		fmattr.up = V2FV( attribs.up );
		FMOD_Studio_System_SetListenerAttributes( m_sys, &fmattr );
	}
	
	FMOD_STUDIO_SYSTEM* m_sys;
};


SoundSystemHandle SND_CreateSystem( int maxchannels, bool rh3d )
{
	FMOD_STUDIO_SYSTEM* sys = NULL;
	if( FMOD_Studio_System_Create( &sys, FMOD_VERSION ) != FMOD_OK )
		return NULL;
	int initflags = FMOD_INIT_NORMAL;
	if( rh3d )
		initflags |= FMOD_INIT_3D_RIGHTHANDED;
	if( FMOD_Studio_System_Initialize( sys, maxchannels, FMOD_STUDIO_INIT_NORMAL, initflags, NULL ) != FMOD_OK )
	{
		FMOD_Studio_System_Release( sys );
		return NULL;
	}
	return new FMODSoundSystem( sys );
}


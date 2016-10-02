

#include "engine.hpp"
#include "sound.hpp"

#include "fmod_studio.h"


struct err2name
{
	int code;
	const char* name;
};
static const err2name fmod_error_codes[] =
{
#define EC( n ) { n, #n }
	EC( FMOD_OK ),
	EC( FMOD_ERR_BADCOMMAND ),
	EC( FMOD_ERR_CHANNEL_ALLOC ),
	EC( FMOD_ERR_CHANNEL_STOLEN ),
	EC( FMOD_ERR_DMA ),
	EC( FMOD_ERR_DSP_CONNECTION ),
	EC( FMOD_ERR_DSP_DONTPROCESS ),
	EC( FMOD_ERR_DSP_FORMAT ),
	EC( FMOD_ERR_DSP_INUSE ),
	EC( FMOD_ERR_DSP_NOTFOUND ),
	EC( FMOD_ERR_DSP_RESERVED ),
	EC( FMOD_ERR_DSP_SILENCE ),
	EC( FMOD_ERR_DSP_TYPE ),
	EC( FMOD_ERR_FILE_BAD ),
	EC( FMOD_ERR_FILE_COULDNOTSEEK ),
	EC( FMOD_ERR_FILE_DISKEJECTED ),
	EC( FMOD_ERR_FILE_EOF ),
	EC( FMOD_ERR_FILE_ENDOFDATA ),
	EC( FMOD_ERR_FILE_NOTFOUND ),
	EC( FMOD_ERR_FORMAT ),
	EC( FMOD_ERR_HEADER_MISMATCH ),
	EC( FMOD_ERR_HTTP ),
	EC( FMOD_ERR_HTTP_ACCESS ),
	EC( FMOD_ERR_HTTP_PROXY_AUTH ),
	EC( FMOD_ERR_HTTP_SERVER_ERROR ),
	EC( FMOD_ERR_HTTP_TIMEOUT ),
	EC( FMOD_ERR_INITIALIZATION ),
	EC( FMOD_ERR_INITIALIZED ),
	EC( FMOD_ERR_INTERNAL ),
	EC( FMOD_ERR_INVALID_FLOAT ),
	EC( FMOD_ERR_INVALID_HANDLE ),
	EC( FMOD_ERR_INVALID_PARAM ),
	EC( FMOD_ERR_INVALID_POSITION ),
	EC( FMOD_ERR_INVALID_SPEAKER ),
	EC( FMOD_ERR_INVALID_SYNCPOINT ),
	EC( FMOD_ERR_INVALID_THREAD ),
	EC( FMOD_ERR_INVALID_VECTOR ),
	EC( FMOD_ERR_MAXAUDIBLE ),
	EC( FMOD_ERR_MEMORY ),
	EC( FMOD_ERR_MEMORY_CANTPOINT ),
	EC( FMOD_ERR_NEEDS3D ),
	EC( FMOD_ERR_NEEDSHARDWARE ),
	EC( FMOD_ERR_NET_CONNECT ),
	EC( FMOD_ERR_NET_SOCKET_ERROR ),
	EC( FMOD_ERR_NET_URL ),
	EC( FMOD_ERR_NET_WOULD_BLOCK ),
	EC( FMOD_ERR_NOTREADY ),
	EC( FMOD_ERR_OUTPUT_ALLOCATED ),
	EC( FMOD_ERR_OUTPUT_CREATEBUFFER ),
	EC( FMOD_ERR_OUTPUT_DRIVERCALL ),
	EC( FMOD_ERR_OUTPUT_FORMAT ),
	EC( FMOD_ERR_OUTPUT_INIT ),
	EC( FMOD_ERR_OUTPUT_NODRIVERS ),
	EC( FMOD_ERR_PLUGIN ),
	EC( FMOD_ERR_PLUGIN_MISSING ),
	EC( FMOD_ERR_PLUGIN_RESOURCE ),
	EC( FMOD_ERR_PLUGIN_VERSION ),
	EC( FMOD_ERR_RECORD ),
	EC( FMOD_ERR_REVERB_CHANNELGROUP ),
	EC( FMOD_ERR_REVERB_INSTANCE ),
	EC( FMOD_ERR_SUBSOUNDS ),
	EC( FMOD_ERR_SUBSOUND_ALLOCATED ),
	EC( FMOD_ERR_SUBSOUND_CANTMOVE ),
	EC( FMOD_ERR_TAGNOTFOUND ),
	EC( FMOD_ERR_TOOMANYCHANNELS ),
	EC( FMOD_ERR_TRUNCATED ),
	EC( FMOD_ERR_UNIMPLEMENTED ),
	EC( FMOD_ERR_UNINITIALIZED ),
	EC( FMOD_ERR_UNSUPPORTED ),
	EC( FMOD_ERR_VERSION ),
	EC( FMOD_ERR_EVENT_ALREADY_LOADED ),
	EC( FMOD_ERR_EVENT_LIVEUPDATE_BUSY ),
	EC( FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH ),
	EC( FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT ),
	EC( FMOD_ERR_EVENT_NOTFOUND ),
	EC( FMOD_ERR_STUDIO_UNINITIALIZED ),
	EC( FMOD_ERR_STUDIO_NOT_LOADED ),
	EC( FMOD_ERR_INVALID_STRING ),
	EC( FMOD_ERR_ALREADY_LOCKED ),
	EC( FMOD_ERR_NOT_LOCKED ),
	EC( FMOD_RESULT_FORCEINT ),
#undef EC
};
struct FMOD_EC : SGRX_Log::Loggable< FMOD_EC >
{
	FMOD_EC( int c ) : code( c ){}
	int code;
	
	const char* name() const
	{
		for( size_t i = 0; i < sizeof(fmod_error_codes)/sizeof(fmod_error_codes[0]); ++i )
		{
			if( fmod_error_codes[ i ].code == code )
				return fmod_error_codes[ i ].name;
		}
		return "<unknown>";
	}
	void Log( SGRX_Log& L ) const
	{
		L << " (FMOD error code=" << code << ", " << name() << ")";
	}
};


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
	
	void Update()
	{
		FMOD_Studio_System_Update( m_sys );
	}
	bool Load( const StringView& file, bool async )
	{
		String respath;
		if( FS_FindRealPath( file, respath ) == false )
		{
			LOG << "Failed to resolve path: " << file;
			return false;
		}
		
		FMOD_STUDIO_BANK* outbank = NULL;
		int initflags = async ? FMOD_STUDIO_LOAD_BANK_NONBLOCKING : FMOD_STUDIO_LOAD_BANK_NORMAL;
		int ret = FMOD_Studio_System_LoadBankFile( m_sys, StackPath(respath), initflags, &outbank );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD bank file: " << respath << FMOD_EC( ret );
			return false;
		}
		
		LOG << "Loaded FMOD bank: " << file << " (resolved to " << respath << ")";
		return true;
	}
	bool EnumerateSoundEvents( Array< RCString >& out )
	{
		LOG << "FMODSoundSystem::EnumerateSoundEvents() called";
		// can we have more, realistically? not yet.
#define MAX_ENUM_BANKS 128
		FMOD_STUDIO_BANK* banks[ MAX_ENUM_BANKS ];
		int bankeventcounts[ MAX_ENUM_BANKS ];
		int bankcount = 0;
		int ret = FMOD_Studio_System_GetBankList( m_sys, banks, MAX_ENUM_BANKS, &bankcount );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to enumerate FMOD banks" << FMOD_EC( ret );
			return false;
		}
		if( !bankcount )
		{
			LOG << "- NO BANKS";
			return false;
		}
		LOG << "- " << bankcount << " banks found";
		
		int totaleventcount = 0;
		for( int i = 0; i < bankcount; ++i )
		{
			int eventcount = 0;
			ret = FMOD_Studio_Bank_GetStringCount( banks[ i ], &eventcount );
			if( ret != FMOD_OK )
			{
				LOG << "Failed to enumerate events of FMOD bank" << FMOD_EC( ret );
				return false;
			}
			bankeventcounts[ i ] = eventcount;
			totaleventcount += eventcount;
		}
		
		out.clear();
		out.reserve( totaleventcount );
		for( int i = 0; i < bankcount; ++i )
		{
			for( int j = 0; j < bankeventcounts[ i ]; ++j )
			{
				char bfr[ 256 ] = {0};
				int outsize = 0;
				ret = FMOD_Studio_Bank_GetStringInfo( banks[ i ], j, NULL, bfr, 256, &outsize );
			//	LOG << bfr;
				if( ret == FMOD_ERR_TRUNCATED )
				{
					LOG << "Failed to get full string info of FMOD bank string '" << bfr << "'" << FMOD_EC( ret );
					// can continue but skip the string for now
				}
				else if( ret != FMOD_OK )
				{
					LOG << "Failed to get string info of FMOD bank" << FMOD_EC( ret );
					return false;
				}
				else if( strncmp( bfr, "event:", 6 ) == 0 )
				{
					out.push_back( bfr + 6 );
				}
			}
		}
		
		return true;
	}
	FMOD_STUDIO_EVENTDESCRIPTION* GetEvent( const StringView& name )
	{
		char bfr[ 256 ] = {0};
		sgrx_snprintf( bfr, sizeof(bfr), "event:%.*s", TMIN( 240, (int) name.size() ), name.data() );
		
		FMOD_STUDIO_EVENTDESCRIPTION* desc = NULL;
		int ret = FMOD_Studio_System_GetEvent( m_sys, bfr, &desc );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to find FMOD event: " << name << FMOD_EC( ret );
			return NULL;
		}
		return desc;
	}
	bool PreloadEventData( const StringView& name )
	{
		FMOD_STUDIO_EVENTDESCRIPTION* desc = GetEvent( name );
		if( !desc )
			return false;
		
		int ret = FMOD_Studio_EventDescription_LoadSampleData( desc );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD sample data for event: " << name << FMOD_EC( ret );
			return false;
		}
		return true;
	}
	bool EventIsOneShot( const StringView& name )
	{
		FMOD_STUDIO_EVENTDESCRIPTION* desc = GetEvent( name );
		if( !desc )
			return false;
		
		FMOD_BOOL isoneshot = 0;
		int ret = FMOD_Studio_EventDescription_IsOneshot( desc, &isoneshot );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD info for event: " << name << FMOD_EC( ret );
			return false;
		}
		return isoneshot != 0;
	}
	SoundEventInstanceHandle CreateEventInstanceRaw( const StringView& name )
	{
		FMOD_STUDIO_EVENTDESCRIPTION* desc = GetEvent( name );
		if( !desc )
			return NULL;
		
		FMOD_STUDIO_EVENTINSTANCE* inst = NULL;
		int ret = FMOD_Studio_EventDescription_CreateInstance( desc, &inst );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to create FMOD event instance: " << name << FMOD_EC( ret );
			return NULL;
		}
		FMOD_BOOL isoneshot = 0;
		ret = FMOD_Studio_EventDescription_IsOneshot( desc, &isoneshot );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to load FMOD info for event: " << name << FMOD_EC( ret );
			return NULL;
		}
		
		FMODSoundEventInstance* evinst = new FMODSoundEventInstance( inst );
		evinst->isOneShot = isoneshot != 0;
		return evinst;
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
	FMOD_STUDIO_BUS* GetBus( const StringView& name )
	{
		FMOD_STUDIO_BUS* bus = NULL;
		int ret = FMOD_Studio_System_GetBus( m_sys, StackPath( name ), &bus );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to find FMOD bus: " << name << FMOD_EC( ret );
			return NULL;
		}
		return bus;
	}
	float GetVolume( const StringView& name )
	{
		FMOD_STUDIO_BUS* bus = GetBus( name );
		if( !bus )
			return 0;
		
		float vol = 0.0f;
		int ret = FMOD_Studio_Bus_GetFaderLevel( bus, &vol );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to get FMOD bus fader level: " << name << FMOD_EC( ret );
			return 0.0f;
		}
		return vol;
	}
	void SetVolume( const StringView& name, float vol )
	{
		FMOD_STUDIO_BUS* bus = GetBus( name );
		if( !bus )
			return;
		
		int ret = FMOD_Studio_Bus_SetFaderLevel( bus, vol );
		if( ret != FMOD_OK )
		{
			LOG << "Failed to get FMOD bus fader level: " << name << FMOD_EC( ret );
		}
	}
	
	FMOD_STUDIO_SYSTEM* m_sys;
};


SoundSystemHandle SND_CreateSystem( int maxchannels, bool rh3d )
{
	FMOD_STUDIO_SYSTEM* sys = NULL;
	int ret = FMOD_Studio_System_Create( &sys, FMOD_VERSION );
	if( ret != FMOD_OK )
	{
		LOG_ERROR << LOG_DATE << "  Failed to create FMOD sound system!" << FMOD_EC( ret );
		return NULL;
	}
	int initflags = FMOD_INIT_NORMAL;
	if( rh3d )
		initflags |= FMOD_INIT_3D_RIGHTHANDED;
	ret = FMOD_Studio_System_Initialize( sys, maxchannels, FMOD_STUDIO_INIT_NORMAL, initflags, NULL );
	if( ret != FMOD_OK )
	{
		LOG_ERROR << LOG_DATE << "  Failed to initialize FMOD sound system!" << FMOD_EC( ret );
		LOG_WARNING << LOG_DATE << "  No sound will be available!";
		FMOD_Studio_System_Release( sys );
		ret = FMOD_Studio_System_Create( &sys, FMOD_VERSION );
		if( ret != FMOD_OK )
		{
			LOG_ERROR << LOG_DATE << "  Failed to create FMOD sound system!" << FMOD_EC( ret );
			return NULL;
		}
		FMOD_SYSTEM* llsys = NULL;
		ret = FMOD_Studio_System_GetLowLevelSystem( sys, &llsys );
		if( ret != FMOD_OK )
		{
			LOG_ERROR << LOG_DATE << "  Failed to get FMOD low level system (for NOSOUND init)" << FMOD_EC( ret );
		}
		else
		{
			ret = FMOD_System_SetOutput( llsys, FMOD_OUTPUTTYPE_NOSOUND );
			if( ret != FMOD_OK )
			{
				LOG_ERROR << LOG_DATE << "  Failed to set NOSOUND output for FMOD sound system!" << FMOD_EC( ret );
			}
			else
			{
				ret = FMOD_Studio_System_Initialize( sys, maxchannels, FMOD_STUDIO_INIT_NORMAL, initflags, NULL );
				if( ret != FMOD_OK )
				{
					LOG_ERROR << LOG_DATE << "  Failed to initialize FMOD sound system with NOSOUND output!" << FMOD_EC( ret );
				}
			}
		}
	//	FMOD_Studio_System_Release( sys );
	//	return NULL;
	}
	return new FMODSoundSystem( sys );
}




#include <ctype.h>

#define USE_VEC2
#define USE_VEC3
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "level.hpp"


#define MAX_HEALTH 1000

const float SPEED_WALK = 0.4f;
const float SPEED_SPRINT = 1.5f;
const float SPEED_RUN = 1.0f;


WeaponType g_WeaponTypes[] =
{
#define WEAPONTYPE_SMG 0
	{
		"SMG",
		30, 30 * 6, // max clip, bag ammo
		800.0f, 1.0f, // bullet speed, time
		0.11f, 0.02f, // damage, damage_rand
		0.09f, 1.0f, // fire rate, reload time
		1.0f, 10.0f, // spread, backfire
		"smg_shoot", "smg_reload", // shoot, reload sounds
	},
};



Weapon::Weapon( BulletSystem* bsys, WeaponType* wt, int initial_ammo ) :
	m_bulletSystem( bsys ),
	m_type( wt ),
	m_fire_timeout( 0 ),
	m_reload_timeout( 0 ),
	m_shooting( false ),
	m_shoot_factor( 0 ),
	m_position( Vec2::Create( 0 ) ),
	m_direction( Vec2::Create( 0 ) ),
	m_owner_speed( 0 )
{
	int ic_ammo = initial_ammo;
	int ib_ammo = 0;
	if( ic_ammo > m_type->max_clip_ammo )
	{
		ic_ammo = m_type->max_clip_ammo;
		ib_ammo = initial_ammo - ic_ammo;
	}
	m_ammo_clip = ic_ammo;
	m_ammo_bag = ib_ammo;
}

void Weapon::SetShooting( bool shoot )
{
	if( m_shooting != shoot )
	{
		if( shoot )
		{
			if( m_reload_timeout && m_ammo_clip )
				StopReload();
		}
		else
		{
			if( !m_ammo_clip && !m_reload_timeout )
				BeginReload();
		}
		
		m_shooting = shoot;
	}
}

float Weapon::GetSpread()
{
	float timespeed = m_owner_speed * 0.1 + m_shoot_factor;
	return m_type->spread + m_type->backfire * timespeed;
}

bool Weapon::BeginReload()
{
	if( !m_ammo_bag || m_ammo_clip >= m_type->max_clip_ammo )
		return false;
	m_reload_timeout = m_type->reload_time;
// TODO 	m_reload_sound.play();
	return true;
}

void Weapon::StopReload()
{
	m_reload_timeout = 0;
// TODO 	m_reload_sound.stop();
}

int Weapon::Tick( float deltaTime, Vec2 pos, Vec2 dir, float speed )
{
	m_position = pos;
	m_direction = dir;
	m_owner_speed = speed;
	m_shoot_factor -= deltaTime;
	if( m_shoot_factor < 0 )
		m_shoot_factor = 0;
	
// TODO 	m_reload_sound.set_position( m_position.x, m_position.y, 0 );
	
	if( m_reload_timeout > 0 )
	{
		m_reload_timeout -= deltaTime;
		if( m_reload_timeout <= 0 )
		{
			int ammo_needed = m_type->max_clip_ammo - m_ammo_clip;
			int ammo_got = IIF( m_ammo_bag > ammo_needed, ammo_needed, m_ammo_bag );
			m_ammo_clip += ammo_got;
			m_ammo_bag -= ammo_got;
		}
		return WEAPON_INACTIVE;
	}
	
	if( m_fire_timeout > 0 )
	{
		m_fire_timeout -= deltaTime;
		return WEAPON_INACTIVE;
	}
	
	if( m_shooting )
	{
		if( m_ammo_clip <= 0 )
		{
			// TODO: click empty
			return WEAPON_EMPTY;
		}
		
// TODO 	m_shoot_sound.stop();
// TODO 	m_shoot_sound.set_position( m_position.x, m_position.y, 0 );
// TODO 	m_shoot_sound.play();
		
		Vec2 bullet_dir = m_direction.Normalized();
		Vec2 bullet_start = m_position + bullet_dir * 12;
		Vec2 bullet_velocity = bullet_dir.Rotate( randf11() * DEG2RAD( GetSpread() ) ) * m_type->bullet_speed;
		
		m_bulletSystem->Add( bullet_start, bullet_velocity, m_type->bullet_time, m_type->damage + randf11() * m_type->damage_rand );
// TODO 	g_World.add_entity( Bullet_create( bullet_start, bullet_velocity, m_type->bullet_time, m_type->damage + randf11() * m_type->damage_rand ) );
		
// TODO 	g_AIDB.emit_sound( bullet_start, AI_SOUND_GUNFIRE, 0.5, 600, m_emit_id );
		
		m_shoot_factor += m_type->fire_rate * 2;
		m_ammo_clip--;
		m_fire_timeout += m_type->fire_rate;
		return WEAPON_SHOT;
	}
	
	return WEAPON_INACTIVE;
}


#if 0
Character::Character( bool isplayer, Vec2 pos, float angle, const StringView& sprite ) :
	m_is_player( isplayer ),
	m_weapon( NULL ),
	m_position( pos ),
	m_velocity( Vec2::Create(0) ),
	m_angle( angle ),
	m_leg_dir( Vec2::Create( 1, 0 ) ),
	m_leg_time( 0.0f ),
	m_leg_factor( 0.0f ),
	m_health( MAX_HEALTH ),
	m_dead( false ),
	m_scale( 32 ),
	m_flash_strength( 0 )
{
	char buf[ 64 ] = {0};
	snprintf( buf, 63, "sprites/%.*s.png", TMIN( (int) sprite.size(), 40 ), sprite.data() );
	m_sprite_base = GR_GetTexture( buf );
	snprintf( buf, 63, "sprites/%.*s_arms.png", TMIN( (int) sprite.size(), 40 ), sprite.data() );
	m_sprite_arms = GR_GetTexture( buf );
	snprintf( buf, 63, "sprites/%.*s_shadow.png", TMIN( (int) sprite.size(), 40 ), sprite.data() );
	m_sprite_shadow = GR_GetTexture( buf );
	m_sprite_dead_char = GR_GetTexture( "sprites/dead_char.png" );
	m_sprite_flash = GR_GetTexture( "sprites/muzzle_flash.png" );
	m_sprite_flash_light = GR_GetTexture( "sprites/muzzle_flash_light.png" );
	
	// TODO backlinks
}

void Character::Die()
{
	if( m_dead )
	{
		m_dead = true;
		// TODO disable collisions
	}
}

void Character::BulletHit( Vec2 hitvec, Vec2 hitpos, Vec2 hitnrm )
{
}

void Character::MoveChar( Vec2 move, float deltaTime )
{
}

void Character::DrawChar2D()
{
}



Player::Player( const Vec3& pos ) :
	Character( true, pos.ToVec2(), 0, "char_player" ),
	m_sprint_time( 0 ),
	m_max_sprint_time( 2 ),
	m_sprint_cooldown( false ),
	m_dead_time( -1 )
{
	m_tex_crosshair = GR_GetTexture( "ui/crosshair.png" );
}

void Player::Tick( float deltaTime, float blendFactor )
{
	if( !m_dead )
	{
		Vec2 move = { MOVE_RIGHT.value - MOVE_LEFT.value, MOVE_DOWN.value - MOVE_UP.value };
		if( move.Length() > 1 )
			move.Normalize();
		
		bool sprinting = false;
		if( SLOW_WALK.state )
			move *= SPEED_WALK;
		else if( SPRINT.state && m_sprint_time < m_max_sprint_time && !m_sprint_cooldown )
		{
			move *= SPEED_SPRINT;
			if( move.Length() > 0 )
			{
				m_sprint_time += deltaTime;
				sprinting = true;
			}
			if( m_sprint_time > m_max_sprint_time )
				m_sprint_cooldown = true;
		}
		if( !sprinting && m_sprint_time > 0 )
			m_sprint_time -= deltaTime;
		if( m_sprint_time <= 0 && m_sprint_cooldown )
			m_sprint_cooldown = true;
		
		Vec2 cp = Game_GetCursorPos(); // TODO WORLD CURSOR POS
	//	float angle = Vec2::Create( cp.x - m_position.x, -( cp.y - m_position.y ) ).Angle();
		
		MoveChar( move, deltaTime );
		
		if( m_weapon )
		{
			m_weapon->SetShooting( SHOOT.state );
			if( RELOAD.state )
				m_weapon->BeginReload();
			bool hasshot = m_weapon->Tick( deltaTime, m_position, Vec2::CreateFromAngle( m_angle ), move.Length() ) == WEAPON_SHOT;
			if( hasshot )
				m_flash_strength = 1;
		}
	}
	else if( m_dead_time >= 0 )
	{
		// TODO
	//	if( Game.FadeOutFactor === null )
	//	{
	//		Game.FadeOutFactor = 0;
	//		CurrentMusic.fade_out();
	//	}
		m_dead_time += deltaTime;
		if( m_dead_time > 3 )
		{
			Game_SetPaused( true );
			// TODO
		//	Game.EnableMenu = false;
		//	
		//	gom =
		//	[
		//		{ text = "Restart level", action = function(){ Game.Restart(); Game.PopOverlayScreen(); } },
		//		{ text = "Return to main menu", action = function(){ Game.End(); Game.PopOverlayScreen(); } },
		//	];
		//	Game.AddOverlayScreen( MenuScreen, { title = "Mission failed", options = gom } );
			
			m_dead_time = -1;
		}
	}
	
//	g_AIDB.put_entity( this.id, if( this.dead, ENTGROUP_GOODGUYS_DEAD, ENTGROUP_GOODGUYS ), m_position );
//	
//	float q = 1 - pow( 1.0f / 1000.0f, deltaTime );
//	Game.SetCameraPos( lerp( Game.CameraPos.x, lerp( this.x, g_WorldCursorPos.x, 0.1 ), q ), lerp( Game.CameraPos.y, lerp( this.y, g_WorldCursorPos.y, 0.1 ), q ) );
}

void Player::Draw2D()
{
	DrawChar2D();
}

void Player::DrawUI()
{
	int W = GR_GetWidth();
	int H = GR_GetHeight();
	int fontsize1 = 16 * 576 / TMIN( W, H );
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	// crosshair (do not show if game paused, it will still move around then)
	if( !Game_IsPaused() )
	{
		float crh_scale = 32 + ( m_weapon ? m_weapon->GetSpread() * 2 : 0 );
		Vec2 cursor_pos = Game_GetCursorPos();
		Vec2 cursor_dir = Vec2::CreateFromAngle( m_angle, crh_scale );
		br.SetTexture( m_tex_crosshair ).Col( 0.9, 0.1, 0.1, m_weapon ? 0.7 : 0.3 ).TurnedBox( cursor_pos.x, cursor_pos.y, cursor_dir.x, cursor_dir.y );
	}
	
	GR2D_SetFont( "fonts/gratis.ttf", fontsize1 );
	
	// HEALTH INFO
	const char* health_text = "HEALTH " "||||||||||" "||||||||||"; /* 10 x 2  lines  + 7 pre */
	br.Col( 0.3, 1 );
	GR2D_DrawTextLine( 32, 32, health_text );
	
	float hf = m_health / (float) MAX_HEALTH;
	float chf = clamp( hf, 0.0f, 1.0f );
	br.Col( 0.8, 0.2, 0.1, 1 );
	GR2D_DrawTextLine( 32, 32, StringView( health_text, 7 + 20 * chf ) );
	
	// SPRINT INFO
	const char* sprint_text = "SPRINT  " "||||||||||" "||||||||||"; /* 10 x 2  lines  + 8 pre */
	if( !m_sprint_cooldown || sin( m_sprint_time * 15 ) > 0 )
	{
		br.Col( 0.3, 0.3, 0.3, IIF( m_sprint_time > 0, 1, 0.5 ) );
		GR2D_DrawTextLine( 32, 64, sprint_text );
		if( m_sprint_time > 0 )
		{
			float sf = 1 - m_sprint_time / (float) m_max_sprint_time;
			float csf = clamp( sf, 0.0f, 1.0f );
			br.Col( 0.1, 0.2, 0.8 );
			GR2D_DrawTextLine( 32, 64, StringView( sprint_text, 8 + 20 * csf ) );
		}
	}
	else
	{
		br.Col( 0.1, 0.2, 0.8 );
		GR2D_DrawTextLine( 32, 64, StringView( sprint_text, 6 ) );
	}
	
	// AMMO INFO
	if( m_weapon )
	{
		StringView name = m_weapon->m_type->name;
		char ammoinfo[ 1000 ] = {0};
		char* outptr = ammoinfo;
		for( size_t i = 0; i < name.size() && outptr < ammoinfo + 900; ++i )
		{
			*outptr++ = islower( name[ i ] ) ? toupper( name[ i ] ) : name[ i ];
		}
		snprintf( ammoinfo + strlen( ammoinfo ), 999, " AMMO\n%d / %d\n%d / %d",
			m_weapon->m_ammo_clip, m_weapon->m_type->max_clip_ammo,
			m_weapon->m_ammo_bag, m_weapon->m_type->max_bag_ammo
		);
		br.Col( 1 );
		
		GR2D_DrawTextLine( W/2, 32, ammoinfo );
		// TODO
		// SS_DrawTextRect( ammoinfo, g_GameFont, [1,1,1], DT_TOP|DT_RIGHT, 32, W-32, 32, H-32 );
		
		if( m_weapon->m_reload_timeout > 0 )
		{
			const char* reload_text = "RELOADING " "||||||||||"; /* 10 x 1  lines  + 10 pre */
			float rel_fac = 1 - m_weapon->m_reload_timeout / m_weapon->m_type->reload_time;
			br.Col( 0.5, 1 );
			GR2D_DrawTextLine( W/3*2, 32, reload_text );
			br.Col( 1 );
			GR2D_DrawTextLine( W/3*2, 32, StringView( reload_text, 10 + rel_fac * 10 ) );
		}
	}
}

bool Player::AddItem( const StringView& item, int count )
{
	String key = item;
	int* ic = m_items.getptr( key );
	if( count < 0 )
	{
		if( !ic || *ic < count )
			return false;
		*ic += count;
	}
	else
	{
		if( !ic )
			m_items.set( key, count );
		else
			*ic += count;
	}
	return true;
}

bool Player::HasItem( const StringView& item, int count )
{
	int* ic = m_items.getptr( item );
	return ic && *ic >= count;
}
#endif



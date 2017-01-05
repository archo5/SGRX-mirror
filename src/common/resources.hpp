

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "systems.hpp"



#define GO_RSRC_MESH     1
#define GO_RSRC_LIGHT    2
#define GO_RSRC_PSYS     3
#define GO_RSRC_RBODY    4
#define GO_RSRC_JOINT    5 // TODO
#define GO_RSRC_SNDSRC   6
#define GO_RSRC_REFPLANE 7
#define GO_RSRC_CAMERA   8
#define GO_RSRC_FLARE    9
#define GO_RSRC_CHAR     10



EXP_STRUCT MeshResource : GOResource, SGRX_MeshInstUserData
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( MeshResource, GO_RSRC_MESH, "Mesh" );
	
	GFW_EXPORT MeshResource( GameObject* obj );
	GFW_EXPORT ~MeshResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void EditorDrawWorld();
	GFW_EXPORT void _UpdateLighting();
	GFW_EXPORT void _UpdateMatrix();
	GFW_EXPORT void MeshInstUser_OnEvent( SGRX_MeshInstance* MI, uint32_t evid, void* data );
	
	FINLINE void _UpEv(){ Game_FireEvent( EID_GOResourceUpdate, this ); }
	
	bool IsVisible() const { return m_isVisible; }
	void SetVisible( bool v ){ m_isVisible = v; m_meshInst->enabled = v; _UpEv(); }
	MeshHandle GetMeshData() const { return m_mesh; }
	GFW_EXPORT void SetMeshData( MeshHandle mesh );
	StringView GetMeshPath() const { return m_mesh ? SV(m_mesh->m_key) : SV(); }
	void SetMeshPath( StringView path ){ SetMeshData( GR_GetMesh( path ) ); }
	int GetLightingMode() const { return m_lightingMode; }
	void SetLightingMode( int v ){ m_lightingMode = v;
		m_meshInst->SetLightingMode( (SGRX_LightingMode) v ); _UpdateLighting(); }
	uint32_t GetLayers() const { return m_meshInst->layers; }
	void SetLayers( uint32_t layers ){ m_meshInst->layers = layers; }
	bool HasBulletInteraction() const { return m_meshInst->userData != NULL; }
	void SetBulletInteraction( bool v ){ m_meshInst->userData = v ? (SGRX_MeshInstUserData*) this : NULL; }
	SGS_METHOD int GetMaterialCount() const { return m_meshInst->GetMaterialCount(); }
	SGS_METHOD String GetMaterial( int i );
	SGS_METHOD void SetMaterial( int i, StringView desc );
	
	SGS_PROPERTY_FUNC( READ SOURCE m_meshInst.item ) SGS_ALIAS( void* meshInst );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME isStatic ) bool m_isStatic;
	SGS_PROPERTY_FUNC( READ IsVisible WRITE SetVisible VARNAME visible ) bool m_isVisible;
	SGS_PROPERTY_FUNC( READ GetMeshData WRITE SetMeshData VARNAME meshData ) MeshHandle m_mesh;
	SGS_PROPERTY_FUNC( READ GetMeshPath WRITE SetMeshPath ) SGS_ALIAS( StringView mesh );
	SGS_PROPERTY_FUNC( READ GetLightingMode WRITE SetLightingMode VARNAME lightingMode ) int m_lightingMode;
	SGS_PROPERTY_FUNC( READ GetLayers WRITE SetLayers ) SGS_ALIAS( uint32_t layers );
	SGS_PROPERTY_FUNC( READ HasBulletInteraction WRITE SetBulletInteraction ) SGS_ALIAS( bool hasBulletInteraction );
	
	// editor-only static mesh parameters
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME lmQuality ) float m_lmQuality;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME castLMS ) bool m_castLMS;
	SGS_PROPERTY_FUNC( READ WRITE VARNAME overrideMaterials ) bool m_overrideMaterials;
	
	GFW_EXPORT SGS_METHOD void SetShaderConst( int v, Vec4 var );
	
	MeshInstHandle m_meshInst;
};


EXP_STRUCT CharacterResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( CharacterResource, GO_RSRC_CHAR, "Character" );
	
	GFW_EXPORT CharacterResource( GameObject* obj );
	GFW_EXPORT ~CharacterResource();
	GFW_EXPORT virtual void FixedUpdate();
	GFW_EXPORT virtual void Update();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT void _UpdateMatrix();
	GFW_EXPORT void _UpdateLighting();
	GFW_EXPORT void _UpdateChar();
	
	SGRX_MeshInstance* GetMeshInst() const { return m_animChar.m_cachedMeshInst; }
	
	SGS_METHOD bool IsPlayingAnim() const { return m_animChar.IsPlayingAnim(); }
	SGS_METHOD void PlayAnim( StringView name, bool loop ){ m_animChar.PlayAnim( name, loop ); }
	SGS_METHOD void StopAnim(){ m_animChar.StopAnim(); }
	SGS_METHOD StringView GetAnimCharPath() const { return m_animChar.animChar ? m_animChar.animChar->m_key : SV(); }
	SGS_METHOD void SetAnimChar( StringView path ){ m_animChar.SetAnimChar( path ); _UpdateChar(); }
	SGS_METHOD StringView GetSkin() const { return m_animChar.skinName; }
	SGS_METHOD void SetSkin( StringView name ){ m_animChar.SetSkin( name ); _UpdateChar(); }
	SGS_METHOD void SetVar( StringView name, float val ){ m_animChar.SetFloat( name, val ); }
	SGS_METHOD Vec3 GetAttachmentPos( StringView atch, Vec3 off SGS_CPPBC_IGNORE( = V3(0) ) )
	{ return m_animChar.GetAttachmentPos( m_animChar.FindAttachment( atch ), off ); }
	SGS_METHOD Vec3 GetLocalAttachmentPos( StringView atch, Vec3 off SGS_CPPBC_IGNORE( = V3(0) ) )
	{ return m_animChar.GetLocalAttachmentPos( m_animChar.FindAttachment( atch ), off ); }
	SGS_METHOD Mat4 GetAttachmentMatrix( StringView atch )
	{ return m_animChar.GetAttachmentMatrix( m_animChar.FindAttachment( atch ) ); }
	SGS_METHOD bool CheckMarker( StringView name ){ return m_animChar.CheckMarker( name ); }
	SGS_METHOD void EnablePhysics(){ m_animChar.EnablePhysics(); }
	SGS_METHOD void DisablePhysics(){ m_animChar.DisablePhysics(); }
	
	SGS_PROPERTY_FUNC( READ GetAnimCharPath WRITE SetAnimChar ) SGS_ALIAS( StringView charPath );
	SGS_PROPERTY_FUNC( READ GetSkin WRITE SetSkin ) SGS_ALIAS( StringView skin );
	
	AnimCharInst m_animChar;
};


EXP_STRUCT LightResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( LightResource, GO_RSRC_LIGHT, "Light" );
	
	GFW_EXPORT LightResource( GameObject* obj );
	GFW_EXPORT ~LightResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT void _UpdateMatrix();
	GFW_EXPORT void _UpdateLight();
	GFW_EXPORT void _UpdateShadows();
	
	FINLINE void _UpEv(){ Game_FireEvent( EID_GOResourceUpdate, this ); }
	
#define RETNIFNOLIGHT _UpEv(); if( !m_light ) return;
	bool IsStatic() const { return m_isStatic; }
	void SetStatic( bool v ){ m_isStatic = v; _UpEv(); _UpdateLight(); }
	int GetType() const { return m_type; }
	void SetType( int v ){ m_type = v; RETNIFNOLIGHT; m_light->type = v; _UpdateShadows(); }
	bool IsEnabled() const { return m_isEnabled; }
	void SetEnabled( bool v ){ m_isEnabled = v; RETNIFNOLIGHT; m_light->enabled = v; }
	Vec3 GetColor() const { return m_color; }
	void SetColor( Vec3 v ){ m_color = v; RETNIFNOLIGHT; m_light->color = v * m_intensity; }
	float GetIntensity() const { return m_intensity; }
	void SetIntensity( float v ){ m_intensity = v; RETNIFNOLIGHT; m_light->color = m_color * v; }
	float GetRange() const { return m_range; }
	void SetRange( float v ){ m_range = v; RETNIFNOLIGHT; m_light->range = v; m_light->UpdateTransform(); }
	float GetPower() const { return m_power; }
	void SetPower( float v ){ m_power = v; RETNIFNOLIGHT; m_light->power = v; }
	float GetAngle() const { return m_angle; }
	void SetAngle( float v ){ m_angle = v; RETNIFNOLIGHT; m_light->angle = v; m_light->UpdateTransform(); }
	float GetAspect() const { return m_aspect; }
	void SetAspect( float v ){ m_aspect = v; RETNIFNOLIGHT; m_light->aspect = v; m_light->UpdateTransform(); }
	bool HasShadows() const { return m_hasShadows; }
	void SetShadows( bool v ){ m_hasShadows = v; RETNIFNOLIGHT; m_light->hasShadows = v; _UpdateShadows(); }
	TextureHandle GetCookieTextureData() const { return m_cookieTexture; }
	void SetCookieTextureData( TextureHandle h ){ m_cookieTexture = h; RETNIFNOLIGHT; m_light->cookieTexture = h; }
	StringView GetCookieTexturePath() const { return m_cookieTexture ? SV(m_cookieTexture->m_key) : SV(""); }
	void SetCookieTexturePath( StringView path ){ SetCookieTextureData( GR_GetTexture( path ) ); }
	
	FINLINE Vec3 GetFinalColor() const { return m_color * m_intensity * m_isEnabled; }
	
	SGS_PROPERTY_FUNC( READ IsStatic WRITE SetStatic VARNAME isStatic ) bool m_isStatic;
	SGS_PROPERTY_FUNC( READ GetType WRITE SetType VARNAME type ) int m_type;
	SGS_PROPERTY_FUNC( READ IsEnabled WRITE SetEnabled VARNAME enabled ) bool m_isEnabled;
	SGS_PROPERTY_FUNC( READ GetColor WRITE SetColor VARNAME color ) Vec3 m_color;
	SGS_PROPERTY_FUNC( READ GetIntensity WRITE SetIntensity VARNAME intensity ) float m_intensity;
	SGS_PROPERTY_FUNC( READ GetRange WRITE SetRange VARNAME range ) float m_range;
	SGS_PROPERTY_FUNC( READ GetPower WRITE SetPower VARNAME power ) float m_power;
	SGS_PROPERTY_FUNC( READ GetAngle WRITE SetAngle VARNAME angle ) float m_angle;
	SGS_PROPERTY_FUNC( READ GetAspect WRITE SetAspect VARNAME aspect ) float m_aspect;
	SGS_PROPERTY_FUNC( READ HasShadows WRITE SetShadows VARNAME hasShadows ) bool m_hasShadows;
	SGS_PROPERTY_FUNC( READ GetCookieTextureData WRITE SetCookieTextureData VARNAME cookieTextureData ) TextureHandle m_cookieTexture;
	SGS_PROPERTY_FUNC( READ GetCookieTexturePath WRITE SetCookieTexturePath ) SGS_ALIAS( StringView cookieTexture );
	// editor-only static light parameters
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME innerAngle ) float m_innerAngle;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME spotCurve ) float m_spotCurve;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME lightRadius ) float m_lightRadius;
	
	LightHandle m_light;
};


EXP_STRUCT FlareResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( FlareResource, GO_RSRC_FLARE, "Flare" );
	
	GFW_EXPORT FlareResource( GameObject* obj );
	
	SGRX_LensFlare m_flare;
};


EXP_STRUCT ParticleSystemResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( ParticleSystemResource, GO_RSRC_PSYS, "Particle system" );
	
	GFW_EXPORT ParticleSystemResource( GameObject* obj );
	GFW_EXPORT ~ParticleSystemResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void EditorDrawWorld();
	GFW_EXPORT virtual void Update();
	
	GFW_EXPORT void sgsSetParticleSystem( StringView path );
	GFW_EXPORT void sgsSetSoundEvent( StringView name );
	GFW_EXPORT void _StartSoundEvent();
	GFW_EXPORT void sgsSetPlaying( bool playing );
	GFW_EXPORT SGRX_Sound3DAttribs _Get3DAttribs();
	
	SGS_PROPERTY_FUNC( READ WRITE sgsSetParticleSystem VARNAME particleSystemPath ) String m_partSysPath;
	SGS_PROPERTY_FUNC( READ WRITE sgsSetSoundEvent VARNAME soundEvent ) String m_soundEventName;
	SGS_PROPERTY_FUNC( READ WRITE sgsSetPlaying VARNAME enabled ) bool m_enabled;
	
	GFW_EXPORT SGS_METHOD void Trigger();
	
	ParticleSystem m_psys;
	bool m_soundEventOneShot;
	SoundEventInstanceHandle m_soundEventInst;
};


EXP_STRUCT SoundSourceResource : GOResource
{
	static bool IsAvailable( GameLevel* lev ){ return lev->GetSoundSys() != NULL; }
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( SoundSourceResource, GO_RSRC_SNDSRC, "Sound source" );
	
	GFW_EXPORT SoundSourceResource( GameObject* obj );
	GFW_EXPORT ~SoundSourceResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void EditorDrawWorld();
	
	GFW_EXPORT void sgsSetSoundEvent( StringView name );
	GFW_EXPORT void _StartSoundEvent();
	GFW_EXPORT void sgsSetPlaying( bool playing );
	GFW_EXPORT SGRX_Sound3DAttribs _Get3DAttribs();
	
	SGS_PROPERTY_FUNC( READ WRITE sgsSetSoundEvent VARNAME soundEvent ) String m_soundEventName;
	SGS_PROPERTY_FUNC( READ WRITE sgsSetPlaying VARNAME enabled ) bool m_enabled;
	
	GFW_EXPORT SGS_METHOD void Trigger();
	
	bool m_soundEventOneShot;
	SoundEventInstanceHandle m_soundEventInst;
};


#define ShapeType_AABB 0
#define ShapeType_Box 1
#define ShapeType_Sphere 2
#define ShapeType_Cylinder 3
#define ShapeType_Capsule 4
#define ShapeType_Mesh 5

EXP_STRUCT RigidBodyResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( RigidBodyResource, GO_RSRC_RBODY, "Rigid body" );
	
	GFW_EXPORT RigidBodyResource( GameObject* obj );
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual Mat4 GetWorldMatrix() const;
	GFW_EXPORT virtual void PrePhysicsFixedUpdate();
	GFW_EXPORT void _UpdateShape();
	
	FINLINE Vec3 GetLinearVelocity() const { return m_body->GetLinearVelocity(); }
	FINLINE void SetLinearVelocity( Vec3 v ){ m_body->SetLinearVelocity( v ); }
	FINLINE Vec3 GetAngularVelocity() const { return m_body->GetAngularVelocity(); }
	FINLINE void SetAngularVelocity( Vec3 v ){ m_body->SetAngularVelocity( v ); }
	FINLINE float GetFriction() const { return m_body->GetFriction(); }
	FINLINE void SetFriction( float v ){ m_body->SetFriction( v ); }
	FINLINE float GetRestitution() const { return m_body->GetRestitution(); }
	FINLINE void SetRestitution( float v ){ m_body->SetRestitution( v ); }
	FINLINE float GetMass() const { return m_body->GetMass(); }
	FINLINE void SetMass( float v ){ m_body->SetMassAndInertia( v, m_body->GetInertia() ); }
	FINLINE Vec3 GetInertia() const { return m_body->GetInertia(); }
	FINLINE void SetInertia( Vec3 v ){ m_body->SetMassAndInertia( m_body->GetMass(), v ); }
	FINLINE float GetLinearDamping() const { return m_body->GetLinearDamping(); }
	FINLINE void SetLinearDamping( float v ){ m_body->SetLinearDamping( v ); }
	FINLINE float GetAngularDamping() const { return m_body->GetAngularDamping(); }
	FINLINE void SetAngularDamping( float v ){ m_body->SetAngularDamping( v ); }
	FINLINE bool IsKinematic() const { return m_body->IsKinematic(); }
	FINLINE void SetKinematic( bool v ){ m_body->SetKinematic( v ); }
	FINLINE bool CanSleep() const { return m_body->CanSleep(); }
	FINLINE void SetCanSleep( bool v ){ m_body->SetCanSleep( v ); }
	FINLINE bool IsEnabled() const { return m_body->GetEnabled(); }
	FINLINE void SetEnabled( bool v ){ m_body->SetEnabled( v ); }
	FINLINE uint16_t GetGroup() const { return m_body->GetGroup(); }
	FINLINE void SetGroup( uint16_t v ){ m_body->SetGroupAndMask( v, GetMask() ); }
	FINLINE uint16_t GetMask() const { return m_body->GetMask(); }
	FINLINE void SetMask( uint16_t v ){ m_body->SetGroupAndMask( GetGroup(), v ); }
	
	void SetShapeType( int type ){ if( type == shapeType ) return; shapeType = type; _UpdateShape(); }
	void SetShapeRadius( float radius ){ if( radius == shapeRadius ) return; shapeRadius = radius;
		if( shapeType == ShapeType_Sphere || shapeType == ShapeType_Capsule ){ _UpdateShape(); } }
	void SetShapeHeight( float height ){ if( height == shapeHeight ) return; shapeHeight = height;
		if( shapeType == ShapeType_Capsule ){ _UpdateShape(); } }
	void SetShapeExtents( Vec3 extents ){ if( extents == shapeExtents ) return; shapeExtents = extents;
		if( shapeType == ShapeType_Box || shapeType == ShapeType_Cylinder || shapeType == ShapeType_AABB ) _UpdateShape(); }
	void SetShapeMinExtents( Vec3 minExtents ){ if( minExtents == shapeMinExtents ) return; shapeMinExtents = minExtents;
		if( shapeType == ShapeType_AABB ){ _UpdateShape(); } }
	void SetShapeMesh( MeshHandle mesh ){ if( mesh == shapeMesh ) return; shapeMesh = mesh;
		if( shapeType == ShapeType_Mesh ){ _UpdateShape(); } }
	StringView GetShapeMeshPath() const { return shapeMesh ? SV(shapeMesh->m_key) : SV(); }
	void SetShapeMeshPath( StringView path ){ SetShapeMesh( GR_GetMesh( path ) ); }
	
	SGS_PROPERTY_FUNC( READ GetLinearVelocity WRITE SetLinearVelocity ) SGS_ALIAS( Vec3 linearVelocity );
	SGS_PROPERTY_FUNC( READ GetAngularVelocity WRITE SetAngularVelocity ) SGS_ALIAS( Vec3 angularVelocity );
	SGS_PROPERTY_FUNC( READ GetFriction WRITE SetFriction ) SGS_ALIAS( float friction );
	SGS_PROPERTY_FUNC( READ GetRestitution WRITE SetRestitution ) SGS_ALIAS( float restitution );
	SGS_PROPERTY_FUNC( READ GetMass WRITE SetMass ) SGS_ALIAS( float mass );
	SGS_PROPERTY_FUNC( READ GetInertia WRITE SetInertia ) SGS_ALIAS( Vec3 inertia );
	SGS_PROPERTY_FUNC( READ GetLinearDamping WRITE SetLinearDamping ) SGS_ALIAS( float linearDamping );
	SGS_PROPERTY_FUNC( READ GetAngularDamping WRITE SetAngularDamping ) SGS_ALIAS( float angularDamping );
	SGS_PROPERTY_FUNC( READ IsKinematic WRITE SetKinematic ) SGS_ALIAS( float kinematic );
	SGS_PROPERTY_FUNC( READ CanSleep WRITE SetCanSleep ) SGS_ALIAS( float canSleep );
	SGS_PROPERTY_FUNC( READ IsEnabled WRITE SetEnabled ) SGS_ALIAS( float enabled );
	SGS_PROPERTY_FUNC( READ GetGroup WRITE SetGroup ) SGS_ALIAS( float group );
	SGS_PROPERTY_FUNC( READ GetMask WRITE SetMask ) SGS_ALIAS( float mask );
	
	SGS_PROPERTY_FUNC( READ WRITE SetShapeType ) int shapeType;
	SGS_PROPERTY_FUNC( READ WRITE SetShapeRadius ) float shapeRadius;
	SGS_PROPERTY_FUNC( READ WRITE SetShapeHeight ) float shapeHeight;
	SGS_PROPERTY_FUNC( READ WRITE SetShapeExtents ) Vec3 shapeExtents;
	SGS_PROPERTY_FUNC( READ WRITE SetShapeMinExtents ) Vec3 shapeMinExtents;
	SGS_PROPERTY_FUNC( READ WRITE SetShapeMesh ) MeshHandle shapeMesh;
	SGS_PROPERTY_FUNC( READ GetShapeMeshPath WRITE SetShapeMeshPath ) SGS_ALIAS( StringView shapeMeshPath );
	
	SGS_METHOD void WakeUp(){ m_body->WakeUp(); }
	
	PhyRigidBodyHandle m_body;
	PhyShapeHandle m_shape;
	Vec3 m_prevPos;
	Quat m_prevRot;
};


EXP_STRUCT ReflectionPlaneResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( ReflectionPlaneResource, GO_RSRC_REFPLANE, "Reflection plane" );
	
	GFW_EXPORT ReflectionPlaneResource( GameObject* obj );
};


EXP_STRUCT CameraResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_RESOURCE( CameraResource, GO_RSRC_CAMERA, "Camera" );
	
	GFW_EXPORT CameraResource( GameObject* obj );
	
	GFW_EXPORT void GetCamera( SGRX_Camera& out );
	
	GFW_EXPORT SGS_METHOD_NAMED( WorldToScreen ) SGS_MULTRET sgsWorldToScreen( Vec3 pos );
	GFW_EXPORT SGS_METHOD_NAMED( WorldToScreenPx ) SGS_MULTRET sgsWorldToScreenPx( Vec3 pos );
	GFW_EXPORT bool GetCursorWorldPoint( Vec3* isp, uint32_t layers = 0xffffffff, Vec2 cpn = Game_GetCursorPosNormalized() );
	GFW_EXPORT SGS_METHOD_NAMED( GetCursorWorldPoint ) SGS_MULTRET sgsGetCursorWorldPoint( uint32_t layers /* = 0xffffffff */ );
	GFW_EXPORT SGS_METHOD_NAMED( GetCursorMeshInst ) SGS_MULTRET sgsGetCursorMeshInst( uint32_t layers /* = 0xffffffff */ );
	
	SGS_PROPERTY int depth;
	SGS_PROPERTY float fieldOfView;
	SGS_PROPERTY float aspectMix;
	SGS_PROPERTY float nearPlane;
	SGS_PROPERTY float farPlane;
	SGS_PROPERTY bool enabled;
};



EXP_STRUCT BhResourceMoveObject : GOBehavior
{
	SGS_OBJECT_INHERIT( GOBehavior );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( BhResourceMoveObject );
	
	GFW_EXPORT BhResourceMoveObject( GameObject* obj );
	GFW_EXPORT void FixedUpdate();
	GFW_EXPORT void Update();
	
	SGS_PROPERTY GOResource::ScrHandle resource;
	SGS_PROPERTY uint8_t mask;
};


EXP_STRUCT BhResourceMoveResource : GOBehavior
{
	SGS_OBJECT_INHERIT( GOBehavior );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( BhResourceMoveResource );
	
	GFW_EXPORT BhResourceMoveResource( GameObject* obj );
	GFW_EXPORT void FixedUpdate();
	GFW_EXPORT void Update();
	
	SGS_PROPERTY GOResource::ScrHandle resource;
	SGS_PROPERTY GOResource::ScrHandle follow;
	SGS_PROPERTY uint8_t mask;
};


EXP_STRUCT IController
{
	virtual Vec3 GetInput( uint32_t iid ){ return V3(0); }
	virtual void Reset(){}
	
	FINLINE Vec3 GetInputV3( uint32_t iid ){ return GetInput( iid ); }
	FINLINE Vec2 GetInputV2( uint32_t iid ){ return GetInput( iid ).ToVec2(); }
	FINLINE float GetInputF( uint32_t iid ){ return GetInput( iid ).x; }
	FINLINE bool GetInputB( uint32_t iid ){ return GetInput( iid ).x > 0.5f; }
	FINLINE bool GetInputB1( uint32_t iid ){ return GetInput( iid ).x > 0.5f; }
	FINLINE bool GetInputB2( uint32_t iid ){ return GetInput( iid ).y > 0.5f; }
	FINLINE bool GetInputB3( uint32_t iid ){ return GetInput( iid ).z > 0.5f; }
};

EXP_STRUCT BhControllerBase : GOBehavior, IController
{
	SGS_OBJECT_INHERIT( GOBehavior );
	ENT_SGS_IMPLEMENT;
	IMPLEMENT_BEHAVIOR( BhControllerBase );
	
	GFW_EXPORT BhControllerBase( GameObject* obj );
	GFW_EXPORT virtual SGS_METHOD Vec3 GetInput( uint32_t iid );
	GFW_EXPORT virtual SGS_METHOD void Reset();
	
	SGS_METHOD SGS_ALIAS( Vec3 GetInputV3( uint32_t iid ) );
	SGS_METHOD SGS_ALIAS( Vec2 GetInputV2( uint32_t iid ) );
	SGS_METHOD SGS_ALIAS( float GetInputF( uint32_t iid ) );
	SGS_METHOD SGS_ALIAS( bool GetInputB( uint32_t iid ) );
	
	SGS_PROPERTY bool enabled;
};

GFW_EXPORT IController* GetObjectController( GameObject* obj, bool def = true );




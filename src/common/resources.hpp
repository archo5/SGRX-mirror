

#pragma once
#include <engine.hpp>
#include <enganim.hpp>
#include <pathfinding.hpp>
#include <sound.hpp>
#include <physics.hpp>
#include <script.hpp>

#include "level.hpp"
#include "systems.hpp"



#define GO_RSRC_MESH 1
#define GO_RSRC_LIGHT 2

enum MatrixMode
{
	MM_None = 0,
	MM_Relative = 1,
	MM_Absolute = 2,
};

EXP_STRUCT MeshResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	
	GFW_EXPORT MeshResource( GameObject* obj );
	GFW_EXPORT ~MeshResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT virtual void EditorDrawWorld();
	GFW_EXPORT void _UpdateLighting();
	GFW_EXPORT void _UpdateMatrix();
	
	FINLINE void _UpEv(){ Game_FireEvent( EID_GOResourceUpdate, this ); }
	
	bool IsVisible() const { return m_isVisible; }
	void SetVisible( bool v ){ m_isVisible = v; m_meshInst->enabled = v; _UpEv(); }
	MeshHandle GetMeshData() const { return m_mesh; }
	GFW_EXPORT void SetMeshData( MeshHandle mesh );
	StringView GetMeshPath() const { return m_mesh ? SV(m_mesh->m_key) : SV(); }
	void SetMeshPath( StringView path ){ SetMeshData( GR_GetMesh( path ) ); }
	Mat4 GetMatrix() const { return m_localMatrix; }
	void SetMatrix( Mat4 m ){ m_localMatrix = m; if( m_matrixMode != MM_None ) _UpdateMatrix(); }
	int GetMatrixMode() const { return m_matrixMode; }
	void SetMatrixMode( int v ){ m_matrixMode = v; _UpdateMatrix(); }
	int GetLightingMode() const { return m_lightingMode; }
	void SetLightingMode( int v ){ m_lightingMode = v;
		m_meshInst->SetLightingMode( (SGRX_LightingMode) v ); _UpdateLighting(); }
	
	SGS_PROPERTY_FUNC( READ SOURCE m_meshInst.item ) SGS_ALIAS( void* meshInst );
	SGS_PROPERTY_FUNC( READ WRITE VARNAME isStatic ) bool m_isStatic;
	SGS_PROPERTY_FUNC( READ IsVisible WRITE SetVisible VARNAME visible ) bool m_isVisible;
	SGS_PROPERTY_FUNC( READ GetMeshData WRITE SetMeshData VARNAME meshData ) MeshHandle m_mesh;
	SGS_PROPERTY_FUNC( READ GetMeshPath WRITE SetMeshPath ) SGS_ALIAS( StringView mesh );
	SGS_PROPERTY_FUNC( READ GetMatrix WRITE SetMatrix VARNAME matrix ) Mat4 m_localMatrix;
	SGS_PROPERTY_FUNC( READ GetMatrixMode WRITE SetMatrixMode VARNAME matrixMode ) int m_matrixMode;
	SGS_PROPERTY_FUNC( READ GetLightingMode WRITE SetLightingMode VARNAME lightingMode ) int m_lightingMode;
	// editor-only static mesh parameters
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME lmQuality ) float m_lmQuality;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME castLMS ) bool m_castLMS;
	
	GFW_EXPORT SGS_METHOD void SetShaderConst( int v, Vec4 var );
	
	MeshInstHandle m_meshInst;
};


EXP_STRUCT LightResource : GOResource
{
	SGS_OBJECT_INHERIT( GOResource );
	ENT_SGS_IMPLEMENT;
	
	GFW_EXPORT LightResource( GameObject* obj );
	GFW_EXPORT ~LightResource();
	GFW_EXPORT virtual void OnTransformUpdate();
	GFW_EXPORT void _UpdateMatrix();
	GFW_EXPORT Mat4 _GetFullMatrix();
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
	Mat4 GetMatrix() const { return m_localMatrix; }
	void SetMatrix( Mat4 m ){ m_localMatrix = m; RETNIFNOLIGHT; if( m_matrixMode != MM_None ) _UpdateMatrix(); }
	int GetMatrixMode() const { return m_matrixMode; }
	void SetMatrixMode( int v ){ m_matrixMode = v; RETNIFNOLIGHT; _UpdateMatrix(); }
	TextureHandle GetCookieTextureData() const { return m_cookieTexture; }
	void SetCookieTextureData( TextureHandle h ){ m_cookieTexture = h; RETNIFNOLIGHT; m_light->cookieTexture = h; }
	StringView GetCookieTexturePath() const { return m_cookieTexture ? m_cookieTexture->m_key : ""; }
	void SetCookieTexturePath( StringView path ){ SetCookieTextureData( GR_GetTexture( path ) ); }
	
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
	SGS_PROPERTY_FUNC( READ GetMatrix WRITE SetMatrix VARNAME matrix ) Mat4 m_localMatrix;
	SGS_PROPERTY_FUNC( READ GetMatrixMode WRITE SetMatrixMode VARNAME matrixMode ) int m_matrixMode;
	// editor-only static light parameters
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME innerAngle ) float m_innerAngle;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME spotCurve ) float m_spotCurve;
	SGS_PROPERTY_FUNC( READ WRITE WRITE_CALLBACK _UpEv VARNAME lightRadius ) float m_lightRadius;
	
	LightHandle m_light;
};




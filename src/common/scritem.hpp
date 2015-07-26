

#pragma once
#include <enganim.hpp>
#include <engext.hpp>
#include <physics.hpp>
#include <script.hpp>
#include <sound.hpp>



enum MeshInstEvent
{
	MIEVT_BulletHit = 1,
};

struct SGRX_MeshInstUserData
{
	SGRX_MeshInstUserData() : dmgDecalSysOverride(NULL), ownerType(0){}
	virtual ~SGRX_MeshInstUserData(){}
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, float amt ){}
	SGRX_DecalSystem* dmgDecalSysOverride;
	uint32_t ownerType;
};



#define SCRITEM_NUM_SLOTS 4
#define SCRITEM_RANGE_STR "[0-3]"

struct SGRX_ScriptedItem : SGRX_MeshInstUserData
{
	typedef sgsHandle< SGRX_ScriptedItem > Handle;
	
	SGS_OBJECT;
	
	static SGRX_ScriptedItem* Create( SGRX_Scene* scene, SGS_CTX, sgsVariable func );
	void Acquire();
	void Release();
	
	void SetLightSampler( SGRX_LightSampler* sampler );
	void Tick( float dt );
	void PreRender();
	
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, float amt );
	
	// ---
	SGS_IFUNC( GETINDEX ) int _getindex(
		SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, int isprop );
	SGS_IFUNC( SETINDEX ) int _setindex(
		SGS_CTX, sgs_VarObj* obj, sgs_Variable* key, sgs_Variable* val, int isprop );
	
	// - common
	SGS_METHOD void SetMatrix( Mat4 mtx );
	
	// - mesh instance
	SGS_METHOD void MICreate( int i, StringView path );
	SGS_METHOD void MIDestroy( int i );
	SGS_METHOD void MISetMesh( int i, StringView path );
	SGS_METHOD void MISetEnabled( int i, bool enabled );
	SGS_METHOD void MISetMatrix( int i, Mat4 mtx );
	
	// - particle system
	SGS_METHOD void PSCreate( int i, StringView path );
	SGS_METHOD void PSDestroy( int i );
	SGS_METHOD void PSLoad( int i, StringView path );
	SGS_METHOD void PSSetMatrix( int i, Mat4 mtx );
	SGS_METHOD void PSPlay( int i );
	SGS_METHOD void PSStop( int i );
	SGS_METHOD void PSTrigger( int i );
	
	// - decal system
	SGS_METHOD void DSCreate( StringView texDecalPath, StringView texFalloffPath, uint32_t size );
	SGS_METHOD void DSDestroy();
	SGS_METHOD void DSResize( uint32_t size );
	SGS_METHOD void DSClear();
	// ---
	
	sgsVariable m_variable;
	
	SceneHandle m_scene;
	SGRX_LightSampler* m_lightSampler;
	
	DecalSysHandle m_decalSys;
	MeshInstHandle m_decalSysMI;
	MeshInstHandle m_meshes[ SCRITEM_NUM_SLOTS ];
	PartSysHandle m_partSys[ SCRITEM_NUM_SLOTS ];
//	LightHandle m_lights[ SCRITEM_NUM_SLOTS ];
	
	Mat4 m_transform;
	Mat4 m_meshMatrices[ SCRITEM_NUM_SLOTS ];
	Mat4 m_partSysMatrices[ SCRITEM_NUM_SLOTS ];
//	Mat4 m_lightMatrices[ SCRITEM_NUM_SLOTS ];
};




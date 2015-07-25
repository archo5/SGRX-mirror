

#pragma once
#include <enganim.hpp>
#include <physics.hpp>
#include <script.hpp>
#include <sound.hpp>



enum MeshInstEvent
{
	MIEVT_BulletHit = 1,
};

struct SGRX_MeshInstUserData
{
	SGRX_MeshInstUserData() : ownerType(0){}
	virtual ~SGRX_MeshInstUserData(){}
	virtual void OnEvent( SGRX_MeshInstance* MI, uint32_t evid, float amt ){}
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
	SGS_METHOD void SetMatrix( Mat4 mtx );
	
	SGS_METHOD void CreateMeshInst( int i, StringView path );
	SGS_METHOD void DestroyMeshInst( int i );
	SGS_METHOD void SetMesh( int i, StringView path );
	SGS_METHOD void SetMeshInstMatrix( int i, Mat4 mtx );
	
	SGS_METHOD void CreatePartSys( int i, StringView path );
	SGS_METHOD void DestroyPartSys( int i );
	SGS_METHOD void LoadPartSys( int i, StringView path );
	SGS_METHOD void SetPartSysMatrix( int i, Mat4 mtx );
	SGS_METHOD void PartSysPlay( int i );
	SGS_METHOD void PartSysStop( int i );
	SGS_METHOD void PartSysTrigger( int i );
	// ---
	
	sgsVariable m_variable;
	
	SceneHandle m_scene;
	SGRX_LightSampler* m_lightSampler;
	
	MeshInstHandle m_meshes[ SCRITEM_NUM_SLOTS ];
	PartSysHandle m_partSys[ SCRITEM_NUM_SLOTS ];
//	LightHandle m_lights[ SCRITEM_NUM_SLOTS ];
	
	Mat4 m_transform;
	Mat4 m_meshMatrices[ SCRITEM_NUM_SLOTS ];
	Mat4 m_partSysMatrices[ SCRITEM_NUM_SLOTS ];
//	Mat4 m_lightMatrices[ SCRITEM_NUM_SLOTS ];
};




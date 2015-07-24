

#pragma once
#include <enganim.hpp>
#include <physics.hpp>
#include <script.hpp>
#include <sound.hpp>



struct SGRX_ScriptedItem
{
	SGS_OBJECT;
	
	static SGRX_ScriptedItem* Create( SGS_CTX, sgsVariable func );
	void Acquire();
	void Release();
	
	sgsVariable m_variable;
	MeshInstHandle m_meshes[ 4 ];
	LightHandle m_lights[ 4 ];
	ParticleSystem m_particleSystems[ 4 ];
	PhyRigidBodyHandle m_bodies[ 4 ];
};




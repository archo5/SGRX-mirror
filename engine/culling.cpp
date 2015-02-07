

#define USE_VEC2
#define USE_VEC3
#define USE_VEC4
#define USE_MAT4
#define USE_ARRAY
#define USE_HASHTABLE
#include "renderer.hpp"



/// CULLING RUNNER ///

void SGRX_PerspectiveAngles( float angle, float aspect, float aamix, float* outh, float* outv )
{
	float tha = tan( DEG2RAD( angle ) / 2.0f );
	if( tha < 0.001f ) tha = 0.001f;
	float itha = 1.0f / tha;
	float xscale = itha / pow( aspect, aamix );
	float yscale = itha * pow( aspect, 1 - aamix );
	if( outh ) *outh = atan( xscale );
	if( outv ) *outv = atan( yscale );
}

static void get_frustum_from_camera( const SGRX_Camera* CAM, SGRX_CullSceneCamera* out )
{
	out->frustum.position = CAM->position;
	out->frustum.direction = CAM->direction;
	out->frustum.up = CAM->up;
	SGRX_PerspectiveAngles( CAM->angle, CAM->aspect, CAM->aamix, &out->frustum.hangle, &out->frustum.vangle );
	out->frustum.znear = CAM->znear;
	out->frustum.zfar = CAM->zfar;
	out->viewProjMatrix.Multiply( CAM->mView, CAM->mProj );
	out->invViewProjMatrix.SetIdentity();
	out->viewProjMatrix.InvertTo( out->invViewProjMatrix );
}

static void get_frustum_from_light( const SGRX_Light* L, SGRX_CullSceneCamera* out )
{
	out->frustum.position = L->position;
	out->frustum.direction = L->direction;
	out->frustum.up = L->updir;
	SGRX_PerspectiveAngles( L->angle, L->aspect, 0.5, &out->frustum.hangle, &out->frustum.vangle );
	out->frustum.znear = L->range * 0.001f;
	out->frustum.zfar = L->range;
	out->viewProjMatrix = L->viewProjMatrix;
	out->invViewProjMatrix.SetIdentity();
	out->viewProjMatrix.InvertTo( out->invViewProjMatrix );
}

static uint32_t _SGRX_Cull_Frustum_MeshList( SGRX_CullSceneCamera* frustum, bool spotlight, Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S )
{
	SGRX_CullScene* CS = S->cullScene;
	
	if( !S->m_meshInstances.size() )
		return 0;
	
	size_t mbsize = MIBuf.size();
	MIBuf.resize( MIBuf.size() + S->m_meshInstances.size() );
	SGRX_MeshInstance** mesh_instances = &MIBuf[ mbsize ];
	
	size_t scratch_block_1 = sizeof( SGRX_CullSceneMesh ) * S->m_meshInstances.size();
	size_t scratch_block_2 = sizeof( uint32_t ) * divideup( S->m_meshInstances.size(), 32 );
	scratchMem.clear();
	scratchMem.resize( scratch_block_1 + scratch_block_2 );
	SGRX_CullSceneMesh* mesh_bounds = (SGRX_CullSceneMesh*) &scratchMem[ 0 ];
	uint32_t* mesh_visiblity = (uint32_t*) &scratchMem[ scratch_block_1 ];
	
	/* fill in instance data */
	uint32_t data_size = 0;
	for( size_t inst_id = 0; inst_id < S->m_meshInstances.size(); ++inst_id )
	{
		SGRX_MeshInstance* MI = (SGRX_MeshInstance*) S->m_meshInstances.item( inst_id ).key;
		if( !MI->mesh || !MI->enabled )
			continue;
		SGRX_IMesh* M = MI->mesh;
		
		mesh_instances[ data_size ] = MI;
		mesh_bounds[ data_size ].transform = MI->matrix;
		mesh_bounds[ data_size ].min = M->m_boundsMin;
		mesh_bounds[ data_size ].max = M->m_boundsMax;
		data_size++;
	}
	
	if( CS && ( spotlight ?
			CS->Camera_MeshList( data_size, frustum, mesh_bounds, mesh_visiblity ) :
			CS->SpotLight_MeshList( data_size, frustum, mesh_bounds, mesh_visiblity ) ) )
	{
		uint32_t outpos = 0;
		for( uint32_t i = 0; i < data_size; ++i )
		{
			if( mesh_visiblity[ i / 32 ] & ( 1 << ( i % 32 ) ) )
			{
				if( i > outpos )
				{
					mesh_instances[ outpos ] = mesh_instances[ i ];
					memcpy( &mesh_bounds[ outpos ], &mesh_bounds[ i ], sizeof( mesh_bounds[0] ) );
				}
				outpos++;
			}
		}
		data_size = outpos;
	}
	
	MIBuf.resize( mbsize + data_size );
	return data_size;
}

void SGRX_Cull_Camera_Prepare( SGRX_Scene* S )
{
	SGRX_Camera* CAM = &S->camera;
	SGRX_CullScene* CS = S->cullScene;
	if( !CS )
		return;
	
	SGRX_CullSceneCamera camera_frustum;
	get_frustum_from_camera( CAM, &camera_frustum );
	
	CS->Camera_Prepare( &camera_frustum );
}

uint32_t SGRX_Cull_Camera_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S )
{
	SGRX_Camera* CAM = &S->camera;
	
	SGRX_CullSceneCamera camera_frustum;
	get_frustum_from_camera( CAM, &camera_frustum );
	
	return _SGRX_Cull_Frustum_MeshList( &camera_frustum, false, MIBuf, scratchMem, S );
}

uint32_t SGRX_Cull_Camera_PointLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S )
{
	SGRX_Camera* CAM = &S->camera;
	SGRX_CullScene* CS = S->cullScene;
	
	if( !S->m_lights.size() )
		return 0;
	
	SGRX_CullSceneCamera camera_frustum;
	get_frustum_from_camera( CAM, &camera_frustum );
	
	size_t mbsize = LIBuf.size();
	LIBuf.resize( LIBuf.size() + S->m_lights.size() );
	SGRX_Light** light_instances = &LIBuf[ mbsize ];
	
	size_t scratch_block_1 = sizeof( SGRX_CullScenePointLight ) * S->m_lights.size();
	size_t scratch_block_2 = sizeof( uint32_t ) * divideup( S->m_lights.size(), 32 );
	scratchMem.clear();
	scratchMem.resize( scratch_block_1 + scratch_block_2 );
	SGRX_CullScenePointLight* light_bounds = (SGRX_CullScenePointLight*) &scratchMem[ 0 ];
	uint32_t* light_visiblity = (uint32_t*) &scratchMem[ scratch_block_1 ];
	
	/* fill in light data */
	uint32_t data_size = 0;
	for( size_t light_id = 0; light_id < S->m_lights.size(); ++light_id )
	{
		SGRX_Light* L = (SGRX_Light*) S->m_lights.item( light_id ).key;
		if( !L->enabled || L->type != LIGHT_POINT )
			continue;
		
		light_instances[ data_size ] = L;
		light_bounds[ data_size ].position = L->position;
		light_bounds[ data_size ].radius = L->range;
		data_size++;
	}
	
	if( CS && CS->Camera_PointLightList( data_size, &camera_frustum, light_bounds, light_visiblity ) )
	{
		uint32_t outpos = 0;
		for( uint32_t i = 0; i < data_size; ++i )
		{
			if( light_visiblity[ i / 32 ] & ( 1 << ( i % 32 ) ) )
			{
				if( i > outpos )
				{
					light_instances[ outpos ] = light_instances[ i ];
					memcpy( &light_bounds[ outpos ], &light_bounds[ i ], sizeof( light_bounds[0] ) );
				}
				outpos++;
			}
		}
		data_size = outpos;
	}
	
	LIBuf.resize( mbsize + data_size );
	return data_size;
}

uint32_t SGRX_Cull_Camera_SpotLightList( Array< SGRX_Light* >& LIBuf, ByteArray& scratchMem, SGRX_Scene* S )
{
	SGRX_Camera* CAM = &S->camera;
	SGRX_CullScene* CS = S->cullScene;
	
	if( !S->m_lights.size() )
		return 0;
	
	SGRX_CullSceneCamera camera_frustum;
	get_frustum_from_camera( CAM, &camera_frustum );
	
	size_t mbsize = LIBuf.size();
	LIBuf.resize( LIBuf.size() + S->m_lights.size() );
	SGRX_Light** light_instances = &LIBuf[ mbsize ];
	
	size_t scratch_block_1 = sizeof( SGRX_CullSceneFrustum ) * S->m_lights.size();
	size_t scratch_block_2 = sizeof( Mat4 ) * S->m_lights.size();
	size_t scratch_block_3 = sizeof( uint32_t ) * divideup( S->m_lights.size(), 32 );
	scratchMem.clear();
	scratchMem.resize( scratch_block_1 + scratch_block_2 + scratch_block_3 );
	SGRX_CullSceneFrustum* light_bounds = (SGRX_CullSceneFrustum*) &scratchMem[ 0 ];
	Mat4* light_matrices = (Mat4*) &scratchMem[ scratch_block_1 ];
	uint32_t* light_visiblity = (uint32_t*) &scratchMem[ scratch_block_1 + scratch_block_2 ];
	
	/* fill in light data */
	uint32_t data_size = 0;
	for( size_t light_id = 0; light_id < S->m_lights.size(); ++light_id )
	{
		SGRX_CullSceneCamera light_frustum;
		SGRX_Light* L = (SGRX_Light*) S->m_lights.item( light_id ).key;
		if( !L->enabled || L->type != LIGHT_SPOT )
			continue;
		
		get_frustum_from_light( L, &light_frustum );
		light_instances[ data_size ] = L;
		light_bounds[ data_size ] = light_frustum.frustum;
		light_matrices[ data_size ] = light_frustum.invViewProjMatrix;
		data_size++;
	}
	
	if( CS && CS->Camera_SpotLightList( data_size, &camera_frustum, light_bounds, light_matrices, light_visiblity ) )
	{
		uint32_t outpos = 0;
		for( uint32_t i = 0; i < data_size; ++i )
		{
			if( light_visiblity[ i / 32 ] & ( 1 << ( i % 32 ) ) )
			{
				if( i > outpos )
				{
					light_instances[ outpos ] = light_instances[ i ];
					memcpy( &light_bounds[ outpos ], &light_bounds[ i ], sizeof( light_bounds[0] ) );
					memcpy( &light_matrices[ outpos ], &light_matrices[ i ], sizeof( light_matrices[0] ) );
				}
				outpos++;
			}
		}
		data_size = outpos;
	}
	
	LIBuf.resize( mbsize + data_size );
	return data_size;
}

void SGRX_Cull_SpotLight_Prepare( SGRX_Scene* S, SGRX_Light* L )
{
	SGRX_CullScene* CS = S->cullScene;
	if( !CS )
		return;
	
	SGRX_CullSceneCamera light_frustum;
	get_frustum_from_light( L, &light_frustum );
	
	CS->SpotLight_Prepare( &light_frustum );
}

uint32_t SGRX_Cull_SpotLight_MeshList( Array< SGRX_MeshInstance* >& MIBuf, ByteArray& scratchMem, SGRX_Scene* S, SGRX_Light* L )
{
	SGRX_CullSceneCamera light_frustum;
	get_frustum_from_light( L, &light_frustum );
	
	return _SGRX_Cull_Frustum_MeshList( &light_frustum, true, MIBuf, scratchMem, S );
}




//
// DEFAULT CULLING FUNCTIONS

#define FRUSTUM_PLANE_NEAR   0
#define FRUSTUM_PLANE_FAR    1
#define FRUSTUM_PLANE_LEFT   2
#define FRUSTUM_PLANE_RIGHT  3
#define FRUSTUM_PLANE_TOP    4
#define FRUSTUM_PLANE_BOTTOM 5

static void SGRX_MakePlaneFromPoints( Vec4& plane, Vec3 p0, Vec3 p1, Vec3 p2 )
{
	Vec3 p0p1 = p1 - p0;
	Vec3 p0p2 = p2 - p0;
	Vec3 nrm = Vec3Cross( p0p1, p0p2 ).Normalized();
	plane = Vec4::Create( nrm.x, nrm.y, nrm.z, Vec3Dot( nrm, p0 ) );
}

static void SGRX_MakePlanesFromBoxPoints( Vec4 planes[6], Vec3 points[8] )
{
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_NEAR   ], points[0], points[1], points[2] );
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_FAR    ], points[7], points[6], points[5] );
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_LEFT   ], points[0], points[3], points[4] );
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_RIGHT  ], points[1], points[5], points[2] );
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_TOP    ], points[2], points[6], points[3] );
	SGRX_MakePlaneFromPoints( planes[ FRUSTUM_PLANE_BOTTOM ], points[0], points[4], points[1] );
}

static void SGRX_GetFrustumPlanes( Vec4 frustum[6], Mat4 mIVP )
{
	static Vec3 psp[8] =
	{
		{  1,  1, 0 },
		{ -1,  1, 0 },
		{ -1, -1, 0 },
		{  1, -1, 0 },
		{  1,  1, 1 },
		{ -1,  1, 1 },
		{ -1, -1, 1 },
		{  1, -1, 1 },
	};
	
	Vec3 wsp[8];
	for( int i = 0; i < 8; ++i )
		wsp[i] = mIVP.TransformPos( psp[i] );
	
	SGRX_MakePlanesFromBoxPoints( frustum, wsp );
}

static void SGRX_GetFrustumAABB( Vec3 aabb[2], Mat4 mIVP )
{
	static Vec3 psp[8] =
	{
		{  1,  1, 0 },
		{ -1,  1, 0 },
		{ -1, -1, 0 },
		{  1, -1, 0 },
		{  1,  1, 1 },
		{ -1,  1, 1 },
		{ -1, -1, 1 },
		{  1, -1, 1 },
	};
	
	Vec3 tmp = mIVP.TransformPos( psp[0] );
	aabb[0] = tmp;
	aabb[1] = tmp;
	for( int i = 1; i < 8; ++i )
	{
		tmp = mIVP.TransformPos( psp[i] );
		aabb[0] = Vec3::Min( aabb[0], tmp );
		aabb[1] = Vec3::Max( aabb[1], tmp );
	}
}

static void SGRX_GetTfAABB( Vec3 aabb[2], Mat4 mtx, Vec3 min, Vec3 max )
{
	Vec3 wsp[8] =
	{
		{ min.x, min.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, min.z },
		{ min.x, max.y, min.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
	};
	
	aabb[1] = aabb[0] = mtx.TransformPos( wsp[0] );
	for( int i = 1; i < 8; ++i )
	{
		Vec3 tmp = mtx.TransformPos( wsp[i] );
		aabb[0] = Vec3::Min( aabb[0], tmp );
		aabb[1] = Vec3::Max( aabb[1], tmp );
	}
}

static int SGRX_FrustumAABBIntersection( Vec4 planes[6], Vec3 min, Vec3 max )
{
	for( int i = 0; i < 6; ++i )
	{
		Vec3 vcmp =
		{
			planes[i].x > 0 ? min.x : max.x,
			planes[i].y > 0 ? min.y : max.y,
			planes[i].z > 0 ? min.z : max.z,
		};
		if( Vec3Dot( vcmp, planes[i].ToVec3() ) > planes[i].w )
			return 0;
	}
	return 1;
}

static int SGRX_FrustumSphereIntersection( Vec4 planes[6], Vec3 pos, float radius )
{
	for( int i = 0; i < 6; ++i )
	{
		if( Vec3Dot( pos, planes[i].ToVec3() ) - radius > planes[i].w )
			return 0;
	}
	return 1;
}

bool SGRX_DefaultCullScene::Camera_MeshList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneMesh* meshes, uint32_t* outbitfield )
{
	uint32_t i, o = 0;
	Vec4 frustum[6];
	
	m_aabbCache.clear();
	m_aabbCache.resize( 2 * count );
	Vec3* meshaabbs = m_aabbCache.data();
	
	SGRX_GetFrustumPlanes( frustum, camera->invViewProjMatrix );
	for( i = 0; i < count; ++i )
		SGRX_GetTfAABB( meshaabbs + i * 2, meshes[ i ].transform, meshes[ i ].min, meshes[ i ].max );
	
	for( i = 0; i < count; )
	{
		int sub = 32;
		uint32_t bfout = 0;
		while( i < count && sub --> 0 )
		{
			uint32_t mask = 1 << ( i % 32 );
			if( SGRX_FrustumAABBIntersection( frustum, meshaabbs[ i * 2 ], meshaabbs[ i * 2 + 1 ] ) )
				bfout |= mask;
			i++;
		}
		outbitfield[ o++ ] = bfout;
	}
	
	free( meshaabbs );
	return 1;
}

bool SGRX_DefaultCullScene::Camera_PointLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullScenePointLight* lights, uint32_t* outbitfield )
{
	uint32_t i, o = 0;
	Vec4 frustum[6];
	
	SGRX_GetFrustumPlanes( frustum, camera->invViewProjMatrix );
	
	for( i = 0; i < count; )
	{
		int sub = 32;
		uint32_t bfout = 0;
		while( i < count && sub --> 0 )
		{
			uint32_t mask = 1 << ( i % 32 );
			if( SGRX_FrustumSphereIntersection( frustum, lights[ i ].position, lights[ i ].radius ) )
				bfout |= mask;
			i++;
		}
		outbitfield[ o++ ] = bfout;
	}
	
	return 1;
}

bool SGRX_DefaultCullScene::Camera_SpotLightList( uint32_t count, SGRX_CullSceneCamera* camera, SGRX_CullSceneFrustum* frusta, Mat4* inv_matrices, uint32_t* outbitfield )
{
	uint32_t i, o = 0;
	Vec4 frustum[6];
	
	m_aabbCache.clear();
	m_aabbCache.resize( 2 * count );
	Vec3* lightaabbs = m_aabbCache.data();
	
	SGRX_GetFrustumPlanes( frustum, camera->invViewProjMatrix );
	for( i = 0; i < count; ++i )
		SGRX_GetFrustumAABB( lightaabbs + i * 2, inv_matrices[ i ] );
	
	for( i = 0; i < count; )
	{
		int sub = 32;
		uint32_t bfout = 0;
		while( i < count && sub --> 0 )
		{
			uint32_t mask = 1 << ( i % 32 );
			if( SGRX_FrustumAABBIntersection( frustum, lightaabbs[ i * 2 ], lightaabbs[ i * 2 + 1 ] ) )
				bfout |= mask;
			i++;
		}
		outbitfield[ o++ ] = bfout;
	}
	
	free( lightaabbs );
	return 1;
}



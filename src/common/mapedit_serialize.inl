

template< class T > void EdGroupManager::Serialize( T& arch )
{
	if( arch.version >= 3 )
	{
		arch.marker( "GRPLST" );
		{
			EdGroup* grp;
			int32_t groupcount = m_groups.size();
			arch << groupcount;
			for( int32_t i = 0; i < groupcount; ++i )
			{
				arch.marker( "GROUP" );
				if( T::IsWriter )
				{
					grp = m_groups.item( i ).value;
					arch << grp->m_id;
					arch << grp->m_parent_id;
					arch << grp->m_name;
				}
				else
				{
					int32_t id = 0, pid = 0;
					String name;
					arch << id;
					arch << pid;
					arch << name;
					grp = _AddGroup( id, pid, name );
				}
				arch << *grp;
			}
		}
	}
	else
		AddRootGroup();
}


template< class T > void EdSurface::Serialize( T& arch )
{
	arch.marker( "SURFACE" );
	SGRX_GUID oldsurfguid = surface_guid;
	arch << surface_guid;
	if( surface_guid != oldsurfguid )
	{
		if( oldsurfguid.NotNull() )
			g_EdLGCont->DeleteSurface( oldsurfguid );
		if( surface_guid.NotNull() )
			g_EdLGCont->RequestSurface( surface_guid );
	}
	arch << texname;
	arch << texgenmode;
	arch << xoff << yoff;
	arch << scale << aspect;
	arch << angle;
	arch( lmquality, arch.version >= 1, 1.0f );
	arch( xfit, arch.version >= 3, 0 );
	arch( yfit, arch.version >= 3, 0 );
}


template< class T > void EdBlock::SerializeT( T& arch )
{
	arch.marker( "BLOCK" );
	SGRX_GUID oldsolidguid = solid_guid;
	arch << solid_guid;
	if( solid_guid != oldsolidguid )
	{
		if( oldsolidguid.NotNull() )
			g_EdLGCont->DeleteSolid( oldsolidguid );
		if( solid_guid.NotNull() )
			g_EdLGCont->RequestSolid( solid_guid );
	}
	if( arch.version >= 3 )
	{
		arch << group;
		arch << position;
	}
	else
	{
		group = 0;
		Vec2 pos = { position.x, position.y };
		arch << pos;
		position = V3( pos.x, pos.y, 0 );
	}
	arch << z0 << z1;
	arch << poly;
	arch << surfaces;
	
	if( T::IsReader )
	{
		subsel.resize_using( GetNumVerts() + GetNumSurfs(), false );
		RegenerateMesh();
	}
}


template< class T > void EdPatchVtx::Serialize( T& arch )
{
	arch << pos;
	for( int i = 0; i < MAX_PATCH_LAYERS; ++i )
	{
		arch << tex[ i ];
		arch << col[ i ];
	}
}


template< class T > void EdPatchLayerInfo::Serialize( T& arch )
{
	SGRX_GUID oldsurfguid = surface_guid;
	arch << surface_guid;
	if( surface_guid != oldsurfguid )
	{
		if( oldsurfguid.NotNull() )
			g_EdLGCont->DeleteSurface( oldsurfguid );
		if( surface_guid.NotNull() )
			g_EdLGCont->RequestSurface( surface_guid );
	}
	arch << texname;
	arch << xoff << yoff;
	arch << scale << aspect;
	arch << angle;
	float lmq = 0;
	arch( lmq, arch.version < 6 );
}


template< class T > void EdPatch::SerializeT( T& arch )
{
	arch.marker( "PATCH" );
	arch << group;
	arch << position;
	arch << xsize << ysize;
	arch << blend;
	arch( m_isLMSolid, arch.version >= 9, true );
	arch( m_isPhySolid, arch.version >= 9, true );
	arch( lmquality, arch.version >= 6, 1.0f );
	for( int y = 0; y < ysize; ++y )
	{
		for( int x = 0; x < xsize; ++x )
			arch << vertices[ x + y * MAX_PATCH_WIDTH ];
	}
	for( int y = 0; y < ysize; ++y )
		arch << edgeflip[ y ];
	for( int l = 0; l < MAX_PATCH_LAYERS; ++l )
		arch << layers[ l ];
}


template< class T > void EdMeshPathPart::Serialize( T& arch )
{
	SGRX_GUID oldsurfguid = surface_guid;
	arch << surface_guid;
	if( surface_guid != oldsurfguid )
	{
		if( oldsurfguid.NotNull() )
			g_EdLGCont->DeleteSurface( oldsurfguid );
		if( surface_guid.NotNull() )
			g_EdLGCont->RequestSurface( surface_guid );
	}
	arch << texname;
	arch << xoff << yoff;
	arch << scale << aspect;
	arch << angle;
	float lmq = 0;
	arch( lmq, arch.version < 6 );
}


template< class T > void EdMeshPath::SerializeT( T& arch )
{
	arch.marker( "MESHPATH" );
	arch << group;
	arch << m_position;
	arch << m_meshName;
	arch << m_lmquality;
	arch( m_isLMSolid, arch.version >= 9, true );
	arch << m_isPhySolid;
	arch << m_skipCut; // - 27.03.2016
	arch << m_doSmoothing;
	arch << m_isDynamic;
	arch << m_intervalScaleOffset;
	arch << m_pipeModeOvershoot;
	arch << m_rotAngles;
	arch << m_scaleUni;
	arch << m_scaleSep;
	arch << m_turnMode;
	arch << m_points;
	for( int mp = 0; mp < MAX_MESHPATH_PARTS; ++mp )
		arch << m_parts[ mp ];
}


template< class T > void EdWorld::Serialize( T& arch )
{
	if( T::IsReader )
		Reset();
	
	arch.marker( "WORLD" );
	SerializeVersionHelper<T> svh( arch, MAP_FILE_VERSION );
	
	svh << m_lighting.ambientColor;
	svh << m_lighting.dirLightDir;
	svh << m_lighting.dirLightColor;
	svh << m_lighting.dirLightDvg;
	svh << m_lighting.dirLightNumSamples;
	svh << m_lighting.lightmapClearColor;
	int radNumBounces = 0;
	svh( radNumBounces, svh.version < 6 );
//	svh << m_lighting.radNumBounces;
	svh << m_lighting.lightmapDetail;
	svh << m_lighting.lightmapBlurSize;
	svh << m_lighting.aoDist;
	svh << m_lighting.aoMult;
	svh << m_lighting.aoFalloff;
	svh << m_lighting.aoEffect;
//	svh << m_lighting.aoDivergence;
	svh << m_lighting.aoColor;
	svh << m_lighting.aoNumSamples;
	
	if( svh.version >= 6 )
		svh << m_lighting.sampleDensity;
	else if( T::IsReader )
		m_lighting.sampleDensity = 1.0f;
	
	if( T::IsWriter )
	{
		ASSERT( !"Writing the old file format is no longer supported" );
	}
	else
	{
		int32_t numblocks;
		svh << numblocks;
		for( int32_t i = 0; i < numblocks; ++i )
		{
			EdBlock* block = new EdBlock;
			svh << *block;
			AddObject( block );
		}
		
		int32_t numents;
		svh << numents;
		for( int32_t i = 0; i < numents; ++i )
		{
			String ty;
			
			svh.marker( "ENTITY" );
			svh << ty;
			
			SGS_CTX = g_Level->GetSGSC();
			SGS_CSCOPE( C );
			Vec3 pos = V3(0);
			
			if( ty == SV("mesh") )
			{
				uint32_t mid = 0;
				Vec3 angles, scaleSep;
				float scaleUni;
				String mesh;
				
				svh( mid, svh.version >= 5 );
				svh << pos;
				svh << angles;
				svh << scaleUni;
				svh << scaleSep;
				svh << mesh;
				
				sgsVariable props = FNewDict();
				FSaveProp( props, "position", pos );
				FSaveProp( props, "rotationXYZ", angles );
				FSaveProp( props, "scale", scaleSep * scaleUni );
				FSaveProp( props, "mesh", mesh );
				FSaveProp( props, "isStatic", true );
				
				sgsVariable object = FNewDict();
				FSaveProp( object, "entity_type", SV("Mesh") );
				object.setprop( "props", props );
				EdEntity* obj = new EdEntity( FVar( SV("Mesh") ).get_string(), false );
				obj->FLoad( object, MAP_FILE_VERSION );
				AddObject( obj );
			}
			else if( ty == SV("light") )
			{
				uint32_t lid = 0;
				Vec3 colorHSV, flareOffset = V3(0), rotation;
				float range, power, radius, flareSize, innerAngle, outerAngle, curve;
				int32_t shadowSampleCount = 0;
				bool dynamic = false, isSpot;
				
				svh( lid, svh.version >= 5 );
				svh << pos;
				svh << range;
				svh << power;
				svh << colorHSV;
				svh << radius;
				if( svh.version >= 7 )
					svh << dynamic;
				svh << shadowSampleCount;
				svh << flareSize;
				if( svh.version >= 2 )
					svh << flareOffset;
				svh << isSpot;
				svh << rotation;
				svh << innerAngle;
				svh << outerAngle;
				svh << curve;
				
				sgsVariable props = FNewDict();
				FSaveProp( props, "position", pos );
				FSaveProp( props, "rotationXYZ", rotation );
				FSaveProp( props, "scale", V3(1) );
				FSaveProp( props, "isStatic", !dynamic );
				FSaveProp( props, "type", isSpot ? LIGHT_SPOT : LIGHT_POINT );
				FSaveProp( props, "enabled", true );
				FSaveProp( props, "color", HSV( V3(colorHSV.x,colorHSV.y,1) ) );
				FSaveProp( props, "intensity", colorHSV.z );
				FSaveProp( props, "range", range );
				FSaveProp( props, "power", power );
				FSaveProp( props, "angle", outerAngle );
				FSaveProp( props, "aspect", 1.0f );
				FSaveProp( props, "innerAngle", innerAngle );
				FSaveProp( props, "spotCurve", curve );
				FSaveProp( props, "lightRadius", radius );
				FSaveProp( props, "hasShadows", true );
				props.setprop( "cookieTexture", sgsVariable() ); // NULL
				FSaveProp( props, "flareSize", flareSize );
				FSaveProp( props, "flareOffset", flareOffset );
				
				sgsVariable object = FNewDict();
				FSaveProp( object, "entity_type", SV("Light") );
				object.setprop( "props", props );
				EdEntity* obj = new EdEntity( FVar( SV("Light") ).get_string(), false );
				obj->FLoad( object, MAP_FILE_VERSION );
				AddObject( obj );
			}
			else if( ty == SV("ltsample") )
			{
				svh << pos;
				
				sgsVariable props = FNewDict();
				FSaveProp( props, "position", pos );
				
				sgsVariable object = FNewDict();
				FSaveProp( object, "entity_type", SV("LightSample") );
				object.setprop( "props", props );
				EdEntity* obj = new EdEntity( FVar( SV("LightSample") ).get_string(), false );
				obj->FLoad( object, MAP_FILE_VERSION );
				AddObject( obj );
			}
			else
			{
				Array< uint32_t > meshIDs;
				String srlzdata;
				
				svh << pos;
				svh << meshIDs;
				svh << srlzdata;
				sgsVariable sgsdata = g_Level->GetScriptCtx().Unserialize( srlzdata );
				
				sgs_PushVar( C, pos );
				sgs_PushVar( C, sgsdata );
				if( g_Level->GetScriptCtx().GlobalCall( String_Concat( "ENT_UPG1_", ty ), 2, 2 ) &&
					sgs_ItemType( C, -2 ) != SGS_VT_NULL &&
					sgs_ItemType( C, -1 ) != SGS_VT_NULL )
				{
					sgsVariable object = FNewDict();
					object.setprop( "entity_type", sgs_GetVar<sgsString>()( C, -2 ) );
					object.setprop( "props", sgs_GetVar<sgsVariable>()( C, -1 ) );
					EdEntity* obj = new EdEntity( sgs_GetVar<sgsString>()( C, -2 ), false );
					obj->FLoad( object, MAP_FILE_VERSION );
					AddObject( obj );
				}
				else
				{
					sgs_Msg( g_Level->GetSGSC(), SGS_WARNING, "Failed to upgrade entity '%s'!",
						StackPath(ty).str );
				}
			}
			
			int32_t size = 0;
			svh( size, svh.version >= 4, 0 );
			if( size > 0 )
			{
				sgs_Msg( g_Level->GetSGSC(), SGS_WARNING, "Subentities CANNOT BE UPGRADED" );
				return;
			}
		}
		
		int32_t numpatches;
		svh << numpatches;
		for( int32_t i = 0; i < numpatches; ++i )
		{
			EdPatch* patch = new EdPatch;
			svh << *patch;
			AddObject( patch );
		}
		
		if( svh.version >= 8 )
		{
			int32_t numpaths;
			svh << numpaths;
			for( int32_t i = 0; i < numpaths; ++i )
			{
				EdMeshPath* mpath = new EdMeshPath;
				svh << *mpath;
				AddObject( mpath );
			}
		}
	}
}



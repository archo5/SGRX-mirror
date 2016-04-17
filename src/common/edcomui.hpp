

#pragma once
#include <engine.hpp>
#include <edgui.hpp>
#include <script.hpp>
#include <sound.hpp>

#include "mpd_api.hpp"


// nullifies mesh lighting
inline void lmm_prepmeshinst( MeshInstHandle mih )
{
	mih->SetLightingMode( SGRX_LM_Dynamic );
	for( int i = 10; i < 16; ++i )
		mih->constants[ i ] = V4(0.15f);
}


inline int SGRX_MtlBlend_FromString( const StringView& sv )
{
	if( sv == "None" ) return SGRX_MtlBlend_None;
	if( sv == "Basic" ) return SGRX_MtlBlend_Basic;
	if( sv == "Additive" ) return SGRX_MtlBlend_Additive;
	if( sv == "Multiply" ) return SGRX_MtlBlend_Multiply;
	return SGRX_MtlBlend_None;
}

inline const char* SGRX_MtlBlend_ToString( int bm )
{
	if( bm == SGRX_MtlBlend_None ) return "None";
	if( bm == SGRX_MtlBlend_Basic ) return "Basic";
	if( bm == SGRX_MtlBlend_Additive ) return "Additive";
	if( bm == SGRX_MtlBlend_Multiply ) return "Multiply";
	return "None";
}


#define LI_SUBBTN_WIDTH 30
struct EDGUIListItemButton : EDGUIButton
{
	EDGUIListItemButton( bool ordered = true )
	{
		tyname = "btn_item";
		m_up.tyname = "btn_up";
		m_dn.tyname = "btn_dn";
		m_del.tyname = "btn_del";
		
		m_up.caption = "[up]";
		m_dn.caption = "[dn]";
		m_del.caption = "[x]";
		
		m_del.SetHighlight( true );
		
		if( ordered )
		{
			Add( &m_up );
			Add( &m_dn );
		}
		Add( &m_del );
	}
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_LAYOUT:
			EDGUIButton::OnEvent( e );
			SetSubitemLayout( &m_up, x1 - LI_SUBBTN_WIDTH * 3, y0, x1 - LI_SUBBTN_WIDTH * 2, y1 );
			SetSubitemLayout( &m_dn, x1 - LI_SUBBTN_WIDTH * 2, y0, x1 - LI_SUBBTN_WIDTH * 1, y1 );
			SetSubitemLayout( &m_del, x1 - LI_SUBBTN_WIDTH * 1, y0, x1 - LI_SUBBTN_WIDTH * 0, y1 );
			return 0;
			
		case EDGUI_EVENT_PAINT:
			if( backColor )
			{
				GR2D_GetBatchRenderer().UnsetTexture().Colu( backColor ).Quad( float(x0), float(y0), float(x1), float(y1) );
			}
			if( textColor && caption.size() )
			{
				GR2D_GetBatchRenderer().Colu( textColor );
				GR2D_DrawTextLine( x0 + 2, round(( y0 + y1 ) / 2.0f), caption, HALIGN_LEFT, VALIGN_CENTER );
			}
			for( size_t i = 0; i < m_subitems.size(); ++i )
			{
				m_subitems[ i ]->OnEvent( e );
			}
			return 1;
			
		}
		return EDGUIButton::OnEvent( e );
	}
	
	EDGUIButton m_up;
	EDGUIButton m_dn;
	EDGUIButton m_del;
};


struct EDGUIShaderPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIShaderPicker()
	{
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading shaders";
		m_options.clear();
		FS_IterateDirectory( "shaders", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[Sh]: " << name;
		if( isdir == false && name.ends_with( ".shd" ) && name.starts_with( "mtl_" ) )
		{
			m_options.push_back( name.part( 4, name.size() - 8 ) );
		}
		return true;
	}
};

struct EDGUITexturePicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUITexturePicker( const StringView& dir = "textures" ) : m_dir( String_Concat( dir, "/" ) )
	{
		caption = "Pick a texture";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading textures";
		m_options.clear();
		m_textures.clear();
		m_options.push_back( "" );
		m_textures.push_back( TextureHandle() );
		FS_IterateDirectory( StringView(m_dir).until_last("/"), this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		if( name == "." || name == ".." )
			return true;
		
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
		LOG << "[T]: " << bfr;
		StringView fullname = bfr;
		
		if( isdir )
		{
			FS_IterateDirectory( fullname, this );
		}
		else if( name.ends_with( ".png" ) || name.ends_with( ".stx" ) || name.ends_with( ".dds" ) )
		{
			m_options.push_back( fullname );
			m_textures.push_back( GR_GetTexture( fullname ) );
		}
		return true;
	}
	void _DrawItem( int i, int x0, int y0, int x1, int y1 )
	{
		BatchRenderer& br = GR2D_GetBatchRenderer();
		
		br.Col( 1 );
		br.SetTexture( m_textures[ i ] );
		br.Quad( x0 + 10, y0 + 4, x1 - 10, y1 - 16 );
		
		br.Col( 0.9f, 1.0f );
		GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, m_options[ i ], HALIGN_CENTER, VALIGN_CENTER );
	}
	
	String m_dir;
	Array< TextureHandle > m_textures;
};


struct EDGUIMeshPickerCore : EDGUIRsrcPicker
{
	EDGUIMeshPickerCore() : m_customCamera( false ), m_scene( GR_CreateScene() )
	{
	}
	~EDGUIMeshPickerCore()
	{
		Clear();
	}
	void Clear()
	{
		m_meshInsts.clear();
	}
	void AddMesh( StringView path )
	{
		MeshInstHandle mih = m_scene->CreateMeshInstance();
		mih->SetMesh( path );
		mih->enabled = false;
		lmm_prepmeshinst( mih );
		mih->Precache();
		m_meshInsts.push_back( mih );
	}
	void _DrawItem( int i, int x0, int y0, int x1, int y1 )
	{
		SGRX_Viewport vp = { x0 + 10, y0 + 4, x1 - 10, y1 - 16 };
		
		if( m_meshInsts[ i ] )
		{
			SGRX_IMesh* M = m_meshInsts[ i ]->GetMesh();
			if( m_customCamera == false )
			{
				Vec3 dst = M->m_boundsMin - M->m_boundsMax;
				Vec3 idst = V3( dst.x ? 1/dst.x : 1, dst.y ? 1/dst.y : 1, dst.z ? 1/dst.z : 1 );
				if( idst.z > 0 )
					idst.z = -idst.z;
				m_scene->camera.direction = idst.Normalized();
				m_scene->camera.position = ( M->m_boundsMax + M->m_boundsMin ) * 0.5f - m_scene->camera.direction * dst.Length() * 0.8f;
				m_scene->camera.znear = 0.1f;
				m_scene->camera.angle = 60;
				m_scene->camera.UpdateMatrices();
			}
			
			SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), m_scene );
			rsinfo.viewport = &vp;
			m_meshInsts[ i ]->enabled = true;
			GR_RenderScene( rsinfo );
			m_meshInsts[ i ]->enabled = false;
		}
		
		BatchRenderer& br = GR2D_GetBatchRenderer();
		br.Col( 0.9f, 1.0f );
		GR2D_DrawTextLine( ( x0 + x1 ) / 2, y1 - 8, m_options[ i ], HALIGN_CENTER, VALIGN_CENTER );
	}
	
	bool m_customCamera;
	Array< MeshInstHandle > m_meshInsts;
	SceneHandle m_scene;
};

struct EDGUIMeshPicker : EDGUIMeshPickerCore, IDirEntryHandler
{
	EDGUIMeshPicker( bool fullpaths = false ) :
		m_fullpaths( fullpaths )
	{
		caption = "Pick a mesh";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading meshes";
		m_options.clear();
		Clear();
		FS_IterateDirectory( "meshes", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		if( name == "." || name == ".." )
			return true;
		char bfr[ 256 ];
		sgrx_snprintf( bfr, 256, "%s/%s", StackString<256>(loc).str, StackString<256>(name).str );
		LOG << "[M]: " << bfr;
		StringView fullname = bfr;
		if( isdir )
		{
			FS_IterateDirectory( fullname, this );
		}
		else if( name.ends_with( ".ssm" ) )
		{
			if( m_fullpaths )
				m_options.push_back( fullname );
			else
			{
				StringView part = fullname.part( sizeof("meshes/")-1,
					fullname.size() - (sizeof("meshes/.ssm")-1) );
				m_options.push_back( part );
			}
			AddMesh( fullname );
		}
		return true;
	}
	
	bool m_fullpaths;
};

inline String ED_GetMeshFromChar( const StringView& charpath )
{
	ByteArray ba;
	if( FS_LoadBinaryFile( charpath, ba ) == false )
		return String();
	
	// basic unserialization to retrieve mesh
	ByteReader br( ba );
	br.marker( "SGRXCHAR" );
	SerializeVersionHelper<ByteReader> arch( br, 2 );
	String mesh;
	arch( mesh );
	return mesh;
}

struct EDGUICharUsePicker : EDGUIMeshPickerCore, IDirEntryHandler
{
	EDGUICharUsePicker( bool fullpaths = false ) :
		m_fullpaths( fullpaths )
	{
		caption = "Pick a char";
		Reload();
	}
	void Reload()
	{
		LOG << "Reloading chars";
		m_options.clear();
		Clear();
		FS_IterateDirectory( "chars", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[Ch]: " << name;
		if( !isdir && name.ends_with( ".chr" ) )
		{
			String fullpath = String_Concat( "chars/", name );
			String mesh = ED_GetMeshFromChar( fullpath );
			if( mesh.size() == 0 )
				return true;
			
			if( m_fullpaths )
				m_options.push_back( fullpath );
			else
				m_options.push_back( name.part( 0, name.size() - 4 ) );
			AddMesh( mesh );
		}
		return true;
	}
	
	bool m_fullpaths;
};


struct EDGUIPartSysPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUIPartSysPicker()
	{
		caption = "Pick a particle system";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Reloading particle systems";
		m_options.clear();
		FS_IterateDirectory( "psys", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[P]: " << name;
		if( !isdir && name.ends_with( ".psy" ) )
		{
			m_options.push_back( String_Concat( "psys/", name ) );
		}
		return true;
	}
};


struct EDGUISoundPicker : EDGUIRsrcPicker
{
	EDGUISoundPicker()
	{
		sys = SND_CreateSystem();
		sys->Load( "sound/master.bank" );
		sys->Load( "sound/master.strings.bank" );
		caption = "Pick a sound event";
		Reload();
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	void Reload()
	{
		LOG << "Enumerating sound events";
		sys->EnumerateSoundEvents( m_options );
		_Search( m_searchString );
	}
	SoundSystemHandle sys;
};


struct EDGUILevelPicker : EDGUIRsrcPicker, IDirEntryHandler
{
	EDGUILevelPicker(){ Reload(); }
	void Reload()
	{
		LOG << "Reloading levels";
		m_options.clear();
		FS_IterateDirectory( "levels", this );
		_Search( m_searchString );
	}
	bool HandleDirEntry( const StringView& loc, const StringView& name, bool isdir )
	{
		LOG << "[L]: " << name;
		if( !isdir && name.ends_with( ".tle" ) )
		{
			m_options.push_back( name.part( 0, name.size() - 4 ) );
		}
		return true;
	}
	virtual void _OnChangeZoom()
	{
		EDGUIRsrcPicker::_OnChangeZoom();
		m_itemHeight /= 4;
	}
	
	virtual int OnEvent( EDGUIEvent* e )
	{
		if( e->type == EDGUI_EVENT_PROPCHANGE && e->target == &m_confirm )
		{
			if( m_confirm.m_value == 1 )
			{
				EDGUIRsrcPicker::_OnPickResource();
			}
			return 1;
		}
		return EDGUIRsrcPicker::OnEvent( e );
	}
	
	EDGUIQuestion m_confirm;
};

struct EDGUILevelOpenPicker : EDGUILevelPicker
{
	EDGUILevelOpenPicker()
	{
		caption = "Pick a level to open";
		m_confirm.caption = "Do you really want to open the level? All unsaved changes will be lost!";
	}
	virtual void _OnPickResource()
	{
		m_confirm.Open( this );
		m_frame->Add( &m_confirm );
	}
};

struct EDGUILevelSavePicker : EDGUILevelPicker
{
	EDGUILevelSavePicker()
	{
		caption = "Pick a level to save or write the name";
		m_confirm.caption = "Do you really want to overwrite the level?";
	}
	virtual void _OnPickResource()
	{
		if( m_options.find_first_at( m_pickedOption ) == NOT_FOUND )
			EDGUIRsrcPicker::_OnPickResource();
		else
		{
			m_confirm.Open( this );
			m_frame->Add( &m_confirm );
		}
	}
	virtual void _OnConfirm()
	{
		_OnPickResource();
	}
};


//
// property inspector
//

struct EDGUIPropertyList : EDGUILayoutRow
{
	struct Item
	{
		EDGUIItemHandle item;
		mpd_Variant cont;
		const mpd_PropInfo* prop;
		size_t parent_id;
	};
	
	void Clear()
	{
		m_lastEditedItem = NOT_FOUND;
		m_items.clear();
	}
	void Add( mpd_Variant data )
	{
		_CreateProperty( this, data );
	}
	void Set( mpd_Variant data )
	{
		Clear();
		Add( data );
	}
	void Reload()
	{
		for( size_t i = 0; i < m_items.size(); ++i )
		{
			Item& ITM = m_items[ i ];
			EDGUIItem* ctrl = ITM.item;
			mpd_Variant val = ITM.cont.getpropbyid( ITM.prop - ITM.cont.get_typeinfo()->vprops() );
			switch( ctrl->type )
			{
			case EDGUI_ITEM_PROP_BOOL: ((EDGUIPropBool*)ctrl)->SetValue( mpd_var_get<bool>( val ) ); break;
			case EDGUI_ITEM_PROP_INT: ((EDGUIPropInt*)ctrl)->SetValue( mpd_var_get<int32_t>( val ) ); break;
			case EDGUI_ITEM_PROP_FLOAT: ((EDGUIPropFloat*)ctrl)->SetValue( mpd_var_get<float>( val ) ); break;
			case EDGUI_ITEM_PROP_VEC2: ((EDGUIPropVec2*)ctrl)->SetValue( mpd_var_get<Vec2>( val ) ); break;
			case EDGUI_ITEM_PROP_VEC3: ((EDGUIPropVec3*)ctrl)->SetValue( mpd_var_get<Vec3>( val ) ); break;
			case EDGUI_ITEM_PROP_STRING: ((EDGUIPropString*)ctrl)->SetValue( val.get_obj<String>() ); break;
			case EDGUI_ITEM_PROP_ENUM_SB: ((EDGUIPropEnumSB*)ctrl)->SetValue( val.get_enum() ); break;
			case EDGUI_ITEM_PROP_RSRC: ((EDGUIPropRsrc*)ctrl)->SetValue( val.get_obj<String>() ); break;
			}
		}
	}
	StringView _GetPropName( const mpd_PropInfo* p )
	{
		StringView propname;
		if( p )
		{
			propname = StringView( p->name, p->namesz );
			const mpd_KeyValue* kv = p->metadata->find( "label" );
			if( kv )
				propname = StringView( kv->value, kv->valuesz );
		}
		return propname;
	}
	void _AddProp( EDGUIItem* prt, EDGUIItem* prop, mpd_Variant cont, const mpd_PropInfo* propinfo, size_t pid, StringView name = SV() )
	{
		prop->id1 = m_items.size();
		prop->caption = name ? name : _GetPropName( propinfo );
		Item item = { prop, cont, propinfo, pid };
		m_items.push_back( item );
		prt->Add( prop );
	}
	void _CreateProperty( EDGUIItem* prt, mpd_Variant item,
		mpd_Variant cont = mpd_Variant(), const mpd_PropInfo* propinfo = NULL, size_t pid = NOT_FOUND )
	{
		mpd_Type type = item.get_type();
		if( type == mpdt_Struct || type == mpdt_Pointer )
		{
			const virtual_MPD* info = item.get_typeinfo();
			ASSERT( info );
			
			if( !strcmp( info->vname(), "String" ) )
			{
				String value = item.get_obj<String>();
				
				EDGUIProperty* prop = NULL;
				if( propinfo )
				{
					const mpd_KeyValue* kv = propinfo->metadata->find( "edit" );
					if( kv )
					{
						EDGUIRsrcPicker* pck = m_pickers.getcopy( StringView( kv->value, kv->valuesz ) );
						if( pck )
						{
							EDGUIPropRsrc* pr;
							prop = pr = new EDGUIPropRsrc( pck, value );
							kv = propinfo->metadata->find( "edit_requestReload" );
							if( kv && kv->value_i32 )
								pr->m_requestReload = true;
						}
					}
				}
				if( !prop )
					prop = new EDGUIPropString( value );
				_AddProp( prt, prop, cont, propinfo, pid );
			}
			else if( !strcmp( info->vname(), "Vec2" ) )
			{
				Vec2 vmin = V2(-FLT_MAX), vmax = V2(FLT_MAX);
				int prec = 2;
				if( propinfo )
				{
					const mpd_KeyValue* kv;
					kv = propinfo->metadata->find("min");
					if( kv )
						vmin = *(Vec2*)kv->value;
					kv = propinfo->metadata->find("max");
					if( kv )
						vmax = *(Vec2*)kv->value;
					kv = propinfo->metadata->find("prec");
					if( kv )
						prec = kv->value_i32;
				}
				
				EDGUIPropVec2* prop = new EDGUIPropVec2( item.get_obj<Vec2>(), prec, vmin, vmax );
				_AddProp( prt, prop, cont, propinfo, pid );
			}
			else if( !strcmp( info->vname(), "Vec3" ) )
			{
				Vec3 vmin = V3(-FLT_MAX), vmax = V3(FLT_MAX);
				int prec = 2;
				if( propinfo )
				{
					const mpd_KeyValue* kv;
					kv = propinfo->metadata->find("min");
					if( kv )
						vmin = *(Vec3*)kv->value;
					kv = propinfo->metadata->find("max");
					if( kv )
						vmax = *(Vec3*)kv->value;
					kv = propinfo->metadata->find("prec");
					if( kv )
						prec = kv->value_i32;
				}
				
				EDGUIPropVec3* prop = new EDGUIPropVec3( item.get_obj<Vec3>(), prec, vmin, vmax );
				_AddProp( prt, prop, cont, propinfo, pid );
			}
			else
			{
				StringView gname = _GetPropName( propinfo );
				if( !gname )
				{
					const mpd_KeyValue* kv = info->vmetadata()->find( "label" );
					if( kv )
						gname = StringView( kv->value, kv->valuesz );
				}
				if( !gname )
					gname = info->vname();
				EDGUIGroup* group = new EDGUIGroup( true, gname );
				size_t mypid = m_items.size();
				_AddProp( prt, group, cont, propinfo, pid );
				
				for( int i = 0, pc = info->vpropcount(); i < pc; ++i )
				{
					const mpd_PropInfo* p = info->vprop( i );
					_CreateProperty( group, item.getpropbyid( i ), item, p, mypid );
				}
				
				const mpd_TypeInfo* indextypes = info->vindextypes();
				if( indextypes )
				{
					// array type
				}
			}
		}
		else if( type == mpdt_Enum )
		{
			const virtual_MPD* info = item.get_typeinfo();
			ASSERT( info );
			EDGUIPropEnumSB* prop = new EDGUIPropEnumSB;
			
			const mpd_EnumValue* v = info->vvalues();
			while( v->name )
			{
				StringView label( v->name, v->namesz );
				const mpd_KeyValue* kv = v->metadata->find( "label" );
				if( kv )
					label = StringView( kv->value, kv->valuesz );
				EDGUIPropEnumSB::Entry entry = { label, v->value };
				prop->m_enum.push_back( entry );
				++v;
			}
			prop->SetValue( item.get_enum() );
			
			_AddProp( prt, prop, cont, propinfo, pid );
		}
		else if( type == mpdt_Bool )
		{
			EDGUIPropBool* prop = new EDGUIPropBool( item.get_bool() );
			_AddProp( prt, prop, cont, propinfo, pid );
		}
		else if( mpd_TypeIsInteger( type ) )
		{
			int32_t vmin = (int32_t) 0x80000000, vmax = (int32_t) 0x7fffffff;
			if( propinfo )
			{
				const mpd_KeyValue* kv;
				kv = propinfo->metadata->find("min");
				if( kv )
					vmin = kv->value_i32;
				kv = propinfo->metadata->find("max");
				if( kv )
					vmax = kv->value_i32;
			}
			
			EDGUIPropInt* prop = new EDGUIPropInt( item.get_int32(), vmin, vmax );
			_AddProp( prt, prop, cont, propinfo, pid );
		}
		else if( mpd_TypeIsFloat( type ) )
		{
			float vmin = -FLT_MAX, vmax = FLT_MAX;
			int prec = 2;
			if( propinfo )
			{
				const mpd_KeyValue* kv;
				kv = propinfo->metadata->find("min");
				if( kv )
					vmin = kv->value_float;
				kv = propinfo->metadata->find("max");
				if( kv )
					vmax = kv->value_float;
				kv = propinfo->metadata->find("prec");
				if( kv )
					prec = kv->value_i32;
			}
			
			EDGUIPropFloat* prop = new EDGUIPropFloat( item.get_float32(), prec, vmin, vmax );
			_AddProp( prt, prop, cont, propinfo, pid );
		}
		else if( type == mpdt_ConstString )
		{
			mpd_StringView sv = item.get_stringview();
			EDGUIPropString* prop = new EDGUIPropString( StringView( sv.str, sv.size ) );
			_AddProp( prt, prop, cont, propinfo, pid );
		}
		else
		{
			EDGUILabel* label = new EDGUILabel;
			_AddProp( prt, label, cont, propinfo, pid );
			
			char bfr[ 256 ];
			sgrx_snprintf( bfr, 256, "<unsupported type:%d name:%s>", (int) item.get_type(), item.get_name() );
			label->caption = bfr;
		}
	}
	virtual int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_PROPEDIT:
		case EDGUI_EVENT_PROPCHANGE:
			if( e->target != this )
			{
				size_t which = e->target->id1;
				Item& ITM = m_items[ which ];
				if( !ITM.prop )
					break;
				
				mpd_Variant cont = ITM.cont;
				const mpd_PropInfo* prop = ITM.prop;
				StringView ts;
				mpd_Variant val;
				switch( e->target->type )
				{
				case EDGUI_ITEM_PROP_BOOL: val = ((EDGUIPropBool*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_INT: val = ((EDGUIPropInt*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_FLOAT: val = ((EDGUIPropFloat*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_VEC2: val = &((EDGUIPropVec2*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_VEC3: val = &((EDGUIPropVec3*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_STRING: val = &((EDGUIPropString*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_ENUM_SB: val = &((EDGUIPropEnumSB*)e->target)->m_value; break;
				case EDGUI_ITEM_PROP_RSRC: val = &((EDGUIPropRsrc*)e->target)->m_value; break;
				}
				if( val.get_type() != mpdt_None )
				{
					cont.setpropbyid( prop - cont.get_typeinfo()->vprops(), val );
				//	mpd_DumpData( ITM.cont );
				}
				
				m_lastEditedItem = which;
				if( e->type == EDGUI_EVENT_PROPEDIT )
					Edited();
				else
					Changed();
			}
			break;
		}
		return EDGUILayoutRow::OnEvent( e );
	}
	
	bool WasPropEdited( mpd_Variant item, const mpd_PropInfo* p ) const
	{
		size_t iid = m_lastEditedItem;
		while( iid < m_items.size() )
		{
			const Item& ITM = m_items[ iid ];
			if( ITM.cont == item && ITM.prop == p )
				return true;
			iid = ITM.parent_id;
		}
		return false;
	}
	
	Array< Item > m_items;
	size_t m_lastEditedItem;
	HashTable< StringView, EDGUIRsrcPicker* > m_pickers;
};


//
// first person scene renderer
//

struct EDGUIRenderView : EDGUIItem, SGRX_DebugDraw
{
	struct FrameInterface
	{
		virtual bool ViewEvent( EDGUIEvent* e ) = 0;
		virtual void DebugDraw() = 0;
	};
	
	EDGUIRenderView( SceneHandle scene, FrameInterface* fiface ) :
		movefwd( false ),
		movebwd( false ),
		movelft( false ),
		movergt( false ),
		movefast( false ),
		moveup( false ),
		movedn( false ),
		look( false ),
		hangle( 0 ),
		vangle( 0 ),
		crpos(V3(0)),
		crdir(V3(0)),
		cursor_aim(false),
		cursor_hpos(V2(0)),
		crplaneheight( 0 ),
		m_edScene( scene ),
		m_mainframe( fiface )
	{
		type = 100000000;
		tyname = "renderview";
		backColor = COLOR_RGBA( 10, 10, 10, 255 );
		
		Vec3 dir = m_edScene->camera.direction;
		Vec2 dir2 = V2( dir.x, dir.y ).Normalized();
		hangle = atan2( dir2.y, dir2.x );
		vangle = asin( m_edScene->camera.direction.z );
		m_edScene->camera.znear = 0.1f;
	}
	int OnEvent( EDGUIEvent* e )
	{
		switch( e->type )
		{
		case EDGUI_EVENT_LAYOUT:
			EDGUIItem::OnEvent( e );
			m_edScene->camera.aspect = ( x1 - x0 ) / (float) ( y1 - y0 );
			EventToFrame( e );
			return 1;
		case EDGUI_EVENT_PAINT:
			{
				SGRX_Viewport vp = { x0, y0, x1, y1 };
				SGRX_RenderScene rsinfo( V4( GetTimeMsec() / 1000.0f ), m_edScene );
				rsinfo.viewport = &vp;
				rsinfo.debugdraw = this;
				GR_RenderScene( rsinfo );
			}
			EventToFrame( e );
			return 1;
		case EDGUI_EVENT_SETFOCUS:
			m_frame->_SetFocus( this );
			return 1;
		case EDGUI_EVENT_LOSEFOCUS:
			movefwd = false;
			movebwd = false;
			movelft = false;
			movergt = false;
			movefast = false;
			moveup = false;
			movedn = false;
			look = false;
			return 1;
		case EDGUI_EVENT_KEYDOWN:
		case EDGUI_EVENT_KEYUP:
			if( EventToFrame( e ) )
			{
				bool down = e->type == EDGUI_EVENT_KEYDOWN && !( e->key.engmod & (KMOD_CTRL|KMOD_ALT) );
				if( e->key.engkey == SDLK_w ) movefwd = down;
				if( e->key.engkey == SDLK_s ) movebwd = down;
				if( e->key.engkey == SDLK_a ) movelft = down;
				if( e->key.engkey == SDLK_d ) movergt = down;
				if( e->key.engkey == SDLK_LSHIFT ) movefast = down;
				if( e->key.engkey == SDLK_q ) moveup = down;
				if( e->key.engkey == SDLK_z ) movedn = down;
			}
			else
			{
				movefwd = false;
				movebwd = false;
				movelft = false;
				movergt = false;
				movefast = false;
				moveup = false;
				movedn = false;
			}
			return 1;
		case EDGUI_EVENT_BTNDOWN:
		case EDGUI_EVENT_BTNUP:
			if( EventToFrame( e ) )
			{
				bool down = e->type == EDGUI_EVENT_BTNDOWN;
				if( e->mouse.button == 1 ) look = down;
				prevcp = Game_GetCursorPos();
			}
			else
				look = false;
			break;
		case EDGUI_EVENT_BTNCLICK:
			EventToFrame( e );
			break;
		case EDGUI_EVENT_MOUSEMOVE:
			{
				cursor_aim = false;
				Vec2 cp = { e->mouse.x, e->mouse.y };
				if( m_edScene->camera.GetCursorRay( safe_fdiv( cp.x - x0, x1 - x0 ), safe_fdiv( cp.y - y0, y1 - y0 ), crpos, crdir ) )
				{
					float dsts[2];
					if( RayPlaneIntersect( crpos, crdir, V4(0,0,1,crplaneheight), dsts ) && dsts[0] > 0 )
					{
						Vec3 isp = crpos + crdir * dsts[0];
						cursor_hpos = V2( isp.x, isp.y );
						cursor_aim = true;
					}
				}
			}
			EventToFrame( e );
			break;
		default:
			EventToFrame( e );
			break;
		}
		return EDGUIItem::OnEvent( e );
	}
	void UpdateCamera( float deltaTime )
	{
		float speed = 1;
		if( movefast )
			speed *= 10;
		speed *= deltaTime;
		
		Vec2 cp = Game_GetCursorPos();
		if( look )
		{
			Vec2 diff = ( cp - prevcp );
			hangle -= diff.x * 0.01f;
			vangle -= diff.y * 0.01f;
			vangle = clamp( vangle, (float) -M_PI * 0.49f, (float) M_PI * 0.49f );
		}
		prevcp = cp;
		
		float c_hangle = cos( hangle ), s_hangle = sin( hangle ), c_vangle = cos( vangle ), s_vangle = sin( vangle );
		Vec3 dir = { c_hangle * c_vangle, s_hangle * c_vangle, s_vangle };
		Vec3 up = m_edScene->camera.updir;
		Vec3 rgt = Vec3Cross( dir, up ).Normalized();
		m_edScene->camera.direction = dir;
		m_edScene->camera.position += ( dir * ( movefwd - movebwd ) + rgt * ( movergt - movelft ) + up * ( moveup - movedn ) ) * speed;
		
		m_edScene->camera.UpdateMatrices();
	}
	Vec2 CPToNormalized( const Vec2& cp ) const
	{
		return V2( safe_fdiv( cp.x - x0, x1 - x0 ), safe_fdiv( cp.y - y0, y1 - y0 ) );
	}
	Vec2 GetViewSize() const
	{
		return V2( x1 - x0, y1 - y0 );
	}
	bool EventToFrame( EDGUIEvent* e ){ return m_mainframe->ViewEvent( e ); }
	void DebugDraw(){ m_mainframe->DebugDraw(); }
	
	Vec2 prevcp;
	bool movefwd;
	bool movebwd;
	bool movelft;
	bool movergt;
	bool movefast;
	bool moveup;
	bool movedn;
	bool look;
	float hangle;
	float vangle;
	
	Vec3 crpos, crdir;
	bool cursor_aim;
	Vec2 cursor_hpos;
	float crplaneheight;
	
	SceneHandle m_edScene;
	FrameInterface* m_mainframe;
};



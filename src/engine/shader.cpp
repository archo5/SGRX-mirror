

#include "engine_int.hpp"

extern IRenderer* g_Renderer;
extern VertexShaderHashTable* g_VertexShaders;
extern PixelShaderHashTable* g_PixelShaders;
extern RenderStateHashTable* g_RenderStates;



static int LineNumber( StringView text, StringView name )
{
	return 1 + text.part( name.data() - text.data() ).count( "\n" );
}

static bool IsValidPassProp( StringView name )
{
	// pass properties
	if( name == "Enabled" ) return true;
	if( name == "Order" ) return true;
	if( name == "Inherit" ) return true;
	
	// render states
	if( name == "WireFill" ) return true;
	if( name == "CullMode" ) return true;
	if( name == "SeparateBlend" ) return true;
	if( name == "ScissorEnable" ) return true;
	if( name == "MultisampleEnable" ) return true;
	if( name == "DepthEnable" ) return true;
	if( name == "DepthWriteEnable" ) return true;
	if( name == "DepthFunc" ) return true;
	if( name == "StencilEnable" ) return true;
	if( name == "StencilReadMask" ) return true;
	if( name == "StencilWriteMask" ) return true;
	if( name == "StencilFrontFailOp" ) return true;
	if( name == "StencilFrontDepthFailOp" ) return true;
	if( name == "StencilFrontPassOp" ) return true;
	if( name == "StencilFrontFunc" ) return true;
	if( name == "StencilBackFailOp" ) return true;
	if( name == "StencilBackDepthFailOp" ) return true;
	if( name == "StencilBackPassOp" ) return true;
	if( name == "StencilBackFunc" ) return true;
	if( name == "StencilRef" ) return true;
	if( name == "DepthBias" ) return true;
	if( name == "SlopeDepthBias" ) return true;
	if( name == "DepthBiasClamp" ) return true;
	if( name == "BlendFactor" ) return true;
	
	static const StringView rtVars[] = { "ColorWrite", "BlendEnable",
		"BlendOp", "SrcBlend", "DstBlend", "BlendOpAlpha", "SrcBlendAlpha", "DstBlendAlpha" };
	for( size_t i = 0; i < sizeof(rtVars)/sizeof(rtVars[0]); ++i )
	{
		if( name.starts_with( rtVars[i] ) )
		{
			if( name.size() == rtVars[i].size() ) return true;
			if( name.size() == rtVars[i].size() + 3 )
			{
				StringView indexpart = name.part( rtVars[i].size() );
				if( indexpart.size() == 3 && indexpart[0] == '[' && indexpart[2] == ']' )
				{
					for( int j = 0; j < 8; ++j )
						if( indexpart[1] == "01234567"[j] )
							return true;
				}
			}
		}
	}
	
	return false;
}

static bool _XSD_LoadPassProps( SV2SVMap& props, StringView passtext, StringView text )
{
	ConfigReader cr( passtext );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( !IsValidPassProp( key ) )
		{
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
				"invalid pass property (" << key << ") at line " << LineNumber( text, key );
			return false;
		}
		props[ key ] = value;
	}
	return true;
}

bool SGRX_XShaderDef::_XSD_LoadProps(
	HashTable< StringView, SV2SVMap >& allprops,
	Array< StringView >& defines,
	StringView text )
{
#define MAX_DISABLEVEL 9999
	int curlevel = 0, disablevel = MAX_DISABLEVEL;
	
	ConfigReader cr( text );
	StringView key, value;
	while( cr.Read( key, value ) )
	{
		if( key == "[[shader]]" )
		{
			StringView shdr = cr.it.until( "[[endshader]]" );
			if( shdr.end() == cr.it.end() )
				return false;
			int newlines = text.part( 0, key.data() - text.data() ).count( "\n" );
			cr.it.skip( shdr.size() );
			cr.it = cr.it.after( "[[endshader]]" );
			
			if( curlevel < disablevel )
			{
				for( int i = 0; i < newlines; ++i )
					shader.append( "\n" ); // for accurate line numbers
				shader.append( shdr );
			}
		}
		else if( key == "[[pass]]" )
		{
			StringView passtext = cr.it.until( "[[endpass]]" );
			if( passtext.end() == cr.it.end() )
				return false;
			cr.it.skip( passtext.size() );
			cr.it = cr.it.after( "[[endpass]]" );
			
			if( curlevel < disablevel )
				_XSD_LoadPassProps( allprops[ value ], passtext, text );
		}
		else if( key == "[[define]]" )
		{
			if( curlevel < disablevel )
				defines.push_back( value );
		}
		else if( key == "[[ifdef]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel++;
				if( defines.find_first_at( value ) == NOT_FOUND )
					disablevel = curlevel;
			}
		}
		else if( key == "[[ifndef]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel++;
				if( defines.find_first_at( value ) != NOT_FOUND )
					disablevel = curlevel;
			}
		}
		else if( key == "[[endif]]" )
		{
			if( curlevel < disablevel )
			{
				curlevel--;
				disablevel = MAX_DISABLEVEL;
			}
		}
		else
		{
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
				"invalid command (" << key << ") at line " << LineNumber( text, key );
			return false;
		}
	}
	return true;
}

#define LoadPassBool( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseBool( *props.getptr( name ) ); }
#define LoadPassInt( out, name, vmin, vmax ) \
	if( props.isset( name ) ){ \
		int64_t v = String_ParseInt( *props.getptr( name ) ); \
		if( v < int64_t(vmin) || v > int64_t(vmax) ){ \
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - " \
				"value for property " << name << " out of range [" << (vmin) \
				<< ";" << (vmax) << "] at line " << LineNumber( text, *props.getptr( name ) ); \
			return false; } \
		out = v; }
#define LoadPassFloat( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseFloat( *props.getptr( name ) ); }
#define LoadPassVec4( out, name ) \
	if( props.isset( name ) ){ \
		out = String_ParseVec4( *props.getptr( name ) ); }
#define LoadPassEnum( out, name, values ) \
	if( props.isset( name ) ){ \
		int v = String_ParseEnum( *props.getptr( name ), values ); \
		if( v == -1 ){ \
			LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - " \
				"value for property " << name << " is invalid at line " \
				<< LineNumber( text, *props.getptr( name ) ); \
			return false; } \
		out = v; }

bool SGRX_XShaderDef::LoadText( StringView text )
{
	shader.clear();
	passes.clear();
	
	HashTable< StringView, SV2SVMap > allprops;
	Array< StringView > defines;
	
	if( _XSD_LoadProps( allprops, defines, text ) == false )
	{
		return false;
	}
	
	for( size_t i = 0; i < allprops.size(); ++i )
	{
		if( String_ParseBool( allprops.item( i ).value.getcopy( "Enabled", "True" ) ) == false )
			continue; // this pass does not exist
		
		SV2SVMap props = allprops.item( i ).value;
		HashTable< StringView, NoValue > inheritChain;
		inheritChain.set( allprops.item( i ).key, NoValue() );
		
		StringView inherit = props.getcopy( "Inherit", "" );
		while( inherit != "" )
		{
			if( inheritChain.isset( inherit ) )
			{
				LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
					"inheritance loop detected while trying to include " << inherit
					<< " from " << inheritChain.item( inheritChain.size() - 1 ).key;
				return false;
			}
			SV2SVMap* subprops = allprops.getptr( inherit );
			if( subprops == NULL )
			{
				LOG_ERROR << LOG_DATE << "  XShaderDef::LoadText failed - "
					"could not inherit " << inherit
					<< " from " << inheritChain.item( inheritChain.size() - 1 ).key;
				return false;
			}
			
			inheritChain.set( inherit, NoValue() );
			inherit = subprops->getcopy( "Inherit", "" );
			for( size_t i = 0; i < subprops->size(); ++i )
			{
				if( props.isset( subprops->item( i ).key ) == false )
					props.set( subprops->item( i ).key, subprops->item( i ).value );
			}
		}
		
		Pass pass;
		pass.name = allprops.item( i ).key;
		pass.render_state.Init();
		pass.order = 0;
		
		LoadPassInt( pass.order, "Order", 0, 0x7fff );
		
		SGRX_RenderState& rs = pass.render_state;
		LoadPassBool( rs.wireFill, "WireFill" );
		static const char* eCullModes[] = { "None", "Back", "Front", NULL };
		LoadPassEnum( rs.cullMode, "CullMode", eCullModes );
		LoadPassBool( rs.separateBlend, "SeparateBlend" );
		LoadPassBool( rs.scissorEnable, "ScissorEnable" );
		LoadPassBool( rs.multisampleEnable, "MultisampleEnable" );
		
		LoadPassBool( rs.depthEnable, "DepthEnable" );
		LoadPassBool( rs.depthWriteEnable, "DepthWriteEnable" );
		static const char* eDepthFuncs[] = { "Never", "Always", "Equal",
			"NotEqual", "Less", "LessEqual", "Greater", "GreaterEqual" };
		LoadPassEnum( rs.depthFunc, "DepthFunc", eDepthFuncs );
		LoadPassBool( rs.stencilEnable, "StencilEnable" );
		LoadPassInt( rs.stencilReadMask, "StencilReadMask", 0, 255 );
		LoadPassInt( rs.stencilWriteMask, "StencilWriteMask", 0, 255 );
		static const char* eStencilOps[] = { "Keep", "Zero", "Replace", 
			"Invert", "Incr", "Decr", "IncrSat", "DecrSat" };
		LoadPassEnum( rs.stencilFrontFailOp, "StencilFrontFailOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontDepthFailOp, "StencilFrontDepthFailOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontPassOp, "StencilFrontPassOp", eStencilOps );
		LoadPassEnum( rs.stencilFrontFunc, "StencilFrontFunc", eDepthFuncs );
		LoadPassEnum( rs.stencilBackFailOp, "StencilBackFailOp", eStencilOps );
		LoadPassEnum( rs.stencilBackDepthFailOp, "StencilBackDepthFailOp", eStencilOps );
		LoadPassEnum( rs.stencilBackPassOp, "StencilBackPassOp", eStencilOps );
		LoadPassEnum( rs.stencilBackFunc, "StencilBackFunc", eDepthFuncs );
		LoadPassInt( rs.stencilRef, "StencilRef", 0, 255 );
		
		LoadPassFloat( rs.depthBias, "DepthBias" );
		LoadPassFloat( rs.slopeDepthBias, "SlopeDepthBias" );
		LoadPassFloat( rs.depthBiasClamp, "DepthBiasClamp" );
		LoadPassVec4( rs.blendFactor, "BlendFactor" );
		
		for( int brtid = 0; brtid < 8; ++brtid )
		{
			SGRX_RenderState::BlendState& bs = rs.blendStates[ brtid ];
			
			static const char* eColorWrites[] = { "None",
				"R", "G", "RG", "B", "RB", "GB", "RGB",
				"A", "RA", "GA", "RGA", "BA", "RBA", "GBA", "RGBA" };
			static const char* eBlendOps[] = { "Add", "Sub", "RevSub", "Min", "Max" };
			static const char* eBlendFactors[] = { "Zero", "One", 
				"SrcColor", "InvSrcColor", "DstColor", "InvDstColor", 
				"SrcAlpha", "InvSrcAlpha", "DstAlpha", "InvDstAlpha", 
				"Factor", "InvFactor" };
			
			// generic version sets all
			LoadPassEnum( bs.colorWrite, "ColorWrite", eColorWrites );
			LoadPassBool( bs.blendEnable, "BlendEnable" );
			LoadPassEnum( bs.blendOp, "BlendOp", eBlendOps );
			LoadPassEnum( bs.srcBlend, "SrcBlend", eBlendFactors );
			LoadPassEnum( bs.dstBlend, "DstBlend", eBlendFactors );
			LoadPassEnum( bs.blendOpAlpha, "BlendOpAlpha", eBlendOps );
			LoadPassEnum( bs.srcBlendAlpha, "SrcBlendAlpha", eBlendFactors );
			LoadPassEnum( bs.dstBlendAlpha, "DstBlendAlpha", eBlendFactors );
			
			// specific version overrides one
			char bfr[ 32 ];
			char numchr = '0' + brtid;
			sgrx_snprintf( bfr, 32, "ColorWrite[%c]", numchr );
			LoadPassEnum( bs.colorWrite, bfr, eColorWrites );
			sgrx_snprintf( bfr, 32, "BlendEnable[%c]", numchr );
			LoadPassBool( bs.blendEnable, bfr );
			sgrx_snprintf( bfr, 32, "BlendOp[%c]", numchr );
			LoadPassEnum( bs.blendOp, bfr, eBlendOps );
			sgrx_snprintf( bfr, 32, "SrcBlend[%c]", numchr );
			LoadPassEnum( bs.srcBlend, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "DstBlend[%c]", numchr );
			LoadPassEnum( bs.dstBlend, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "BlendOpAlpha[%c]", numchr );
			LoadPassEnum( bs.blendOpAlpha, bfr, eBlendOps );
			sgrx_snprintf( bfr, 32, "SrcBlendAlpha[%c]", numchr );
			LoadPassEnum( bs.srcBlendAlpha, bfr, eBlendFactors );
			sgrx_snprintf( bfr, 32, "DstBlendAlpha[%c]", numchr );
			LoadPassEnum( bs.dstBlendAlpha, bfr, eBlendFactors );
		}
		
		passes.push_back( pass );
	}
	
	return true;
}

XShdInstHandle GR_CreateXShdInstance( const SGRX_XShaderDef& def )
{
	XShdInstHandle h = new SGRX_XShdInst;
	return h;
}





struct RenderPass
{
	bool isShadowPass;
	bool isBasePass;
	int numPL;
	int numSL;
};

void OnMakeRenderState( const RenderPass& pass, const SGRX_Material& mtl, SGRX_RenderState& out )
{
	out.cullMode = mtl.flags & SGRX_MtlFlag_Nocull ? SGRX_RS_CullMode_None : SGRX_RS_CullMode_Back;
	
	if( pass.isShadowPass == false )
	{
		bool decal = ( mtl.flags & SGRX_MtlFlag_Decal ) != 0;
		bool ltovr = pass.isBasePass == false && pass.isShadowPass == false;
		out.depthBias = decal ? -1e-5f : 0;
		out.depthWriteEnable = ( ltovr || decal || mtl.blendMode != SGRX_MtlBlend_None ) == false;
		out.blendStates[ 0 ].blendEnable = ltovr || mtl.blendMode != SGRX_MtlBlend_None;
		if( ltovr || mtl.blendMode == SGRX_MtlBlend_Additive )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_One;
		}
		else if( mtl.blendMode == SGRX_MtlBlend_Multiply )
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_Zero;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_SrcColor;
		}
		else
		{
			out.blendStates[ 0 ].srcBlend = SGRX_RS_Blend_SrcAlpha;
			out.blendStates[ 0 ].dstBlend = SGRX_RS_Blend_InvSrcAlpha;
		}
	}
	else
	{
		out.depthBias = 1e-5f;
		out.slopeDepthBias = 0.5f;
		out.cullMode = SGRX_RS_CullMode_None;
	}
}

void OnLoadMtlShaders( const RenderPass& pass,
	const StringView& defines, const SGRX_Material& mtl,
	const SGRX_MeshInstance* MI, VertexShaderHandle& VS, PixelShaderHandle& PS )
{
	if( pass.isBasePass == false && pass.isShadowPass == false &&
		( IS_FLAG_SET( mtl.flags, SGRX_MtlFlag_Unlit ) || MI->GetLightingMode() == SGRX_LM_Unlit ) )
	{
		PS = NULL;
		VS = NULL;
		return;
	}
	
	String name = "mtl:";
	name.append( mtl.shader );
	
	if( pass.isShadowPass )
		name.append( ":SHADOW_PASS" );
	else
	{
		char bfr[32];
		// lighting mode
		{
			int lmode = MI->GetLightingMode();
			if( mtl.flags & SGRX_MtlFlag_Unlit )
				lmode = SGRX_LM_Unlit;
			sgrx_snprintf( bfr, 32, "%d", lmode );
			name.append( ":LMODE " );
			name.append( bfr );
		}
		if( pass.isBasePass )
			name.append( ":BASE_PASS" );
		if( pass.numPL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numPL );
			name.append( ":NUM_POINTLIGHTS " );
			name.append( bfr );
		}
		if( pass.numSL )
		{
			sgrx_snprintf( bfr, 32, "%d", pass.numSL );
			name.append( ":NUM_SPOTLIGHTS " );
			name.append( bfr );
		}
	}
	
	// misc. parameters
	name.append( ":MOD_BLENDCOLOR 0" );
	name.append( ":" );
	name.append( defines ); // scene defines
	
	if( mtl.flags & SGRX_MtlFlag_VCol )
		name.append( ":VCOL" ); // color multiplied by vertex color
	
	PS = GR_GetPixelShader( name );
	
	if( MI->IsSkinned() )
		name.append( ":SKIN" );
	
	VS = GR_GetVertexShader( name );
}



XShdInstHandle GR_CreateXShdInstance( const SGRX_MeshInstance* MI, const SGRX_Material& mtl )
{
	static const RenderPass passes[] =
	{
		{ true, false, 0, 0 }, // shadow pass
		{ false, true, 16, 0 }, // base + 16 point lights
		{ false, false, 16, 0 }, // 16 point lights
		{ false, false, 0, 2 }, // 2 spotlights
	};
	
	XShdInstHandle h = new SGRX_XShdInst;
	
	for( int i = 0; i < (int) SGRX_ARRAY_SIZE( passes ); ++i )
	{
		SGRX_XShdInst::Pass pass;
		pass.order = i * 1000 + 1500;
		
		// load render state
		SGRX_RenderState rs;
		rs.Init();
		OnMakeRenderState( passes[ i ], mtl, rs );
		pass.renderState = GR_GetRenderState( rs );
		
		// load shaders
		OnLoadMtlShaders( passes[ i ], "", mtl, MI,
			pass.vertexShader, pass.pixelShader );
		
		pass.vtxInputMap = GR_GetVertexInputMapping(
			pass.vertexShader, MI->GetMesh()->m_vertexDecl );
		
		h->passes.push_back( pass );
	}
	
	return h;
}



static bool OnLoadShaderFile( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata );
static bool ParseShaderIncludes( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata );

static void GetShaderCacheFilename( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, String& name )
{
	LOG_FUNCTION;
	
	name = SGRXPATH_CACHE_SHADERS;
	name.append( rinfo.shaderCacheSfx );
	name.append( "/" );
	
	StringView it = key;
	while( it.size() )
	{
		char ch = it.ch();
		it.skip(1);
		
		if( ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || ch == '_' )
			name.push_back( ch );
		else if( name.last() != '$' )
			name.push_back( '$' );
	}
	
	name.append( sfx );
	name.append( ".csh" );
}

static bool GetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, ByteArray& outdata )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( rinfo, sfx, key, filename );
	
	if( VERBOSE ) LOG << "Loading precompiled shader: " << filename << " (type=" << rinfo.shaderCacheSfx << ", key=" << key << ")";
	return FS_LoadBinaryFile( filename, outdata );
}

static bool SetCompiledShader( const SGRX_RendererInfo& rinfo, const char* sfx, const StringView& key, const ByteArray& data )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	String filename;
	GetShaderCacheFilename( rinfo, sfx, key, filename );
	
	if( VERBOSE ) LOG << "Saving precompiled shader: " << filename << " (type=" << rinfo.shaderCacheSfx << ", key=" << key << ")";
	return FS_SaveBinaryFile( filename, data.data(), data.size() );
}

static bool OnLoadShader( const SGRX_RendererInfo& rinfo, const StringView& key, String& outdata )
{
	LOG_FUNCTION;
	
	if( !key )
		return false;
	
	if( key.part( 0, 4 ) == "mtl:" )
	{
		int i = 0;
		String prepend;
		StringView mtl, cur, it = key.part( 4 );
		while( it.size() )
		{
			i++;
			cur = it.until( ":" );
			if( i == 1 )
			{
				mtl = cur;
			}
			else if( cur.size() )
			{
				prepend.append( STRLIT_BUF( "#define " ) );
				prepend.append( cur.data(), cur.size() );
				prepend.append( STRLIT_BUF( "\n" ) );
			}
			it.skip( cur.size() + 1 );
		}
		
		String mtl_data;
		if( !OnLoadShaderFile( rinfo, String_Concat( "mtl_", mtl ), mtl_data ) )
			return false;
		
		outdata.clear();
		outdata.reserve( prepend.size() + mtl_data.size() );
		outdata.append( prepend );
		outdata.append( mtl_data );
		return true;
	}
	return OnLoadShaderFile( rinfo, key, outdata );
}

static bool OnLoadShaderFile( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata )
{
	LOG_FUNCTION;
	
	String filename = SGRXPATH_SRC_SHADERS;
	filename.push_back( '/' );
	filename.append( path.data(), path.size() );
	filename.append( STRLIT_BUF( ".shd" ) );
	
	if( !FS_LoadTextFile( filename, outdata ) )
	{
		LOG_WARNING << "Failed to load shader file: " << filename << " (type=" << rinfo.shaderCacheSfx << ", path=" << path << ")";
		return false;
	}
	
	char bfr[ 400 ];
	sgrx_snprintf( bfr, 400, "#define %s\n#line 1 \"%s\"\n",
		StackString<100>(rinfo.shaderTypeDefine).str, StackPath(path).str );
	outdata.insert( 0, bfr, sgrx_snlen( bfr, 400 ) );
	
	return ParseShaderIncludes( rinfo, path, outdata );
}

static bool ParseShaderIncludes( const SGRX_RendererInfo& rinfo, const StringView& path, String& outdata )
{
	LOG_FUNCTION;
	
	String basepath = path.up_to_last( "/" ).str();
	String nstr;
	StringView it = outdata, inc;
	while( ( inc = it.from( "#include" ) ) != StringView() )
	{
		// generate path
		String incstr = inc.after( "\"" ).until( "\"" ).str();
		String incpath = String_Concat( basepath, incstr );
		
		// append prior data
		nstr.append( it.data(), inc.data() - it.data() );
		
		// contents of new file (includes starting directive)
		String incfiledata;
		if( !OnLoadShaderFile( rinfo, incpath, incfiledata ) )
			return false;
		nstr.append( incfiledata );
		
		// directive for original file
		int linenum = StringView(outdata).part( 0, inc.data() - outdata.data() ).count( "\n" ) + 1;
		char bfr[ 32 ];
		sgrx_snprintf( bfr, 32, "%d", linenum );
		nstr.append( "#line " );
		nstr.append( bfr );
		nstr.append( " \"" );
		nstr.append( path );
		nstr.append( "\"\n" );
		
		// continue
		it = inc.after( "\"" ).after( "\"" );
	}
	nstr.append( it );
	
	outdata = nstr;
	return true;
}



SGRX_IVertexShader::~SGRX_IVertexShader()
{
	g_VertexShaders->unset( m_key );
}

SGRX_IPixelShader::~SGRX_IPixelShader()
{
	g_PixelShaders->unset( m_key );
}


VertexShaderHandle GR_GetVertexShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IVertexShader* shd = g_VertexShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( GetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find vertex shader: " << path;
		return VertexShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Vertex, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile vertex shader: " << path;
			LOG << errors;
			LOG << "---";
			exit(1);
			return VertexShaderHandle();
		}
		
		SetCompiledShader( g_Renderer->GetInfo(), ".vs", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreateVertexShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreateVertexShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_VertexShaders->set( shd->m_key, shd );
	
	if( VERBOSE ) LOG << "Loaded vertex shader: " << path;
	return shd;
}

PixelShaderHandle GR_GetPixelShader( const StringView& path )
{
	LOG_FUNCTION_ARG( path );
	
	String code;
	String errors;
	ByteArray comp;
	
	SGRX_IPixelShader* shd = g_PixelShaders->getcopy( path );
	if( shd )
		return shd;
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( GetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp ) )
		{
			goto has_compiled_shader;
		}
	}
	
	if( !OnLoadShader( g_Renderer->GetInfo(), path, code ) )
	{
		LOG_ERROR << LOG_DATE << "  Could not find pixel shader: " << path;
		return PixelShaderHandle();
	}
	
	if( g_Renderer->GetInfo().compileShaders )
	{
		if( !g_Renderer->CompileShader( path, ShaderType_Pixel, code, comp, errors ) )
		{
			LOG_ERROR << LOG_DATE << "  Failed to compile pixel shader: " << path;
			LOG << errors;
			LOG << "---";
			exit(1);
			return PixelShaderHandle();
		}
		
		SetCompiledShader( g_Renderer->GetInfo(), ".ps", path, comp );
		
has_compiled_shader:
		shd = g_Renderer->CreatePixelShader( path, comp );
	}
	else
	{
		// TODO: I know...
		ByteArray bcode;
		bcode.resize( code.size() );
		memcpy( bcode.data(), code.data(), code.size() );
		shd = g_Renderer->CreatePixelShader( path, bcode );
	}
	
	if( !shd )
	{
		// error already printed in renderer
		return NULL;
	}
	
	shd->m_key = path;
	g_PixelShaders->set( shd->m_key, shd );
	
	if( VERBOSE ) LOG << "Loaded pixel shader: " << path;
	return shd;
}



SGRX_IRenderState::~SGRX_IRenderState()
{
	g_RenderStates->unset( m_info );
}

void SGRX_IRenderState::SetState( const SGRX_RenderState& state )
{
	m_info = state;
}

const SGRX_RenderState& RenderStateHandle::GetInfo()
{
	static SGRX_RenderState dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}


RenderStateHandle GR_GetRenderState( const SGRX_RenderState& state )
{
	LOG_FUNCTION;
	
	SGRX_IRenderState* rs = g_RenderStates->getcopy( state );
	if( rs )
		return rs;
	
	rs = g_Renderer->CreateRenderState( state );
	rs->m_info = state;
	g_RenderStates->set( state, rs );
	
	if( VERBOSE ) LOG << "Created render state";
	return rs;
}



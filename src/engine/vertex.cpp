

#include "engine_int.hpp"

extern IRenderer* g_Renderer;
extern VertexDeclHashTable* g_VertexDecls;
extern VtxInputMapHashTable* g_VtxInputMaps;



#define USAGE_PADDING 723
const char* VDeclInfo_Parse( VDeclInfo* info, const char* text )
{
	LOG_FUNCTION;
	
	int offset = 0, count = 0, align32 = 0;
	if( *text == '|' )
	{
		align32 = 1;
		text++;
	}
	while( *text )
	{
		int usage = -1, type = -1, size = -1;
		char chr_usage, chr_type, chr_mult;
		chr_usage = *text++; if( !chr_usage ) return "unexpected end of sequence";
		chr_type = *text++; if( !chr_type ) return "unexpected end of sequence";
		chr_mult = *text++; if( !chr_mult ) return "unexpected end of sequence";
		
		if( chr_usage == 'p' ) usage = VDECLUSAGE_POSITION;
		else if( chr_usage == 'c' ) usage = VDECLUSAGE_COLOR;
		else if( chr_usage == 'n' ) usage = VDECLUSAGE_NORMAL;
		else if( chr_usage == 't' ) usage = VDECLUSAGE_TANGENT;
		else if( chr_usage == 'w' ) usage = VDECLUSAGE_BLENDWT;
		else if( chr_usage == 'i' ) usage = VDECLUSAGE_BLENDIDX;
		else if( chr_usage == '0' ) usage = VDECLUSAGE_TEXTURE0;
		else if( chr_usage == '1' ) usage = VDECLUSAGE_TEXTURE1;
		else if( chr_usage == '2' ) usage = VDECLUSAGE_TEXTURE2;
		else if( chr_usage == '3' ) usage = VDECLUSAGE_TEXTURE3;
		else if( chr_usage == 'x' ) usage = USAGE_PADDING;
		else return "usage type specifier not recognized";
		
		if( chr_type == 'f' )
		{
			if( chr_mult == '1' ){ type = VDECLTYPE_FLOAT1; size = 4; }
			else if( chr_mult == '2' ){ type = VDECLTYPE_FLOAT2; size = 8; }
			else if( chr_mult == '3' ){ type = VDECLTYPE_FLOAT3; size = 12; }
			else if( chr_mult == '4' ){ type = VDECLTYPE_FLOAT4; size = 16; }
			else return "invalid multiplier";
		}
		else if( chr_type == 'b' )
		{
			if( chr_mult == '1' || chr_mult == '2' || chr_mult == '3' )
				return "type/multiplier combo not supported";
			else if( chr_mult == '4' ){ type = VDECLTYPE_BCOL4; size = 4; }
			else return "invalid multiplier";
		}
		else return "invalid data type specified";
		
		if( usage != USAGE_PADDING )
		{
			info->offsets[ count ] = offset;
			info->types[ count ] = type;
			info->usages[ count ] = usage;
			count++;
		}
		offset += size;
	}
	
	if( align32 )
		offset = ( ( offset + 31 ) / 32 ) * 32;
	
	if( offset == 0 )
		return "vertex is empty";
	if( offset > 255 )
		return "vertex size too big (> 255)";
	
	info->count = count;
	info->size = offset;
	
	return NULL;
}

bool GetAABBFromVertexData( const VDeclInfo& info, const char* vdata, size_t vdsize, Vec3& outMin, Vec3& outMax )
{
	int i;
	const char* vdend;
	float tmp1, tmp2;
	Vec3 tmpv;
	uint8_t* pudata;
	if( vdsize < info.size )
		goto fail;
	
	for( i = 0; i < info.count; ++i )
	{
		if( info.usages[ i ] == VDECLUSAGE_POSITION )
			break;
	}
	if( i == info.count )
		goto fail;
	
	vdata += info.offsets[ i ];
	vdend = vdata + vdsize;
	switch( info.types[ i ] )
	{
	case VDECLTYPE_FLOAT1: /* read 1 float, y;z=0;0 */
		tmp1 = ((float*) vdata)[0];
		outMin.Set( tmp1, 0, 0 );
		outMax.Set( tmp1, 0, 0 );
		vdata += info.size;
		while( vdata + info.size <= vdend )
		{
			tmp1 = ((float*) vdata)[0];
			if( outMin.x > tmp1 ) outMin.x = tmp1;
			if( outMax.x < tmp1 ) outMax.x = tmp1;
			vdata += info.size;
		}
		break;
	case VDECLTYPE_FLOAT2: /* read 2 floats, z=0 */
		tmp1 = ((float*) vdata)[0];
		tmp2 = ((float*) vdata)[1];
		outMin.Set( tmp1, tmp2, 0 );
		outMax.Set( tmp1, tmp2, 0 );
		vdata += info.size;
		while( vdata + info.size <= vdend )
		{
			tmp1 = ((float*) vdata)[0];
			tmp2 = ((float*) vdata)[1];
			if( outMin.x > tmp1 ) outMin.x = tmp1;
			if( outMax.x < tmp1 ) outMax.x = tmp1;
			if( outMin.y > tmp2 ) outMin.y = tmp2;
			if( outMax.y < tmp2 ) outMax.y = tmp2;
			vdata += info.size;
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4: /* read 3 floats */
		tmpv = Vec3::CreateFromPtr( (float*) vdata );
		outMin = tmpv;
		outMax = tmpv;
		vdata += info.size;
		while( vdata + info.size <= vdend )
		{
			tmpv = Vec3::CreateFromPtr( (float*) vdata );
			outMin = Vec3::Min( outMin, tmpv );
			outMax = Vec3::Max( outMax, tmpv );
			vdata += info.size;
		}
		break;
	case VDECLTYPE_BCOL4: /* read u8[3] */
		pudata = (uint8_t*) vdata;
		tmpv.Set( pudata[0] * (1.0f/255.0f), pudata[1] * (1.0f/255.0f), pudata[2] * (1.0f/255.0f) );
		outMin = tmpv;
		outMax = tmpv;
		vdata += info.size;
		while( vdata + info.size <= vdend )
		{
			tmpv.Set( pudata[0] * (1.0f/255.0f), pudata[1] * (1.0f/255.0f), pudata[2] * (1.0f/255.0f) );
			outMin = Vec3::Min( outMin, tmpv );
			outMax = Vec3::Max( outMax, tmpv );
			vdata += info.size;
		}
		break;
	}
	
	return true;
	
fail:
	outMin.Set( 0, 0, 0 );
	outMax.Set( 0, 0, 0 );
	return false;
}


void VDeclInfo::TransformVertices( Mat4 xf, void* data, size_t vtxCount ) const
{
	SGRX_CAST( uint8_t*, bytes, data );
	for( size_t i = 0; i < vtxCount; ++i )
	{
		for( uint8_t e = 0; e < count; ++e )
		{
			if( usages[ e ] != VDECLUSAGE_POSITION &&
				usages[ e ] != VDECLUSAGE_NORMAL &&
				usages[ e ] != VDECLUSAGE_TANGENT )
				continue;
			if( types[ e ] == VDECLTYPE_BCOL4 )
				continue;
			
			uint8_t type = types[ e ];
			if( usages[ e ] == VDECLUSAGE_TANGENT )
				type = VDECLTYPE_FLOAT3;
			Vec4 value = { 0, 0, 0, 1 };
			switch( type )
			{
			case VDECLTYPE_FLOAT4:
				value.w = ((float*)(bytes + offsets[ e ]))[3];
			case VDECLTYPE_FLOAT3:
				value.z = ((float*)(bytes + offsets[ e ]))[2];
			case VDECLTYPE_FLOAT2:
				value.y = ((float*)(bytes + offsets[ e ]))[1];
			case VDECLTYPE_FLOAT1:
				value.x = ((float*)(bytes + offsets[ e ]))[0];
			}
			
			if( usages[ e ] != VDECLUSAGE_POSITION )
				value.w = 0;
			value = xf.Transform( value );
			if( usages[ e ] == VDECLUSAGE_POSITION && value.w != 0 )
				value /= value.w;
			
			switch( type )
			{
			case VDECLTYPE_FLOAT4:
				((float*)(bytes + offsets[ e ]))[3] = value.w;
			case VDECLTYPE_FLOAT3:
				((float*)(bytes + offsets[ e ]))[2] = value.z;
			case VDECLTYPE_FLOAT2:
				((float*)(bytes + offsets[ e ]))[1] = value.y;
			case VDECLTYPE_FLOAT1:
				((float*)(bytes + offsets[ e ]))[0] = value.x;
			}
		}
		bytes += size;
	}
}

void VDeclInfo::TransformTexcoords( Vec4 mul, Vec4 add, void* data, size_t vtxCount ) const
{
	SGRX_CAST( uint8_t*, bytes, data );
	for( size_t i = 0; i < vtxCount; ++i )
	{
		for( uint8_t e = 0; e < count; ++e )
		{
			if( usages[ e ] != VDECLUSAGE_TEXTURE0 &&
				usages[ e ] != VDECLUSAGE_TEXTURE1 &&
				usages[ e ] != VDECLUSAGE_TEXTURE2 &&
				usages[ e ] != VDECLUSAGE_TEXTURE3 )
				continue;
			
			Vec4 value = { 0, 0, 0, 0 };
			switch( types[ e ] )
			{
			case VDECLTYPE_BCOL4:
				value = Col32ToVec4( *(uint32_t*)(bytes + offsets[ e ]) );
				break;
			case VDECLTYPE_FLOAT4:
				value.w = ((float*)(bytes + offsets[ e ]))[3];
			case VDECLTYPE_FLOAT3:
				value.z = ((float*)(bytes + offsets[ e ]))[2];
			case VDECLTYPE_FLOAT2:
				value.y = ((float*)(bytes + offsets[ e ]))[1];
			case VDECLTYPE_FLOAT1:
				value.x = ((float*)(bytes + offsets[ e ]))[0];
			}
			
			value = value * mul + add;
			
			switch( types[ e ] )
			{
			case VDECLTYPE_BCOL4:
				*(uint32_t*)(bytes + offsets[ e ]) = Vec4ToCol32( value );
				break;
			case VDECLTYPE_FLOAT4:
				((float*)(bytes + offsets[ e ]))[3] = value.w;
			case VDECLTYPE_FLOAT3:
				((float*)(bytes + offsets[ e ]))[2] = value.z;
			case VDECLTYPE_FLOAT2:
				((float*)(bytes + offsets[ e ]))[1] = value.y;
			case VDECLTYPE_FLOAT1:
				((float*)(bytes + offsets[ e ]))[0] = value.x;
			}
		}
		bytes += size;
	}
}


bool VD_ExtractFloat1( const VDeclInfo& vdinfo, int vcount, const void* verts, float* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(float*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *((uint8_t*)verts+stride*i+ofs)/255.0f;
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat2( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec2* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2( *(float*)((uint8_t*)verts+stride*i+ofs), 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec2*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V2(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat4( const VDeclInfo& vdinfo, int vcount, const void* verts, Vec4* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec4*)((uint8_t*)verts+stride*i+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f );
		}
		break;
	}
	return true;
}

bool VD_ExtractByte4Clamped( const VDeclInfo& vdinfo, int vcount, const void* verts, uint32_t* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage ), stride = vdinfo.size;
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4( *(float*)((uint8_t*)verts+stride*i+ofs), 0, 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4), 0, 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT3:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*(float*)((uint8_t*)verts+stride*i+ofs),
				*(float*)((uint8_t*)verts+stride*i+ofs+4),
				*(float*)((uint8_t*)verts+stride*i+ofs+8), 0 ) );
		}
		break;
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( *(Vec4*)((uint8_t*)verts+stride*i+ofs) );
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = Vec4ToCol32( V4(
				*((uint8_t*)verts+stride*i+ofs)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+1)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+2)/255.0f,
				*((uint8_t*)verts+stride*i+ofs+3)/255.0f ) );
		}
		break;
	}
	return true;
}

bool VD_ExtractFloat3P( const VDeclInfo& vdinfo, int vcount, const void** vertptrs, Vec3* outp, int usage )
{
	int ty = vdinfo.GetType( usage ), ofs = vdinfo.GetOffset( usage );
	if( ty == -1 || ofs == -1 )
		return false;
	
	switch( ty )
	{
	case VDECLTYPE_FLOAT1:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3( *(float*)((uint8_t*)vertptrs[i]+ofs), 0, 0 );
		}
		break;
	case VDECLTYPE_FLOAT2:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*(float*)((uint8_t*)vertptrs[i]+ofs),
				*(float*)((uint8_t*)vertptrs[i]+ofs+4), 0 );
		}
		break;
	case VDECLTYPE_FLOAT3:
	case VDECLTYPE_FLOAT4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = *(Vec3*)((uint8_t*)vertptrs[i]+ofs);
		}
		break;
	case VDECLTYPE_BCOL4:
		for( int i = 0; i < vcount; ++i )
		{
			outp[ i ] = V3(
				*((uint8_t*)vertptrs[i]+ofs)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+1)/255.0f,
				*((uint8_t*)vertptrs[i]+ofs+2)/255.0f );
		}
		break;
	}
	return true;
}

void VD_LerpTri( const VDeclInfo& vdinfo, int vcount, void* outbuf, Vec3* factors, const void* v1, const void* v2, const void* v3 )
{
	SGRX_CAST( char*, coutbuf, outbuf );
	SGRX_CAST( const char*, cv1, v1 );
	SGRX_CAST( const char*, cv2, v2 );
	SGRX_CAST( const char*, cv3, v3 );
	for( int i = 0; i < vdinfo.count; ++i )
	{
		int off = vdinfo.offsets[ i ];
		char* ocoutbuf = coutbuf + off;
		const char *ocv1 = cv1 + off, *ocv2 = cv2 + off, *ocv3 = cv3 + off;
		
		switch( vdinfo.types[ i ] )
		{
		case VDECLTYPE_FLOAT1:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(float*)( ocoutbuf + v * vdinfo.size ) = *(float*)ocv1 * f.x + *(float*)ocv2 * f.y + *(float*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT2:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec2*)( ocoutbuf + v * vdinfo.size ) = *(Vec2*)ocv1 * f.x + *(Vec2*)ocv2 * f.y + *(Vec2*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT3:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec3*)( ocoutbuf + v * vdinfo.size ) = *(Vec3*)ocv1 * f.x + *(Vec3*)ocv2 * f.y + *(Vec3*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_FLOAT4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(Vec4*)( ocoutbuf + v * vdinfo.size ) = *(Vec4*)ocv1 * f.x + *(Vec4*)ocv2 * f.y + *(Vec4*)ocv3 * f.z;
			}
			break;
		case VDECLTYPE_BCOL4:
			for( int v = 0; v < vcount; ++v )
			{
				const Vec3& f = factors[ v ];
				*(BVec4*)( ocoutbuf + v * vdinfo.size ) = *(BVec4*)ocv1 * f.x + *(BVec4*)ocv2 * f.y + *(BVec4*)ocv3 * f.z;
			}
			break;
		}
	}
}



SGRX_IVertexDecl::~SGRX_IVertexDecl()
{
	g_VertexDecls->unset( m_key );
}

const VDeclInfo& VertexDeclHandle::GetInfo()
{
	static VDeclInfo dummy_info = {0};
	if( !item )
		return dummy_info;
	return item->m_info;
}

SGRX_IVertexInputMapping::~SGRX_IVertexInputMapping()
{
	g_VtxInputMaps->unset( m_key );
}


VertexDeclHandle GR_GetVertexDecl( const StringView& vdecl )
{
	LOG_FUNCTION_ARG( vdecl );
	
	SGRX_IVertexDecl* VD = g_VertexDecls->getcopy( vdecl );
	if( VD )
		return VD;
	
	VDeclInfo vdinfo = {0};
	const char* err = VDeclInfo_Parse( &vdinfo, StackString< 64 >( vdecl ) );
	if( err )
	{
		LOG_ERROR << LOG_DATE << "  Failed to parse vertex declaration - " << err << " (" << vdecl << ")";
		return NULL;
	}
	
	VD = g_Renderer->CreateVertexDecl( vdinfo );
	if( !VD )
	{
		// error already printed in renderer
		return NULL;
	}
	
	VD->m_key = vdecl;
	g_VertexDecls->set( VD->m_key, VD );
	
	if( VERBOSE ) LOG << "Created vertex declaration: " << vdecl;
	return VD;
}


VtxInputMapHandle GR_GetVertexInputMapping( SGRX_IVertexShader* vs, SGRX_IVertexDecl* vd )
{
	LOG_FUNCTION;
	
	if( vs == NULL || vd == NULL )
		return NULL;
	
	SGRX_VtxInputMapKey key = { vs, vd };
	SGRX_IVertexInputMapping* vim = g_VtxInputMaps->getcopy( key );
	if( vim )
		return vim;
	
	vim = g_Renderer->CreateVertexInputMapping( vs, vd );
	if( vim == NULL )
	{
		// valid outcome
		return vim;
	}
	
	vim->m_key = key;
	g_VtxInputMaps->set( key, vim );
	
	if( VERBOSE ) LOG << "Created vertex input mapping";
	return vim;
}


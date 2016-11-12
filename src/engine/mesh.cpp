

#define __STDC_FORMAT_MACROS 1

#include "renderer.hpp"


//
// MESH PARSING

/* FORMAT
	BUFFER: (min size = 4)
	- uint32 size
	- uint8 data[size]
	
	SMALLBUF: (min size = 1)
	- uint8 size
	- uint8 data[size]
	
	PART: (min size = 19)
	- uint32 voff
	- uint32 vcount
	- uint32 ioff
	- uint32 icount
	- uint8 texcount
	- smallbuf shader
	- smallbuf textures[texcount]
	
	MESH:
	- magic "SS3DMESH"
	- uint32 flags
	
	- float boundsmin[3]
	- float boundsmax[3]
	
	- buffer vdata
	- buffer idata
	- smallbuf format
	- uint8 numparts
	- part parts[numparts]
	
	minimum size = 12+24+10 = 46
*/

static int md_parse_buffer( char* buf, size_t size, char** outptr, uint32_t* outsize )
{
	if( size < 4 )
		return 0;
	memcpy( outsize, buf, 4 );
	if( size < *outsize + 4 )
		return 0;
	*outptr = buf + 4;
	return 1;
}

static int md_parse_smallbuf( char* buf, size_t size, char** outptr, uint8_t* outsize )
{
	if( size < 1 )
		return 0;
	memcpy( outsize, buf, 1 );
	if( size < (size_t) *outsize + 1 )
		return 0;
	*outptr = buf + 1;
	return 1;
}

const char* MeshData_Parse( char* buf, size_t size, MeshFileData* out )
{
	uint8_t p, t, b;
	size_t off = 0;
	if( size < 46 || memcmp( buf, "SS3DMESH", 8 ) != 0 )
		return "file too small or not SS3DMESH";
	memcpy( &out->dataFlags, buf + 8, 4 );
	memcpy( &out->boundsMin, buf + 12, 12 );
	memcpy( &out->boundsMax, buf + 24, 12 );
	off = 36;
	if( !md_parse_buffer( buf + off, size - off, &out->vertexData, &out->vertexDataSize ) )
		return "failed to parse VDATA buffer";
	off += 4 + out->vertexDataSize;
	if( !md_parse_buffer( buf + off, size - off, &out->indexData, &out->indexDataSize ) )
		return "failed to parse IDATA buffer";
	off += 4 + out->indexDataSize;
	if( !md_parse_smallbuf( buf + off, size - off, &out->formatData, &out->formatSize ) )
		return "failed to parse FORMAT buffer";
	off += 1 + out->formatSize;
	if( off >= size )
		return "mesh incomplete (missing part data)";
	out->numParts = (uint8_t) buf[ off++ ];
	if( out->numParts > MAX_MESH_FILE_PARTS )
		return "invalid part count";
	for( p = 0; p < out->numParts; ++p )
	{
		MeshFilePartData* pout = out->parts + p;
		memset( pout, 0, sizeof(*pout) );
		if( out->dataFlags & MDF_MTLINFO )
		{
			if( off + 20 > size )
				return "mesh incomplete (corrupted part data)";
			pout->flags = buf[ off++ ];
			pout->blendMode = buf[ off++ ];
		}
		if( out->dataFlags & MDF_PARTNAMES )
		{
			if( off + 19 + 16 * 4 > size )
				return "mesh incomplete (corrupted part data)";
			if( !md_parse_smallbuf( buf + off, size - off, &pout->nameStr, &pout->nameStrSize ) )
				return "failed to parse part name";
			off += 1 + pout->nameStrSize;
			// node transform matrix
			memcpy( &pout->nodeTransform, buf + off, 16 * 4 );
			off += 16 * 4;
		}
		if( off + 18 > size )
			return "mesh incomplete (corrupted part data)";
		memcpy( &pout->vertexOffset, buf + off, 4 ); off += 4;
		memcpy( &pout->vertexCount, buf + off, 4 ); off += 4;
		memcpy( &pout->indexOffset, buf + off, 4 ); off += 4;
		memcpy( &pout->indexCount, buf + off, 4 ); off += 4;
		memcpy( &pout->materialTextureCount, buf + off, 1 ); off += 1;
		for( t = 0; t < pout->materialTextureCount + 1; ++t )
		{
			if( !md_parse_smallbuf( buf + off, size - off, &pout->materialStrings[t], &pout->materialStringSizes[t] ) )
				return "failed to parse material string buffer";
			off += 1 + pout->materialStringSizes[t];
		}
	}
	if( out->dataFlags & MDF_SKINNED )
	{
		if( off >= size )
			return "mesh incomplete (missing bone data)";
		out->numBones = (uint8_t) buf[ off++ ];
		if( out->numBones > SGRX_MAX_MESH_BONES )
			return "invalid bone count";
		for( b = 0; b < out->numBones; ++b )
		{
			MeshFileBoneData* bout = out->bones + b;
			memset( bout, 0, sizeof(*bout) );
			if( !md_parse_smallbuf( buf + off, size - off, &bout->boneName, &bout->boneNameSize ) )
				return "failed to parse bone name string buffer";
			off += 1 + bout->boneNameSize;
			if( off >= size )
				return "mesh bone data incomplete [parent id]";
			bout->parent_id = buf[ off++ ];
			// printf( "pid = %d, namesize = %d, name = %.*s\n", (int)bout->parent_id, (int)bout->boneNameSize, (int)bout->boneNameSize, bout->boneName );
			if( off + 64 > size )
				return "mesh bone data incomplete [matrix]";
			memcpy( bout->boneOffset.a, buf + off, sizeof(Mat4) /* 64 */ );
			off += 64;
		}
	}
	else
		out->numBones = 0;
	return NULL;
}


const char* AnimFileParser::Parse( ByteReader& br )
{
	uint32_t numAnims, sectionLength = 0;
	br.marker( "SS3DANIM" );
	if( br.error )
		return "file not SS3DANIM";
	br << numAnims;
	for( uint32_t i = 0; i < numAnims; ++i )
	{
		br << sectionLength;
		
		Anim anim;
		br << anim.nameSize;
		anim.name = (char*) br.at();
		br.padding( anim.nameSize );
		br << anim.frameCount;
		br << anim.speed;
		br << anim.trackCount;
		anim.trackDataOff = trackData.size();
		br << anim.markerCount;
		anim.markerDataOff = markerData.size();
		
		if( br.error )
			return "failed to read animation data";
		
		for( uint8_t t = 0; t < anim.trackCount; ++t )
		{
			br << sectionLength;
			
			Track track;
			br << track.nameSize;
			track.name = (char*) br.at();
			br.padding( track.nameSize );
			
			track.dataPtr = (float*) br.at();
			br.padding( sizeof( float ) * 10 * anim.frameCount );
			
			if( br.error )
				return "failed to read track data";
			
			trackData.push_back( track );
		}
		
		for( uint8_t m = 0; m < anim.markerCount; ++m )
		{
			Marker marker;
			
			br.memory( marker.name, MAX_ANIM_MARKER_NAME_LENGTH );
			br << marker.frame;
			
			if( br.error )
				return "failed to read marker data";
			
			markerData.push_back( marker );
		}
		
		animData.push_back( anim );
	}
	return NULL;
}


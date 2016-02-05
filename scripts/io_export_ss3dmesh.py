
import bpy
from bpy.props import *
from mathutils import *
from pprint import pprint
from copy import copy
from itertools import islice
import math, struct, csv, sys
import os.path


""" FORMAT
	
	BUFFER: (min size = 4)
	- uint32 size
	- uint8 data[size]
	
	SMALLBUF: (min size = 1)
	- uint8 size
	- uint8 data[size]
	
	PART: (min size = 20)
	- uint8 flags
	- uint8 blendmode
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
"""


bl_info = {
	"name": "SS3DMESH Mesh Format (.ssm)",
	"author": "Arvīds Kokins",
	"version": (0, 5, 2),
	"blender": (2, 6, 9),
	"api": 38019,
	"location": "File > Export > SS3DMESH (.ssm)",
	"description": "SS3DMESH Mesh Export (.ssm)",
	"warning": "",
	"wiki_url": "http://cragegames.com",
	"tracker_url": "http://cragegames.com",
	"category": "Import-Export"
}


def write_smallbuf( f, bytebuf ):
	if len( bytebuf ) > 255:
		raise Exception( "smallbuf too big" )
	if type( bytebuf ) == str:
		bytebuf = bytes( bytebuf, "UTF-8" )
	f.write( struct.pack( "B", len( bytebuf ) ) )
	f.write( bytebuf )
#

def write_buffer( f, bytebuf ):
	if len( bytebuf ) > 0xffffffff:
		raise Exception( "buffer too big" )
	if type( bytebuf ) == str:
		bytebuf = bytes( bytebuf, "UTF-8" )
	f.write( struct.pack( "L", len( bytebuf ) ) )
	f.write( bytebuf )
#

def serialize_matrix( m ):
	return struct.pack( "16f",
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3] )

def write_part( f, part ):
	if len( part["textures"] ) > 8:
		raise Exception( "too many textures (max. 8 allowed)" )
	f.write( struct.pack( "=BB", part["flags"], part["blendmode"] ) )
	write_smallbuf( f, part["name"] )
	f.write( serialize_matrix( part[ "mtx" ] ) )
	f.write( struct.pack( "=LLLLB", part["voff"], part["vcount"],
		part["ioff"], part["icount"], len( part["textures"] ) ) )
	write_smallbuf( f, bytes( part["shader"], "UTF-8" ) )
	for tex in part["textures"]:
		write_smallbuf( f, tex.replace("\\", "/") )
#

magicmtx = Matrix.Rotation( -math.pi/2, 4, "X" )

def write_mesh( f, meshdata, armdata, boneorder ):
	is_skinned = armdata != None
	bbmin = meshdata["bbmin"]
	bbmax = meshdata["bbmax"]
	vertices = meshdata["vertices"]
	indices = meshdata["indices"]
	format = meshdata["format"]
	parts = meshdata["parts"]
	
	if len( parts ) > 255:
		raise Exception( "too many parts (max. 255 allowed)" )
	
	is_i32 = len( vertices ) > 65535
	
	print( "--- MESH STATS ---" )
	print( "Vertex count: %d" % ( len(vertices) ) )
	print( "Index count: %d" % ( len(indices) ) )
	print( "Format string: " + format )
	print( "Part count: %d" % ( len(parts) ) )
	for part_id, part in enumerate( parts ):
		print( "- part %d: name=%s voff=%d vcount=%d ioff=%d icount=%d flags=%d blendmode=%d texcount=%d shader='%s'" % (
			part_id, part["name"], part["voff"], part["vcount"], part["ioff"], part["icount"],
			part["flags"], part["blendmode"], len( part["textures"] ), part["shader"] ) )
	
	f.write( bytes( "SS3DMESH", "UTF-8" ) )
	# mesh data flags, 0x100 = extended mtl data, 0x200 = part name data
	f.write( struct.pack( "L", 0x100 + 0x200 +
		(1 if is_i32 else 0) * 0x01 + \
		(1 if is_skinned else 0) * 0x80 ) )
	f.write( struct.pack( "6f", bbmin.x, bbmin.y, bbmin.z, bbmax.x, bbmax.y, bbmax.z ) )
	
	vdata = bytes()
	for vertex in vertices:
		vdata += vertex
	write_buffer( f, vdata )
	
	idata = bytes()
	if( is_i32 ):
		for index in indices:
			idata += struct.pack( "L", index )
	else:
		for index in indices:
			idata += struct.pack( "H", index )
	write_buffer( f, idata )
	
	write_smallbuf( f, format )
	f.write( struct.pack( "B", len(parts) ) )
	for part in parts:
		write_part( f, part )
	
	if is_skinned:
		f.write( struct.pack( "B", len(boneorder) ) )
		for bonename in boneorder:
			bone = armdata.bones[ bonename ]
			print( "Bone found: " + bone.name )
			write_smallbuf( f, bone.name )
			pid = 255
			m = bone.matrix_local * magicmtx
			if bone.parent is not None:
				m = ( bone.parent.matrix_local * magicmtx ).inverted() * m
				for bpid, pbone in enumerate(boneorder):
					if bone.parent.name == pbone:
						pid = bpid
						break
			#
			# print(m)
			f.write( struct.pack( "B", pid ) )
			f.write( serialize_matrix( m ) )
		#
	#
	
	return
#

def write_anims( f, anims ):
	
	print( "--- ANIMATIONS' STATS ---" )
	print( "Count: %d" % len( anims ) )
	
	f.write( bytes( "SS3DANIM", "UTF-8" ) )
	f.write( struct.pack( "L", len( anims ) ) )
	
	i = 0
	for anim in anims:
		i += 1
		print( "Animation #%d: %s" % ( i, anim["name"] ) )
		
		a_name = anim["name"]
		a_frames = anim["frames"]
		a_tracks = anim["tracks"]
		a_markers = anim["markers"]
		a_speed = anim["speed"]
		
		a_name_bytes = bytes( a_name, "UTF-8" )
		animbuf = struct.pack( "B", len(a_name_bytes) ) + a_name_bytes
		animbuf += struct.pack( "=LfBB", a_frames, a_speed, len( a_tracks ), len( a_markers ) )
		
		for track_name, track_matrices in a_tracks.items():
			track_name_bytes = bytes( track_name, "UTF-8" )
			trackbuf = struct.pack( "B", len(track_name_bytes) ) + track_name_bytes
			
			for fid in range(a_frames):
				mtx = track_matrices[ fid ]
				
				# decompose
				pos = mtx.to_translation()
				rot = mtx.to_quaternion()
				scl = mtx.to_scale()
				
				# write
				trackbuf += struct.pack( "10f",
					pos.x, pos.y, pos.z,
					rot.x, rot.y, rot.z, rot.w,
					scl.x, scl.y, scl.z
				)
			#
			
			animbuf += struct.pack( "L", len(trackbuf) ) + trackbuf
		#
		
		for marker in a_markers:
			animbuf += struct.pack( "=16sL", bytes( marker["name"], "UTF-8" ), marker["frame"] )
		
		write_buffer( f, animbuf )
	#
#

def serialize_vec3_array( arr ):
	out = bytes()
	for item in arr:
		out += struct.pack( "fff", item.x, item.y, item.z )
	return out

def serialize_quat_array( arr ):
	out = bytes()
	for item in arr:
		out += struct.pack( "ffff", item.x, item.y, item.z, item.w )
	return out

def wrap_small_string( strdata ):
	data = bytes( strdata, "UTF-8" )
	return struct.pack( "B", len(data) ) + data

def wrap_chunk( name, data ):
	return bytes( name, "UTF-8" ) + struct.pack( "L", len( data ) ) + data

def reduce_array( arr ):
	if len(arr) > 1:
		diff = False
		for item in arr:
			if item != arr[0]:
				diff = True
				break
		if diff == False:
			arr = arr[:1]
	return arr

def write_anims_anbd( f, anims ):
	
	print( "--- ANIMATIONS' STATS ---" )
	print( "Count: %d" % len( anims ) )
	
	anbd_chunk = bytes()
	
	i = 0
	for anim in anims:
		i += 1
		print( "Animation #%d: %s" % ( i, anim["name"] ) )
		
		a_name = anim["name"]
		a_frames = anim["frames"]
		a_tracks = anim["tracks"]
		a_markers = anim["markers"]
		a_speed = anim["speed"]
		
		anim_chunk = struct.pack( "fH", a_speed, a_frames )
		
		# parse animation
		float_data = bytes()
		track_data = bytes()
		track_count = 0
		
		for track_name, track_matrices in a_tracks.items():
			track_offset = len( float_data ) // 4
			pos_arr = []
			rot_arr = []
			scl_arr = []
			
			# decompose animation matrices into position/rotation/scale
			for fid in range(a_frames):
				mtx = track_matrices[ fid ]
				
				pos = mtx.to_translation()
				rot = mtx.to_quaternion()
				scl = mtx.to_scale()
				
				pos_arr.append( pos )
				rot_arr.append( rot )
				scl_arr.append( scl )
			#
			
			# see if each list is constant and reduce it if so
			pos_arr = reduce_array( pos_arr )
			rot_arr = reduce_array( rot_arr )
			scl_arr = reduce_array( scl_arr )
			
			track_data += wrap_small_string( track_name )
			track_data += struct.pack( "LHHHH", track_offset, len(pos_arr), len(rot_arr), len(scl_arr), 0 )
			track_count += 1
			
			float_data += serialize_vec3_array( pos_arr )
			float_data += serialize_quat_array( rot_arr )
			float_data += serialize_vec3_array( scl_arr )
		#
		
		# floats
		anim_chunk += struct.pack( "L", len( float_data ) // 4 ) + float_data
		
		# tracks
		anim_chunk += struct.pack( "L", track_count ) + track_data
		
		# markers
		anim_chunk += struct.pack( "L", len( a_markers ) )
		for marker in a_markers:
			anim_chunk += struct.pack( "16sH", bytes( marker["name"], "UTF-8" ), marker["frame"] )
		
		anbd_chunk += wrap_chunk( "ANIM", anim_chunk ) + wrap_small_string( a_name )
	#
	
	anbd_chunk = wrap_chunk( "SGRXANBD", anbd_chunk )
	f.write( anbd_chunk )
#

def addCached( olist, o, minpos = 0 ):
	try:
		return olist[ minpos: ].index( o )
	except:
		olist.append( o )
		return len( olist ) - 1 - minpos
#

def find_in_userdata( obj, key, default = None ):
	for prop in obj.items():
		if type( prop[ 1 ] ) in ( int, str, float, bool ):
			if prop[ 0 ] == key:
				return prop[ 1 ]
	return default
#

def parse_materials( geom_node, textures ):
	materials = []
	print( "Parsing materials... ", end="" )
	for mtl in geom_node.data.materials:
		outmtl = { "textures": [], "shader": "default", "flags": 0, "blendmode": 0 }
		for tex in  mtl.texture_slots:
			outmtl["textures"].append( textures[ tex.name ] if tex != None else "" )
		while len(outmtl["textures"]) and outmtl["textures"][-1] == "":
			outmtl["textures"].pop()
		shdr = find_in_userdata( mtl, "shader" )
		if type( shdr ) == str:
			outmtl["shader"] = shdr
		bmode = find_in_userdata( mtl, "blendmode" )
		if type( bmode ) == str:
			if bmode == "none":
				outmtl["blendmode"] = 0
			if bmode == "basic":
				outmtl["blendmode"] = 1
			if bmode == "additive":
				outmtl["blendmode"] = 2
			if bmode == "multiply":
				outmtl["blendmode"] = 3
		if find_in_userdata( mtl, "unlit", False ):
			outmtl["flags"] |= 1
		if find_in_userdata( mtl, "nocull", False ):
			outmtl["flags"] |= 2
		materials.append( outmtl )
	print( "OK!" )
	return materials

def parse_geometry( geom_node, textures, opt_boneorder, props ):
	
	if props.apply_modifiers == "NONE":
		MESH = geom_node.to_mesh( bpy.context.scene, False, "PREVIEW" )
	else:
		preview_settings = []
		if props.apply_modifiers == "SKIPARM":
			for mod in geom_node.modifiers:
				preview_settings.append( mod.show_viewport )
				if mod.type == "ARMATURE":
					mod.show_viewport = False
		#
		MESH = geom_node.to_mesh( bpy.context.scene, True, "PREVIEW" )
		if props.apply_modifiers == "SKIPARM":
			for i, mod in enumerate( geom_node.modifiers ):
				mod.show_viewport = preview_settings[ i ]
		#
	#
		
	opt_vgroups = geom_node.vertex_groups if len(geom_node.vertex_groups) else None
	
	materials = parse_materials( geom_node, textures )
	
	print( "Generating geometry for %s... " % geom_node.name, end="" )
	MESH.calc_normals_split()
	
	# SORT BY MATERIAL
	MID2FACES = {} # material index -> faces
	FACE2MID = {} # face -> material index
	
	for face in MESH.polygons:
		
		if face.material_index not in MID2FACES:
			MID2FACES[ face.material_index ] = []
		MID2FACES[ face.material_index ].append( face )
	
	# GENERATE COMPACT DATA
	Plist = []
	Nlist = []
	Tlists = [ [] for tl in MESH.uv_layers ]
	Clists = [ [] for cl in MESH.vertex_colors ]
	genParts = [] # array of Part ( array of Face ( array of Vertex ( position index, normal index, texcoord indices, color indices ) ) )
	foundMIDs = []
	
	if len( Tlists ) > 2:
		print( "Too many UV layers" )
		Tlists = Tlists[:1]
	if len( Clists ) > 1:
		print( "Too many color layers" )
		Clists = Clists[:1]
	
	for flist_id in MID2FACES:
		genPart = []
		flist = MID2FACES[ flist_id ]
		m_id = 0
		for face in flist:
			m_id = face.material_index
			if flist_id != m_id:
				continue
			
			genFace = []
			
			for vid in range( len( face.vertices ) ):
				v_id = face.vertices[ vid ]
				l_id = face.loop_start + vid
				VTX = MESH.vertices[ v_id ]
				
				pos_id = addCached( Plist, VTX.co )
				if face.use_smooth != False:
					nrm_id = addCached( Nlist, MESH.loops[ l_id ].normal )
				else:
					nrm_id = addCached( Nlist, face.normal )
				
				genVertex = [ pos_id, nrm_id ]
				
				for si in range( len( Tlists ) ):
					txc_id = addCached( Tlists[ si ], MESH.uv_layers[ si ].data[ l_id ].uv )
					genVertex.append( txc_id )
				for si in range( len( Clists ) ):
					col_id = addCached( Clists[ si ], MESH.vertex_colors[ si ].data[ l_id ].color )
					genVertex.append( col_id )
				
				if opt_vgroups != None and opt_boneorder != None:
					groupweights = []
					for vg in VTX.groups:
						for grp in opt_vgroups:
							if vg.group == grp.index:
								groupweights.append([ opt_boneorder.index( grp.name ), vg.weight ])
								break
							#
						#
					#
					# sort by importance
					groupweights.sort( key=lambda x: x[1], reverse = True )
					# trim useless
					for gwoff, gw in enumerate(groupweights):
						if gw[1] < 1.0/256.0:
							del groupweights[ gwoff: ]
							break
					# check if more than 4, warn / trim if so
					if len(groupweights) > 4:
						print( "Too many weights (%d > 4) for vertex %d at %s" % ( len(groupweights), v_id, VTX.co ) )
						del groupweights[ 4: ]
					if len(groupweights) == 0:
						groupweights.append([ 0, 1.0 ])
					# renormalize
					wsum = 0.0
					for gw in groupweights:
						wsum += gw[1]
					wsum /= 255.0
					for gw in groupweights:
						gw[1] = round( gw[1] / wsum )
					# finish quantization to bytes
					wsum = 0.0
					for gw in groupweights:
						wsum += gw[1]
					gwoff = 0
					while wsum != 255:
						sgnadd = -1 if wsum > 255 else 1
						groupweights[ gwoff ][1] += sgnadd
						wsum += sgnadd
					# compress
					while len(groupweights) < 4:
						groupweights.append([ 0, 0 ])
					gw_groups = struct.pack( "4B", groupweights[0][0], groupweights[1][0], groupweights[2][0], groupweights[3][0] )
					gw_weights = struct.pack( "4B", groupweights[0][1], groupweights[1][1], groupweights[2][1], groupweights[3][1] )
					
					genVertex.append( gw_groups )
					genVertex.append( gw_weights )
				#
				
				genFace.append( genVertex )
			genPart.append( genFace )
		genParts.append( genPart )
		foundMIDs.append( m_id )
	#
	
	# VALIDATION
	if len( Plist ) <= 0:
		raise Exception( "Mesh has no vertices!" )
	#
	
	# CONVERT TO VERTEX BUFFER FORMAT
	vertices = []
	indices = []
	parts = []
	defmtl = { "textures": [], "shader": "default", "flags": 0, "blendmode": 0 }
	
	mtl_num = -1
	for part in genParts:
		mtl_num += 1
		mtl_id = foundMIDs[ mtl_num ]
		vroot = len(vertices)
		outpart = {
			"name": geom_node.name + "#" + str(mtl_num),
			"mtx": geom_node.matrix_world,
			"voff": len(vertices),
			"vcount": 0,
			"ioff": len(indices),
			"icount": 0,
			"flags": materials[ mtl_id ]["flags"] if mtl_id in materials else defmtl["flags"],
			"blendmode": materials[ mtl_id ]["blendmode"] if mtl_id in materials else defmtl["blendmode"],
			"shader": materials[ mtl_id ]["shader"] if mtl_id in materials else defmtl["shader"],
			"textures": materials[ mtl_id ]["textures"] if mtl_id in materials else defmtl["textures"],
		}
		
		for face in part:
			tmpidcs = []
			for vertex in face:
				P = Plist[ vertex[0] ]
				N = Nlist[ vertex[1] ]
				vertexdata = struct.pack( "3f3f", P.x, P.y, P.z, N.x, N.y, N.z )
				vip = 2
				for si in range( len( Tlists ) ):
					T = Tlists[ si ][ vertex[ vip ] ]
					vip += 1
					vertexdata += struct.pack( "2f", T.x, 1 - T.y )
				for si in range( len( Clists ) ):
					C = Clists[ si ][ vertex[ vip ] ]
					vip += 1
					vertexdata += struct.pack( "4B", int(C.r * 255), int(C.g * 255), int(C.b * 255), 255 )
				if opt_vgroups != None and opt_boneorder != None:
					vertexdata += vertex[ vip ] # indices (groups)
					vip += 1
					vertexdata += vertex[ vip ] # weights
					vip += 1
				tmpidcs.append( addCached( vertices, vertexdata, vroot ) )
			#
			for i in range( 2, len( tmpidcs ) ):
				indices.append( tmpidcs[ 0 ] )
				indices.append( tmpidcs[ i ] )
				indices.append( tmpidcs[ i - 1 ] )
			#
		#
		
		outpart["vcount"] = len(vertices) - outpart["voff"]
		outpart["icount"] = len(indices) - outpart["ioff"]
		parts.append( outpart )
	#
	
	# TANGENT SPACE CALC
	if len( Tlists ) > 0:
		tan1list = [ Vector([0,0,0]) for i in range(len(vertices)) ]
		tan2list = [ Vector([0,0,0]) for i in range(len(vertices)) ]
		hitlist = [ 0 for i in range(len(vertices)) ]
		
		for part in parts:
			voff = part["voff"]
			ioff = part["ioff"]
			for i in range( 0, part["icount"], 3 ):
				i1 = indices[ ioff + i + 0 ] + voff
				i2 = indices[ ioff + i + 1 ] + voff
				i3 = indices[ ioff + i + 2 ] + voff
				
				Pdc1 = struct.unpack( "3f", vertices[ i1 ][ :12 ] )
				Pdc2 = struct.unpack( "3f", vertices[ i2 ][ :12 ] )
				Pdc3 = struct.unpack( "3f", vertices[ i3 ][ :12 ] )
				
				v1 = Vector([ Pdc1[0], Pdc1[1], Pdc1[2] ])
				v2 = Vector([ Pdc2[0], Pdc2[1], Pdc2[2] ])
				v3 = Vector([ Pdc3[0], Pdc3[1], Pdc3[2] ])
				
				Tdc1 = struct.unpack( "2f", vertices[ i1 ][ 24:32 ] )
				Tdc2 = struct.unpack( "2f", vertices[ i2 ][ 24:32 ] )
				Tdc3 = struct.unpack( "2f", vertices[ i3 ][ 24:32 ] )
				
				w1 = Vector([ Tdc1[0], Tdc1[1], 0 ])
				w2 = Vector([ Tdc2[0], Tdc2[1], 0 ])
				w3 = Vector([ Tdc3[0], Tdc3[1], 0 ])
				
				x1 = v2.x - v1.x;
				x2 = v3.x - v1.x;
				y1 = v2.y - v1.y;
				y2 = v3.y - v1.y;
				z1 = v2.z - v1.z;
				z2 = v3.z - v1.z;
				
				s1 = w2.x - w1.x;
				s2 = w3.x - w1.x;
				t1 = w2.y - w1.y;
				t2 = w3.y - w1.y;
				
				ir = s1 * t2 - s2 * t1
				if True: # abs( ir ) > 0.000001:
					r = abs( ir ) # 1.0 / ir
					sdir = Vector([(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r])
					tdir = Vector([(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r])
					
					tan1list[ i1 ] += sdir
					tan1list[ i2 ] += sdir
					tan1list[ i3 ] += sdir
					
					tan2list[ i1 ] += tdir
					tan2list[ i2 ] += tdir
					tan2list[ i3 ] += tdir
					
					hitlist[ i1 ] += 1
					hitlist[ i2 ] += 1
					hitlist[ i3 ] += 1
				else:
					print( "Bad triangle UV map!!! invR: %f | UV0: %f;%f | UV1: %f;%f | UV2: %f;%f" % \
						(ir, Tdc1[0],Tdc1[1],Tdc2[0],Tdc2[1],Tdc3[0],Tdc3[1]))
				#
			#
		#
		
		for v_id, vertex in enumerate( vertices ):
			Ndc = struct.unpack( "3f", vertex[ 12:24 ] )
			n = Vector([ Ndc[0], Ndc[1], Ndc[2] ])
			t = tan1list[ v_id ]
			t2 = tan2list[ v_id ]
			
			outtan = ( t - n * n.dot( t ) ).normalized()
			if outtan == Vector([0.0,0.0,0.0]):
				outtan = Vector([0.0,0.0,1.0])
				print( "Tangent was detected to be 0,0,0 on vertex %d - changed to 0,0,1" % v_id )
				Tdc = struct.unpack( "2f", vertex[ 24:32 ] )
				print( "HC: %d, Nrm: %s, Tx1: %s, Tg2: %s" % ( hitlist[ v_id ], n, Tdc, t2 ) )
			sign = -1.0 if n.cross( t ).dot( t2 ) < 0.0 else 1.0
			vertices[ v_id ] = vertex[ :24 ] + struct.pack( "4f", outtan.x, outtan.y, outtan.z, sign ) + vertex[ 24: ]
		#
	#
	
	# AABB
	bbmin = Plist[0].copy()
	bbmax = Plist[0].copy()
	for pos in Plist:
		if bbmin.x > pos.x:
			bbmin.x = pos.x
		if bbmin.y > pos.y:
			bbmin.y = pos.y
		if bbmin.z > pos.z:
			bbmin.z = pos.z
		if bbmax.x < pos.x:
			bbmax.x = pos.x
		if bbmax.y < pos.y:
			bbmax.y = pos.y
		if bbmax.z < pos.z:
			bbmax.z = pos.z
	#
	
	# FORMAT STRING
	format = "pf3nf3"
	if len( Tlists ) > 0:
		format += "tf4"
	for si in range( len( Tlists ) ):
		format += "%df2" % ( si )
	for si in range( len( Clists ) ): # only one expected
		format += "cb4"
	if opt_vgroups != None and opt_boneorder != None:
		format += "ib4wb4"
	#
	
	print( "OK!" )
	
	if MESH is not geom_node.data:
		bpy.data.meshes.remove( MESH )
	
	return {
		"bbmin": bbmin, "bbmax": bbmax,
		"vertices": vertices, "indices": indices,
		"format": format, "parts": parts,
	}
#

def gen_empty_mesh_data():
	return {
		"bbmin": Vector([ sys.float_info.max, sys.float_info.max, sys.float_info.max ]),
		"bbmax": Vector([ -sys.float_info.max, -sys.float_info.max, -sys.float_info.max ]),
		"vertices": [], "indices": [], "format": "pf3nf3", "parts": [],
	}
#

def mesh_data_add( meshdata, ndata ):
	# AABB
	if meshdata["bbmin"].x > ndata["bbmin"].x:
		meshdata["bbmin"].x = ndata["bbmin"].x
	if meshdata["bbmin"].y > ndata["bbmin"].y:
		meshdata["bbmin"].y = ndata["bbmin"].y
	if meshdata["bbmin"].z > ndata["bbmin"].z:
		meshdata["bbmin"].z = ndata["bbmin"].z
	if meshdata["bbmax"].x < ndata["bbmax"].x:
		meshdata["bbmax"].x = ndata["bbmax"].x
	if meshdata["bbmax"].y < ndata["bbmax"].y:
		meshdata["bbmax"].y = ndata["bbmax"].y
	if meshdata["bbmax"].z < ndata["bbmax"].z:
		meshdata["bbmax"].z = ndata["bbmax"].z
	
	# parts
	for part in ndata["parts"]:
		part = copy(part)
		part["voff"] += len(meshdata["vertices"])
		part["ioff"] += len(meshdata["indices"])
		meshdata["parts"].append( part )
	
	# format diff
	format_part_order = [ "pf3", "nf3", "tf4", "0f2", "1f2",
		"3f2", "4f2", "cb4", "ib4", "wb4", "DUMMY",
	]
	# - split into chunks
	oldfmt_A = meshdata["format"]
	oldfmt_B = ndata["format"]
	fmt_A = [oldfmt_A[i:i+3] for i in range(0, len(oldfmt_A), 3)] + [ "DUMMY" ]
	fmt_B = [oldfmt_B[i:i+3] for i in range(0, len(oldfmt_B), 3)] + [ "DUMMY" ]
	
	# - iterate through
	i = 0
	while i < len(fmt_A) or i < len(fmt_B):
		if fmt_A[ i ] == fmt_B[ i ]:
			i += 1
			continue
		# - formats not equal, need to pad one side
		# - pad the side whose index is higher
		if format_part_order.index( fmt_A[ i ] ) > format_part_order.index( fmt_B[ i ] ):
			fmt_tgt = fmt_A
			fmt_src = fmt_B
			data_tgt = meshdata["vertices"]
		else:
			fmt_src = fmt_A
			fmt_tgt = fmt_B
			data_tgt = ndata["vertices"]
		# - pad the format
		curfmt = fmt_src[ i ]
		fmt_tgt.insert( i, curfmt )
		# - calculate padding offset
		pad_offset = 0
		for f in islice( fmt_src, i ):
			nominal = 4 if f[1] == "f" else 1
			multiplier = int(f[2])
			pad_offset += nominal * multiplier
		# - calculate padding data
		if curfmt == "pf3" or curfmt == "nf3":
			raise Exception( "UNEXPECTED PADDING FORMAT" )
		pad_core = struct.pack( "f", 0.0 ) if curfmt[1] == "f" else struct.pack( "B", 0 )
		pad_mult = int(curfmt[2])
		pad_data = pad_core * pad_mult
		# print( "pad with %d bytes of data by format %s" % ( len(pad_data), curfmt ) )
		# - perform padding on each vertex
		for v in range( len( data_tgt ) ):
			data_tgt[ v ] = data_tgt[ v ][ : pad_offset ] + pad_data + data_tgt[ v ][ pad_offset : ]
		# - move on because formats have been made equal here
		i += 1
	#
	
	# - set new format
	meshdata["format"] = "".join( fmt_A[:-1] )
	
	# validate vertex size
	if len(meshdata["vertices"]) > 0 and len(ndata["vertices"]) > 0:
		if len(meshdata["vertices"][0]) != len(ndata["vertices"][0]):
			raise Exception(
				"Vertex sizes not equal: old=%d new=%d fmtA=%s fmtB=%s newfmtA=%s newfmtB=%s" % (
					len(meshdata["vertices"][0]),
					len(ndata["vertices"][0]),
					oldfmt_A,
					oldfmt_B,
					"".join( fmt_A[:-1] ),
					"".join( fmt_B[:-1] ),
				)
			)
	#
	
	# combine vertex/index data
	meshdata["vertices"] += ndata["vertices"]
	meshdata["indices"] += ndata["indices"]
#

def parse_armature( node ):
	for mod in node.modifiers:
		if mod.type == "ARMATURE":
			return mod.object
	return None
#

def generate_bone_order( armdata ):
	if armdata == None:
		return None
	bonelist = []
	bonequeue = []
	for bone in armdata.bones:
		if bone.parent == None:
			bonequeue.append( bone )
	#
	while len(bonequeue) != 0:
		bone = bonequeue.pop(0)
		bonelist.append( bone.name )
		for bone in bone.children:
			bonequeue.append( bone )
	#
	return bonelist
#

def parse_animations( armobj, boneorder, filepath ):
	animations = []
	if armobj is not None and armobj.animation_data is not None:
		print( "Generating animations... " )
		oldact = armobj.animation_data.action
		for action in bpy.data.actions:
			if action.use_fake_user is False:
				continue # do not export animations that are not pinned (likely to be deleted)
			armobj.animation_data.action = action
			anim_tracks = {}
			for bonename in boneorder:
				anim_tracks[ bonename ] = []
			frame_begin, frame_end = [ int(x) for x in action.frame_range ]
			for frame in range( frame_begin, frame_end + 1 ):
				bpy.context.scene.frame_set( frame )
				for bonename in boneorder:
					bone = armobj.pose.bones[ bonename ]
					track = anim_tracks[ bonename ]
					track.append( magicmtx.inverted() * bone.matrix_basis.copy() * magicmtx )
				#
			#
			anim_markers = []
			for pmrk in action.pose_markers:
				anim_markers.append({ "name": pmrk.name, "frame": pmrk.frame })
			#
			animspeed = bpy.context.scene.render.fps / bpy.context.scene.render.fps_base
			animlistname = os.path.dirname( filepath ) + "/" + action.name + ".animlist.csv"
			print( "Looking for animation descriptor - " + animlistname )
			if os.path.isfile( animlistname ):
				print( "Found it, decoding action..." )
				try:
					animlist = csv.reader( open( animlistname, "r" ), delimiter = ",", quotechar = '"' )
					for anim in animlist:
						tstart = int(anim[0], 10)
						tend = int(anim[1], 10)
						tname = anim[2].strip()
						anim_sliced_tracks = {}
						for track_name, track_matrices in anim_tracks.items():
							anim_sliced_tracks[ track_name ] = track_matrices[ tstart : tend ]
						animations.append({ "name": tname, "frames": tend - tstart, "tracks": anim_sliced_tracks, "speed": animspeed })
					#
				except IOError as e:
					print( "I/O error({0}): {1}".format(e.errno, e.strerror) )
				except ValueError:
					print( "Could not convert data to an integer." )
				#
			else:
				print( "Did not find it, will append the whole action." )
				animations.append({ "name": action.name, "frames": frame_end - frame_begin + 1, "tracks": anim_tracks, "markers" : anim_markers, "speed": animspeed })
			#
		#
		armobj.animation_data.action = oldact
		print( "\tOK!" )
	#
	return animations
#

def parse_textures():
	textures = {}
	print( "Parsing textures... ", end="" )
	for tex in bpy.data.textures:
		texpath = ""
		if hasattr( tex, "image" ) and tex.image != None:
			texpath = tex.image.filepath[ 2: ]
		textures[ tex.name ] = texpath
	print( "OK!" )
	return textures

def write_ss3dmesh( ctx, props ):
	filepath = props.filepath
	print( "\n\\\\\n>>> SS3DMESH Exporter v0.5!\n//\n\n" )
	print( "Exporting..." )
	
	textures = parse_textures()
	
	meshdata = gen_empty_mesh_data()
	armobj = None
	armdata = None
	boneorder = []
	for node in ctx.scene.objects:
		if node.type != "MESH":
			continue
		
		if props.export_selected and not node.select:
			continue
		
		cur_armobj = parse_armature( node )
		# do not allow multiple armatures
		if armobj is not None and cur_armobj is not None and armobj.name != cur_armobj.name:
			props.report( {"ERROR"},
				"multiple armatures are not supported (curr=%s, new=%s)" % (
					armobj.name, cur_armobj.name ) )
			return {'CANCELLED'}
		# do this only for the first time
		if armobj is None and cur_armobj is not None:
			armobj = cur_armobj
			armdata = None if armobj is None else armobj.data
			boneorder = generate_bone_order( armdata )
		cur_meshdata = parse_geometry( node, textures, boneorder, props )
		mesh_data_add( meshdata, cur_meshdata )
	
	if props.export_anim:
		print( "Parsing animations..." )
		animations = parse_animations( armobj, boneorder, filepath )
		print( "OK!" )
	
	print( "Writing mesh... " )
	with open( filepath, 'wb' ) as f:
		write_mesh( f, meshdata, armdata, boneorder )
	
	if props.export_anim:
		if len(animations) == 0:
			print( "No animations found!" )
		else:
			print( "Writing animations... " )
			with open( filepath + ".anm", 'wb' ) as f:
				write_anims( f, animations )
	#
	
	print( "\n\\\\\n>>> Done!\n//\n\n" )

	return {'CANCELLED'}
#

def write_sgrxanbd( ctx, filepath ):
	print( "\n\\\\\n>>> SGRXANBD Exporter v0.5!\n//\n\n" )
	print( "Exporting..." )
	
	print( "Parsing nodes... ", end="" )
	geom_node = bpy.context.active_object
	if geom_node == None:
		for node in ctx.scene.objects:
			if node.type == "MESH":
				geom_node = node
				break
	#
	if geom_node == None:
		print( "ERROR: no MESH nodes in the active object!" )
		return {'CANCELLED'}
	print( "OK!" )
	
	armobj = parse_armature( geom_node )
	if armobj is None:
		print( "No armature found" )
		return {'CANCELLED'}
	else:
		armdata = armobj.data
	boneorder = generate_bone_order( armdata )
	
	print( "Parsing animations..." )
	animations = parse_animations( armobj, boneorder, filepath )
	print( "OK!" )
	
	if len(animations) == 0:
		print( "No animations found!" )
	else:
		print( "Writing animations... " )
		with open( filepath, 'wb' ) as f:
			write_anims_anbd( f, animations )
	#
	
	print( "\n\\\\\n>>> Done!\n//\n\n" )

	return {'CANCELLED'}

# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty


apply_mod_ui_items = [
	( "NONE", "None", "Don't apply any modifiers" ),
	( "SKIPARM", "All except armatures", "Apply non-armature modifiers" ),
	( "ALL", "All", "Apply all modifiers" ),
]


class ExportSS3DMESH( bpy.types.Operator, ExportHelper ):
	'''SS3DMESH Exporter'''
	bl_idname = "export.ss3dmesh"
	bl_label = "[SGRX] Export .ssm"
	bl_options = {'REGISTER', 'UNDO'}

	# ExportHelper mixin class uses this
	filename_ext = ".ssm"

	filter_glob = StringProperty(
		default = "*.ssm",
		options = {'HIDDEN'},
	)
	export_anim = BoolProperty(name="Export animation", default=False)
	export_selected = BoolProperty(name="Export selected mesh only", default=True)
	apply_modifiers = EnumProperty(items=apply_mod_ui_items,
		name="Apply modifiers", default="SKIPARM")

	@classmethod
	def poll( cls, ctx ):
		return ctx.active_object is not None

	def execute( self, ctx ):
		return write_ss3dmesh( ctx, self )

class ExportSGRXANBD( bpy.types.Operator, ExportHelper ):
	'''SGRXANBD (anim. bundle) Exporter'''
	bl_idname = "export.sgrxanbd"
	bl_label = "[SGRX] Export .anb"
	bl_options = {'REGISTER', 'UNDO'}
	
	filename_ext = ".anb"
	
	filter_glob = StringProperty(
		default = "*.anb",
		options = {'HIDDEN'},
	)
	
	@classmethod
	def poll( cls, ctx ):
		return ctx.active_object is not None
	
	def execute( self, ctx ):
		return write_sgrxanbd( ctx, self.filepath )


# Only needed if you want to add into a dynamic menu
def menu_func_export( self, ctx ):
	self.layout.operator( ExportSS3DMESH.bl_idname, text="SS3DMESH Exporter" )
	self.layout.operator( ExportSGRXANBD.bl_idname, text="SGRXANBD (anim. bundle) Exporter" )



def makeMaterial(name, uvlayer):
	tex = bpy.data.textures.new(name, type = 'IMAGE')
	
	mat = bpy.data.materials.new(name)
	mat.diffuse_color = (1,1,1)
	mat.diffuse_shader = 'LAMBERT'
	mat.diffuse_intensity = 1.0 
	mat.specular_color = (1,1,1)
	mat.specular_shader = 'COOKTORR'
	mat.specular_intensity = 0.2
	mat.alpha = 1
	mat.ambient = 1
	
	mtex = mat.texture_slots.add()
	mtex.texture = tex
	mtex.texture_coords = 'UV'
	mtex.use_map_color_diffuse = True
	mtex.mapping = 'FLAT'
	mtex.uv_layer = uvlayer
	
	return mat

class DialogOperator(bpy.types.Operator):
	bl_idname = "object.generate_material"
	bl_label = "Generate Material"
	
	prop_mtlname = StringProperty(name="Material name", default="newmtl")
	
	def execute(self, context):
		active = bpy.context.active_object.data
		if hasattr(active, "materials"):
			mtlname = self.prop_mtlname
			active.materials.append( makeMaterial( mtlname, active.uv_layers.keys()[0] ) )
			self.report({'INFO'}, "Material created: %s!" % mtlname)
		else:
			self.report({'INFO'}, "Object cannot have materials!")
		#
		return {'FINISHED'}
	
	def invoke(self, context, event):
		return context.window_manager.invoke_props_dialog(self)



class DialogPanel(bpy.types.Panel):
	bl_label = "Texturing Tools"
	bl_space_type = "VIEW_3D"
	bl_region_type = "UI"
	
	def draw(self, context):
		self.layout.operator("object.generate_material")


def register():
	bpy.utils.register_class( ExportSS3DMESH )
	bpy.utils.register_class( ExportSGRXANBD )
	bpy.utils.register_class( DialogOperator )
	bpy.utils.register_class( DialogPanel )
	bpy.types.INFO_MT_file_export.append( menu_func_export )
	# bpy.types.VIEW3D_PT_tools_object.append( generate_material_func )


def unregister():
	bpy.utils.unregister_class( ExportSS3DMESH )
	bpy.utils.unregister_class( ExportSGRXANBD )
	bpy.utils.unregister_class( DialogOperator )
	bpy.utils.unregister_class( DialogPanel )
	bpy.types.INFO_MT_file_export.remove( menu_func_export )
	# bpy.types.VIEW3D_PT_tools_object.remove( generate_material_func )


if __name__ == "__main__":
	register()
#

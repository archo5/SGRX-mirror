
import os

ENGINE_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
print( "Engine root: %s" % ENGINE_ROOT )

class SGRX:
	PROJECT_FOLDERS = [
		"data-%s",
		"data-%s/src",
		"data-%s/src/chars",
		"data-%s/src/editor",
		"data-%s/src/levels",
		"data-%s/src/psys",
		"data-%s/src/sys",
		"data-%s/cache",
		"data-%s/cache/levels",
		"data-%s/cooked",
		"data-%s/cooked/levels",
		"data-%s/cooked/shadercache_d3d11",
	]
	
	@classmethod
	def create_project( self, name ):
		print( "Creating project: %s" % name )
		for folder in self.PROJECT_FOLDERS:
			path = ENGINE_ROOT + "/" + ( folder % name )
			print( "mkdir", path )
			os.mkdir( path )
		print( "Success!" )


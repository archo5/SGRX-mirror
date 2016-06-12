
import argparse, sys
from engine import *
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class TOOL:
	root = None
	cur_frame = None
	project = None


class Dialog(tk.Toplevel):

	def __init__(self, parent, title = None):
		
		tk.Toplevel.__init__(self, parent)
		self.transient(parent)
		
		if title:
			self.title(title)
		
		self.parent = parent
		
		self.result = None
		
		body = tk.Frame(self)
		self.initial_focus = self.body(body)
		# body.pack(padx=5, pady=5)
		
		self.buttonbox()
		
		self.grab_set()
		
		if not self.initial_focus:
			self.initial_focus = self
		
		self.protocol("WM_DELETE_WINDOW", self.cancel)
		self.geometry("+%d+%d" % (parent.winfo_rootx()+50,
								  parent.winfo_rooty()+50))
		self.initial_focus.focus_set()
		self.wait_window(self)
		
	#
	# construction hooks
	def body(self, master):
		# create dialog body.  return widget that should have
		# initial focus.  this method should be overridden
		pass
		
	def buttonbox(self):
		# add standard button box. override if you don't want the
		# standard buttons
		box = tk.Frame(self)
		w = ttk.Button(box, text="OK", width=10, command=self.ok, default=tk.ACTIVE)
		w.pack(side=tk.LEFT, padx=5, pady=5)
		w = ttk.Button(box, text="Cancel", width=10, command=self.cancel)
		w.pack(side=tk.LEFT, padx=5, pady=5)
		self.bind("<Return>", self.ok)
		self.bind("<Escape>", self.cancel)
		# box.pack()
		
	#
	# standard button semantics
	def ok(self, event=None):
		if not self.validate():
			self.initial_focus.focus_set() # put focus back
			return
		self.withdraw()
		self.update_idletasks()
		self.apply()
		self.cancel()
		
	def cancel(self, event=None):
		# put focus back to the parent window
		self.parent.focus_set()
		self.destroy()
		
	#
	# command hooks
	def validate(self):
		return 1 # override
	def apply(self):
		pass # override


class NewProjectDialog(Dialog):
	def body( self, master ):
		
		self.title( "Create a new project" )
		
		self.name_entry = ttk.Entry( self )
		self.name_entry.grid( row=0, column=0, sticky="nesw", padx=5, pady=5 )
		
		self.newproj_btn = ttk.Button( self,
			text="Create", command=self.ok )
		self.newproj_btn.grid( row=0, column=1, sticky="nes", padx=5, pady=5 )
		
	def buttonbox(self):
		pass
		
	def apply( self ):
		name = self.name_entry.get()
		SGRX.create_project( name )


class SelectProjectFrame(tk.Frame):
	def __init__( self, master ):
		tk.Frame.__init__( self, master )
		self.grid( sticky="nesw" )
		self.columnconfigure( 0, weight=1 )
		self.rowconfigure( 0, weight=1 )
		
		# project list
		self.projlist = tk.Listbox( self )
		self.projlist.grid( row=0, column=0, sticky="nesw" )
		self.projlist.bind( "<Double-1>", lambda x: self.select_project(False) )
		self.reload()
		
		# action buttons
		self.btnframe = tk.Frame( self )
		self.btnframe.grid( row=1, column=0, sticky="esw", padx=5, pady=5 )
		
		self.openproj_btn = ttk.Button( self.btnframe,
			text="Open project", command=lambda: self.select_project() )
		self.openproj_btn.pack( side=tk.LEFT, padx=5, pady=5 )
		
		self.newproj_btn = ttk.Button( self.btnframe,
			text="New project", command=lambda: self.new_project() )
		self.newproj_btn.pack( side=tk.LEFT, padx=5, pady=5 )
		
		self.bind( "<FocusIn>", self.on_focus )
		
	def on_focus( self, event ):
		self.reload()
		
	def reload( self ):
		self.projlist.delete( 0, tk.END )
		for e in os.listdir( ENGINE_ROOT ):
			if e.startswith( "data-" ) and e != "data-common":
				self.projlist.insert( tk.END, e[5:] )
	
	def select_project( self, loud = True ):
		sel = self.projlist.curselection()
		if not sel:
			if loud:
				messagebox.showwarning( "Error", "Project is not selected" )
			return
		projname = self.projlist.get( sel )
		print( "Selected project: %s" % projname )
		TOOL.project = projname
		TOOL.cur_frame = ManagementFrame( self.master )
		TOOL.root.title( "SGRX Engine - project '%s'" % projname )
		self.grid_forget()
		self.destroy()
	
	def new_project( self ):
		win = NewProjectDialog( self )


class ManagementFrame(tk.Frame):
	def __init__( self, master ):
		tk.Frame.__init__(self, master)
		self.grid( sticky="nesw" )
		self.columnconfigure(0, weight=1)
		self.rowconfigure(0, weight=1)


### ARGUMENT PARSING ###
parser = argparse.ArgumentParser(description='SGRX Engine')
parser.add_argument('--newproj', dest='newproj', type=str,
	help='create a new project with the specified name')
args = parser.parse_args()

### PROJECT FILE ###
if args.newproj:
	SGRX.create_project( args.newproj )
	sys.exit(0)

### GUI TOOL ###
root = tk.Tk()
root.title('SGRX Engine')
root.geometry('640x480')
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)
TOOL.root = root
TOOL.cur_frame = SelectProjectFrame(root)
root.mainloop()


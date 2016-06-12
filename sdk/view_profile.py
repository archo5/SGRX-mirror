import argparse
import os
import tkinter as tk
from tkinter import ttk
from collections import defaultdict

def ReadStatsFile():
	global args
	with open("../bin/stats.txt") as f:
		lines = f.readlines()
	
	summed = defaultdict(float)
	for line in lines:
		at = line.rfind(": ")
		path = line[:at]
		time = float(line[at+2:])
		summed[ path ] += time
	
	out = []
	for path, time in summed.items():
		if time < args.mintime:
			continue
		path = path.split("<-")
		path.reverse()
		out.append((path, '->'.join(path), time))
	
	return sorted(out, key=lambda x: -x[2])

class ViewProfileApp(tk.Frame):

	def __init__(self, master):
		global args
		# The GUI
		tk.Frame.__init__(self, master)
		self.tree = ttk.Treeview(self)
		ysb = ttk.Scrollbar(self, orient='vertical', command=self.tree.yview)
		xsb = ttk.Scrollbar(self, orient='horizontal', command=self.tree.xview)
		self.tree.configure(yscroll=ysb.set, xscroll=xsb.set)
		
		self.tree.grid(row=0, column=0, sticky="news")
		ysb.grid(row=0, column=1, sticky='ns')
		xsb.grid(row=1, column=0, sticky='ew')
		self.grid(sticky="nesw")
		self.columnconfigure(0, weight=1)
		self.rowconfigure(0, weight=1)
		
		# Contents
		stats = ReadStatsFile()
		if args.flat:
			parents = defaultdict(bool)
			for path, pathstr, time in stats:
				parents['->'.join(path[:-1])] = True
			for path, pathstr, time in stats:
				if pathstr in parents:
					continue
				self.tree.insert('', 'end', text="%s: %f" % (pathstr, time), open=True)
		else:
			nodes = {}
			for path, pathstr, time in stats:
				node = self.tree.insert(
					nodes['->'.join(path[:-1])] if len(path) > 1 else '',
					'end',
					text="%s: %f" % (path[-1], time),
					open=len(path) == 1)
				nodes[ pathstr ] = node

parser = argparse.ArgumentParser(description='Profiling data (stats.txt) viewer')
parser.add_argument('--min', '-m',
	dest='mintime', type=float, default=0.0,
	help='min. time to show entry')
parser.add_argument('--flat', '-f',
	dest='flat', action='store_true',
	help='enable flat (leaf-list-only) profile')
args = parser.parse_args()

root = tk.Tk()
root.title('View profiling data')
root.geometry('1024x768')
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)
app = ViewProfileApp(root)
root.mainloop()

# imports
import os
import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as an
from IPython.display import display, HTML

#%%
# function to compile a single iteration
def compile_iteration(i, py, px, direct):
    
    # load in all the arrays
    arrays = [[] for row in range(0, py)] 
    for row in range(0, py):
        for col in range(0, px):
            index = row * px + col
            filename = direct + str(i) + "-" + str(index) + ".csv"
            arrays[row].append(np.loadtxt(filename, dtype=int, delimiter=','))

    # stack them into one
    output = np.vstack([np.hstack(row) for row in arrays])
    output = output.astype(int)
    
    # save the stacked array
    name = direct + str(i) + "-all.csv"
    np.savetxt(name, output, delimiter=',', fmt='%i')
    print("created " + name)
	
	
#%%
# read the config file from a directory to find partitions and iterations
def read_config(directory):
     # check directory is real
    assert os.path.isdir(directory), "Given directory does not exist"

    # load the config file
    f_config = open(directory + "_config.txt")
    t_config = f_config.read()
    f_config.close()
    print("Config\n")
    print(t_config)
    
    # find the grid dimensions
    tmp = t_config.split("Partitions")[1]
    tmp = tmp.split(")")[0]
    tmp = tmp.split("(")[1]
    py = int(tmp.split(",")[0])
    px = int(tmp.split(",")[1])
    
    # find the number of iterations
    tmp = t_config.split("Iterations")[1]
    tmp = tmp.split("\n")[0]
    iterations = int(tmp)
    
    return px, py, iterations
	

#%%
# join a directory of results into single arrays
def compile_directory(directory):
    
    px, py, iterations = read_config(directory)

    # create the complied arrays
    for it in range(0, iterations + 1):
        compile_iteration(it, py, px, directory)
        
        
		
#%%
def load_results(directory, node, start, end):
    
    # check directory is real
    assert os.path.isdir(directory), "Given directory does not exist"
    
    # generate all file names
    files = [directory + str(i) + "-" + str(node) + ".csv" for i in range(start, end + 1)]

    # check all files exist
    assert all([os.path.isfile(f) for f in files]), "One or more iteration files does not exist"
    
    # load the data and check the dimensions match
    results = [np.loadtxt(f, dtype=int, delimiter=',') for f in files]
    assert all([results[0].shape == r.shape for r in results]), "Save files have incorrect dimensions"
    
    return results

#%%
# animation class
class animate_imshow():
    def __init__(self, arrays, dt=1, t0=1):
        """
        Array is a list of 2d arrays or a 3d array with time on axis=0.
        Note in jupyter the animation needs to be done"
        """ 
        # setup data
        self.arr = arrays
        self.Nt = len(arrays)
        self.Nx = len(arrays[0])
        self.dt = dt
        self.t0 = t0

        # setup the figure
        self.fig = plt.figure(figsize=(6, 6))
        self.im = plt.imshow(arrays[0], animated=True, cmap='viridis')
        self.ax = plt.gca()
        self.text = self.ax.text(0.02*self.Nx, 0.04*self.Nx, 'Time: ' + str(t0), color='w')
        
        # settings
        self.delay = 10
        self.title = "title"
        self.xlabel = "xlabel"
        self.ylabel = "ylabel"

        # run the animation
    def run(self, html = False):
        self.ax.set(title=self.title, xlabel=self.xlabel, ylabel=self.ylabel)
        self.ani = an.FuncAnimation(self.fig, self.update, frames=range(self.Nt),
                                    interval=self.delay, blit=True)
        
        if html: # use this if in ipython/jupyter notebooks
            plt.close(self.fig)
            self.jshtml = self.ani.to_jshtml()
            display(HTML(self.jshtml))
    
    def save_html(self, filename = "output.html"):
        if not hasattr(self, 'jshtml'):
            self.jshtml = self.ani.to_jshtml()
        f = open(filename, 'w')
        f.write(self.jshtml)
        f.close()
        print(filename + " created")
            
    def update(self, f):
        self.im.set_array(self.arr[f])
        time = int(f * self.dt + self.t0)
        self.text.set_text('Time: %i' % time)
        return self.im, self.text
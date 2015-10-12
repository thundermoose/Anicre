import numpy as np
import matplotlib.pyplot as plt 

A=np.loadtxt("output_r_0.txt")

fig=plt.figure()

plt.plot(A[:,0],A[:,1],label='Neutron')
plt.plot(A[:,0],A[:,2],label='Neutron')
plt.legend()
plt.savefig("radius.pdf")
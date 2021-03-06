import numpy as np
import matplotlib.pyplot as plt 

A=np.loadtxt("output_r_0.txt")

fig=plt.figure()

plt.plot(A[:,0],A[:,1],label='Protron')
plt.plot(A[:,0],A[:,2],label='Neutron')

plt.legend()
plt.savefig("radius.pdf")

print 'Integral P',np.trapz(A[:,1],x=A[:,0])
print 'Integral N',np.trapz(A[:,2],x=A[:,0])
import numpy as np
import matplotlib.pyplot as plt
import sys

data = np.genfromtxt(sys.argv[1], delimiter=',', names=['x', 'y'])

plt.plot(data['x'], data['y'])
plt.ylabel('Cost')
plt.xlabel('Temperature')
plt.savefig(sys.argv[1] + '-plt.png')

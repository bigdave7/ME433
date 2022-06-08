import time
import random
from ulab import numpy as np # to get access to ulab numpy functions

a = np.arange(1024)

f_x = 200000
f_y = 400000
f_z = 800000

x = np.sin(f_x * a)
y = np.sin(f_y * a)
z = np.sin(f_z * a)

waves = x + y + z

wavfft = np.fft.fft(waves)

wavfft1 = wavfft[1]

for i in range(0, 80, 1):
    print((wavfft1[i],0,0))
    time.sleep(0.05)

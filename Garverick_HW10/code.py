# Write your code here :-)
import board
import time
from ulab import numpy as np


t0 = 0
T = 2 * np.pi
num_steps = 1024
dt = (T - t0) / num_steps

t_list = np.linspace(t0, T, num_steps)
print(t_list)

signal = []
for t in t_list:
    wave1 = np.sin(t)
    wave2 = 5 * np.sin(.25 * t)
    wave3 = 2 * np.sin(0.5 * t)

    signal.append(wave1 + wave2 + wave3)

signal = np.array(signal)

real, imag = np.fft.fft(signal)
#freq = np.fft.fftfreq(t.shape[-1])
for i in range(len(real)):
    print("(" + str(real[i]) + ",)")
    time.sleep(0.05)

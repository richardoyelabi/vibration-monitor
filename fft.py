import numpy as np
import matplotlib.pyplot as plt

# Read the vibration data from the log file
vibration_data = np.genfromtxt('vibration_log.csv', delimiter=',', skip_header=3)

# Extract the vibration values from the data
vibration_values = vibration_data[:, 0]

# Calculate the time step (assuming constant time intervals in the log)
time_step = vibration_data[1, 2] - vibration_data[0, 2]

# Perform FFT analysis
fft_values = np.fft.fft(vibration_values)
frequencies = np.fft.fftfreq(len(vibration_values), d=time_step)

# Keep only the positive frequencies and corresponding FFT values
positive_frequencies = frequencies[:len(vibration_values) // 2]
positive_fft_values = 2.0 / len(vibration_values) * np.abs(fft_values[:len(vibration_values) // 2])

# Plot the FFT results
plt.figure()
plt.plot(positive_frequencies, positive_fft_values)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Amplitude')
plt.title('FFT Analysis of Vibration Data')
plt.grid(True)
plt.show()
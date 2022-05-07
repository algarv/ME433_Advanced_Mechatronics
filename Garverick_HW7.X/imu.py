# read data from the imu and plot
import matplotlib.pyplot as plt 
import serial

ser = serial.Serial('/dev/ttyUSB0',230400)
print('Opening port: ')
print(ser.name)

read_samples = 10 # anything bigger than 1 to start out
ax = []
ay = []
az = []
gx = []
gy = []
gz = []
temp = []
roll = []
pitch = []
yaw = []
print('Requesting data collection...')
ser.write(b'\n')

fig, axs = plt.subplots(4,1, sharex='col')
while read_samples > 1:
    data_read = ser.read_until(b'\n',200) # get the data as bytes
    data_text = str(data_read,'utf-8') # turn the bytes to a string
    data = [float(i) for i in data_text.split()] # turn the string into a list of floats
    print(data)
    if(len(data)==11):
        read_samples = int(data[0]) # keep reading until this becomes 1
        ax.append(data[1])
        ay.append(data[2])
        az.append(data[3])
        gx.append(data[4])
        gy.append(data[5])
        gz.append(data[6])
        temp.append(data[7])
        roll.append(data[8])
        pitch.append(data[9])
        yaw.append(data[10])
        print(read_samples)
        

print('Data collection complete')
# plot it
t = range(len(ax)) # time array
axs[0].plot(t,ax,'r*-', label = "Ax")
axs[0].plot(t,ay,'b*-', label = "Ay")
axs[0].plot(t,az,'k*-', label = "Az")
axs[0].set_ylabel('G value')
axs[0].legend()

t = range(len(gx)) # time array
axs[1].plot(t,gx,'r*-',label="Gx")
axs[1].plot(t, gy,'b*-', label="Gy")
axs[1].plot(t,gz,'k*-', label="Gz")
axs[1].set_ylabel('Omega value')
axs[1].legend()

t = range(len(temp)) # time array
axs[2].plot(t,temp,'r*-')
axs[2].set_ylabel('Temperature value')

t = range(len(temp)) # time array
axs[3].plot(t,roll,'r*-', label = "Roll")
axs[3].plot(t,pitch,'b*-', label = "Pitch")
axs[3].plot(t,yaw,'k*-', label = "Yaw")
axs[3].set_ylabel('Degrees')
axs[3].set_xlabel('sample')
axs[3].legend()
plt.show()

# be sure to close the port
ser.close()

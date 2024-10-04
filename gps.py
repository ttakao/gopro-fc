from dronekit import connect as connection
from logging import getLogger

def getlocation():
    # connectstring = "tcp:127.0.0.1:5762" # sitl pre-defined
    connectstring = "/dev/ttyACM0,115200" #USB
    vehicle = connection(connectstring, wait_ready=False)
    vehicle.wait_ready('autopilot_version')
    log.debug("connected Flight Controller")
    # if failed, terminate program by ConnectionRefusedError
    lat = vehicle.location.global_frame.lat
    lon = vehicle.location.global_frame.lon
    alt = vehicle.location.global_frame.alt
    log.info("location: lat="+str(lat)+" lon="+str(lon)+ " alt="+str(alt))
    vehicle.close()
    
    return(lat, lon, alt)

if __name__ == "__main__":
    x = getlocation()
    print(x[0])
    print(x[1])
    print(x[2])

  
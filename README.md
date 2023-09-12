# Cigarette smoke detector

### Introduction
Are you tired of cigarette smoke infiltrating your living space through an open window? The annoyance and health concerns associated with this problem can be quite overwhelming. To address this issue and gain better control over your indoor environment, I've created this project – a comprehensive system that not only detects cigarette smoke but also monitors various environmental parameters, including temperature and humidity, and provides real-time alerts when necessary.

My neighbor's smoking habits negatively impacted my indoor air quality and comfort. To tackle this problem, I developed an Arduino-based solution using the PPD42NS sensor for smoke detection. Beyond smoke, the system also measures temperature, humidity, and other environmental data. All this information is sent to a Raspberry Pi server, where it's displayed on a Grafana dashboard. Additionally, to ensure you can take immediate action, notifications are sent to your <b>smart watch</b> when smoke levels become unacceptable or other environmental parameters reach critical levels.

### Circuit
An Arduino is connected to a Grove Base Shield, which is connected to several Grove sensors from Seeed Studio. Specifically, we have the Grove Gas Sensor MQ5, Grove Dust Sensor, and Grove UV Sensor. Grove Dust Sensor is a PPD42NS sensor that is used to detect cigarette smoke. An Ethernet Shield W5100 is also connected to the Arduino, enabling communication with the server. Finally, a DHT11 sensor is connected to the Arduino, allowing for outdoor temperature and humidity monitoring.
![image](https://github.com/GabTux/cigarette-smoke-detector/assets/24779546/364dc9d7-a2ff-4b0e-99df-1579a8fa5e92)

### Cigarette smoke detection
The PPD42NS sensor can measure particle concentrations in the air, with its smallest detectable particles being PM2.5 particles, which have an average size of over 1µm. Various sources confirm that PM2.5 particles are present in most tobacco smoke.

The idea is to monitor particle concentrations in the air and trigger smoke detection when concentrations rise. This approach proved effective by monitoring the number of particles in 0.01 cubic feet (pcs/0.01cf). Under normal conditions, values range from 0 to low thousands, but during a "smoking break," they can easily reach tens of thousands. Because the sensor is enclosed, smoke doesn't immediately dissipate in the wind, giving the sensor a chance to detect it. Particle monitoring and calculations follow Seeed Studio's official wiki guidelines.

### Server
The server has four main components - Mosquitto, Node-RED, InfluxDB, and Grafana. All these components were installed using Docker, with the help of the IOTStack project, which greatly simplifies the entire installation and management process. The resulting YAML file can be found in the source code.

![image](https://github.com/GabTux/cigarette-smoke-detector/assets/24779546/151ef8f8-1b5b-4a1b-8f14-f80322739242)


### Results
All monitored data is clearly displayed on the Grafana dashboard, which can be customized as needed. I can access Grafana from any local device, such as a mobile phone or smart TV, allowing me to monitor even on the go.

Smoke detection works reliably unless extremely strong wind outside could scatter particles. However, thanks to the protective plastic enclosure, such scattering doesn't occur even in windy conditions. With the connection to the smart wristband, I receive priority notifications on my wrist, making it highly unlikely for me to miss any alerts (the wristband also vibrates).

The graph displays anomalies, during which I verified that my neighbor was indeed smoking. The anomalies around 22:30 may seem unusually significant, but I can attest that during that time, an extremely strong and unfamiliar odor was present in the vicinity, further confirming the smoking activity.

![image](https://github.com/GabTux/cigarette-smoke-detector/assets/24779546/8ee4217f-012f-4c7f-a50a-1644624c84d5)

![image](https://github.com/GabTux/cigarette-smoke-detector/assets/24779546/d4b61598-96cf-4692-918b-2e6ddd62de90)


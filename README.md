# Smart Server Power Switch    

## Purpose  
This project allows remote power control of a home server via a web app, reducing unnecessary power consumption. Instead of switching the AC power, a NodeMCU (ESP8266) electronically simulates the motherboard’s power button press using a BC547 transistor. Automating the AC power switch was avoided due to challenges in ensuring the server turns on automatically when power is applied.  

## Working  
- The ESP8266 connects to WiFi and an MQTT broker (HiveMQ Cloud), listening for a `"power"` command via MQTT. When received, it triggers the BC547 transistor, momentarily shorting the motherboard’s power switch terminals to simulate a physical button press.  

- The motherboard’s power LED voltage is monitored using the ESP8266's ADC (`A0`) via a voltage divider to determine if the server is ON or OFF, providing real-time status feedback. This data is sent via MQTT and displayed on the web app. 

- The ESP8266 also sends a heartbeat pulse every 3 seconds to indicate its online status.  

- A manual push button allows local power control in both offline and online modes. 

- The computer's actual blue power LED is wired to the ESP8266 instead of the motherboard, reflecting the real-time power status while its original terminals are rerouted through the ESP8266.

**Note:** I have used the **TaskScheduler library** to ensure that all functions/tasks run **in parallel and smoothly without blocking execution**. Initially, I attempted the usual `millis()`-based approach, but it did not work perfectly in all scenarios.

## Web App
![image](https://github.com/user-attachments/assets/1d7db221-c1a7-46a8-b298-f2fbaee00639)

The web app is hosted on **Netlify** and provides:  
- Remote control of the server power switch.  
- Real-time ESP8266 status (Online/Offline).  
- Live server power status (ON/OFF).  

## Connections  
| Component                                          | ESP8266 Pin         | Description                                   |  
|----------------------------------------------------|:-------------------:|-----------------------------------------------|  
| BC547 Transistor (Base)                            | D2                  | Controls the motherboard power switch         |  
| Motherboard's Power Switch Terminal                | Collector & Emitter | Simulates button press                        |  
| Motherboard's Power LED pins (via Voltage Divider) | A0                  | Reads server power status                     |  
| Manual Push Button                                 | D5                  | Local power control                           |  
| Power LED                                          | D7                  | For indicating the power status               |

## Future Improvements  
- **Logs & History Tracking** – The web app will store timestamps of server ON/OFF events and ESP8266 disconnections/reconnections for historical tracking.  
- **Live Power Monitoring** – A current sensor will measure real-time power consumption, displaying it on the web app for better power management and anomaly detection.  

## License  
This project is licensed under the [MIT License](LICENSE).

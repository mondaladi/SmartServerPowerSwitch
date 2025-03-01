#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include <TaskScheduler.h>

// WiFi & MQTT Configuration (Replace with your credentials)
const char* ssid = "your_SSID";  // WiFi SSID
const char* password = "your_PASSWORD";  // WiFi Password
const char* mqtt_server = "your_MQTT_BROKER";  // MQTT broker address
const int mqtt_port = 8883;  // MQTT broker port (secure MQTT over TLS)
const char* mqtt_user = "your_MQTT_USERNAME";  // MQTT username
const char* mqtt_pass = "your_MQTT_PASSWORD";  // MQTT password
const char* mqtt_topic_cmd = "pc/control";  // MQTT topic for receiving control commands
const char* mqtt_topic_status = "pc/status";  // MQTT topic for sending PC power status
const char* mqtt_topic_esp_status = "esp/status";  // MQTT topic for sending ESP8266 status

// Pin Definitions
#define POWER_BUTTON D2   // GPIO4 to control BC547 transistor for power switch
#define MANUAL_BUTTON D5  // GPIO13 for local push button input
#define POWER_LED A0      // ADC pin for monitoring power LED voltage
#define INDICATOR_LED D7  // Indicator LED to reflect PC power status

WiFiClientSecure espClient;  // Secure WiFi client for MQTT over TLS
PubSubClient client(espClient);  // MQTT client
Bounce button = Bounce();  // Debounced button handling
Scheduler runner;  // Task scheduler

// Function Declarations (For clarity)
void connectMQTT();
void publishStatus();
void checkButton();
void pressPowerButton();
void callback(char* topic, byte* payload, unsigned int length);
int powerStatus();

// Task Definitions (interval, run count, function, scheduler reference)
Task taskMQTT(5000, TASK_FOREVER, &connectMQTT, &runner);  
Task taskStatus(3000, TASK_FOREVER, &publishStatus, &runner); 
Task taskButton(50, TASK_FOREVER, &checkButton, &runner);  

void setup() {
    Serial.begin(115200);

    // Configure GPIO pins
    pinMode(POWER_BUTTON, OUTPUT);
    pinMode(MANUAL_BUTTON, INPUT_PULLUP);
    pinMode(INDICATOR_LED, OUTPUT);

    // Configure debounce settings for manual button
    button.attach(MANUAL_BUTTON);
    button.interval(20);

    // Start WiFi Connection
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to WiFi...");
    
    // Set MQTT client for insecure connection (no certificate verification)
    espClient.setInsecure();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    // Enable all tasks in the scheduler
    taskMQTT.enable();
    taskStatus.enable();
    taskButton.enable();
}

void connectMQTT() {
    // Attempt MQTT connection only if WiFi is connected and client is disconnected
    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect("ESP_PC_Control", mqtt_user, mqtt_pass)) {
            Serial.println("Connected to MQTT!");
            client.subscribe(mqtt_topic_cmd);  // Subscribe to control topic
        } else {
            Serial.println("MQTT connection failed, retrying...");
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';  // Null-terminate the received message
    String message = String((char*)payload);

    // If the received command is "power", simulate pressing the power button
    if (message.equals("power")) {
        pressPowerButton();
    }
}

void pressPowerButton() {
    // Simulate a press of the power button for 1.5 seconds
    digitalWrite(POWER_BUTTON, HIGH);
    delay(1500);
    digitalWrite(POWER_BUTTON, LOW);
}

int powerStatus() {
    // Read the power LED voltage and determine if the PC is ON (assuming >2V means ON)
    return (analogRead(POWER_LED) * 3.3 / 1024.0 > 2.0) ? 1 : 0;
}

void publishStatus() {
    if (client.connected()) {  
        client.publish(mqtt_topic_esp_status, "Online");  // Report ESP8266 online status
        client.publish(mqtt_topic_status, powerStatus() ? "ON" : "OFF");  // Report PC power status
    }
}

void checkButton() {
    button.update();  // Update button state

    // If the button is pressed (transition from HIGH to LOW), simulate power button press
    if (button.fell()) {
        pressPowerButton();
    }

    // Update indicator LED based on PC power status
    digitalWrite(INDICATOR_LED, powerStatus() ? HIGH : LOW);
}

void loop() {
    runner.execute();  // Run scheduled tasks
    client.loop();  // Maintain MQTT connection
}

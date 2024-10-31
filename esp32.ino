#include <WiFi.h>
#include <WiFiClient.h>

// Pin configuration
const int DOOR_SENSOR_PIN = 15;    // Pin connected to the door sensor
const int BUZZER_PIN = 13;         // Pin connected to the buzzer

// Constants for WiFi and light control
const uint16_t DEFAULT_PORT = 5577;  // Default port for Magic Home bulbs
const int DEFAULT_RETRIES = 3;       // Default retry attempts for state changes
const int TIMEOUT = 5000;            // Timeout in milliseconds

class WifiLedBulb {
  private:
    String ipaddr;
    uint16_t port;
    WiFiClient client;
    
  public:
    WifiLedBulb(String ip, uint16_t p = DEFAULT_PORT) : ipaddr(ip), port(p) {}

    void setup() {
      connect();
      update_state();
    }

    void connect() {
      if (client.connected()) {
        client.stop();
      }

      Serial.print("Connecting to bulb at ");
      Serial.println(ipaddr);

      if (!client.connect(ipaddr.c_str(), port)) {
        Serial.println("Connection failed!");
        return;
      }

      Serial.println("Connected to bulb.");
    }

    void close() {
      if (client.connected()) {
        client.stop();
        Serial.println("Connection closed.");
      }
    }

    void turnOn() {
      _change_state(true);
    }

    void turnOff() {
      _change_state(false);
    }

    void update_state() {
      // Send a state query message to the bulb
      const uint8_t query_state_msg[] = {0x81, 0x8A, 0x8B, 0x96};
      sendMessage(query_state_msg, sizeof(query_state_msg));

      // Read and process the response
      uint8_t response[14];
      if (client.read(response, 14) > 0) {
        // Handle response parsing (based on Magic Home protocol)
        Serial.println("State updated.");
      } else {
        Serial.println("Failed to update state.");
      }
    }

  private:
    void _change_state(bool turn_on) {
      // Construct the message to change state (turn on/off)
      uint8_t msg[4] = {0x71, turn_on ? 0x23 : 0x24, 0x0F, 0x00};  // Placeholder for checksum

      // Calculate checksum as sum of first three bytes, modulo 256
      uint8_t checksum = (msg[0] + msg[1] + msg[2]) % 256;
      msg[3] = checksum;  // Set the checksum in the last byte

      // Send the message
      sendMessage(msg, sizeof(msg));
      
      // Read the response
      uint8_t response[4];
      if (client.read(response, 4) > 0) {
        Serial.print("State changed to: ");
        Serial.println(turn_on ? "ON" : "OFF");
      } else {
        Serial.println("Failed to change state.");
      }
    }

    void sendMessage(const uint8_t *message, size_t length) {
      if (!client.connected()) {
        connect();
      }

      client.write(message, length);
      client.flush();
      delay(50);  // Give the bulb some time to process the message
    }
};

// Global instance of WifiLedBulb
WifiLedBulb bulb("Your Bulb IP");

// Function to check if the door is open
bool isDoorOpen() {
  return digitalRead(DOOR_SENSOR_PIN) == HIGH;
}

// WiFi credentials
const char* ssid = "";
const char* password = "";

// ESP32-CAM IP address
const char* cam_ip = "1";  // Adjust to your ESP32-CAM's IP
const uint16_t cam_port = 80;  // The port for the ESP32-CAM (usually 80)

// WiFi client
WiFiClient client;

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Setup WiFi
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");

  // Initialize the door sensor and buzzer
  pinMode(DOOR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Turn buzzer off initially

  // Setup the light
  bulb.setup();
}

bool door_closed = true;
void loop() {
  // Check the door state
  if (isDoorOpen()) {
    door_closed = false;
    // If the door is open, turn the light on and the buzzer on
    Serial.println("Door is OPEN! Turning on buzzer and light.");
    digitalWrite(BUZZER_PIN, HIGH);  // Buzzer ON
    bulb.turnOn();                   // Light ON
    // Send notification to ESP32-CAM
    if (client.connect(cam_ip, cam_port)) {
      Serial.println("Notifying ESP32-CAM about door opening...");

      // Send a simple message like an HTTP request (or it can be a custom protocol)
      client.println("GET /doorOpen HTTP/1.1");
      client.println("Host: " + String(cam_ip));
      client.println("Connection: close");
      client.println();

      // Close the connection
      client.stop();

    } else {
      Serial.println("Failed to connect to ESP32-CAM.");
    }
  } else {
    // If the door is closed, turn the light off and the buzzer off
    Serial.println("Door is CLOSED! Turning off buzzer and light.");
    digitalWrite(BUZZER_PIN, LOW);   // Buzzer OFF
    if (!door_closed)
    {
      bulb.turnOff();  // Light OFF
      door_closed = true;
    }
  }
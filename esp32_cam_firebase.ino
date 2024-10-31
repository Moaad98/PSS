#include <WiFi.h>
#include <FirebaseClient.h>
#include "esp_camera.h"
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Firebase project credentials
#define FIREBASE_HOST ""  // Use your Firebase database URL
#define FIREBASE_AUTH "your_firebase_database_secret"  // Use your Firebase database secret
#define FIREBASE_PROJECT_ID ""
#define FIREBASE_STORAGE_BUCKET_ID ""


// Firebase authentication with service account (JSON content here)
const char* service_account_json = R"EOF(
{
  //Add you JSON content here
}

)EOF";

// Camera pins (adjust according to your ESP32-CAM model)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

WiFiServer server(80);

FirebaseClient firebaseClient;

void setup_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 120;
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void uploadImageToFirebase(camera_fb_t *fb) {
  // Define the boundary for the multipart/form-data request
  String boundary = "----ESP32CamBoundary";

  // Define the URL for Firebase Storage (replace with your bucket name)
  String url = "https://firebasestorage.googleapis.com/v0/b/" FIREBASE_STORAGE_BUCKET_ID "/o?uploadType=media&name=images/" + String(millis()) + ".jpg";

  HTTPClient http;
  http.begin(url);  // Initialize HTTP request
  http.addHeader("Authorization", "Bearer YOUR_OAUTH2_ACCESS_TOKEN");  // Replace with OAuth2 token
  http.addHeader("Content-Type", "image/jpeg");

  // Post the image buffer to Firebase
  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Image uploaded successfully.");
    Serial.println(response);
  } else {
    Serial.println("Error uploading image to Firebase.");
    Serial.println(httpResponseCode);  // Print the error code for more debugging
  }

  http.end();  // End the HTTP request
}


void captureAndUploadImage() {
    // Turn on the flash (LED)
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, HIGH);  // Turn the flash ON
  // Capture an image
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return; 
  }
  
  digitalWrite(FLASH_LED_PIN, LOW);

  // Upload the image to Firebase
  uploadImageToFirebase(fb);

  // Return the frame buffer to free memory
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address

  // Start the server to listen for door open requests
  server.begin();

  // Initialize the camera
  setup_camera();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {
    Serial.println("New client connected");
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

   // Check if the client requested the door open event
    if (request.indexOf("GET /doorOpen") != -1) {
      Serial.println("Door open event received, capturing image...");
      captureAndUploadImage();  // Capture and upload image when door is open
    } 
    else if (request.indexOf("GET /") != -1) {
      Serial.println("Root path requested.");
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
      client.print("<html><body><h1>ESP32-CAM is running</h1></body></html>");
    } 
    else {
      Serial.println("Unknown request received.");
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }

  delay(100);  // Small delay for stability
}  // Delay to avoid rapid toggling
  delay(100);
}
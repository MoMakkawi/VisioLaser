#include "esp_camera.h"  // Camera library for ESP32
#include <WiFi.h>        // WiFi library to connect to wireless network

// ======================================================================
// WARNING: PSRAM (external RAM) is REQUIRED for UXGA resolution or high JPEG
// quality. Boards like ESP32-Wrover and AI-Thinker usually have PSRAM.
// Ensure the correct board and partition scheme is selected in your IDE.
// ======================================================================

// =======================
// Select your camera model
// =======================
// Uncomment ONLY the model that matches your board

//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
//#define CAMERA_MODEL_M5STACK_ESP32CAM
//#define CAMERA_MODEL_M5STACK_UNITCAM
//#define CAMERA_MODEL_M5STACK_CAMS3_UNIT
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL
//#define CAMERA_MODEL_XIAO_ESP32S3
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3

#include "camera_pins.h"  // Pin definition file for selected board

// =========================
// WiFi Configuration
// =========================
const char *ssid = "LabFab";         // WiFi SSID
const char *password = "my5tere!";   // WiFi Password

// Function declarations
void startCameraServer();          // Starts web server to stream images
void setupLedFlash(int pin);       // LED flash setup function (if defined)

// ============
// Setup Method
// ============
void setup() {
  Serial.begin(115200);             // Start serial communication
  Serial.setDebugOutput(true);      // Enable debug output
  Serial.println();

  // ========================
  // Camera configuration
  // ========================
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;              // External clock frequency
  config.frame_size = FRAMESIZE_UXGA;          // Set resolution
  config.pixel_format = PIXFORMAT_JPEG;        // JPEG for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;     // Frame buffer in PSRAM
  config.jpeg_quality = 12;                    // JPEG quality (lower is better)
  config.fb_count = 1;                         // Number of frame buffers

  // Optimize if PSRAM is found
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;   // Grab latest frame for lower latency
    } else {
      // Lower settings if PSRAM not found
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // If not using JPEG, reduce frame size for face detection
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  // ESP-EYE requires pull-up on input buttons
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // ======================
  // Initialize the camera
  // ======================
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // ======================
  // Configure sensor
  // ======================
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // Flip image vertically
    s->set_brightness(s, 1);   // Slight brightness increase
    s->set_saturation(s, -2);  // Slight saturation decrease
  }

  // Lower default resolution to increase streaming speed
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);  // 320x240 resolution
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  // If camera has flash LED, configure it
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // ================
  // Connect to WiFi
  // ================
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);  // Disable WiFi sleep to reduce latency

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  // Start camera web server
  startCameraServer();

  // Print URL for accessing camera stream
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

// ============
// Loop Method
// ============
void loop() {
  // Nothing to do here – all handling is done by web server
  delay(10000);  // Idle delay
}

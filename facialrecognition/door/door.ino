/**********************************************************************************
TITLE: ESP32 CAMERA Face Recognition Door Lock System

 ***************************************************************************************/


#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_NO_PSRAM

#include "camera_pins.h"

#define RED 13
#define WHITE 4 // onboard white LED
#define LOCK 12 // relayS

//const char* ssid = "";
//const char* password = "";
const char* ssid     = "testing";
const char* password = "smarthome2022";

void startCameraServer();

boolean matchFace = false;
boolean openLock = false;
long prevMillis=0;
int interval = 6000;  //DELAY


void setup() {
  pinMode(LOCK,OUTPUT);
  pinMode(RED,OUTPUT);
  pinMode(WHITE,OUTPUT);
  digitalWrite(LOCK,LOW);
  digitalWrite(RED,HIGH);
  digitalWrite(WHITE,LOW);
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  if(matchFace==true && openLock==false)
  {
    openLock=true;
    digitalWrite(LOCK,LOW);
    //digitalWrite(WHITE,HIGH);
    digitalWrite(RED,LOW);
    if ((WiFi.status()  == WL_CONNECTED))
  {
    String auth_token_board = "klCfUGwGbloWl1037O-yQgJaC9Bc5f_f";
    String pin_number = "V5";
    String value = "1";
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    http.begin("http://sgp1.blynk.cloud/external/api/update?token=" + auth_token_board + "&" + pin_number + "=" + value); //HTTP
    Serial.println("http://sgp1.blynk.cloud/external/api/update?token=" + auth_token_board + "&" + pin_number + "=" + value);

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
    prevMillis=millis();
    Serial.print("UNLOCK DOOR");    
   }
   if (openLock == true && millis()-prevMillis > interval)
   {
    openLock=false;
    matchFace=false;
    digitalWrite(LOCK,HIGH);
    //digitalWrite(WHITE,LOW);
    digitalWrite(RED,HIGH);
    Serial.print("LOCK DOOR");
    }
    digitalWrite(LOCK,HIGH);
    delay(5000);
    digitalWrite(LOCK,LOW);
}


//#include <TJpg_Decoder.h>
//#include <SPI.h>
//#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
//#include <ESPAsyncWebServer.h>

//AsyncWebServer server(80);

#define BOTtoken "5171028233:AAHrNfS4gLBWCjHUnSgMndVkkzzvPSyvmzM"
#define CHAT_ID "977939521"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
 
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
#define GFXFF 1
#define FSB9 &FreeSerifBold9pt7b
 
//TFT_eSPI tft = TFT_eSPI();

const char* ssid = "testing";
const char* password = "smarthome2022";
//const char* ssid = "";
//const char* password = "";
const unsigned long timeout = 30000; // 30 seconds
 
const int buttonPin = 4;    // the number of the pushbutton pin
int buttonState;             
int lastButtonState = LOW;   
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool isNormalMode = true;
 
//bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
//{
//   // Stop further decoding as image is running off bottom of screen
//  if ( y >= tft.height() ) return 0;
// 
//  // This function will clip the image block rendering automatically at the TFT boundaries
//  tft.pushImage(x, y, w, h, bitmap);
// 
//  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
//  // tft.drawRGBBitmap(x, y, bitmap, w, h);
// 
//  // Return 
//  return 1;
//}
 
 
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  pinMode(buttonPin, INPUT);
 
//  Serial.println("INIT DISPLAY");
//  tft.begin();
//  tft.setRotation(3);
//  tft.setTextColor(0xFFFF, 0x0000);
//  tft.fillScreen(TFT_YELLOW);
//  tft.setFreeFont(FSB9);
  
 
  //TJpgDec.setJpgScale(1);
  //TJpgDec.setSwapBytes(true);
  //TJpgDec.setCallback(tft_output);
  
  Serial.println("INIT CAMERA");
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
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
 
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
}
 
bool wifiConnect(){
  unsigned long startingTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  //bot.sendMessage(CHAT_ID, "Bot started up", "");
 
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    if((millis() - startingTime) > timeout){
      return false;
    }
  }
  return true;
}
 
void buttonEvent(){
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == HIGH) {
        isNormalMode = !isNormalMode;
 
        //Additional Code 
        if(!isNormalMode)
          sendingImage();
        //   
      }
    }
  }
  lastButtonState = reading;
}
 
camera_fb_t* capture(){
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  fb = esp_camera_fb_get();
  return fb;
}
 
void showingImage(){
  camera_fb_t *fb = capture();
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }else{
    //TJpgDec.drawJpg(0,0,(const uint8_t*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
}
 
void parsingResult(String response, camera_fb_t *fb){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, response);
  JsonArray array = doc.as<JsonArray>();
  int yPos = 4;
  int bottle = 0;
  //tft.setRotation(1);
  for(JsonVariant v : array){
    JsonObject object = v.as<JsonObject>();
    const char* description = object["description"];
    float score = object["score"];
    String label = "";
    //String String(description)
    label += description;
    if (label=="Gesture")
    {
      Serial.println("gesture detected");
      bot.sendMessage(CHAT_ID, "gesture detected!!", "");
      const char* myDomain = "api.telegram.org";
      String getAll = "";
      String getBody = "";
    
      if (client.connect(myDomain, 443)) {
        Serial.println("Connection successful");
        
        String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + String(CHAT_ID) + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
        String tail = "\r\n--RandomNerdTutorials--\r\n";
    
        uint16_t imageLen = fb->len;
        uint16_t extraLen = head.length() + tail.length();
        uint16_t totalLen = imageLen + extraLen;
      
        client.println("POST /bot"+String(BOTtoken)+"/sendPhoto HTTP/1.1");
        client.println("Host: " + String(myDomain));
        client.println("Content-Length: " + String(totalLen));
        client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
        client.println();
        client.print(head);
      
        uint8_t *fbBuf = fb->buf;
        size_t fbLen = fb->len;
        for (size_t n=0;n<fbLen;n=n+1024) {
          if (n+1024<fbLen) {
            client.write(fbBuf, 1024);
            fbBuf += 1024;
          }
          else if (fbLen%1024>0) {
            size_t remainder = fbLen%1024;
            client.write(fbBuf, remainder);
          }
        }  
        
        client.print(tail);
        
        esp_camera_fb_return(fb);
        
        int waitTime = 10000;   // timeout 10 seconds
        long startTimer = millis();
        boolean state = false;
        
        while ((startTimer + waitTime) > millis()){
          Serial.print(".");
          delay(100);      
          while (client.available()) {
            char c = client.read();
            if (state==true) getBody += String(c);        
            if (c == '\n') {
              if (getAll.length()==0) state=true; 
              getAll = "";
            } 
            else if (c != '\r')
              getAll += String(c);
            startTimer = millis();
          }
          if (getBody.length()>0) break;
        }
        client.stop();
        Serial.println(getBody);
      }
      else {
        getBody="Connected to api.telegram.org failed.";
        Serial.println("Connected to api.telegram.org failed.");
      }
      
//      Serial.println(WiFi.localIP());
//      server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request)
//      {
//        Serial.println("Request received");
//        request->send(200, "text/plain", "gesture");
//        });
//        server.begin();
      
    }
    label += ":";
    label += score;

    //tft.drawString(label, 8, yPos, GFXFF);
    //yPos += 16;
    Serial.println(label);
  }
  //tft.setRotation(3);
}
 
void postingImage(camera_fb_t *fb){
  HTTPClient client;
  
  client.begin("http://192.168.61.243:8888/imageUpdate"); //need change ip addr
  //client.begin("http://192.168.1.20:8888/imageUpdate");
  client.addHeader("Content-Type", "image/jpeg");
  int httpResponseCode = client.POST(fb->buf, fb->len);
  if(httpResponseCode == 200){
    String response = client.getString();
    parsingResult(response, fb);
  }else{
    //tft.setRotation(1);
    //Error
    //tft.drawString("Check Your Server!!!", 8, 4, GFXFF);
    //tft.setRotation(3);
    Serial.println("check server");
  }
 
  client.end();
  WiFi.disconnect();
}
 
void sendingImage(){
  camera_fb_t *fb = capture();
  if(!fb || fb->format != PIXFORMAT_JPEG){
    Serial.println("Camera capture failed");
    esp_camera_fb_return(fb);
    return;
  }else{
    //TJpgDec.drawJpg(0,0,(const uint8_t*)fb->buf, fb->len);
    //tft.setRotation(1);
    //tft.drawString("Wifi Connecting!", 8, 4, GFXFF);
    //tft.setRotation(3);
    Serial.println("wifi connecting");
    if(wifiConnect()){
      //tft.drawString("Wifi Connected!", 8, 4, GFXFF);
      //TJpgDec.drawJpg(0,0,(const uint8_t*)fb->buf, fb->len);
      Serial.println("wifi connected");
      postingImage(fb);
    }else{
      //tft.setRotation(1);
      //tft.drawString("Check Wifi credential!", 8, 4, GFXFF);
      //tft.setRotation(3);
      Serial.println("check wifi pw");
    }
    esp_camera_fb_return(fb);
  }
}
 
void loop() {
  buttonEvent();
  
  //if(isNormalMode)
    //showingImage();
  
}

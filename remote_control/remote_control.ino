
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "xbm.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include <time.h>


TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

// Wifi Credentials
const char* ssid     = "testing";
const char* password = "smarthome2022";
//const char* ssid     = "";
//const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600*7;
const int   daylightOffset_sec = 3600;

//Authentication Tokens
String Auth_token_board1 =  "uQwVfri3GWKGPhvsndyCgA4ZKr7hSiCu";
String Auth_token_board2 =  "klCfUGwGbloWl1037O-yQgJaC9Bc5f_f";
String auth_token_board;

// variables

String pin_number;
String value; // Data to be sent
int light_state;
int fan_state;
int motion_state;
int door_state;
String objectResult;
int rainSensor;

uint8_t board = 1; // board = 1 || board = 2

uint16_t calData[5] = { 569, 3299, 1110, 1528, 7 };


// Switch position and size
#define x_button 50 //buttonwidth +50
#define y_button 36 //buttonheight +36
#define x_start 10
#define y_start 20
#define x_touch 310

#define lightId 1
#define fanId 2
#define motionId 3
#define doorId 4
#define ILI9341_GREY 0x2104
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

void setup(void)
{
  Serial.begin(9600);
  tft.init();

  tft.setRotation(2);
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_WHITE);
  //print_logo();

  delay(1000);

  // clear screen
  tft.fillScreen(TFT_BLACK);

  tft.setRotation(3);
  show_wifi_connectivity();

  // clear screen
  tft.fillScreen(TFT_BLACK);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  light_state=0;
  fan_state=0;
  motion_state=0;
  door_state=0;
  objectResult="";
  rainSensor=0;
  buttons(); // print buttons on screen
//  tft.setCursor(x_start, y_start*3);
//  tft.setTextFont(2);
//  tft.setTextSize(2);
//  tft.println("testing");
  printLocalTime();
//  print_logo();
    
}

void loop()
{
  getstate();//get current switchstates from blynk
  statecolours();//update colours
  printLocalTime();
  //getObjectResult();//get latest object detected
  
  uint16_t x, y;
  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    //int Touch_Y_Cor = map(y, 50, 150, 0, 240);

    Serial.print("x - "); Serial.println(x);
    Serial.print("y - "); Serial.println(y);
    
    if ((x<x_touch) && (x>(x_touch-x_button))){
      if ((y>y_start) && (y<(y_start+y_button))){
        if(light_state == 0){
          Serial.println("Light is on");
          light_state = 1;
          SW_control(lightId,light_state);
        }
        else{
          Serial.println("Light is off");
          light_state = 0;
          SW_control(lightId,light_state);
        }
      }
    }
    
    if ((x<(x_touch-75)) && (x>((x_touch-75)-x_button))){
      if ((y>y_start) && (y<(y_start+y_button))){
        if(fan_state == 0){
          Serial.println("fan is on");
          fan_state = 1;
          SW_control(fanId,fan_state);
        }
        else{
          Serial.println("fan is off");
          fan_state = 0;
          SW_control(fanId,fan_state);
        }
      }
    }

    if ((x<(x_touch-75*2)) && (x>((x_touch-75*2)-x_button))){
      if ((y>y_start) && (y<(y_start+y_button))){
        if(motion_state == 0){
          Serial.println("motion is on");
          motion_state = 1;
          SW_control(motionId,motion_state);
        }
        else{
          Serial.println("motion is off");
          motion_state = 0;
          SW_control(motionId,motion_state);
        }
      }
    }    
  
    if ((x<(x_touch-75*3)) && (x>((x_touch-75*3)-x_button))){
      if ((y>y_start) && (y<(y_start+y_button))){
        if(door_state == 0){
          Serial.println("door is open");
          door_state = 1;
          SW_control(doorId,door_state);
        }
        else{
          Serial.println("door is closed");
          door_state = 0;
          SW_control(doorId,door_state);
        }
      }
    }
    delay(2000);
  }

}


void statecolours()
{
  if(light_state == 1){
    tft.drawXBitmap(x_start, y_start, light, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN);
  }
  else if(light_state == 0){
    tft.drawXBitmap(x_start, y_start, light, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  }

  if(fan_state == 1){
    tft.drawXBitmap(x_start+75, y_start, fan, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN);
  }
  else if(fan_state == 0){
    tft.drawXBitmap(x_start+75, y_start, fan, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  }

  if(motion_state == 1){
    tft.drawXBitmap(x_start+(75*2),y_start, motion, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN);
  }
  else if(motion_state == 0){
    tft.drawXBitmap(x_start+(75*2),y_start, motion, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  }

  if(door_state == 1){
    tft.drawXBitmap(x_start+(75*3), y_start, door, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN);
  }
  else if(door_state == 0){
    tft.drawXBitmap(x_start+(75*3), y_start, door, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  }
  ringMeter(rainSensor, 0, 4095, x_start, y_start*3, 52, "Rain (Ohm)", RED2GREEN);
  rainSensor = 0;
}

void buttons()
{
  
  tft.drawXBitmap(x_start, y_start, light, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  tft.drawXBitmap(x_start+75, y_start, fan, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  tft.drawXBitmap(x_start+(75*2),y_start, motion, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);
  tft.drawXBitmap(x_start+(75*3), y_start, door, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);

}

void SW_control (int id, int state)
{
  if (id == 1)//if id is light
  {
    auth_token_board = Auth_token_board1;
    pin_number="V26";
    if (state == 1)
    {
      value="1";
    }
    else if(state == 0)
    {
      value="0";
    }
  }

    if (id == 2)//if id is fan
  {
    auth_token_board = Auth_token_board1;
    pin_number="V4";
    if (state == 1)
    {
      value="1";
    }
    else if(state == 0)
    {
      value="0";
    }
  }

    if (id == 3)//if id is motion
  {
    auth_token_board = Auth_token_board1;
    pin_number="V27";
    if (state == 1)
    {
      value="1";
    }
    else if(state == 0)
    {
      value="0";
    }
  }

    if (id == doorId)//if id is door
  {
    auth_token_board = Auth_token_board2;
    Serial.println(auth_token_board);
    pin_number="V5";
    if (state == 1)
    {
      value="1";
    }
    else if(state == 0)
    {
      value="0";
    }
  }


  if ((WiFi.status()  == WL_CONNECTED))
  {

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


}

void getstate()
{
  String payload;
  //rainSensor=0;
  if ((WiFi.status()  == WL_CONNECTED))
  {
    HTTPClient http;
    //http.begin("http://sgp1.blynk.cloud/external/api/get?token=" + Auth_token_board1 + "&V26&V4&27");
    http.begin("http://sgp1.blynk.cloud/external/api/get?token=" + Auth_token_board2 + "&" + "V5");
    
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println(payload);
        door_state = payload.toInt();
      }
    } 
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.begin("http://sgp1.blynk.cloud/external/api/get?token=" + Auth_token_board1 + "&V26&V4&V27&V5");
    httpCode = http.GET();
    
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        char payLoadArray[payload.length()];
        
        payload.toCharArray(payLoadArray, payload.length());
        Serial.println(payload);
        Serial.println(payLoadArray[7]);
        Serial.println(payLoadArray[14]);
        Serial.println(payLoadArray[22]);
        light_state = atoi(&payLoadArray[7]);
        fan_state = atoi(&payLoadArray[14]);
        motion_state = atoi(&payLoadArray[22]);
        rainSensor = atoi(&payLoadArray[29]);
        Serial.println(rainSensor);
      }
    } 
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  
}

void print_logo()
{
  int x=0;
  int y=0;
  // Draw bitmap with top left corner at x,y with foreground only color
  // Bits set to 1 plot as the defined color, bits set to 0 are not plotted
  //              x  y  xbm   xbm width  xbm height  color
  tft.drawXBitmap(x, y, wallpaper, logoWidth,logoWidth, TFT_BLACK);
  //tft.drawXBitmap(x_start+(75*3), y_start, door, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED);

}

void show_wifi_connectivity()
{
  WiFi.begin(ssid, password);
  tft.setCursor(110, 100);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(105, 100);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);


  tft.setCursor(105, 100);
  tft.setTextFont(2);
  tft.setTextSize(1);

  tft.println(WiFi.localIP());
  tft.setCursor(105, 120);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.println("Connected");
  delay(2000);
}

void getObjectResult()
{
  struct tm timeinfo;
  String newObject;
  String period;
  HTTPClient http;
 
  http.begin("http://192.168.1.254/test");//need to find ip address of esp32cam when setting up
  int httpCode = http.GET();
 
  if (httpCode > 0) {
     
    newObject = http.getString();
    Serial.println(httpCode);
    //Serial.println(newObject);
    //compare with existing objectresult
    if (objectResult != newObject)
    {
      objectResult = newObject;
      tft.setCursor(x_start, y_start*3);
      tft.setTextFont(2);
      tft.setTextSize(1);
      tft.print(objectResult);
//      if (timeinfo.tm_hour>=12)
//      {
//        period = "AM";
//        }
//        else
//        {
//          period = "PM";
//          }
//      tft.print(&timeinfo, "%I:%M");
//      tft.print(period);
    }
    
    delay(1000);
  }
 
  else {
    Serial.println("Error on HTTP request");
  }
 
  http.end(); 
 
  //delay(30000);
}

void printLocalTime(){
  
  struct tm timeinfo;
  String period;
//  String toprint;
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//  Serial.print("Day of week: ");
//  Serial.println(&timeinfo, "%A");
//  Serial.print("Month: ");
//  Serial.println(&timeinfo, "%B");
//  Serial.print("Day of Month: ");
//  Serial.println(&timeinfo, "%d");
//  Serial.print("Year: ");
//  Serial.println(&timeinfo, "%Y");
//  Serial.print("Hour: ");
//  Serial.println(&timeinfo, "%H");
//  Serial.print("Hour (12 hour format): ");
//  Serial.println(&timeinfo, "%I");
//  Serial.print("Minute: ");
//  Serial.println(&timeinfo, "%M");
//  Serial.print("Second: ");
//  Serial.println(&timeinfo, "%S");
//
//  Serial.println("Time variables");
//  char timeHour[3];
//  strftime(timeHour,3, "%H", &timeinfo);
//  Serial.println(timeHour);
//  char timeWeekDay[10];
//  strftime(timeWeekDay,10, "%A", &timeinfo);
//  Serial.println(timeWeekDay);
//  Serial.println();
  if (timeinfo.tm_hour>=12)
  {
    period = "PM";
  }
  else
  {
    period = "AM";
  }
  tft.setCursor(x_start, y_start*8);
  tft.setTextFont(4);
  tft.setTextSize(2);
  tft.print(&timeinfo, "%I:%M");
  tft.setTextSize(1);
  tft.print(period);
  tft.setCursor(x_start, y_start*10);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.println(&timeinfo, "%A, %B %d %Y");
}

int ringMeter(int value, int vmin, int vmax, int x, int y, int r, char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 4;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int text_colour = 0; // To hold the text colour

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
  byte inc = 5; // Draw segments every 5 degrees, increase to 10 for segmented ring

  // Draw colour blocks every inc degrees
  for (int i = -angle; i < angle; i += inc) {

    // Choose colour from scheme
    int colour = 0;
    switch (scheme) {
      case 0: colour = ILI9341_RED; break; // Fixed colour
      case 1: colour = ILI9341_GREEN; break; // Fixed colour
      case 2: colour = ILI9341_BLUE; break; // Fixed colour
      case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
      case 4: colour = rainbow(map(i, -angle, angle, 63, 127)); break; // Green to red (high temperature etc)
      case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
      default: colour = ILI9341_BLUE; break; // Fixed colour
    }

    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_GREY);
    }
  }

  // Convert value to a string
  char buf[10];
  byte len = 4; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);

  // Set the text colour to default
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  // tft.setTextColor(text_colour, ILI9341_BLACK);
  
  // Print value, if the meter is large then use big font 6, othewise use 4
  if (r > 84) tft.drawCentreString(buf, x - 5, y - 20, 6); // Value in middle
  else tft.drawCentreString(buf, x - 5, y - 20, 4); // Value in middle

  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if (r > 84) tft.drawCentreString(units, x, y + 30, 4); // Units display
  else tft.drawCentreString(units, x, y + 5, 2); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

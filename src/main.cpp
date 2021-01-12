#include <PxMatrix.h>
#include <Adafruit_I2CDevice.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// WIFI credentials
const char* ssid = "OpenWrt";
const char* password = "13nonelephant";

// TCP server at port 80, IP 192.168.26.160 will respond to HTTP requests
IPAddress local_ip(192,168,26,160);
IPAddress gateway(192,168,26,254);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

// Pins for LED MATRIX

#define matrix_width 64
#define matrix_height 64

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 20; //10-50 is usually fine

PxMATRIX display(matrix_width,matrix_height,P_LAT,P_OE,P_A,P_B,P_C,P_D,P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16_t myCOLORS[8]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA,myBLACK};

// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}

void display_update_enable(bool is_enable)
{
  if (is_enable)
    display_ticker.attach(0.004, display_updater);
  else
    display_ticker.detach();
}

void drawImage(int x1, int y1, int x2, int y2, uint16_t color){
 for (int yy = y1; yy < (y2 + 1); yy++)
 {
   for (int xx = x1; xx < (x2 + 1); xx++)
   {
     display.drawPixel(xx, yy, color);
   }
 }
}

void drawImage(int x, int y, uint16_t color)
{
  drawImage(x, matrix_width, y, matrix_height, color);
}

void cycleColor(int x, int y, int cnt){
  switch (cnt)
  {
    case 0:
      drawImage(x,y, myRED);
      break;
    case 1:
      drawImage(x,y, myGREEN);
      break;
    case 2:
      drawImage(x,y, myBLUE);
      break;
    case 3:
      drawImage(x,y, myCYAN);
      break;
    case 4:
      drawImage(x,y, myMAGENTA);
      break;
    case 5:
      drawImage(x,y, myYELLOW);
      break;
    case 6:
      drawImage(x,y, myWHITE);
      break;
    case 7:
      drawImage(x,y, myBLACK);
      break;
    default:
      break;
  }
}

void testAllLEDs(){
  for(int i = 0; i < 8; i++){
    
    display.fillScreen(myCOLORS[i]);
    //cycleColor(0,0,i);
    delay(2000);
    display.clearDisplay();
  }  
}

void testDrawText(){
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2,0);
  display.print("LINE 1");
  display.setTextColor(myMAGENTA);
  display.setCursor(2,8);
  display.print("LINE 2");
  display.setTextColor(myYELLOW);
  display.setCursor(2,16);
  display.print("LINE 3");
  display.setTextColor(myGREEN);
  display.setCursor(2,24);
  display.print("LINE 4");
  display.setTextColor(myRED);
  display.setCursor(2,32);
  display.print("LINE 5");
  display.setTextColor(myBLUE);
  display.setCursor(2,40);
  display.print("LINE 6");
  display.setTextColor(myWHITE);
  display.setCursor(2,48);
  display.print("LINE 7");
  display.setTextColor(myCYAN);
  display.setCursor(2,56);
  display.print("LINE 8");
  delay(3000);
  display.clearDisplay();
}

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";

// HTML web page to handle input field
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Input Text: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
    <form action="/get">
    Input Data: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void printTextLine(std::string text, int line){
  display.clearDisplay();
  display.setTextColor(myWHITE);
  display.setCursor(2,line * 8);
  display.print(text.c_str());
}

void handleText(String inputMessage){
  int pos = 0; // pos value
  char* delim = "%nl"; // value to delimit
  int delimLen = 3; // length of delim
  char* remaining; // input to copy to
  strcpy(remaining, inputMessage.c_str());
  char* pch; // pointer to next delim starting char
  char* toprint; // for the while loop
  int linecount = 0; // what line we're printing on

  Serial.println("Looking for newlines in %" + inputMessage + "%");

  bool c = true;
  while(c){
    pch = strstr(remaining,delim);
    if(pch == NULL){
      c = false;
      strcpy(toprint, remaining);
      Serial.println("No newline found, copying remaining...");
    }else{
      pos = remaining - pch;
      Serial.println("Newline found at position " + pos);
      strncpy(toprint, remaining, pos);
      strcpy(remaining, remaining + pos);
    }
    printTextLine(toprint, linecount);
    Serial.print("Printing line %");
    Serial.print(toprint);
    Serial.println("%");
    linecount++;
  }


  /* //old version
    size_t pos = 0;
  int count = 0;
  std::string token;
  std::string delimiterNL = "%nl";
  std::string s = inputMessage.c_str();
  Serial.println(s.c_str());

  while ((pos = s.find(delimiterNL)) != std::string::npos) {
      token = s.substr(0, pos);
      s.erase(0, pos + delimiterNL.length());
      printTextLine(token, count);
      count++;
      Serial.println(token.c_str());
  }
  printTextLine(s, count);
  Serial.println(s.c_str());
  */
}

void handleImage(String inputMessage){

}

void setup() {

  // start serial for debug
  Serial.begin(9600);

  // Define your display layout here. We are using 1/32
  display.begin(32);

  // set delay with the multiplexer due to cheap display (deleting this causes half the lines to not show)
  //display.setMuxDelay(1,1,1,1,1); //old version
  display.setMuxDelay(1,0,0,0,0);

  // Define your scan pattern here {LINE, ZIGZAG, ZAGGIZ, WZAGZIG, VZAG} (default is LINE)
  display.setScanPattern(LINE);

  // Define multiplex implemention here {BINARY, STRAIGHT} (default is BINARY)
  display.setMuxPattern(BINARY);

  display.fillScreen(myBLACK);
  display_update_enable(true);
  delay(1000);
  display.clearDisplay();

  testDrawText();
  //testAllLEDs();
  
  // Setup WIfi to connect to openwrt
  //WiFi.config(local_ip, gateway, subnet); // static IP assignment
  WiFi.begin(ssid, password);

  display.setTextColor(myRED);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    display.setCursor(2,16);
    display.print("Cnnctng");
    delay(100);
    display.clearDisplay();
    delay(400);
    count++;
    if(count % 10 == 0){
      delay(2000);
    }
  }
  display.setTextColor(myGREEN);
  display.setCursor(2,24);
  display.print("Connected");
  delay(200);
  display.setCursor(2,32);
  display.print("IP Addr:");
  display.setCursor(2,40);
  display.print(WiFi.localIP());
  delay(10);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", index_html);
});

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      handleText(inputMessage);
    }
    else if(request->hasParam(PARAM_INPUT_2)){

    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  delay(100);
}

void loop() {
  delay(1);
}

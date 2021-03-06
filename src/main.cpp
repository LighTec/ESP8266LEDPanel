#include <PxMatrix.h>
#include <Adafruit_I2CDevice.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AnimatedGIF.h>

/**
 * TODO
 * 1. Additional text colors (orange anyways)
 * 2. Figure out if direct image processing would work
 * 3. Text color coding
 * 4. Text Jump commands
 * 
 * 
 * 
**/

// WIFI credentials
const char* ssid = "OpenWrt";
const char* password = "13nonelephant";

// for image support
AnimatedGIF gif;

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

/*    <form action="/get">
    Input Data: <input type="text" name="input2">
    <input type="submit" value="Submit">*/

// HTML web page to handle input field (note: uses raw literal strings)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Input Text: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form method="post" enctype="multipart/form-data">
    <input type="file" name="input2">
    <input class="button" type="submit" value="Upload">
</form><br>
  <a href="\/docs">Documentation</a>
</body></html>)rawliteral";

// HTML web page for teaching input commands
const char tutorial_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
table, th, td {
  border: 1px solid black;
  border-collapse: collapse;
}
th, td {
  padding: 5px;
}
</style>
  </head><body>
<h1>Documentation</h1>
<h2>Text Formatting Codes</h2>
<h3>Newline &amp; Line Jumping</h3>
<p>Newline: *nl<br />Jump to specific line (0-7 valid): *j# where # is the line number. (e.g. *j2)</p>
<h3>Colour Codes</h3>
<p>Eight builtin colour codes are available for use with the *c# code, where # is the letter code of the colour. Currently available colours are red (r), green (g), blue (g), cyan (c), magenta (m), yellow (y), white (w), black (x). Example: *cr, *cw.</p>
<p>The text starts with *cw, and when a colour code is detected, it will continue writing in that colour until told otherwise. For example, in the text "White Red White", for the word "Red" to be coloured red but all other letters coloured white, the following code would have to be used: "White *crRed *cwWhite"</p>
<h2>Inputting Images</h2>
<p>todo</p>
<h2>Error Codes</h2>
<p>Whenever new data is entered into the device (either text or image data), it will return an error code on the "submit" page. Below are all error codes, and their meanings:</p>
<table>
<tbody>
<tr>
<th>Value</th>
<th>Category</th>
<th>Description</th>
</tr>
 <tr>
<td>-3</td>
<td>Generic</td>
<td>Function Not Implemented</td>
</tr>
<tr>
<td>-2</td>
<td>Input</td>
<td>Failure to Process Data, Check for Incorrect Usage</td>
</tr>
<tr>
<td>-1</td>
<td>Generic</td>
<td>Reserved for Future Use</td>
</tr>
<tr>
<td>0</td>
<td>Generic</td>
<td>OK</td>
</tr>
<tr>
<td>1</td>
<td>Text</td>
<td>Attempted to Draw Line Outside of Bounds (0-7)</td>
<tr>
<td>2</td>
<td>Text</td>
<td>Attempted to Use Invalid Colour</td>
</tr>
<tr>
<td>3</td>
<td>Text</td>
<td>Text Too Long for Buffer</td>
</tr>
<tr>
<td>10</td>
<td>Image</td>
<td>Image Size Incorrect</td>
</tr>
</tr>
</tbody>
</table>
<a href=\"/\">Return to Home Page</a>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void printTextLine(std::string text, int line){
  display.setTextColor(myWHITE);
  display.setCursor(2,line * 8);
  display.print(text.c_str());
}

int handleText(String inputMessage){
  display.clearDisplay();
  int pos = 0; // pos value
  char delim[] = "*nl"; // value to delimit
  int delimLen = 3; // length of delim
  char remaining[1024]; // input to copy to
  strcpy(remaining, inputMessage.c_str());
  char* pch; // pointer to next delim starting char
  char toprint[1024]; // for the while loop
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
      pos = pch - remaining;
      Serial.println("Newline found at position " + pos);
      strncpy(toprint, remaining, pos);
      toprint[pos] = '\0';
      strcpy(remaining, (pch + delimLen));
    }
    printTextLine(toprint, linecount);
    Serial.print("Printing line %");
    Serial.print(toprint);
    Serial.println("%");
    linecount++;
  }
  return 0;
}

int handleImage(String inputMessage){
  Serial.printf("HandleImage called, not implemented yet!\n");
  return -3;
}


//
// The memory management functions are needed to keep operating system
// dependencies out of the core library code
//
// memory allocation callback function
void * GIFAlloc(uint32_t u32Size)
{
  return malloc(u32Size);
} /* GIFAlloc() */
// memory free callback function
void GIFFree(void *p)
{
  free(p);
} /* GIFFree() */

void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y;

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + pDraw->iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < pDraw->iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        if (iCount) // any opaque pixels?
        {
          for(int xOffset = 0; xOffset < iCount; xOffset++ ){
            dma_display.drawPixelRGB565(x + xOffset, y, usTemp[xOffset]);
          }
          x += iCount;
          iCount = 0;
        }
        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    }
    else
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<pDraw->iWidth; x++)
      {
        dma_display.drawPixelRGB565(x, y, usPalette[*s++]);
      }
    }
} /* GIFDraw() */

int handlegif(uint8_t *data, size_t len){
  Serial.println("Processing gif.");
  
  int gif_width = data[6];
  int gif_height = data[8];
  Serial.printf("Gif Width: %d, height: %d\n", gif_width, gif_height);

  if((gif_width < 0 || gif_height < 0) || (gif_width > matrix_width || gif_height > matrix_height)){
    Serial.println("gif height/width out of bounds.");
    return 10;
  }

  return 0;
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
  request->send_P(200, "text/html", index_html);});

  server.on("/docs", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", tutorial_html);});

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    int retval = -2;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      retval = handleText(inputMessage);
    }else if(request->hasParam(PARAM_INPUT_2)){
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      retval = handleImage(inputMessage);
    }else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br>Response code: " + retval + ".<br><a href=\"/\">Return to Home Page</a>");
    });

  //server.on("/upload", HTTP_POST, [] (AsyncWebServerRequest *request){
  //  handlegif();
  //});

  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("UploadStart: %s\n", filename.c_str());
    }
    int retval = handlegif(data, len);
    String retvalStr = "" + retval;
    if(final){
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
    }
    request->send(200, "text/html", "File uploaded.<br>Response code: " + retvalStr + ".<br><a href=\"/\">Return to Home Page</a>");
  });

  server.onNotFound(notFound);
  server.begin();
  delay(100);
}

void loop() {
  delay(1);
}

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "index.h"

const char* ssid = "SSID";
const char* pass = "password";
const char* host_name = "Tally-Light-1";
// Set your Static IP address
IPAddress local_IP(192, 168, 0, 51);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);
//Status Check Vars
bool pvw_active = false;
bool pgm_active = false;

//LED config
const int led = 2;    //D4 wemos mini also on board led
#define LED_PIN    D7   //D8 wemos mini for NeoPixel
#define LED_COUNT 12

// group[x][2] x is number of groups
int group[2][2] = {{0,5},{5,5}}; //{start,count} for each group

// Web server port
const uint16_t port = 80;

// Web server object declaration
ESP8266WebServer server(port);
//NeoPixel onject declaration
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void neo_set(int grp, int r, int g, int b){
  int start, end;
  if(grp == 0){
     start = 0;
     end = strip.numPixels();
  } else{
    start = group[grp-1][0];
    end = start + group[grp-1][1];
  }
  for(int i= start; i<end; i++) {
    strip.setPixelColor(i,strip.Color(r,g,b)); //RED
    strip.show();
  }
}

/**GET NOT FULLY IMPLEMENTED
 * USE FOR Testing of LEDs not actual tallying
 */
void handleGET()
{
    /*String message = "get args: ";
    for (int i = 0; i < server.args(); i++) {

    message += "Arg no. " + (String)i + " –> ";   //Include the current iteration value
    message += server.argName(i) + ": ";     //Get the name of the parameter
    message += server.arg(i) + "\n";              //Get the value of the parameter
    
    } 
    Serial.print(message);*/
    
    // *** Turn on or off the LED ***
    if (server.arg("led") == "ON"){
        int r_val = server.arg("red").toInt();
        int g_val = server.arg("green").toInt();
        int b_val = server.arg("blue").toInt();
        int grp_val = server.arg("group").toInt();
        digitalWrite(led, LOW);
        Serial.print("LED ON via GET\n");
        neo_set(grp_val, r_val, g_val, b_val);; }
    else if (server.arg("led") == "OFF") {
        int grp_val = server.arg("group").toInt();
        digitalWrite(led, HIGH);
        Serial.print("LED OFF via GET\n"); 
        neo_set(grp_val, 0, 0, 0 );}
    if (server.arg("/")){
      server.send_P(200, "text/html", index_html);
    }
}

void handlePOST() {
    /*String message = "post args: ";
    for (int i = 0; i < server.args(); i++) {

    message += "Arg no. " + (String)i + " –> ";   //Include the current iteration value
    message += server.argName(i) + ": ";     //Get the name of the parameter
    message += server.arg(i) + "\n";              //Get the value of the parameter
    
    } 
    Serial.print(message);*/
    /* Handle PVW and PGM status */
    if(server.arg("PVW") == "ON") {
      Serial.print("PVW True \n");
        pvw_active = true;
    } else if(server.arg("PVW") == "OFF") {
        Serial.print("PVW False \n");
        pvw_active = false;
    }
    if(server.arg("PGM") == "ON") {
      Serial.print("PGM True \n");
        pgm_active = true;
    } else if(server.arg("PGM") == "OFF") {
        pgm_active = false;
        Serial.print("PGM False \n");
    }
    
    if (server.arg("led") == "ON"){
         int r_val = server.arg("red").toInt();
         int g_val = server.arg("green").toInt();
         int b_val = server.arg("blue").toInt();
         int grp_val = server.arg("group").toInt();
        digitalWrite(led, LOW);
        Serial.print("LED ON via POST\n");
        neo_set(grp_val, r_val, g_val, b_val); }
    else if (server.arg("led") == "OFF" && !pvw_active && !pgm_active) {
        int grp_val = server.arg("group").toInt();
        digitalWrite(led, HIGH);
        Serial.print("LED OFF via POST\n");
        neo_set(grp_val, 0, 0, 0 );}
    
    server.sendHeader("Location","/");
    server.send(303);
}
void setup()
{
    //Set Neopixels
    strip.begin();        //initialize neopixel
    strip.show();         //set all off
    strip.setBrightness(255);  // max=255 
    
    // Set pin as output
    pinMode(led, OUTPUT);
    // Setup serial communication
    Serial.begin(74880);
 
    // *** Connect to a WiFi acces point ***
      // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }
    Serial.printf("Connecting to %s ", ssid);
    WiFi.hostname(host_name);
    
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        digitalWrite(led, !digitalRead(led)); 
    }
    
    MDNS.begin(host_name);
    
    Serial.printf(" Connected\n");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
 
    // *** Start web server ***
    server.on("/", HTTP_GET, handleGET);
    server.on("/", HTTP_POST, handlePOST);
    server.begin();
    Serial.printf("Web server started at port %d\n", port);
    MDNS.addService("http", "tcp", 80);
}
 
void loop()
{
    MDNS.update();
    server.handleClient();
}

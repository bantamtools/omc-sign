/*************************************************** 
 * This is an example for the Adafruit CC3000 Wifi Breakout & Shield
 * 
 * Modifed by Mike Estee for controlling the Other Machine Co. sign.
 * We use an Adafruit AVR32u4 Breakout + CC3000 Wifi Breakout to create
 * a wireless driver.
 *
 * Designed specifically to work with the Adafruit WiFi products:
 * ----> https://www.adafruit.com/products/1469
 * 
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing 
 * products from Adafruit!
 * 
 * Written by Limor Fried & Kevin Townsend for Adafruit Industries.  
 * BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_NeoPixel.h>

#define PIN PIN_B4
#define LED_COUNT 218

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

/*
This example does a test of the TCP client capability:
 * Initialization
 * Optional: SSID scan
 * AP connection
 * DHCP printout
 * DNS lookup
 * Optional: Ping
 * Connect to website and print out webpage contents
 * Disconnect
 SmartConfig is still beta and kind of works but is not fully vetted!
 It might not work on all networks!
 */
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   PIN_D2  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  PIN_D3
#define ADAFRUIT_CC3000_CS    PIN_B0

// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
SPI_CLOCK_DIV2); // you can change this clock speed

#undef WLAN_SSID
#define WLAN_SSID       "some_network"           // cannot be longer than 32 characters!
#undef WLAN_PASS
#define WLAN_PASS       "some_password"
#include "password.h"    // actual passwords go here. DON'T CHECK THEM IN.

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2


#define LISTEN_PORT 1717
Adafruit_CC3000_Server signServer(LISTEN_PORT);

/**************************************************************************/
/*!
 @brief  Sets up the HW and the CC3000 module (called automatically
 on startup)
 */
/**************************************************************************/

uint32_t ip;

void setup(void)
{
  // LED strip init.
  strip.begin();
  for( int i=0; i<LED_COUNT; i++ )
    strip.setPixelColor(i, 0x000002);
  strip.show(); // dark

  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print("Free RAM: "); 
  Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    strip.setPixelColor(0, 0xFF0000);
    strip.show();
    while(1);
  }

  // Optional SSID scan
  // listSSIDResults();

  if( !cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY))
  {
    Serial.println(F("Failed!"));
    strip.setPixelColor(0, 0xFF00FF);
    strip.show();
    while(1);
  }

  Serial.println(F("Connected!"));

  strip.setPixelColor(0, 0x0000FF);
  strip.show();

  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while( !cc3000.checkDHCP() )
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  strip.setPixelColor(0, 0x00FF00);
  strip.show();
  delay(1000);
  strip.setPixelColor(0, 0x0);
  strip.show();

  // Start listening for connections
  signServer.begin();
  Serial.println(F("Listening for connections..."));
}

int pixel = 0;
const int max_pixel = 60;
void loop(void)
{
  const int LENGTH = 255;
  char buffer[LENGTH+1] = "";

  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = signServer.available();
  if (client)
  {
    buffer[0] = 0;

    // Check if there is data available to read.

    int count = 0;
    while( client.available() > 0 )
    {
      if( count < LENGTH )
      {
        uint8_t ch = tolower(client.read());

        // end of color, or end of line
        if( ch == '\n' || ch == ',' )
        {
          uint32_t color = 0;
          int len = count;

          // optional # or 0x prefix
          char *p = buffer;
          if( p[0]=='0' && p[1]=='x' )
            p += 2, len -= 2;
          else if( p[0]=='#' )
            p += 1, len -= 1;

          // convert
          color = strtol(p, NULL, 16);

          // if this was 3 chars instead of six, shift up to 24bits
          if( len==3 )
            color = ((color&0xF)<<4) | ((color&0xF0)<<8) | ((color&0xF00)<<12);

          strip.setPixelColor(pixel++, color);

          // update and reset pixel counter
          if( ch == '\n' )
          {
            pixel = 0;
            strip.show();
            //             Serial.println("");
          }
          //           else
          //             Serial.print(",");

          // reset buffer
          count = 0;           
        }

        // accumulate
        else if( isdigit(ch) || ch=='a' || ch=='b' || ch=='c' || ch=='d' || ch=='e' || ch=='f' || ch=='#' || ch=='x' )
        {
          buffer[count] = ch;
          buffer[count+1] = 0;
          //           Serial.print((char)ch);
          count++;
        }
      }
    }
  }
}

/**************************************************************************/
/*!
 @brief  Begins an SSID scan and prints out all the visible networks
 */
/**************************************************************************/

void listSSIDResults(void)
{
  uint8_t valid, rssi, sec, index;
  char ssidname[33]; 

  index = cc3000.startSSIDscan();

  Serial.print(F("Networks found: ")); 
  Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);

    Serial.print(F("SSID Name    : ")); 
    Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}

/**************************************************************************/
/*!
 @brief  Tries to read the IP address and other connection details
 */
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); 
    cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); 
    cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); 
    cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); 
    cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); 
    cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}


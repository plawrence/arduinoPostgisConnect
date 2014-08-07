#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <sha256.h>

#define ADAFRUIT_CC3000_IRQ   3 // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5 // These can be
#define ADAFRUIT_CC3000_CS   10 // any two pins
// Hardware SPI required for remaining pins.
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(
  ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
  SPI_CLOCK_DIVIDER);
  
#define WLAN_SSID     "thenetwork"  // 32 characters max
#define WLAN_PASS     "thepass"
#define WLAN_SECURITY WLAN_SEC_WPA2 // WLAN_SEC_UNSEC/WLAN_SEC_WEP/WLAN_SEC_WPA/WLAN_SEC_WPA2

const char PROGMEM
  // RESTapi application credentials -- see notes above -- DO NOT SHARE.
  consumer_key[]  = "PUT_YOUR_CONSUMER_KEY_HERE",
  access_token[]  = "PUT_YOUR_ACCESS_TOKEN_HERE",
  signingKey[]    = "PUT_YOUR_CONSUMER_SECRET_HERE"      // Consumer secret
    "&"             "PUT_YOUR_ACCESS_TOKEN_SECRET_HERE", // Access token secret
  // The ampersand is intentional -- do not delete!
  // Other globals.  You probably won't need to change these. ---------------
  endpoint[]      = "http://theserverip",
  agent[]         = "Arduino-Upload-Test v1.0";
const char
  host[]          = "serverip";
const unsigned long
  dhcpTimeout     = 60L * 1000L, // Max time to wait for 	address from DHCP
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server
unsigned long
  currentTime = 0L;
Adafruit_CC3000_Client
  client;        // For WiFi connections
  
  #define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

// --------------------------------------------------------------------------

void setup(void) {

  uint32_t ip = 0L, t;

  Serial.begin(115200);

  Serial.print(F("Hello! Initializing CC3000..."));
  if(!cc3000.begin()) hang(F("failed. Check your wiring?"));

  uint16_t firmware = checkFirmwareVersion();
  if (firmware < 0x113) {
    hang(F("Wrong firmware version!"));
  }

  Serial.print(F("OK\r\nDeleting old connection profiles..."));
  if(!cc3000.deleteProfiles()) hang(F("failed."));

  Serial.print(F("OK\r\nConnecting to network..."));
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));

  // Get initial time from time server (make a few tries if needed)
  //for(uint8_t i=0; (i<5) && !(currentTime = getTime()); delay(5000L), i++);

  // Initialize crypto library
  //Sha1.initHmac_P((uint8_t *)signingKey, sizeof(signingKey) - 1);
  Sha256.init();
}

// On error, print PROGMEM string to serial monitor and stop
void hang(const __FlashStringHelper *str) {
  Serial.println(str);
  for(;;);
}

uint8_t countdown = 23; // Countdown to next time server query (once per ~24hr)
  
  


void loop(){
    unsigned long		ip;
    String data;
    uint8_t *hash;
    String hashpass;
    hashpass+="yourpassword";
    data+="";
    data+="data=12.7"; // Use HTML encoding for commas.  This is where the data stream will be inserted.
    data+="&loc_id=1";
    data+="&submit=Submit"; // Submitting data
  
    unsigned long t = millis();
	
    
	
    Serial.print(F("Locating REST server..."));
    cc3000.getHostByName((char *)host, &ip);
    
    cc3000.printIPdotsRev(ip);
    
    do {
      client = cc3000.connectTCP(ip, 80);
    } while((!client.connected()) &&
        ((millis() - t) < connectTimeout));
	
	// Connect to numeric IP
    Serial.print(F("OK\r\nConnecting to server..."));
    t = millis();
    do {
      client = cc3000.connectTCP(ip, 80);
    } while((!client.connected()) &&
          ((millis() - t) < connectTimeout));
	
	if (client.connected()) {
		Serial.println("connected, issuing http request");
		client.println("POST http://theserverip/tdisplay.php  HTTP/1.1");
		client.println("Host: theserverip");
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.print("Content-Length: ");
		client.println(data.length());
		client.println();
                Sha256.print(hashpass);
                hash = Sha256.result();
                String hashstr = String((int) hash);
		client.print(data);
                client.print("&hash=");
                client.print(hashstr);
		client.println();

		//Prints your post request out for debugging
		Serial.println("POST http://theserverip/tdisplay.php HTTP/1.1");
		Serial.println("Host: theserverip");
		Serial.println("Content-Type: application/x-www-form-urlencoded");
		Serial.println("Connection: close");
		Serial.print("Content-Length: ");
		Serial.println(data.length());
		Serial.println();
                Serial.println(hashstr);
		Serial.print(data);
		Serial.println();
	}
	delay(2000);

	if (client.connected()) {
	Serial.println();
	Serial.println("disconnecting.");
	client.close();
	}
    //sendSensorData("1254");

}  
  
  
/*boolean sendSensorData(char *msg) { 
  uint8_t                  *in, out, i;
  char                      nonce[9],       // 8 random digits + NUL
                            searchTime[11], // 32-bit int + NUL
                            b64[29];
  unsigned long             startTime, t, ip;
  static const char PROGMEM b64chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
  Sha1.print(F("POST&http%3A%2F%2F"));
  Sha1.print(host);
  //urlEncode(Sha1, endpoint, true, false);
  //Sha1.print(F("&oauth_consumer_key%3D"));
  //Sha1.print(F2(consumer_key));
  //Sha1.print(F("%26oauth_nonce%3D"));
  //Sha1.print(nonce);
  //Sha1.print(F("%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D"));
  //Sha1.print(searchTime);
  //Sha1.print(F("%26oauth_token%3D"));
  //Sha1.print(F2(access_token));
  //Sha1.print(F("%26oauth_version%3D1.0%26status%3D"));
  //urlEncode(Sha1, msg, false, true);
  Sha1.print(

  // base64-encode SHA-1 hash output.  This is NOT a general-purpose base64
  // encoder!  It's stripped down for the fixed-length hash -- always 20
  // bytes input, always 27 chars output + '='.
  /*for(in = Sha1.resultHmac(), out=0; ; in += 3) { // octets to sextets
    b64[out++] =   in[0] >> 2;
    b64[out++] = ((in[0] & 0x03) << 4) | (in[1] >> 4);
    if(out >= 26) break;
    b64[out++] = ((in[1] & 0x0f) << 2) | (in[2] >> 6);
    b64[out++] =   in[2] & 0x3f;
  }
  b64[out] = (in[1] & 0x0f) << 2;
  // Remap sextets to base64 ASCII chars
  for(i=0; i<=out; i++) b64[i] = pgm_read_byte(&b64chars[b64[i]]);
  b64[i++] = '=';
  b64[i++] = 0;*/
  
  // Hostname lookup
  /*Serial.print(F("Locating sensor server..."));
  cc3000.getHostByName((char *)host, &ip);

  // Connect to numeric IP
  Serial.print(F("OK\r\nConnecting to server..."));
  t = millis();
  do {
    client = cc3000.connectTCP(ip, 80);
  } while((!client.connected()) &&
          ((millis() - t) < connectTimeout));

  if(client.connected()) { // Success!
    Serial.print(F("OK\r\nIssuing HTTP request..."));
    
    Serial.println(encodedLength(msg));
    Serial.println(b64);

    // Unlike the hash prep, parameters in the HTTP request don't require sorting.
    client.fastrprint(F("POST "));
    client.fastrprint(F2(endpoint));
    client.fastrprint(F(" HTTP/1.1\r\nHost: "));
    client.fastrprint(host);
    client.fastrprint(F("\r\nUser-Agent: "));
    client.fastrprint(F2(agent));
    client.fastrprint(F("\r\nConnection: close\r\n"
                       "Content-Type: application/x-www-form-urlencoded;charset=UTF-8\r\n"
                       "Content-Length: "));
    client.print(7 + encodedLength(msg));
    client.fastrprint(F("\r\nAuthorization: Oauth oauth_consumer_key=\""));
    client.fastrprint(F2(consumer_key));
    client.fastrprint(F("\", oauth_nonce=\""));
    client.fastrprint(nonce);
    client.fastrprint(F("\", oauth_signature=\""));
    urlEncode(client, b64, false, false);
    client.fastrprint(F("\", oauth_signature_method=\"HMAC-SHA1\", oauth_timestamp=\""));
    client.fastrprint(searchTime);
    client.fastrprint(F("\", oauth_token=\""));
    client.fastrprint(F2(access_token));
    client.fastrprint(F("\", oauth_version=\"1.0\"\r\n\r\nstatus="));
    urlEncode(client, msg, false, false);

    Serial.print(F("OK\r\nAwaiting response..."));
    int c = 0;
    // Dirty trick: instead of parsing results, just look for opening
    // curly brace indicating the start of a successful JSON response.
    while(((c = timedRead()) > 0) && (c != '{'));
    if(c == '{')   Serial.println(F("success!"));
    else if(c < 0) Serial.println(F("timeout"));
    else           Serial.println(F("error (invalid Twitter credentials?)"));
    client.close();
    return (c == '{');
  } else { // Couldn't contact server
    Serial.println(F("failed"));
    return false;
  }
}

  
// For URL-encoding functions below
static const char PROGMEM hexChar[] = "0123456789ABCDEF";

// URL-encoding output function for Print class.
// Input from RAM or PROGMEM (flash).  Double-encoding is a weird special
// case for Oauth (encoded strings get encoded a second time).
void urlEncode(
  Print      &p,       // EthernetClient, Sha1, etc.
  const char *src,     // String to be encoded
  boolean     progmem, // If true, string is in PROGMEM (else RAM)
  boolean     x2)      // If true, "double encode" parenthesis
{
  uint8_t c;

  while((c = (progmem ? pgm_read_byte(src) : *src))) {
    if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
       ((c >= '0') && (c <= '9')) || strchr_P(PSTR("-_.~"), c)) {
      p.write(c);
    } else {
      if(x2) p.print("%25");
      else   p.write('%');
      p.write(pgm_read_byte(&hexChar[c >> 4]));
      p.write(pgm_read_byte(&hexChar[c & 15]));
    }
    src++;
  }
}

// Returns would-be length of encoded string, without actually encoding
int encodedLength(char *src) {
  uint8_t c;
  int     len = 0;

  while((c = *src++)) {
    len += (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
            ((c >= '0') && (c <= '9')) || strchr_P(PSTR("-_.~"), c)) ? 1 : 3;
  }

  return len;
}
  
  
  
  
  
  
  
  
/*
  DigitalReadSerial
 Reads a digital input on pin 2, prints the result to the serial monitor 
 
 This example code is in the public domain.
 */

// digital pin 2 has a pushbutton attached to it. Give it a name:
//int pushButton = 2;
//
//// the setup routine runs once when you press reset:
//void setup() {
//  // initialize serial communication at 9600 bits per second:
//  Serial.begin(9600);
//  // make the pushbutton's pin an input:
//  pinMode(pushButton, INPUT);
//}
//
//// the loop routine runs over and over again forever:
//void loop() {
//  // read the input pin:
//  int buttonState = digitalRead(pushButton);
//  // print out the state of the button:
//  Serial.println(buttonState);
//  delay(1);        // delay in between reads for stability
//}


// Helper functions. --------------------------------------------------------

uint16_t checkFirmwareVersion(void)
{
  uint8_t major, minor;
  uint16_t version;
  
#ifndef CC3000_TINY_DRIVER  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
    version = 0;
  }
  else
  {
    Serial.print(F("Firmware V. : "));
    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
    version = major; version <<= 8; version |= minor;
  }
#endif
  return version;
}

// Read from client stream with a 5 second timeout.  Although an
// essentially identical method already exists in the Stream() class,
// it's declared private there...so this is a local copy.
int timedRead(void) {
  unsigned long start = millis();
  while((!client.available()) && ((millis() - start) < responseTimeout));
  return client.read();  // -1 on timeout
}

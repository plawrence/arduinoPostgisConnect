#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <sha256.h>
#include "RESTpasswords.h"

#define ADAFRUIT_CC3000_IRQ   3 // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5 // These can be
#define ADAFRUIT_CC3000_CS   10 // any two pins
// Hardware SPI required for remaining pins.
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(
  ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
  SPI_CLOCK_DIVIDER);
  
#define WLAN_SECURITY WLAN_SEC_WPA2 // WLAN_SEC_UNSEC/WLAN_SEC_WEP/WLAN_SEC_WPA/WLAN_SEC_WPA2

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
    unsigned long ip;
    String data = "";
    uint8_t *hash;
    data+="data=13"; // Use HTML encoding for commas.  This is where the data stream will be inserted.
    data+="&loc_id=1";
    data+="&submit=Submit"; // Submitting data
  
    unsigned long t = millis();
	
    
	
    Serial.println(F("Locating REST server..."));
    //the below function doesn't seem to be working....
    cc3000.getHostByName((char *)host, &ip);
    //ip = 919130993;
    
    cc3000.printIPdotsRev(ip);

	// Connect to numeric IP
    Serial.print(F("OK\r\nConnecting to server..."));
    t = millis();
    do {
      client = cc3000.connectTCP(ip, 80);
    } while((!client.connected()) &&
          ((millis() - t) < connectTimeout));
	
	if (client.connected()) {
		Serial.println("connected, issuing http request");
		client.print("POST http://");
                client.print(host);
                client.println("/tdisplay.php  HTTP/1.1");
		client.print("Host: ");
                client.println(host);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Connection: close");
		client.print("Content-Length: ");
		client.println(data.length());
		client.println();
                Sha256.print(hashpass);
                hash = Sha256.result();
		client.print(data);
                client.print("&hash=");
                printClientHash(hash);
		client.println();

		//Prints your post request out for debugging
		Serial.print("POST http://");
                Serial.print(host);
                Serial.println("/postgisInsert.php  HTTP/1.1");
		Serial.print("Host: ");
                Serial.println(host);
		Serial.println("Content-Type: application/x-www-form-urlencoded");
		Serial.println("Connection: close");
		Serial.print("Content-Length: ");
		Serial.println(data.length());
		Serial.println();
		Serial.print(data);
                Serial.print("&hash=");
                printHash(hash);
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

unsigned int ip_to_int (const char * ip)
{
    /* The return value. */
    unsigned v = 0;
    /* The count of the number of bytes processed. */
    int i;
    /* A pointer to the next digit to process. */
    const char * start;

    start = ip;
    for (i = 0; i < 4; i++) {
        /* The digit being processed. */
        char c;
        /* The value of this byte. */
        int n = 0;
        while (1) {
            c = * start;
            start++;
            if (c >= '0' && c <= '9') {
                n *= 10;
                n += c - '0';
            }
            /* We insist on stopping at "." if we are still parsing
               the first, second, or third numbers. If we have reached
               the end of the numbers, we will allow any character. */
            else if ((i < 3 && c == '.') || i == 3) {
                break;
            }
            else {
                return 0;
            }
        }
        if (n >= 256) {
            return 0;
        }
        v *= 256;
        v += n;
    }
    return v;
}

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<32; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

void printClientHash(uint8_t* hash) {
  int i;
  for (i=0; i<32; i++) {
    client.print("0123456789abcdef"[hash[i]>>4]);
    client.print("0123456789abcdef"[hash[i]&0xf]);
  }
  client.println();
}

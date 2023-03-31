/**
   @file

   EspNowUnicast.ino demonstrates how to transmit unicast ESP-NOW messages with @c WifiEspNow .
   You need two ESP8266 or ESP32 devices to run this example.

   Unicast communication requires the sender to specify the MAC address of the recipient.
   Thus, you must modify this program for each device.

   The recommended workflow is:
   @li 1. Flash the program onto device A.
   @li 2. Run the program on device A, look at serial console for its MAC address.
   @li 3. Copy the MAC address of device A, paste it in the @c PEER variable below.
   @li 4. Flash the program that contains A's MAC address onto device B.
   @li 5. Run the program on device A, look at serial console for its MAC address.
   @li 6. Copy the MAC address of device B, paste it in the @c PEER variable below.
   @li 7. Flash the program that contains B's MAC address onto device A.
*/

 
#include <WifiEspNow.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

#include "AESLib.h"

//-------------------------------AES-----------
//---------wifi-credential-------
const char* ssid = "JADUR";
const char* password = "jadur19677";

const char* host = "192.168.0.104"; //esp_server ip
const uint16_t port = 17;           //esp_server port


String data_send    = "hello Esp next data will be encrypted";
String data_capture = "" ;
//
////--------physical-indicator--------
#define BlueLED LED_BUILTIN
//#define RedLED 14


AESLib aesLib;

String plaintext = "HELLO WORLD!";

char cleartext[256];
char ciphertext[512];

// AES Encryption Key
byte aes_key[] = { 0x3E , 0xE2, 0x33, 0x55, 0XA2, 0x65, 0x75, 0x5C, 0xC3, 0x26, 0x70, 0x08, 0x06, 0x67, 0x78, 0x40 };

// General initialization vector
byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


//-----------------------peer-to-peer--

// The recipient MAC address. It must be modified for each device. 
static uint8_t PEER[] {0x58, 0xBF, 0x25, 0xDC, 0xED, 0xE8}; //connect to CLIENT so client mac is here

String SendedText = "";
bool peer_mode =  false;

//--------------------------Functions-----
//for via server transfering data
String encrypt(char * msg, uint16_t msgLen, byte iv[]) {
  int cipherlength = aesLib.get_cipher64_length(msgLen);
  char encrypted[cipherlength]; // AHA! needs to be large, 2x is not enough
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  Serial.print("encrypted = "); Serial.println(encrypted);
  return String(encrypted);
}

String decrypt(char * msg, uint16_t msgLen, byte iv[]) {
  char decrypted[msgLen];
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}



//for peer to peer
void printReceivedMessage(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count,
                          void* arg)
{
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  String RecievedData;
  for (int i = 0; i < static_cast<int>(count); ++i) {
    RecievedData += static_cast<char>(buf[i]); 
  }
  Serial.print("Encrypted_recieved_data:");
  Serial.print(RecievedData);
  Serial.println();

  // Decrypt
    uint16_t dlen = RecievedData.length();
    sprintf(ciphertext, "%s", RecievedData.c_str());
    String decrypted = decrypt( ciphertext, dlen, dec_iv);
    Serial.print("Cleartext: ");
    Serial.println(decrypted);

//    sprintf(cleartext, "%s", decrypted.c_str());    // must not exceed 255 bytes; may contain a newline
  
  //comparison
    if (decrypted == SendedText) {
      Serial.println("SUCCESS");
//      ledBlink(BlueLED, 2000 , 1);
//      Serial.println("data is correct");
      SendedText = "";
    } else {
      Serial.println("FAILURE");
      //      ledBlink(RedLED , 2000 , 1);
      Serial.println("order to attack");
      SendedText = "";
    }
}




void setup()
{
  Serial.begin(115200);
  Serial.println();

  while (!Serial);

  Serial.println("Select '1' for peer to peer and '2' for Router based");

  while (!Serial.available()); //stuck here untill user pressed any char

  while (Serial.available() > 0) {
    char ch = Serial.read();

    if ( ch == '1') {
      peer_mode = true;
      Serial.println("peer_mode on");
    }
  }

  pinMode(BlueLED , OUTPUT);  //blueled for indicator

  if (peer_mode) {
    //for peer to peer communication
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.disconnect();
    WiFi.softAP("ESPNOW", nullptr, 3);
    WiFi.softAPdisconnect(false);
    // WiFi must be powered on to use ESP-NOW unicast.
    // It could be either AP or STA mode, and does not have to be connected.
    // For best results, ensure both devices are using the same WiFi channel.

    Serial.print("MAC address of this node is ");
    Serial.println(WiFi.softAPmacAddress());

    bool ok = WifiEspNow.begin();
    if (!ok) {
      Serial.println("WifiEspNow.begin() failed");
      ESP.restart();
    }

    WifiEspNow.onReceive(printReceivedMessage, nullptr);

    ok = WifiEspNow.addPeer(PEER);
    if (!ok) {
      Serial.println("WifiEspNow.addPeer() failed");
      ESP.restart();
    }
  }


  if (!peer_mode) {
    //------------WIFI-initialiazation--------
    //We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //----------AES parameter-------
    aesLib.set_paddingmode(paddingMode::CMS);
  }
}
void loop()
{ 
  if (Serial.available()) {

    String readBuffer = Serial.readStringUntil('\r');
    Serial.println("INPUT:" + readBuffer);

    sprintf(cleartext, "%s", readBuffer.c_str());    // must not exceed 255 bytes; may contain a newline

    // Encrypt
    uint16_t clen = String(cleartext).length();
    String encrypted = encrypt(cleartext, clen, enc_iv);
    sprintf(ciphertext, "%s", encrypted.c_str());
    Serial.print("Ciphertext: ");
    Serial.println(encrypted);
    delay(1000);

    SendedText = readBuffer;
    if (peer_mode) {
      char msg[250];
      int len = snprintf(msg, sizeof(msg), "%s",
                         encrypted.c_str());
      WifiEspNow.send(PEER, reinterpret_cast<const uint8_t*>(msg), len);
      delay(1000);

    }

    if (!peer_mode) {

      //--------------send ciphertext to ESP_2------
      Serial.print("connecting to ");
      Serial.print(host);
      Serial.print(':');
      Serial.println(port);

      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      if (!client.connect(host, port)) {
        Serial.println("connection failed");
        delay(5000);
        return;
      }

      // This will send a string to the server
      Serial.println("sending encrypted data to server ESP");
      if (client.connected()) {
        client.println(encrypted);
      }

      // wait for data to be available
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          delay(5000);    //default 60000
          return;
        }
      }

      // Read all the lines of the reply from server and print them to Serial
      Serial.println("receiving from remote server");
      while (client.available()) {
        char ch = char(client.read());
        data_capture += ch;
        Serial.print(ch);
        delay(2);
      }
      Serial.println(); 

    // Decrypt
    uint16_t dlen = data_capture.length();
    sprintf(ciphertext, "%s", data_capture.c_str());
    String decrypted = decrypt( ciphertext, dlen, dec_iv);
    Serial.print("Cleartext: ");
    Serial.println(decrypted);

    //comparison
    if (decrypted.equals(cleartext)) {
      Serial.println("SUCCESS");
      ledBlink(BlueLED, 2000 , 1);
      Serial.println("data is correct");
    } else {
      Serial.println("FAILURE");
      //      ledBlink(RedLED , 2000 , 1);
      Serial.println("order to attack");
    }


    // Close the connection
    Serial.println();
    Serial.println("closing connection");
    client.stop();

    //clearing for next data
    for (int i = 0; i < 16; i++) {
      enc_iv[i] = 0;
      dec_iv[i] = 0;
    }

    data_capture = "" ; //clearing string for next session
   }
  } 
}

void ledBlink(int pin , int blink_rate , int blink_iteration) {
  for (int i = 0 ; i < blink_iteration ; i++) {
    digitalWrite(pin, LOW); //turn on LED
    delay(blink_rate);
    digitalWrite(pin, HIGH);//turn off LED
    delay(blink_rate);
  }
}

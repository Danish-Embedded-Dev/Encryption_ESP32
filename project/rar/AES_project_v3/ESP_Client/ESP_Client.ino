#include <ESP8266WiFi.h>
#include "AESLib.h"



//---------wifi-credential-------
const char* ssid = "JADUR";
const char* password = "jadur1967";

const char* host = "192.168.0.100"; //esp_server ip
const uint16_t port = 17;           //esp_server port


String data_send    = "hello Esp next data will be encrypted";
String data_capture = "" ;

//--------physical-indicator--------
#define BlueLED LED_BUILTIN
#define RedLED 14


AESLib aesLib;

String plaintext = "HELLO WORLD!";

char cleartext[256];
char ciphertext[512];

// AES Encryption Key
byte aes_key[] = { 0x3E , 0xE2, 0x33, 0x55, 0XA2, 0x65, 0x75, 0x5C, 0xC3, 0x26, 0x70, 0x08, 0x06, 0x67, 0x78, 0x40 };

// General initialization vector
byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port
  delay(2000);

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


  pinMode(BlueLED , OUTPUT);
  pinMode(RedLED  , OUTPUT);
  digitalWrite(BlueLED, HIGH);//turn off BlueLED
  digitalWrite(RedLED, HIGH); //turn off RedLED
  
  //----------AES parameter-------
  aesLib.set_paddingmode(paddingMode::CMS);



  Serial.println("Enter text to be encrypted into console (no feedback) and press ENTER (newline):");
}



void loop() {

  if (Serial.available()) {

    String readBuffer = Serial.readStringUntil('\n');
    Serial.println("INPUT:" + readBuffer);

    sprintf(cleartext, "%s", readBuffer.c_str());    // must not exceed 255 bytes; may contain a newline

    // Encrypt
    uint16_t clen = String(cleartext).length();
    String encrypted = encrypt(cleartext, clen, enc_iv);
    sprintf(ciphertext, "%s", encrypted.c_str());
    Serial.print("Ciphertext: ");
    Serial.println(encrypted);
    delay(1000);

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
      ledBlink(RedLED , 2000 , 1);
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


void ledBlink(int pin , int blink_rate , int blink_iteration) {
  for (int i = 0 ; i < blink_iteration ; i++) { 
    digitalWrite(pin, LOW); //turn on LED
    delay(blink_rate);
    digitalWrite(pin, HIGH);//turn off LED
    delay(blink_rate);
  }
}

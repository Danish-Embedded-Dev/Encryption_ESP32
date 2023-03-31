#include <ESP8266WiFi.h>
#include "AESLib.h"

//---------connect to local WIFI---------
const char* ssid = "JADUR";
const char* password = "jadur1967";

const uint16_t port = 17;           //esp_server port
WiFiServer server(port);

//-----------AES parameter----------

AESLib aesLib;

String plaintext = "";
String data_capture = "" ;

char cleartext[256];
char ciphertext[512];

// AES Encryption Key
byte aes_key[] = { 0x3E , 0xE2, 0x33, 0x55, 0XA2, 0x65, 0x75, 0x5C, 0xC3, 0x26, 0x70, 0x08, 0x06, 0x67, 0x78, 0x40 };

// General initialization vector
byte enc_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // iv_block gets written to, provide own fresh copy...
byte dec_iv[N_BLOCK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  Serial.begin(115200);

  // Connect to WiFi network
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());


  //----------AES parameter-------
  aesLib.set_paddingmode(paddingMode::CMS);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000
  delay(100); //delay to wait buffer to filled

  while (client.available()) {
    char c = client.read();
    data_capture += c ;
    //    client.print(c); //data echo
  }

  // Decrypt
  uint16_t dlen = data_capture.length();
  sprintf(ciphertext, "%s", data_capture.c_str());
  String decrypted = decrypt( ciphertext, dlen, dec_iv);
  Serial.print("Cleartext: ");
  Serial.println(decrypted);

  sprintf(cleartext, "%s", decrypted.c_str());    // must not exceed 255 bytes; may contain a newline
   
  // Encrypt
  uint16_t clen = String(cleartext).length();
  String encrypted = encrypt(cleartext, clen, enc_iv);
  sprintf(ciphertext, "%s", encrypted.c_str());
  Serial.print("Ciphertext: ");
  Serial.println(encrypted);

  //sending cipher to client_ESP
  client.print(encrypted);
  
  Serial.println(F("Disconnecting from client"));

  //clearing for next data
  for (int i = 0; i < 16; i++) {
    enc_iv[i] = 0;
    dec_iv[i] = 0;
  }

  data_capture = "" ; //clearing string for next session
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

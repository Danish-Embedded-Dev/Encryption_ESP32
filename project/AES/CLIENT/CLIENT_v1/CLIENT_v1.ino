/**
 * @file
 *
 * EspNowUnicast.ino demonstrates how to transmit unicast ESP-NOW messages with @c WifiEspNow .
 * You need two ESP8266 or ESP32 devices to run this example.
 *
 * Unicast communication requires the sender to specify the MAC address of the recipient.
 * Thus, you must modify this program for each device.
 *
 * The recommended workflow is:
 * @li 1. Flash the program onto device A.
 * @li 2. Run the program on device A, look at serial console for its MAC address.
 * @li 3. Copy the MAC address of device A, paste it in the @c PEER variable below.
 * @li 4. Flash the program that contains A's MAC address onto device B.
 * @li 5. Run the program on device A, look at serial console for its MAC address.
 * @li 6. Copy the MAC address of device B, paste it in the @c PEER variable below.
 * @li 7. Flash the program that contains B's MAC address onto device A.
 */

//for peer to peer communicationn 
#include <WifiEspNow.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif

// The recipient MAC address. It must be modified for each device.
//static uint8_t PEER[]{0xB6, 0xE6, 0x2D, 0x09, 0x71, 0x8C};

//static uint8_t PEER[]{0xC6, 0x5B, 0xBE, 0x62, 0xC6, 0x5E};//spring mac
static uint8_t PEER[]{0xB6, 0xE6, 0x2D, 0x09, 0x71, 0x8C}; //non-spring


bool peer_mode =  false; 

void printReceivedMessage(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count,
                     void* arg)
{
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3],
                mac[4], mac[5]);
  for (int i = 0; i < static_cast<int>(count); ++i) {
    Serial.print(static_cast<char>(buf[i]));
  }
  Serial.println();
}



void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  while(!Serial);
  
  Serial.println("Select '1' for peer to peer and '2' for Router based");
  
  while(!Serial.available());  //stuck here untill user pressed any char

  while(Serial.available()>0){
    char ch = Serial.read();

    if ( ch == '1'){
      peer_mode = true;
      Serial.println("peer_mode on");
    }
  }

 if(peer_mode){
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


 if(!peer_mode){
    pinMode(LED_BUILTIN,OUTPUT);
 }
}

void loop()
{
  if(peer_mode){
    char msg[60];
    int len = snprintf(msg, sizeof(msg), "hello ESP-NOW from %s at %lu",
                       WiFi.softAPmacAddress().c_str(), millis());
    WifiEspNow.send(PEER, reinterpret_cast<const uint8_t*>(msg), len);
    delay(1000);
  }

  if(!peer_mode){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN,LOW);
    delay(1000);
  }
  
}

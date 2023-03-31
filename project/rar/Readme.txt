->Here we are using Esp8266 to Esp8266 data sending and receiving via tcp client/server protocol as our communication medium 
->here both esp are connected to local wifi router so that both get onto the same private network 
->get ip of the server esp and put the ip on the client esp so that client esp send the encrypted data to server esp and
  then server esp decrypt the data and encrypt it again and send back to client esp 
->the encrypted data then recieved by client esp via tcp packet and then it decrypt it 
->while sending the data client esp make copy of sended data and after recieveing the data it compare it with sended data to check weather the 
  data recieved properly or damage by network of some noise .
->two led our connected to digital output one is Red one which turn when ever attack is call and when data is clear blue LED will turn on 


library Used
https://www.arduino.cc/reference/en/libraries/aeslib/
https://github.com/suculent/thinx-aes-lib
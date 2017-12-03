#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
boolean onConnection, authorize, handShaked;
String data, dataHandShake;
void setup(){
  while (!Serial);
  Serial.begin(9600);
  Serial.println(F("Now is Debugging Mode with SysOut"));
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPALevel( RF24_PA_HIGH ) ;
  clearData();
  closeConnection();
}
void loop(){
  while (!onConnection){
    if (getSignal()){
      if (data != ""){
        Serial.println("Signal Received, Opening Connection from " + data);
        onConnection = true;     
      }
    }
  }
  while (onConnection){
    Serial.println("Doing Transaction with Services");
    transaction();
    Serial.println("Get Transaction Result, Send it To Door");
    sendRespond(authorize);
    Serial.println("Respond has Been Send, Clearing Data and Waiting for Connection");
    clearData();
    closeConnection();
  }
}
boolean getSignal(){
  boolean stat = false;
  openListeningConnection();
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    data = String(text);
    stat = true;
  }
  return stat;
}
void transaction(){
  while(!Serial.available()){};
  if (Serial.read() == 'a'){
    authorize = true;
  } else if (Serial.read() == 'b'){
    authorize = false;
  }
}
void sendRespond(boolean authorize){
  int i = 0;
  openWritingConnection();
  while (i < 17){
    if (authorize){
      Serial.println(F("Authorizing..."));
      const char text[] = "AccessGranted";
      radio.write(&text, sizeof(text));
    } else {
      Serial.println(F("Denied..."));
      const char text[] = "AccessGrantid";
      radio.write(&text, sizeof(text));
    }
    i++;
    delay(50);
//    for (int i = 0; i < 20; i++){
//      Serial.println("Waiting Hand Shake");
//      handShakingReceive();
//      delay(100);
//    }
  }
}
void handShakingReceive(){
  openListeningConnection();
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    dataHandShake = String(text);
    Serial.println("Data Hand Sake = " + dataHandShake);
    if (dataHandShake == "ok"){
      Serial.println("Hand Shaked!");
      handShaked = true;
      //handShakingSend();
    }
  }
  delay(100);
}
void handShakingSend(){
  openWritingConnection();
  const char respond[] = "oks";
  radio.write(&respond, sizeof(respond));
}
void openWritingConnection(){
  radio.openWritingPipe(rxAddr);
  radio.stopListening();
}
void openListeningConnection(){
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();
}
void clearData(){
  data = "";
  dataHandShake = "";
  authorize = false;
  handShaked = false;
}
void closeConnection(){
  onConnection = false;
}

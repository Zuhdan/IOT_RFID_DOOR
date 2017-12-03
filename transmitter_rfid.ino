#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <nRF24L01.h>
#include <RF24.h>
MFRC522 mfrc522(10, 9);
RF24 radio(7, 8);
byte readCard[4];
const byte rxAddr[6] = "00001";
uint8_t successRead;
boolean onConnection, authorize, result, handShaked, handShakingReceived;
String data, readedCard, dataHandShake;
void setup() {
  Serial.begin(9600);
  Serial.println("Ready, Tap The Card!");
  SPI.begin();
  mfrc522.PCD_Init();
  radio.begin();
  radio.setRetries(15, 15);
  radio.setPALevel( RF24_PA_HIGH ) ;
  clearData();
  closeConnection();
}
void loop () {
  do {
    successRead = getID();
  } while (!successRead);
  if (successRead){
    Serial.println("ID Readed");
    readCards();
    Serial.println("ID Card is " + readedCard);
    sendSignal(readedCard);
    Serial.println("Getting Authorize, and Showing Result");
    result = gettingAuthorize();
    showResult(result);
    clearData();
  }
}
uint8_t getID() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  mfrc522.PICC_HaltA();
  return 1;
}
void readCards(){
  for ( uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    readedCard = String(readedCard + readCard[i]);
  }
}
void sendSignal(String readedCards){
  //todo, isSignalReceived
  openWritingConnection();
  String builder = "FirstDoor," + readedCards;
  int builder_len = builder.length() + 1;
  const char charBuilder[builder_len];
  builder.toCharArray(charBuilder, builder_len);
  radio.write(&charBuilder, sizeof(charBuilder));
  Serial.println(charBuilder);
  Serial.println("Signal has been Sent!");
}
boolean gettingAuthorize(){
  boolean res;
  Serial.println("Open Listening Connection");
  openListeningConnection();
  while (!onConnection){
    if (getSignal()){
      onConnection = true;  
    }
  }
  while (onConnection){
    res = transaction();
    closeConnection();
    clearData();
  }
  return res;
}
boolean getSignal(){
  boolean stat = false;
  openListeningConnection();
  if (radio.available() > 0)
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    data = String(text);
    Serial.println(data);
    if (data != ""){
      Serial.println("Signal Auth Received");
      radio.stopListening();
      stat = true;
    }
    //data == "AccessGranted" || data == "AccessDenied"
  }
  return stat;
}
boolean transaction(){
  boolean res;
  if (data == "AccessGranted"){
    res = true;
  } else if (data == "AccessGrantid"){
    res = false;
  }
  return res;
}
void showResult(boolean result){
   if (result){
     Serial.println("------> DOOR UNLOCKED <------");
   } else {
     Serial.println("------> DOOR STILL LOCKED <------");
   }
}
void handShaking(){
  Serial.println("state handShakingReceived = " + handShakingReceived);
  //while (!handShakingReceived){
    openWritingConnection();
    Serial.println("Sending handShaking OK 12 times");
    for (int i = 0; i < 20; i++){
      Serial.println("Sending Hand Sake");
      const char respond[] = "ok";
      radio.write(&respond, sizeof(respond));
      delay (100);
    }
//    handShakingReceive();
  //}
}
void handShakingReceive(){
  openListeningConnection();
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    dataHandShake = String(text);
    Serial.println("Data Hand Sake = " + dataHandShake);
    if (dataHandShake == "oks"){
      Serial.println("Hand Shaked!");
      handShakingReceived = true;
    }
  }
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
  readedCard = "";
  successRead = 0;
  data = "";
  dataHandShake = "";
  handShakingReceived = false;
  authorize = false;
}
void closeConnection(){
  onConnection = false;
}


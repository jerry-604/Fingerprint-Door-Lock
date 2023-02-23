CODE
/***************************************************
 Code for fingerprint lock using example code from the 
 optical fingerprint sensor library

  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h>
#include <Servo.h>
//lcd d7 becomes 4
//lcd d6 becomes 5


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define MOTOR  13 
Servo MServ;  

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#define BRHT 3
#define BUP 4
#define BDWN 5
#define BLFT 6
//initializing shield buttons




#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;
int locked = 135;
int unlocked = 50;

void setup()
{
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.clear();
  lcd.print("setup");
  /**
   * inititlisation as well as making sure the sensor is wired correctly
   */

  //servo init
  MServ.attach(MOTOR);
  MServ.write(locked); 
  //button up and down
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  //finger.getTemplateCount();

  //if (finger.templateCount == 0) {
  //  Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  //}
  //else {
  //  Serial.println("Waiting for valid finger...");
  //    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  //}
}

uint8_t readnumber(void) {
  uint8_t num = 1;
  num = random(100);

  //while (num == 0) {
    //while (! Serial.available());
    //num = Serial.parseInt();
  //}
  return num;
}
bool d4curr = true;
bool d4prev = true;
bool d5curr = true;
bool d5prev = true;
bool kloop = true;
bool scanloop = true;
//for rising touches


void loop()                     // run over and over again
{
  
  if (kloop){
    //prevent shell from spam
    Serial.println("choose up to set finger, choose down to open door");
    lcd.clear();
    lcd.print("up to attempt");
    lcd.setCursor(0, 1);
    lcd.print("down to add fing");
    
    kloop = false;
  }
  
  d4prev = d4curr;
  d4curr = digitalRead(4);
  if(d4curr == false && d4prev == true){
    kloop = true;
    scanloop = true;
    //calls the enroll(add) function which scans and adds a fingerprint to the sensor
    MServ.write(locked);
    while(scanloop){
      d4prev = d4curr;
      d4curr = digitalRead(4);
      if(d4curr == false && d4prev == true){
        scanloop = false;
      }
      
      lcd.clear();
      lcd.print("place");
      lcd.setCursor(0, 1);
      lcd.print("finger");
      int p = add();
      if(p != FINGERPRINT_OK){
        Serial.println("failed. to scan");
        lcd.setCursor(0, 0);
        lcd.clear();
        lcd.print("Failed");
        lcd.setCursor(0, 1);
        lcd.print("try again");
        delay(700);
        }
      else{
        lcd.clear();
        lcd.print("finger");
        lcd.setCursor(0,1);
        lcd.print("registered");
        scanloop = false;
      }
    }
  }

  d5prev = d5curr;
  d5curr = digitalRead(5);
  if(d5curr == false && d5prev == true){
    kloop = true;
    //calls the checkfinger(getfingerprintid) function which scans a fingerprint
      //and checks if its in the database
      //the user should be let in on success
    scanloop = true;
    
    while(scanloop){
      d5prev = d5curr;
      d5curr = digitalRead(5);
      if(d5curr == false && d5prev == true){
        scanloop = false;
      }

      int p = getFingerprintID();
      Serial.println();
      Serial.println();
      //Serial.println(FINGERPRINT_OK);
      if(p != FINGERPRINT_OK){
        Serial.println("failed. click up to try again");
        lcd.clear();
        // print the number of seconds since reset:
        lcd.print("failed scan");
        lcd.setCursor(0,1);
        lcd.print("another");
        }
      else{
        lcd.clear();
        lcd.print("finger accepted");
        lcd.setCursor(0, 1);
        lcd.print("opening");
        MServ.write(unlocked);
        delay(3000);
        MServ.write(locked);
        scanloop = false;
      }
      
    }
    
  }
  
  
  
}

bool add(){
  //custom function that scans finger and 
  Serial.println("Ready to enroll a fingerprint!");
  //Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();//random dont if alr add

  Serial.print("Enrolling ID #");
  Serial.println(id);

  //while (!  getFingerprintEnroll() );
  return getFingerprintEnroll();
  
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  lcd.clear();
  lcd.print("place finger");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.print("image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.clear();
  lcd.print("remove");
  lcd.setCursor(0, 1);
  lcd.print("finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.clear();
  lcd.print("put same");
  lcd.setCursor(0, 1);
  lcd.print("finger");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
      lcd.clear();
      lcd.print("prints");
      lcd.setCursor(0, 1);
      lcd.print("matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return FINGERPRINT_OK;
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  //return finger.fingerID;
  return p;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
/////////
////
///

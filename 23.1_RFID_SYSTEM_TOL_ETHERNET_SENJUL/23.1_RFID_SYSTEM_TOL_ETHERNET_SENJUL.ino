#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> //6.17.3
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.1.50"; // change to your PC's IP address
String PATH_NAME   = "/tolsystem/data-api.php";
String getData;

#define SS_PIN 9
#define RST_PIN 8
#define buzzer 7
//inisialisasi pembuatan variable
int R = 2;
int Y = 4;
int G = 5;
int IR = 6;
int kunci = 0;
int outServo= 3; //Pin untuk Servo
Servo myservo;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

void setup() {
  Serial.begin(115200);
  pinMode (buzzer,OUTPUT);
  pinMode(IR,INPUT);
  myservo.attach(outServo);
  while(!Serial);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  
  //START IP DHCP
  Serial.println("Konfigurasi DHCP, Silahkan Tunggu!");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Gagal!");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet Tidak tereteksi :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Hubungkan kabel Ethernet!");
    }
    while (true) {delay(1);}
  }  
  //End DHCP 
  delay(5000); 
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());  
  client.connect(HOST_NAME, HTTP_PORT);
  Serial.println("SIAP DIGUNAKAN!");
  buzzerBunyi();
  digitalWrite (R, HIGH);
  digitalWrite (Y, HIGH);
  digitalWrite (G, HIGH);
  delay(250);
  digitalWrite (R, LOW);
  digitalWrite (Y, LOW);
  digitalWrite (G, LOW);
  delay(250);
  digitalWrite (R, HIGH);
  digitalWrite (Y, HIGH);
  digitalWrite (G, HIGH);
  delay(250);
  digitalWrite (R, LOW);
  digitalWrite (Y, LOW);
  digitalWrite (G, LOW);
  delay(1000);
  myservo.write(0);
  Serial.println("SELAMAT DATANG");
  delay(500);
  Serial.println("TEMPELKAN KARTU!");
}

void loop() {
  //Baca data
   //Program yang akan dijalankan berulang-ulang
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String uidString;
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     uidString.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "" : ""));
     uidString.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.print("Message : ");
  uidString.toUpperCase();
  Serial.println(uidString);
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);

  String namatol = "Bandung";

  //POST TO WEB
    client.connect(HOST_NAME, HTTP_PORT);
    client.println(HTTP_METHOD + " " + PATH_NAME + 
                   "?rfid=" + String(uidString) + 
                   "&namatol=" + namatol + 
                   " HTTP/1.1");
                   
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header
  
    while(client.connected()) {
      if(client.available()){
        char endOfHeaders[] = "\r\n\r\n";
        client.find(endOfHeaders);
        getData = client.readString();
        getData.trim();
        
        //AMBIL DATA JSON
        const size_t capacity = JSON_OBJECT_SIZE(9) + 170; //cari dulu nilainya pakai Arduino Json 5 Asisten
        DynamicJsonDocument doc(capacity);
        //StaticJsonDocument<192> doc;
        DeserializationError error = deserializeJson(doc, getData);


        const char* no_dibaca       = doc["no"]; 
        const char* rfid_dibaca     = doc["rfid"]; 
        const char* nama_dibaca     = doc["nama"]; 
        const char* alamat_dibaca   = doc["alamat"]; 
        const char* telepon_dibaca  = doc["telepon"]; 
        const char* saldo           = doc["saldo"]; // "989000"
        const char* tanggal         = doc["tanggal"]; // "2021-12-09 10:55:10"
        const char* namatol         = doc["namatol"]; // "jakarta"
        const char* tarif           = doc["tarif"]; // "5000"
     
        
       //LOGIKA
       if(String(nama_dibaca)!=""){
        Serial.println("BERHASIL!");
        Serial.println(getData);
        //POST TO SERIAL
         Serial.print("No        = ");Serial.println(no_dibaca);
         Serial.print("RFID      = ");Serial.println(rfid_dibaca);
         Serial.print("Nama      = ");Serial.println(nama_dibaca);
         Serial.print("Alamat    = ");Serial.println(alamat_dibaca);
         Serial.print("Telepon   = ");Serial.println(telepon_dibaca);         
         Serial.print("Saldo     = ");Serial.println(saldo);
         Serial.print("Waktu     = ");Serial.println(tanggal);
         Serial.print("Pintu TOL = ");Serial.println(namatol);
         Serial.print("Tarif TOL = ");Serial.println(tarif);
         buzzerBunyi();
         Serial.println("Berhasil!");
         digitalWrite (G, HIGH);
         digitalWrite (R, LOW);
         digitalWrite (Y, LOW);
         delay(500);
         Serial.println("Silahkan Masuk");
         myservo.write(90); //0-180
         delay(1000); //Jeda waktu perulagan seama 500 mili detik
          
          while(1){
           int irbaca = digitalRead(IR);
           if(irbaca == 1){
            continue;
           }else{
            delay(500);
            buzzerBunyi();
            myservo.write(0);
            Serial.println("Gerbang Tertutup");
            break;
           }
          }
      
       }else if(String(saldo)<="4999"{
        digitalWrite (Y, HIGH);
        digitalWrite (R, LOW);
        digitalWrite (G, LOW);
        Serial.println("SALDO TIDAK MENCUKUPI");
        buzzerBunyiGagal();
        delay(200);
        Serial.println("SILAHKAN LAKUKAN ISI ULANG SALDO TERLEBIH DAHULU ATAU PINJAM KARTU MILIK PENGENDARA LAIN DIBELAKANG");
        delay(200);
        Serial.println("TEMPELKAN KARTU!");
        digitalWrite (R, HIGH);
        digitalWrite (Y, LOW);
        digitalWrite (G, LOW);
       }
      }else if(String(nama_dibaca)==""){
        digitalWrite (Y, HIGH);
        digitalWrite (R, LOW);
        digitalWrite (G, LOW);
        Serial.println("KARTU TIDAK TERDAFTAR");
        buzzerBunyiGagal();
        Serial.println("TEMPELKAN KARTU!");
        digitalWrite (R, HIGH);
        digitalWrite (Y, LOW);
        digitalWrite (G, LOW);
       }
      } 
    }
}

  
void buzzerBunyi(){
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
  
}


void buzzerBunyi2(){
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);  
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
  
}


void buzzerBunyiGagal(){
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
}

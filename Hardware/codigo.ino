#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <RH_RF95.h>

#define DHTPIN1 16     
#define DHTPIN2 17     
#define DHTTYPE DHT22  

#define HUMIDITY_SENSOR_1 12
#define HUMIDITY_SENSOR_2 14
#define HUMIDITY_SENSOR_3 27
#define HUMIDITY_SENSOR_4 26
#define HUMIDITY_SENSOR_5 25

#define RFM95_CS 10    
#define RFM95_RST 13    
#define RFM95_INT 2    

#define RF95_FREQ 915.0

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  Serial.begin(9600);

  dht1.begin();
  dht2.begin();

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("Fallo inicializando LoRa!");
    while (1);
  }

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Fallo al establecer la frecuencia LoRa!");
    while (1);
  }

  rf95.setTxPower(23, false); 
}

void loop() {
  int soilMoisture[] = {
    analogRead(HUMIDITY_SENSOR_1),
    analogRead(HUMIDITY_SENSOR_2),
    analogRead(HUMIDITY_SENSOR_3),
    analogRead(HUMIDITY_SENSOR_4),
    analogRead(HUMIDITY_SENSOR_5)
  };

  float temp1 = dht1.readTemperature();
  float hum1 = dht1.readHumidity();
  float temp2 = dht2.readTemperature();
  float hum2 = dht2.readHumidity();

  if (isnan(temp1) || isnan(hum1) || isnan(temp2) || isnan(hum2)) {
    Serial.println("Error leyendo los sensores DHT22!");
    return;
  }

  Serial.print("Hum1: "); Serial.print(soilMoisture[0]);
  Serial.print(" | Hum2: "); Serial.print(soilMoisture[1]);
  Serial.print(" | Temp1: "); Serial.print(temp1);
  Serial.print(" | HumRel1: "); Serial.println(hum1);

  char packet[100];
  snprintf(packet, sizeof(packet), 
           "M1:%d M2:%d M3:%d M4:%d M5:%d T1:%.2f H1:%.2f T2:%.2f H2:%.2f",
           soilMoisture[0], soilMoisture[1], soilMoisture[2], 
           soilMoisture[3], soilMoisture[4], temp1, hum1, temp2, hum2);

  rf95.send((uint8_t *)packet, strlen(packet));
  rf95.waitPacketSent();

  delay(10000); 
}
#define BLYNK_TEMPLATE_ID "TMPL6-oPE1N79"
#define BLYNK_TEMPLATE_NAME "Monitoring tanaman tomat"
#define BLYNK_AUTH_TOKEN "9wKP_FE9c9CGWFbjrdMly1QYhGCV70a6"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

const int LDR_PIN = 33;
const int LED_PIN = 14;
const int DHTType = DHT22;
const int SERVO_PIN = 25;
const int servoMinAngle = 0;
const int servoMaxAngle = 180;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHT_PIN 4
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTType);
#define Echo 15
#define Trig 2
#define BUZZER 26
#define LCD_COLUMNS 20
#define LCD_LINES 4
#define I2C_ADDR 0x27

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
Servo servo;

float jarak = 0;
int nada;
int ldrValue = 0;
float temperature = 0;
float humidity = 0;

BLYNK_WRITE(V3)
{
  nada = param.asInt();
}

BlynkTimer timer;

void updateBlynkData()
{ 
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, jarak);
  Blynk.virtualWrite(V3, nada);
}

void readDHTSensor()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read DHT sensor data");
  }
}

void updateLED()
{
  if (ldrValue < 500)
    digitalWrite(LED_PIN, LOW);
  else
    digitalWrite(LED_PIN, HIGH);
}

void updateServo()
{
  float humidity = dht.readHumidity();
  int servoAngle = map(humidity, 66, 0, servoMinAngle, servoMaxAngle);
  servo.write(servoAngle);
}

void ukur_jarak()
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  unsigned long duration = pulseIn(Echo, HIGH);
  jarak = duration * 0.034 / 2 + 1;
}

void readLDR()
{
  ldrValue = analogRead(LDR_PIN);
}

void buzzControl()
{
  if (nada == 0 || (nada > 0 && jarak > 400)) {
    noTone(BUZZER);
  } else {
    tone(BUZZER, nada);
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  timer.setInterval(5000L, readDHTSensor);
  timer.setInterval(1000L, readLDR);
  timer.setInterval(1000L, ukur_jarak);
  timer.setInterval(5000L, updateBlynkData); 
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  Blynk.virtualWrite(V4, 0); // Matikan buzzer secara default
  Blynk.syncVirtual(V4); 
}

void loop()
{
  Blynk.run();
  timer.run();
  readLDR();
  readDHTSensor();
  ukur_jarak();
  updateLED();
  updateServo();
  updateBlynkData();
  buzzControl();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperature: " + String(temperature) + "\xDF" + "C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: " + String(humidity) + " %" );

  lcd.setCursor(0, 2);
  lcd.print("Cahaya: ");
  if (ldrValue < 500) {
    lcd.print("Terang");
  } else {
    lcd.print("Gelap");
  }

  lcd.setCursor(0, 3);
  lcd.print("Ada Hama? ");
  if (jarak > 400) {
    lcd.print("Aman");
    Serial.println("Aman");
    noTone(BUZZER);
  } else if (jarak > 200 && jarak < 400) {
    lcd.print("mendekat");
    Serial.println("Hama mendekat");
    noTone(BUZZER);
  } else if (jarak < 200) {
    lcd.print("ADA!");
    Serial.println("Ada Hama");
    buzzControl();
  }

  Serial.print("LDR: ");
  Serial.println(ldrValue);
  Serial.println("Temperature: " + String(temperature) + " C");
  Serial.println("Humidity: " + String(humidity) + " %" );
  Serial.print("Jarak: ");
  Serial.print(jarak);
  Serial.println(" cm");

  digitalWrite(BUZZER, HIGH);
  delay(1000);
}

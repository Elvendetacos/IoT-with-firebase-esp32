#include <math.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Megacable_L6hc8VA"
#define WIFI_PASSWORD "DAyX4ZFJrrEzJT7QHQ"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAJlJYGGVx2-9nGlgKkE5wJJj0092Y6MPs"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://amili-7cf9e-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "alex.amili@gmail.com"
#define USER_PASSWORD "amili2709"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

int controlA=13;
int controlB=12;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
int suich = 33;
const int LDR = 35;
const int Res = 10;
float valorLDR = 0.0;
int humedad1 = 36;
int humedad2 = 39;
const int term = 32;
const int thermistor = 32;
const double VCC = 3.3;
const double R2 = 10000;           
const double adc_resolution = 4095; 
char buffer[50];

const double A = 0.001129148;   
const double B = 0.000234125;
const double C = 0.0000000876741;

int controlA=13;
int controlB=12;

void setup() {
  Serial.begin(115200);
  pinMode(LDR, INPUT);
  pinMode(suich, INPUT_PULLUP);

  pinMode(controlA, OUTPUT);
  pinMode(controlB, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
}

double Thermister(int RawADC) {
  double Temp;
  Temp = log(((40950000 / RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  Temp = Temp - 273.15;  // Converierte de Kelvin a Celsius
  //Para convertir Celsius a Farenheith esriba en esta linea: Temp = (Temp * 9.0)/ 5.0 + 32.0;
  return Temp;
}

float Luces(){
  float valorLDR = 0.0;
  int tension = analogRead(LDR);
  float voltaje = (tension * 4.000) / 4095;

  if (voltaje >= 2.0) {
    tension = tension - 1500;
    float tension1 = tension;
    valorLDR = (3276.00 * Res / tension1) - Res;

  } else {
    tension = tension;
    float tension1 = tension;
    valorLDR = (4095.00 * Res / tension1) - Res;
  }

  float lux = 800.69 * pow(valorLDR, -1.253);
  lux = round(lux * 10) / 10; 
  return lux;
}

int suichValue(){
  int suich_valor = digitalRead(suich);
  return suich_valor;
}

int humedadFunction1(){
  int valorHumedad1 = map(analogRead(humedad1), 0, 4095, 100, 0);
  return valorHumedad1;
}

int humedadFunction2(){
  int valorHumedad2 = map(analogRead(humedad2), 0, 4095, 100, 0);
  return valorHumedad2;
}

double temperaturaSensor(){
    double Vout, Rth, temperature, adc_value; 

  adc_value = analogRead(thermistor);
  Vout = (adc_value * VCC) / adc_resolution;
  Rth = (VCC * R2 / Vout) - R2;

  temperature = (1 / (A + (B * log(Rth)) + (C * pow((log(Rth)),3)))); 

  temperature = temperature - 273.15;
  temperature = round(temperature * 10) / 10; 
  return temperature;
}


void loop() {

  float valorLux = Luces();
  int suichable = suichValue();
  int humedadValorFinal1 = humedadFunction1();
  int humedadValorFinal2 = humedadFunction2();
  double Temperatura = temperaturaSensor();

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/amili/esp32/humidity1"), humedadValorFinal1) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/amili/esp32/humidity1")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/amili/esp32/humidity2"), humedadValorFinal2) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/amili/esp32/humidity2")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.setDouble(fbdo, F("/amili/esp32/temperature"), Temperatura) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.getDouble(fbdo, F("/amili/esp32/temperature")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.setFloat(fbdo, F("/amili/esp32/light"), valorLux) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.getFloat(fbdo, F("/amili/esp32/light")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/amili/esp32/waterLevel"), suichable) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.getInt(fbdo, F("/amili/esp32/waterLevel")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/amili/esp32/productKey"), "1234qwer") ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/amili/esp32/productKey")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

    String valorFirebase = Firebase.getFloat(fbdo, F("/amili/statistics/humidity/median")) ? String(fbdo.to<float>()) : String(fbdo.errorReason());
    snprintf(buffer, sizeof(buffer), valorFirebase.c_str());
    Serial.print(buffer);
    int valorNumerico = valorFirebase.toInt();

    if (valorNumerico < 80 || Temperatura > 27) {
      digitalWrite(controlA, LOW);
      digitalWrite(controlB, HIGH);
      Serial.println("Bomba prendida");
      delay(30000); 
    } else {
      digitalWrite(controlA, LOW);
      digitalWrite(controlB, LOW);
      Serial.println("Bomba apagada");
      delay(30000); 
    }     

    FirebaseJson json;

    Serial.println();

    count++;
  }
}





/// PLEASE AVOID THIS ////

// Please avoid the following inappropriate and inefficient use cases
/**
 *
 * 1. Call get repeatedly inside the loop without the appropriate timing for execution provided e.g. millis() or conditional checking,
 * where delay should be avoided.
 *
 * Everytime get was called, the request header need to be sent to server which its size depends on the authentication method used,
 * and costs your data usage.
 *
 * Please use stream function instead for this use case.
 *
 * 2. Using the single FirebaseData object to call different type functions as above example without the appropriate
 * timing for execution provided in the loop i.e., repeatedly switching call between get and set functions.
 *
 * In addition to costs the data usage, the delay will be involved as the session needs to be closed and opened too often
 * due to the HTTP method (GET, PUT, POST, PATCH and DELETE) was changed in the incoming request.
 *
 *
 * Please reduce the use of swithing calls by store the multiple values to the JSON object and store it once on the database.
 *
 * Or calling continuously "set" or "setAsync" functions without "get" called in between, and calling get continuously without set
 * called in between.
 *
 * If you needed to call arbitrary "get" and "set" based on condition or event, use another FirebaseData object to avoid the session
 * closing and reopening.
 *
 * 3. Use of delay or hidden delay or blocking operation to wait for hardware ready in the third party sensor libraries, together with stream functions e.g. Firebase.RTDB.readStream and fbdo.streamAvailable in the loop.
 *
 * Please use non-blocking mode of sensor libraries (if available) or use millis instead of delay in your code.
 *
 * 4. Blocking the token generation process.
 *
 * Let the authentication token generation to run without blocking, the following code MUST BE AVOIDED.
 *
 * while (!Firebase.ready()) <---- Don't do this in while loop
 * {
 *     delay(1000);
 * }
 *
 */
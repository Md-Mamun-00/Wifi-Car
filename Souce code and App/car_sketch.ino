/******************* WiFi Robot Remote + Object Follower Mode - L298N 2A ********************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

// connections for drive Motor FR & BR
int enA = D1;
int in1 = D2;
int in2 = D3;
// connections for drive Motor FL & BL
int in3 = D4;
int in4 = D5;
int enB = D6;

// ultrasonic setup:
const int trigPin = D7;
const int echoPin = D8;
const int wifiLedPin = D0;

int distanceCm = 0;
int distanceKeep = 10;
int SPEED = 800;
bool isFollowing = false;

String command;
String sta_ssid = "";
String sta_password = "";

ESP8266WebServer server(80);
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("*WiFi Robot Remote + Object Follower Mode - L298N 2A*");
  Serial.println("-----------------------------------------------------");

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(wifiLedPin, OUTPUT);
  digitalWrite(wifiLedPin, HIGH);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  // Hostname
  String chip_id = String(ESP.getChipId(), HEX);
  chip_id = "wificar-" + chip_id.substring(chip_id.length() - 4);
  String hostname = chip_id;
  Serial.println("Hostname: " + hostname);

  // WiFi connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("Connecting to: " + sta_ssid);

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && millis() - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n*WiFi-STA-Mode*");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiLedPin, LOW);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("\nWiFi failed. AP Mode activated.");
    Serial.print("AP IP: ");
    Serial.println(myIP);
    digitalWrite(wifiLedPin, HIGH);
  }

  // Web + OTA
  server.on("/", HTTP_handleRoot);
  server.onNotFound(HTTP_handleRoot);
  server.begin();
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  command = server.arg("State");

  if (command == "U_1") {
    isFollowing = true;
    Serial.println("Object Following Mode ENABLED");
  }
  else if (command == "U_0") {
    isFollowing = false;
    Stop();
    Serial.println("Object Following Mode DISABLED");
  }

  if (isFollowing) {
    followObject();
    return;
  }

  // Remote commands
  if      (command == "F") Forward();
  else if (command == "B") Backward();
  else if (command == "R") TurnRight();
  else if (command == "L") TurnLeft();
  else if (command == "G") ForwardLeft();
  else if (command == "H") BackwardLeft();
  else if (command == "I") ForwardRight();
  else if (command == "J") BackwardRight();
  else if (command == "S") Stop();
  else if (command == "0") SPEED = 330;
  else if (command == "1") SPEED = 400;
  else if (command == "2") SPEED = 470;
  else if (command == "3") SPEED = 540;
  else if (command == "4") SPEED = 610;
  else if (command == "5") SPEED = 680;
  else if (command == "6") SPEED = 750;
  else if (command == "7") SPEED = 820;
  else if (command == "8") SPEED = 890;
  else if (command == "9") SPEED = 960;
  else if (command == "q") SPEED = 1023;
}

// Root Handler
void HTTP_handleRoot() {
  server.send(200, "text/html", "");
  if (server.hasArg("State")) {
    Serial.print("Command: ");
    Serial.println(server.arg("State"));
  }
}

// Ultrasonic distance function
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  int echoTime = pulseIn(echoPin, HIGH, 30000);
  int distance = echoTime / 58.26;
  return (distance > 0) ? distance : 999;
}

// Object follower logic
void followObject() {
  distanceCm = getDistance();
  Serial.print("Distance: ");
  Serial.println(distanceCm);

  if (distanceCm < distanceKeep && distanceCm >= 0) {
    Backward();
  }
  else if (distanceCm > (distanceKeep + 3) && distanceCm < (distanceKeep + 20)) {
    Forward();
  }
  else if (distanceCm > (distanceKeep + 20)) {
    TurnRight();
  }
  else if (distanceCm >= distanceKeep && distanceCm <= (distanceKeep + 3)) {
    Stop();
  }

  delay(200);
}

// Movement functions
void Forward() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void Backward() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void TurnRight() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void TurnLeft() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void ForwardLeft() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED / 3);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void BackwardLeft() {
  analogWrite(enA, SPEED);
  analogWrite(enB, SPEED / 3);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void ForwardRight() {
  analogWrite(enA, SPEED / 3);
  analogWrite(enB, SPEED);
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
}

void BackwardRight() {
  analogWrite(enA, SPEED / 3);
  analogWrite(enB, SPEED);
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
}

void Stop() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
}
#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke library

#include <AimHangul.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Image_print.h"

#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xF800
#define YELLOW 0xFFE0
#define BLACK 000000

const char* ssid     = "gbshs_com1 2.4G";  // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "happygbs";         // The password of the Wi-Fi network

const String url = "https://raw.githubusercontent.com/piz2a/dreamhigh2024-arduino/refs/heads/master/scene-test.json";

const int btnPin = A0;
const int btnThreshold = 500;
bool btn_prev;
unsigned long timer = 0;
const unsigned long refreshPeriod = 1000;  // 1000 ms마다 크롤링

// 이미지를 여기에 등록해야 함.
uint8_t getImage(std::string imageName) {
  switch (imageName) {
    case std::string("clorox"):
      return clorox_image;
    case std::string("sunny"):
      return sunny_image;
  }
}


class SceneManager {
private:
  int currentIndex;
  std::vector<std::function<void()>> scenes;
public:
  SceneManager() { currentIndex = 0; }
  void add(std::function<void()> func) { scenes.push_back(func); }
  void load() {
    if (currentIndex < scenes.size())
      scenes[currentIndex]();
    else if (scenes.size() > 0)
      currentIndex = currentIndex < (scenes.size() - 1) ? currentIndex : (scenes.size() - 1);
    else {
      tft.fillScreen(BLACK);
      AimHangul_x4(50,70, "로딩 중...", WHITE);
    }
  }
  void next() {
    currentIndex = (currentIndex + 1) % scenes.size();
    load();
  }
};

SceneManager sceneManager;

void setup() {
  btn_prev = analogRead(btnPin) > btnThreshold;

  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);
  
  tft.begin();
  tft.setRotation(0);
  wifiConnect();

  sceneManager.load();
}

void loop() {
  if (millis() >= timer) {  // refreshPeriod마다
    String jsonString = getJson();  // R"([...])"
    parseScene(jsonString);
    timer += refreshPeriod;
  }

  // 버튼 입력 들어오면 다음 장면으로 전환 - 더블클릭 구분
  bool btn = analogRead(btnPin) > btnThreshold;

  if (btn == true && btn_prev == false) {
    sceneManager.next();
  }

  btn_prev = analogRead(btnPin) > btnThreshold;

  delay(10);
}

void wifiConnect() {
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    tft.setCursor(0,0);
    tft.fillScreen(TFT_BLACK);
    tft.println("CONNECTING...");
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}

String getJson() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient https;
    String fullUrl = url;
    Serial.println("Requesting " + fullUrl);
    if (https.begin(client, fullUrl)) {
      int httpCode = https.GET();
      Serial.println("============== Response code: " + String(httpCode));
      String result = "[]";
      if (httpCode > 0) {
        result = https.getString();
        Serial.println(result);
      }
      https.end();
      return result;
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  return "[]";
}

void parseScene(String jsonString) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString.c_str());

  if (error) {
    Serial.print("Parsing Error: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray array = value.as<JsonArray>();
  for (JsonVariant v : array) {
    if (v.is<JsonObject>) {
      JsonObject obj = v.as<JsonObject>();
      std::string templateName = obj["template"];
      try {
        switch (templateName) {
          case std::string("4row"):
            sceneManager.add([obj]() { row4(obj["row1"], obj["row2"], obj["row3"], obj["row4"], obj["image"]); });
            break;
          case std::string("big-number"):
            sceneManager.add([obj]() { bigNumber(obj["number"], obj["unit"], obj["image"]); });
            break;
        }
      } catch (...) {
        Serial.print("Error occured in template ");
        Serial.println(templateName);
      }
    }
  }
}

void status() {  // 시간도 출력하면 좋을 것 같다
  tft.drawRect(0, 0, 240 ,20, TFT_BLACK);
  tft.setCursor(0, 0);
  AimHangul_h2(20, 5, "YY/MM/DD  DAY", WHITE);  // 한글 출력 가능한 함수 - (x좌표, y좌표, 글, 색)
  tft.drawLine(0, 25, 240, 25, WHITE);  // 직선을 그리는 함수 - (출발점 x, 출발점 y, 도착점 x, 도착점 y, 색)
}

void row4(std::string row1, std::string row2, std::string row3, std::string row4, std::string image) {
  tft.fillScreen(BLACK);  // 화면 전체를 채우는 함수 - (색)
  status();
  tft.setCursor(10, 210);  // 커서 좌표 설정 - (x좌표, y좌표)
  tft.setTextSize(2);  // 글자 크기 설정 - (크기)
  tft.println(row1);  // 글자(한글 제외) 작성 - (글)
  tft.setCursor(10, 235);
  tft.println(row2);
  tft.setCursor(10, 260);
  tft.println(row3);
  tft.setCursor(10, 285);
  tft.println(row4);
  tft.setCursor(0, 30);
  tft.drawBitmap(56, 50, getImage(image), 128, 128, TFT_RED, TFT_BLACK);  // 이미지 출력(단색 이미지만) - (x, y, 비맵, 너비, 높이, 전경 색, 배경 색)
}

void bigNumber(std::string number, std::string unit, std::string image) {
  tft.fillScreen(BLACK);
  status();
  tft.drawBitmap(56, 50, getImage(image), 128, 128, WHITE, BLACK);
  tft.setTextSize(6);
  tft.setCursor(20, 240);
  tft.println(number);
  AimHangul(165, 250, unit, WHITE);
}

//AimHangul(20) < AimHangul_x2 < AimHangul_v2(40), AimHangul_x4

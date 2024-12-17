#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke library

#include <AimHangul.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <math.h>
#include "Image_print.h"

#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xF800
#define YELLOW 0xFFE0
#define BLACK 0x0000

#define BG_COLOR TFT_BLACK
#define FG_COLOR TFT_WHITE

// [5G 안됨] Wi-Fi 네크워크의 SSID
const char* ssid     = "gbshs-rain 2.4G";
// Wi-Fi 네트워크의 비밀번호
const char* password = "happygbs";
// Flask API URL
const String url = "https://flask-hello-world-azure-nu.vercel.app/";

unsigned long timer = 0;
const unsigned long refreshPeriod = 10000;  // ms마다 크롤링
String dateString = "";

// 이미지를 여기에 등록해야 함.
const uint8_t* getImage(const String& imageName) {
  if (imageName == "clorox") {
    return clorox_image;
  } else if (imageName == "thermo") {
    return thermo_image;
  } else if (imageName == "clock") {
    return clock_image;
  } else if (imageName == "hangang") {
    return hangang_image;
  }
  return nullptr;
}

class SceneManager {
private:
  int currentIndex;
  int lastIndex;  // 이전 장면 인덱스를 저장
  std::vector<std::function<void()>> scenes;

public:
  SceneManager() : currentIndex(0), lastIndex(-1) {}  // 초기값 설정

  void clear() { 
    scenes.clear(); 
    lastIndex = -1;  // 장면이 초기화되면 이전 상태도 리셋
  }

  void add(std::function<void()> func) {
    scenes.push_back(func);
  }

  void load() {
    if (currentIndex != lastIndex) {  // 현재 장면과 이전 장면이 다를 때만 갱신
      tft.fillScreen(BG_COLOR);          // 화면 초기화
      status();
      if (currentIndex < scenes.size()) {
        scenes[currentIndex]();       // 현재 장면 호출
      }
      lastIndex = currentIndex;       // 마지막 인덱스 갱신
      // AimHangul_x4(50,70, "로딩 중...", WHITE);
    }
  }

  void next() {
    currentIndex = (currentIndex + 1) % scenes.size();
    Serial.print("new currentIndex: ");
    Serial.println(currentIndex);
    load();
  }
};

SceneManager sceneManager;

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);
  tft.setTextColor(FG_COLOR);

  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);

  wifiConnect();

  sceneManager.load();
}

void loop() {
  if (millis() >= timer) {  // refreshPeriod마다
    String jsonString = getJson();  // R"([...])"
    if (jsonString != "[]") {
      parseScene(jsonString);
      sceneManager.load();
    } else {  // if there's no internet
      AimHangul_h2(20, 5, "No Internet", FG_COLOR);
    }
    timer += refreshPeriod;
    Serial.print("Free Memory: ");
    Serial.println(ESP.getFreeHeap());
  }

  // 버튼 입력 들어오면 다음 장면으로 전환 - 더블클릭 구분
  if (detect_touch()) {
    sceneManager.next();
    delay(50);
  }
  delay(10);
}

void wifiConnect() {
  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    if (i >= 30) {
      Serial.println("Connection failed");
      return;
    }
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    tft.setTextSize(2);
    tft.setCursor(10, 30);
    if (i % 3 == 1) {
      tft.fillScreen(TFT_BLACK);
      tft.print("CONNECTING.");
    } else if (i % 3 == 2) {
      tft.print("CONNECTING..");
    } else {
      tft.print("CONNECTING...");
    }
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
    String fullUrl = url + "?timer=" + String(timer);
    Serial.println("Requesting " + fullUrl);
    if (https.begin(client, fullUrl)) {
      int httpCode = https.GET();
      Serial.println("============== Response code: " + String(httpCode));
      String result = "[]";
      if (httpCode > 0) {
        result = https.getString();
        // Serial.println(result);
      }
      https.end();
      return result;
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  return "[]";
}

void status() {
  tft.drawRect(0, 0, 240 ,20, BG_COLOR);
  tft.setCursor(0, 0);
  AimHangul_h2(20, 5, dateString, FG_COLOR);  // 한글 출력 가능한 함수 - (x좌표, y좌표, 글, 색)
  tft.drawLine(0, 25, 240, 25, FG_COLOR);  // 직선을 그리는 함수 - (출발점 x, 출발점 y, 도착점 x, 도착점 y, 색)
}

struct Point {
  double x;
  double y;
};

// 시계바늘 끝점 좌표를 계산하는 함수
struct Point calculateHandEndpoint(int centerX, int centerY, double length, double angleDegrees) {
  double angleRadians = angleDegrees * (PI / 180.0); // 각도를 라디안으로 변환
  struct Point endpoint;
  endpoint.x = centerX + length * sin(angleRadians);
  endpoint.y = centerY - length * cos(angleRadians);
  return endpoint;
}


void clockScene(const String& timeString) {
  int centerX = 120, centerY = 125;
  double hourHandLength = 38, minuteHandLength = 51;

  // "hh:mm" 형식에서 시와 분 추출
  int hour = timeString.substring(0, 2).toInt(); // 시간 추출
  int minute = timeString.substring(3, 5).toInt(); // 분 추출

  // 시침과 분침의 각도 계산
  double hourAngle = (hour % 12) * 30 + minute * 0.5; // 시침 각도
  double minuteAngle = minute * 6;                   // 분침 각도

  // 각도를 기반으로 끝점 좌표 계산
  struct Point hourHandEnd = calculateHandEndpoint(centerX, centerY, hourHandLength, hourAngle);
  struct Point minuteHandEnd = calculateHandEndpoint(centerX, centerY, minuteHandLength, minuteAngle);

  // 결과를 시리얼 모니터에 출력
  Serial.print("Hour hand endpoint: (");
  Serial.print(hourHandEnd.x);
  Serial.print(", ");
  Serial.print(hourHandEnd.y);
  Serial.println(")");

  Serial.print("Minute hand endpoint: (");
  Serial.print(minuteHandEnd.x);
  Serial.print(", ");
  Serial.print(minuteHandEnd.y);
  Serial.println(")");

  tft.setTextSize(6);
  tft.setCursor(30, 240);
  tft.println(timeString);

  int image_size = 200;
  tft.drawBitmap(centerX - image_size / 2, 35, clock_image, image_size, image_size, FG_COLOR, BG_COLOR);  // (120, 114)
  tft.drawCircle(centerX, centerY, 3, GREEN);
  tft.drawLine(centerX, centerY, hourHandEnd.x, hourHandEnd.y, GREEN);
  tft.drawLine(centerX, centerY, minuteHandEnd.x, minuteHandEnd.y, GREEN);
}

void row4Scene(const String& row1, const String& row2, const String& row3, const String& row4, const JsonArray& units, const String& image) {
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
  String unit;

  unit = units[1].as<String>();
  if (unit == "\u00b0C") {
    tft.setCursor(140, 210 + 25*0);
    tft.printf("%cC", 0xF7);
  } else {
    AimHangul(140, 210 + 25*0, unit, FG_COLOR);
  }
  
  const uint8_t* imageC = getImage(image);
  if (imageC != nullptr) {
    tft.drawBitmap(40, 30, imageC, 160, 160, FG_COLOR, BG_COLOR);
  }
}

void bigNumber(const String& number, const String& unit, const String& image) {
  Serial.println(number);
  Serial.println(unit);
  Serial.println(image);
  const uint8_t* imageC = getImage(image);
  if (imageC != nullptr) {
    tft.drawBitmap(20, 35, imageC, 200, 200, FG_COLOR, BG_COLOR);
  }
  tft.setTextSize(6);
  tft.setCursor(20, 240);
  tft.println(number);
  if (unit == "\u00b0C") {
    tft.setTextSize(3);
    tft.setCursor(165, 250);
    tft.printf("%cC", 0xF7);
  } else {
    AimHangul(165, 250, unit, FG_COLOR);
  }
}

void parseScene(String jsonString) {
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, jsonString.c_str());

  if (error) {
    Serial.print("Parsing Error: ");
    Serial.println(error.c_str());
    return;
  }

  sceneManager.clear();
  JsonArray array = doc.as<JsonArray>();
  for (JsonVariant v : array) {
    if (v.is<JsonObject>()) {
      JsonObject obj = v.as<JsonObject>();
      
      String templateName = obj["template"];
      if (templateName == "clock") {
        dateString = obj["date"].as<String>();
        String timeString = obj["time"].as<String>();
        sceneManager.add([=]() {
          clockScene(timeString);
        });
      }

      // JSON 필수 데이터 체크
      if (!obj["template"] || !obj["image"]) {
        
        Serial.println("Invalid JSON object detected");
        continue;
      }
      if (templateName == "4row") {
        JsonArray rows = obj["row"].as<JsonArray>();
        if (rows.size() < 4) {
          Serial.println("Insufficient rows in 4row template");
          continue;
        }
        String row1 = rows[0].as<String>();
        String row2 = rows[1].as<String>();
        String row3 = rows[2].as<String>();
        String row4 = rows[3].as<String>();
        String image = obj["image"].as<String>();
        JsonArray units = obj["unit"].as<JsonArray>();
        Serial.println(units[0].as<String>());

        sceneManager.add([=]() {
          row4Scene(row1, row2, row3, row4, units, image);
        });
      } else if (templateName == "big-number") {
        if (!obj["number"] || !obj["unit"]) {
          Serial.println("Missing number or unit in big-number template");
          continue;
        }
        String number = obj["number"].as<String>();
        String unit = obj["unit"].as<String>();
        String image = obj["image"].as<String>();
        sceneManager.add([=]() {
          bigNumber(number, unit, image);
        });
      }
    }
  }
}

int detect_touch() {
  uint16_t x, y; // To store the touch coordinates
  bool pressed = tft.getTouch(&x, &y);
  if (!digitalRead(2)) {
    Serial.println("touched");
    return 1;
  }
  return 0;
}
//AimHangul(20) < AimHangul_x2 < AimHangul_v2(40), AimHangul_x4

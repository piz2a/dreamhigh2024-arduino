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
#define BLACK 0x0000

// [5G 안됨] Wi-Fi 네크워크의 SSID
const char* ssid     = "ssid";
// Wi-Fi 네트워크의 비밀번호
const char* password = "password";

const String url = "https://raw.githubusercontent.com/piz2a/dreamhigh2024-arduino/refs/heads/master/scene-test.json";

const int btnPin = A0;
const int btnThreshold = 500;
bool btn_prev;
unsigned long timer = 0;
const unsigned long refreshPeriod = 5000;  // 5000 ms마다 크롤링

// 이미지를 여기에 등록해야 함.
const uint8_t* getImage(const String& imageName) {
    if (imageName == "clorox") {
        return clorox_image;
    } else if (imageName == "sunny") {
        return sunny_image;
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
      tft.fillScreen(BLACK);          // 화면 초기화
      if (currentIndex < scenes.size()) {
        scenes[currentIndex]();       // 현재 장면 호출
      }
      lastIndex = currentIndex;       // 마지막 인덱스 갱신
      // AimHangul_x4(50,70, "로딩 중...", WHITE);
    }
  }

  void next() {
    currentIndex = (currentIndex + 1) % scenes.size();
    load();
  }
};

SceneManager sceneManager;

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);
  
  btn_prev = analogRead(btnPin) > btnThreshold;

  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);
  
  wifiConnect();

  sceneManager.load();
}

void loop() {
  if (millis() >= timer) {  // refreshPeriod마다
    String jsonString = getJson();  // R"([...])"
    parseScene(jsonString);
    sceneManager.load();
    timer += refreshPeriod;
    Serial.print("Free Memory: ");
    Serial.println(ESP.getFreeHeap());
  }

  // 버튼 입력 들어오면 다음 장면으로 전환 - 더블클릭 구분
  bool btn = analogRead(btnPin) > btnThreshold;

  if (btn == true && btn_prev == false) {
    sceneManager.next();
  }

  btn_prev = btn;

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
  return R"([
	{
		"template": "big-number",
		"number": "1300",
		"unit": "원/100mL",
		"image": "clorox"
	},
	{
		"template": "4row",
		"row": [
			"Temp: __._`C",
			"Humidity: __._%",
			"Max Temp: __._`C",
			"Min_Temp: __._`C"
		],
		"image": "sunny"
	}
])";
}

void status() {  // 시간도 출력하면 좋을 것 같다
  tft.drawRect(0, 0, 240 ,20, TFT_BLACK);
  tft.setCursor(0, 0);
  AimHangul_h2(20, 5, "YY/MM/DD  DAY", WHITE);  // 한글 출력 가능한 함수 - (x좌표, y좌표, 글, 색)
  tft.drawLine(0, 25, 240, 25, WHITE);  // 직선을 그리는 함수 - (출발점 x, 출발점 y, 도착점 x, 도착점 y, 색)
}

void row4Scene(const String& row1, const String& row2, const String& row3, const String& row4, const String& image) {
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

void bigNumber(const String& number, const String& unit, const String& image) {
  status();
  Serial.println(number);
  Serial.println(unit);
  Serial.println(image);
  tft.drawBitmap(56, 50, getImage(image), 128, 128, WHITE, BLACK);
  tft.setTextSize(6);
  tft.setCursor(20, 240);
  tft.println(number);
  AimHangul(165, 250, unit, WHITE);
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

      // JSON 필수 데이터 체크
      if (!obj["template"] || !obj["image"]) {
        Serial.println("Invalid JSON object detected");
        continue;
      }

      String templateName = obj["template"];
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
        sceneManager.add([=]() {
          row4Scene(row1, row2, row3, row4, image);
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

//AimHangul(20) < AimHangul_x2 < AimHangul_v2(40), AimHangul_x4

#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke library
#include <AimHangul.h>
#include "Image_print.h"
#include <vector>
#include <string>
#include <iostream>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

//#include <nlohmann/json.hpp>

//using json = nlohmann::json;

const char* ssid     = "gbshs_com1 2.4G";  // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "happygbs";         // The password of the Wi-Fi network

String url = "https://raw.githubusercontent.com/piz2a/SnareStrokeGAN/refs/heads/master/record.txt";

#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xF800
#define YELLOW 0xFFE0
#define BLACK 000000






std::vector<std::string> scenes = {"thermo", "chlorox"}; // 장면 나열
std::string scene = "thermo";


void setup() {
  
  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);
  
  tft.begin();
  tft.setRotation(0);
  wifiConnect();

  new_scene("thermo");
  tft.println(get(""));
}

void loop() {
  delay(1000);

  // 버튼 입력 들어오면 다음 장면으로 전환 - 더블클릭 구분
}

void wifiConnect() {
  //Serial.begin(115200);         // Start the Serial communication to send messages to the computer
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

String get(String query) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient https;
    String fullUrl = url + query;
    Serial.println("Requesting " + fullUrl);
    if (https.begin(client, fullUrl)) {
      int httpCode = https.GET();
      Serial.println("============== Response code: " + String(httpCode));
      String result = "";
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
  return "{}";
}

/*
String parse(String jsonString) {
  std::string std_payload = payload.c_str(); // String -> std::string 변환

  try {
    // JSON 파싱
    json parsed_json = json::parse(std_payload);
    // "text" 키의 값 추출
    if (parsed_json.contains("text")) {
        String text_value = String(parsed_json["text"].get<std::string>().c_str());
        Serial.println("Value of 'text': " + text_value);
        return text_value;
    } else {
        Serial.println("'text' key not found in JSON.");
        return "";
    }
  } catch (const json::parse_error& e) {
    Serial.print("Error parsing JSON: ");
    Serial.println(e.what());
    return "";
  }
}*/

void new_scene(std::string scene) {  //이 함수 내에 장면 구성, 이 함수를 호출하면 화면 변경 - (장면)
  if (scene == std::string("thermo")) {
    thermo();
  }
  else if (scene == std::string("clorox")){
    clorox();
  }
  else {
    tft.fillScreen(BLACK);
    AimHangul_x4(50,70, "장면 없음", WHITE);
  }
}
void status() {
  tft.drawRect(0,0,240,20,TFT_BLACK);
  tft.setCursor(0,0);
  AimHangul_h2(20,5,"YY/MM/DD  DAY", WHITE);  // 한글 출력 가능한 함수 - (x좌표, y좌표, 글, 색)
  tft.drawLine(0,25,240,25, WHITE);  // 직선을 그리는 함수 - (출발점 x, 출발점 y, 도착점 x, 도착점 y, 색)
}
void thermo() {
  tft.fillScreen(BLACK);  // 화면 전체를 채우는 함수 - (색)
  status();
  tft.setCursor(10,210);  // 커서 좌표 설정 - (x좌표, y좌표)
  tft.setTextSize(2);  // 글자 크기 설정 - (크기)
  tft.println("Temp: __._`C");  // 글자(한글 제외) 작성 - (글)
  tft.setCursor(10,235);
  tft.println("Humidity: __._%");
  tft.setCursor(10,260);
  tft.println("Max Temp: __._`C");
  tft.setCursor(10,285);
  tft.println("Min_Temp: __._`C");
  tft.setCursor(0,30);
  tft.drawBitmap(56,50, sunny_image, 128,128, TFT_RED, TFT_BLACK);  // 이미지 출력(단색 이미지만) - (x, y, 비맵, 너비, 높이, 전경 색, 배경 색)
}

void clorox() {
  tft.fillScreen(BLACK);
  status();
  tft.drawBitmap(56,50,clorox_image, 128, 128, WHITE, BLACK);
  tft.setTextSize(6);
  tft.setCursor(20,240);
  tft.println("1300");
  AimHangul(165,250,"원/100mL",WHITE);
}


//AimHangul(20) < AimHangul_x2 < AimHangul_v2(40), AimHangul_x4


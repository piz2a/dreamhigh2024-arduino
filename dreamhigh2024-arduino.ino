#include <TFT_eSPI.h> // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke library
#include <AimHangul.h>
#include <vector>
#include <string>
#include <iostream>
#include <Image_print.h>
//#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
//#include <nlohmann/json.hpp>

//using json = nlohmann::json;

const char* ssid     = "203mirroring";  // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "happygbs";         // The password of the Wi-Fi network

String url = "https://raw.githubusercontent.com/piz2a/SnareStrokeGAN/refs/heads/master/record.txt";

#define WHITE 0xFFFF
#define GREEN 0x07E0
#define RED 0xF800
#define YELLOW 0xFFE0
#define BLACK 000000



std::vector<std::string> scenes = {"Lobby", "thermo"}; // 필요하면 사용 - 근데 필요 없을 듯
std::string scene = "Lobby";


void setup() {
  //wifiConnect();
  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);
  
  tft.begin();
  tft.setRotation(0);

  new_scene(scene);
}

void loop() {
  delay(1000);
  scene = scenes[1];
  new_scene(scene);
  // 버튼 입력 들어오면 다음 장면으로 전환 - 더블클릭 구분
}
/*
void wifiConnect() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
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

void new_scene(std::string scene) {  //이 함수 내에 장면 구성, 이 함수를 호출하면 화면 변경
  if (scene == std::string("Lobby")) {
    tft.fillScreen(RED);
  }
  else if (scene == std::string("thermo")) {
    tft.fillScreen(BLACK);
    
    tft.setCursor(0,0);
    AimHangul_h2(20,5,"YY/MM/DD  DAY", WHITE);
    tft.drawLine(0,25,240,25, WHITE);
    tft.setCursor(10,210);
    tft.setTextSize(2);
    tft.println("Temp: __._'C");
    tft.setCursor(10,235);
    tft.println("Humidity: __._%");
    tft.setCursor(10,260);
    tft.println("Max Temp: __._'C");
    tft.setCursor(10,285);
    tft.println("Min_Temp: __._'C");
    tft.setCursor(0,30);
    tft.drawBitmap(0,0,sunny_image, 180, 180, TFT_WHITE);
  }
  else {
    tft.fillScreen(BLACK);
    AimHangul_x4(0,70, "장면 없음", WHITE);
  }
}

// 이미지 출력 함수 - 이미지 저장 라이브러리 만들자~

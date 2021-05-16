//#include <SimpleKalmanFilter.h>
#include <WiFi.h>
//#include <HTTPClient.h>
#include <esp32DHT.h>
DHT22 sensor;

int cb = 13; //chân cảm biến
int relay = 25;
// thời gian làm mới
//const long SERIAL_REFRESH_TIME = 100;
//long refresh_time;

#define check 30 // check > 30 phần trăm
const char* ssid     = "Yen Vy Dang";
const char* password = "vanloi8923";

//SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);

void setup() {
 Serial.begin(112500);
  sensor.setup(5);  // optionally use another RMT channel: sensor.setup(23, RMT_CHANNEL_2);
  sensor.onData([](float humid, float temp) {
    Serial.printf("Temp: %.1f°C\nHumid: %.1f%%\n", temp, humid);
  });
  sensor.onError([](uint8_t error) {
    Serial.printf("Error: %d-%s\n", error, sensor.getError());
  });

  //Kết nối wifi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.print("IP: ");
  Serial.print(WiFi.localIP());
  //========= PHẦN OUTPUT/INPUT CHO THIẾT BỊ BƠM ========
  pinMode(cb, INPUT);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  //  pinMode(cb, INPUT);
  //  pinMode(relay, OUTPUT);
  //  digitalWrite(relay, LOW);

}
// gửi dữ liệu
//void SendRequest(int moisture)
//{
//  if(WiFi.status() == WL_CONNECTED){
//    HTTPClient http;
//    http.begin("http://192.168.43.60:3000/tb?analog="+String(moisture));
//    http.addHeader("Content-Type","text/html");
//    int httpResponseCode = http.GET();
//    if(httpResponseCode == 200){
//      Serial.println("Send");
//    }
//    else{
//      Serial.println("Can't Send");
//    }
//
//  }
//}

void loop() {
   static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 30000) {
      lastMillis = millis();
      sensor.read();
      Serial.print("Read DHT...\n");
  }
  //
  //  // ========= Phần tính toán nhiễu ========
  //  // Đọc giá trị tham chiếu từ 0 to 100
  //  float real_value = analogRead(cb) / 1024.0 * 50.0;
  //
  //  // thêm giá trị tham chiếu và sử dụng
  //  float measured_value = real_value + random(-50, 50) / 50.0;
  //
  //  // Tính toán giá trị kalman filter
  //  float estimated_value = simpleKalmanFilter.updateEstimate(measured_value);
  //  Serial.print(estimated_value);
  //  Serial.println();
  //  //  if (millis() > refresh_time) {
  //  //    Serial.print(real_value,1);
  //  //    Serial.print(",");
  //  //    Serial.print(measured_value,1);
  //  //    Serial.print(",");
  //  //    Serial.print(estimated_value,1);
  //  //    Serial.println();
  //  //
  //  //    refresh_time = millis() + SERIAL_REFRESH_TIME;
  //  //  }
  ////========= Phần tính toán cho thiết bị bơm ========
//    int val = analogRead(cb); // đọc chân biến
  //  int phantramcb = map(val, 0, 1023, 0, 100); // tính phần trăm cb
  //  int phantramthuc = 100 - phantramcb;// tính phần trăm thực
//    Serial.print("Do am: \t");
//    Serial.print(val);
//    delay(2000);
  //  Serial.print(" ");
  //  Serial.print("~"); Serial.print(" ");
  //  Serial.print(phantramthuc); Serial.print("% \n");
  //  delay(2000);
  //  if (phantramthuc >= check) {
  //    digitalWrite(relay, LOW); //off
  //  }
  //  else {
  //    digitalWrite(relay, HIGH); //on
  //  }
  //  //  SendRequest(val);

}

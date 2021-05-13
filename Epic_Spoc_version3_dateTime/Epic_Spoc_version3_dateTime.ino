#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include <WiFiServer.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SocketIoClient.h>
#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library

#include <SimpleKalmanFilter.h> // Thư viện của kalman

SocketIoClient webSocket;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Wiring for ESP32 NodeMCU boards: VDD to 3V3, GND to GND, SDA to 21, SCL to 22, nWAKE to D3 (or GND)
CCS811 ccs811(23); // nWAKE on 23, SDA on 21 , SCL on 22
#define ANALOG_PIN 32


//========= PHẦN CHÂN CẢM BIẾN CỦA THIẾT BỊ BƠM ========
int cb = 12; //chân cảm biến
int relay = 25;// relay cảm biến
bool turnQuat = false;

//========= PHẦN CHÂN CẢM BIẾN CỦA THIẾT BỊ FAN ========
int FAN = 26;
String macaddress = "";

String serverName = "http://192.168.1.11:3000/device";

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

#define check 30 // check > 30 phần trăm của phần cảm biến

//===================================WIFI=============================
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

char host[] = "192.168.1.11";
int port = 3000;
char path[] = "/socket.io/?transport=websocket";

const char* ssid     = "Lac Hong University";
const char* password = "";



SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);// Thư viện của kalman

//============= socket connected=================
void socket_Connected(const char * payload, size_t length) {
  Serial.println("Connected To Server");
  Serial.println(payload);
}


void socket_Fan(const char * payload, size_t length) {
  turnQuat = !turnQuat;
  if (turnQuat) {
    digitalWrite(FAN, HIGH);
    Serial.println("FAN ON");
  } else {
    digitalWrite(FAN, LOW);
    Serial.println("FAN OFF");
  }
  //  Serial.print("id: ");
  //  Serial.println(String(payload));
  //  if (String(payload) == "true") {
  //    digitalWrite(FAN, HIGH);
  //    Serial.println("FAN ON");
  //  } else {
  //    digitalWrite(FAN, LOW);
  //    Serial.println("FAN OFF");
  //  }
}

void sendServer() {
  char* mess = "Esp32 Connect";
  webSocket.emit("fromServer", mess);
}
void socket_DoAm() {
  //  if (deviceDoAm() < 30) {
  char* mess = "send";
  digitalWrite(cb, HIGH);
  webSocket.emit("esp_sendRequest_DoAm", "\"send\"");
  //}
}
//void SendDoAm(){
//  if (deviceDoAm()<30){
//    char* mess="send";
//    digitalWrite(FAN,HIGH);
//    delay(5000);
//    webSocket.emit("esp_sendRequest_DoAm",mess);
//  }
//}

void setUp_WiFi() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void setup() {

  setUp_WiFi();

  //========= PHẦN OUTPUT/INPUT CHO THIẾT BỊ BƠM ========
  pinMode(cb, INPUT);
  digitalWrite(cb, HIGH);

  //========= PHẦN OUTPUT/INPUT CHO THIẾT BỊ LỌC ========
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);

  //========= PHẦN CHÂN CẢM BIẾN CỦA THIẾT BỊ CO2 ========
  Serial.println("");
  // Enable I2C
  Wire.begin();
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if ( !ok ) Serial.println("setup: CCS811 begin FAILED");
  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("setup: CCS811 start FAILED");

  macaddress = WiFi.macAddress();
  Serial.println("macAddress: " + macaddress);
  // Init and get the time
  data_time();

  //socket
  webSocket.on("connect", socket_Connected);
  webSocket.on("esp_turnQuat", socket_Fan);
  webSocket.begin(host, port, path);
}

int a = 0;
void LoadDoAm() {
  if (macaddress != "") {
    if (a == 0) {
      Serial.println("Load");
      socket_DoAm();
      ++a;
    }
  }
}

void inPutTruongHop() {
  Serial.println(String(macaddress) + "&dateTime=" + data_time() + "&TrangThaiQuat=" + String(turnQuat) + "&DoAm=" + deviceDoAm()  + "&CO2=" + String(deviceOXI()));
  // SendRequest();
}
//======================================================================
//======================================================================
int timeRequest = -1;
int detalTime = 1;


void loop() {
  // webSocket.loop();
  // cái ngu
  if (abs(data_time() - timeRequest) >= detalTime) {
    Serial.println(data_time());
    Serial.println(timeRequest);
    Serial.println(detalTime);
    timeRequest = data_time();
    Serial.println("Send Data");
    // Các Event Thực Hiện Với Điều Kiện detalTime
    //SendRequest();
    Serial.println(timeRequest);
    //////////////////////////////////////////////
  }


  //  LoadDoAm(); delay(3000);
  //  if (Serial.available() > 0) {
  //    inPutTruongHop();
  //    delay(3000);
  //  }
  //  if (Serial.available() > 0) {
  //    char checkAll = Serial.read();
  //
  //    switch (checkAll)
  //    {
  //      case 'T': // start
  //        while (Serial.read() != 'N')
  //        {
  //Serial.println(String(macaddress) + "&dateTime=" + data_time() + "&TrangThaiQuat=" + String(turnQuat) + "&DoAm=" + deviceDoAm()  + "&CO2=" + String(deviceOXI()));
  //        }
  //        break;
  //      default:
  //        break;
  //    }
  //  }
}
void SendRequest()
{
  //  mac addresss
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.1.11:3000/device/" + String(macaddress) + "?TrangThaiQuat=" + String(turnQuat) + "&DoAm=" + deviceDoAm()  + "&CO2=" + String(deviceOXI()));//"?dateTime=" + data_time()
    http.addHeader("Content-Type", "text/html");
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {

      String respone = http.getString();

      Serial.println(httpResponseCode);
      Serial.println(respone);
    }
    else {
      Serial.println("Can't Send");
      Serial.println(httpResponseCode);
    }
  }
}

// ========= PHẦN TÍNH TOÁN NHIỄU ========
void kalman() {

  // Đọc giá trị tham chiếu từ 0 to 100
  float real_value = analogRead(cb) / 1024.0 * 100.0;

  // thêm giá trị tham chiếu và sử dụng
  //float measured_value =real_value + random(-100, 100)/100.0;

  // Tính toán giá trị kalman filter
  float estimated_value = simpleKalmanFilter.updateEstimate(real_value);

  Serial.print(estimated_value);
  Serial.println();
}

// ========= PHẦN TÍNH TOÁN PUMP ========
float deviceDoAm() {
  //========= PHẦN TÍNH TOÁN CHO THIẾT BỊ PUMP ========
  //
  //  int val = analogRead(cb); // đọc chân biến
  //  int phantramcb = map(val, 0, 1023, 0, 100); // tính phần trăm cb
  //  int phantramthuc = 100 - phantramcb;// tính phần trăm thực
  //  return val * 100 / 4095;
  //  Serial.print("Do am: \t");
  //  Serial.print(val);
  //  Serial.print(" ");
  //  Serial.print("~"); Serial.print(" ");
  //  Serial.print(phantramthuc); Serial.print("% \n");

  //check phần trăm thực
  //  if (phantramthuc >= check) {
  //    //    digitalWrite(relay, LOW); //off
  //    return false;
  //  }
  //  else {
  //    // digitalWrite(relay, HIGH); //on
  //    return true;
  //  }.
  //========= PHẦN TÍNH TOÁN CHO THIẾT BỊ PUMP ========
  analogReadResolution(10);//Thiet lap do phan giai ADC, co the dung 8 10 12
  //  Serial.println(F("\nSensor soil:"));
  //  //  Serial.print("\tSoil moisture: ");
  float val = analogRead(ANALOG_PIN);
  //  Serial.print("\tValue analogRead val: ");
  //  Serial.println(val, 6);
  float vol = (float)val * 3.3 / 1023.0;
  Serial.println(vol);
}


// ========= PHẦN TÍNH TOÁN FAN ========
bool deviceFAN() {
  //  if (Serial.available() > 0) {
  //    char comandCharacter = Serial.read();
  //    if (comandCharacter == 'S') {
  //      digitalWrite(FAN, HIGH);
  //      Serial.println("FAN ON");
  //    }
  //    else
  //    {
  //      digitalWrite(FAN, LOW);
  //      Serial.println("FAN OFF");
  //    }
  //  }
  turnQuat = !turnQuat;
  if (turnQuat) {
    digitalWrite(FAN, HIGH);
    return true;
  } else {
    digitalWrite(FAN, LOW);
    return false;
  }
}
// ========= PHẦN TÍNH TOÁN OXI ========
String deviceOXI() {
  // Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw);
  if ( errstat == CCS811_ERRSTAT_OK ) {
    return String(eco2);
  }
  else {
    return ("null");
  }
  //    Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
  //    Serial.println();

  //else if( errstat & CCS811_ERRSTAT_I2CFAIL ) {
  //    Serial.println("CCS811: I2C error");
  //  } else {
  //    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX);
  //    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) );
  //  }
}

// ==================== PHẦN TÍNH DATATIME ======================
int data_time() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  timeClient.setTimeOffset(25200);
  /// hour_minnute_second
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // month_day_year
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  char timeWeekDay[30];
  strftime(timeWeekDay, 30, "%m/%d/%Y-", &timeinfo);
  formattedDate  = timeClient.getFormattedTime();
  // Get hours(:00:) in formattedDate (00:00:00)
  String hours = "";
  for (int i = 3 ; i < 5; i++) {
    hours += formattedDate[i];
  }
  // Serial.print(timeWeekDay);

  return hours.toInt();
  //Serial.print(" " + formattedDate);
}
//có time thực formattedDate
//%H:%M:%S"
//  Serial.print("Day of week: ");
//  Serial.println(&timeinfo, "%A");
//  //  Serial.print("Month: ");
//  Serial.print(&timeinfo, "%B");
//  //  Serial.print("Day of Month: ");
//  Serial.println(&timeinfo, "%d");
//  //  Serial.print("Year: ");
//  Serial.println(&timeinfo, "%Y");
//  Serial.print("Hour: ");
//  Serial.println(&timeinfo, "%H");
//  Serial.print("Hour (12 hour format): ");
//  Serial.println(&timeinfo, "%I");
//  Serial.print("Minute: ");
//  Serial.println(&timeinfo, "%M");
//  Serial.print("Second: ");
//  Serial.println(&timeinfo, "%S");
//
//  Serial.println("Time variables");
//  char timeHour[3];
//  strftime(timeHour,3, "%H", &timeinfo);
//  Serial.println(timeHour);
//  char timeWeekDay[10];
//  strftime(timeWeekDay,10, "%A", &timeinfo);
//  Serial.println(timeWeekDay);
//  Serial.println();
//  timeClient.update();
//  Serial.println(timeClient.getFormattedTime());
//
//  while (!timeClient.update()) {
//    timeClient.forceUpdate();
//  }
//
//  dayStamp = formattedDate.substring(0);
//  Serial.print("DATE: " + dayStamp);
//    while (!timeClient.update()) {
//      timeClient.forceUpdate();
//    }
//    // The formattedDate comes with the following format:
//    // 2018-05-28T16:00:13Z
//    // We need to extract date and time
//    formattedDate  = timeClient.getFormattedTime();
//    Serial.println(formattedDate);
//
//    // Extract date
//    int splitT = formattedDate.indexOf("T");
//    dayStamp = formattedDate.substring(0, splitT);
//    Serial.print("DATE: ");
//    Serial.println(dayStamp);
//    // Extract time
//    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length());
//    Serial.print("HOUR: ");
//    Serial.println(timeStamp);

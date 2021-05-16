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

#define LED_PIN 2
#define ANALOG_PIN 32

#include <SimpleKalmanFilter.h> // Thư viện của kalman

SocketIoClient webSocket;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Wiring for ESP32 NodeMCU boards: VDD to 3V3, GND to GND, SDA to 21, SCL to 22, nWAKE to D3 (or GND)
CCS811 ccs811(23); // nWAKE on 23, SDA on 21 , SCL on 22

//========= PHẦN CHÂN CẢM BIẾN CỦA THIẾT BỊ BƠM ========
int cb = 14; //chân cảm biến
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

char host[] = "10.0.9.180";
int port = 3000;
char path[] = "/socket.io/?transport=websocket";

const char* ssid     = "Van Khai";
const char* password = "Password";
bool checkWIFI=false;



SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);// Thư viện của kalman

//============= socket connected=================
void socket_Connected(const char * payload, size_t length) {
  Serial.println("Connected To Server");
  Serial.println(payload);}

// Send Server
void sendServer() {
  char* mess = "Esp32 Connect";
  webSocket.emit("fromServer", mess);
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
}

const char *ID = "\" fsdfs\"";
/// socket_checkNhan_themNuoc
void socket_checkNhan_themNuoc(const char * payload, size_t length) {
  ID = payload;
}
/// socket_checkNhan_Air
//void socket_checkNhan_Air(const char * payload, size_t length){
//  char *notification = "";
//}
void setUp_WiFi() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  checkWIFI = true;
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
int timeRequest = 999999999;
void setup() {
  setUp_WiFi();
  //========= PHẦN OUTPUT/INPUT CHO THIẾT BỊ BƠM ========
  pinMode(cb, INPUT);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

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

  timeRequest = data_time();
}

void inPutTruongHop() {
  Serial.println(String(macaddress) + "&TrangThaiQuat=" + String(turnQuat) + "&DoAm=" + deviceDoAm()  + "&CO2=" + String(deviceOXI()));
  // SendRequest();
}
//======================================================================
//======================================================================


int timeIndexMayBom = 60 * 60;
int timeIndexNhiemVu;
int deltaTime = 10;
int checkNuocTrongBe = 0;
bool run = false;
bool checkRun = false;
int randNumber = random(0, 10);
int numberPM1 = 17;
int numberPM25 = 72;
int numberPM10 = 167; 
char* khanCap0 = "{\"MucNuoc\":false, \"KhanCap\":0}";
char* khanCap1 = "{\"MucNuoc\":false, \"KhanCap\":1}";




void loop() {
  if (checkWIFI){
   // webSocket.loop();
  
  if (data_time() - timeRequest >= deltaTime) {          // TIME thực - timeRequest tự cho >= deltatime(20s)
    timeRequest = data_time() + 1;                       // VD data_time = 919 ==> timeRequest = 920
    if (data_time() - timeRequest < 0) {                 // 2 cái trừ nhau < 0 nên chạy điều kiện bên trong
      XuLyMayBom(5);
      // Hàm xử lý máy bơm
      checkAir();
    }
    //SendRequest();
//    checkAir();
  }
  /* Sau delta time 2 là 5s. Ở đây là kiểm tra nước trong bể còn thì thoát không thì gửi dữ liệu lên server*/

  if (data_time() - timeIndexMayBom > 0) {               // data_time(h hiện tại * 60) - timeIndexMayBom(60*60) Đương nhiên là > 0,
    timeIndexMayBom = 60 * 60;                           //
    BatTatMayBom(false);                                 //
    Serial.println("Tắt Máy Bơm");
    if (checkNuocTrongBe == 1) {                         // out ra 
      return;
    } else if (ID == "") {
      webSocket.emit("esp_checkNhan_themNuoc", "\"0\"");                  // emit ("{"MucNuoc":"false","Khan cap":"0"}")
    } else {
      webSocket.emit("esp_checkNhan_themNuoc","\"0\"");
      ID = "";
    }
  }
  }
  
}

void XuLyMayBom(int deltaTime2) {
  timeRequest = data_time();                               // timeRequest bằng thời gian hiện tại
  if (deviceDoAm() < 2) {
    //    if (nhanNhiemVu(false)) {
    if (checkNuocTrongBe == 1) {                           // check nước trong bể còn hay không
      checkRun = true;                                     // Đầu tiên cho checkRun = false
      BatTatMayBom(true);                                  // On máy bơm theo hàm BatTatMayBom
      Serial.println("Bật Máy Bơm");                       //
      timeIndexMayBom = data_time() + deltaTime2 - 1;       // sau 5s thì ngừng
      Serial.println(timeIndexMayBom);                     //
    } else if (ID == "") {
      webSocket.emit("esp_checkNhan_themNuoc", "\"1\"");   //Serial.println("Nuoc Trong Be Het");            // emit ("{"MucNuoc":"false","Khan cap":"0"}")
    } else {
      webSocket.emit("esp_checkNhan_themNuoc","\"1\"");
      ID = "";
    }
    //    } else Serial.println( randNumber); //Serial.println("Lay ID client");
  } else return;
}
/* check bơm */
void BatTatMayBom(bool checkBomIndex) {
  if (checkBomIndex) {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
}

boolean nhanNhiemVu(bool checkNhiemVu) {
  if (checkNhiemVu) {
    Serial.println("quay lai buoc 3");
  } else {
    Serial.println("ID client");
  }
}
// check_Air
void checkAir(){
  String isCheckAir;
  if(isCheckAir == "numberPM1")
  {
    Serial.println("gửi dữ liệu của PM1.0"); // webSocket.emit("socket_checkNhan_Air"+ numberPM1);  
  }

//   else if(isCheckAir == "numberPM25")
//  Serial.println("gửi dữ liệu của PM2.5");//webSocket.emit("socket_checkNhan_Air"+numberPM25);
  else{
    Serial.println("gửi dữ liệu của PM10");//webSocket.emit("socket_checkNhan_Air"+numberPM10);
  }
}

void SendRequest()
{
  //  mac addresss
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://10.0.9.180:3000/device/" + String(macaddress) + "?TrangThaiQuat=" + String(turnQuat) + "&DoAm=" + deviceDoAm()  + "&CO2=" + String(deviceOXI()));
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
  analogReadResolution(10);//Thiet lap do phan giai ADC, co the dung 8 10 12
  int val = analogRead(ANALOG_PIN);
  float vol = (float)val * 3.3 / 1023;
  return vol;

}


// ========= PHẦN TÍNH TOÁN FAN ========
bool deviceFAN() {
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
  // Get giây(::00) in formattedDate (00:00:00)
  //    String hours="";
  //  for (int i =3 ;i < 5;i++){
  //    hours += formattedDate[i];
  //  }
  String txt = "";
  for (int i = 3; i <= 4; i++) {
    txt += formattedDate[i];
  }
  int hours = txt.toInt();
  txt = "";
  for (int i = 6; i <= 7; i++) {
    txt += formattedDate[i];
  }
  int seconds = txt.toInt();

  return hours * 60 + seconds;
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

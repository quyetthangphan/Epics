#include <WiFi.h>
#include <SocketIoClient.h>

SocketIoClient webSocket;

int FAN = 26;

void socket_Connected(const char * payload, size_t length) {
  Serial.println("Connected To Server");
  Serial.println(payload);
}

void socket_Fan(const char * payload, size_t length) {
  Serial.print("id: ");
  Serial.println(String(payload));
  if (String(payload) == "true") {
    digitalWrite(FAN, HIGH);
    Serial.println("FAN ON");
  } else {
    digitalWrite(FAN, LOW);
    Serial.println("FAN OFF");
  }
}

void sendServer() {
  char* mess = "Esp32 Connect";
  webSocket.emit("fromServer", mess);
}

char host[] = "192.168.43.253";
int port = 3000;
char path[] = "/socket.io/?transport=websocket";


const char* ssid     = "Server";
const char* password = "12346789";


void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  webSocket.on("connect", socket_Connected);
  webSocket.on("esp32_turnQuat", socket_Fan);
  webSocket.begin(host, port, path);

  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);
}

void loop() {
  webSocket.loop();
}

//code for ESP_1

//Libs--------------------------------------
#include <WiFi.h>

//constanten--------------------------------
#define ledpinR   21 // GPIO21
#define ledpinG   22 // GPIO22
#define ledpinB   23 // GPIO23

#define butpin    19 // GPIO19

#define SERVER_PORT 4080

const unsigned long one_second = 1000;
const unsigned long wifiTimeout = 60000; // 1 minute timeout
unsigned long prevMillis = 0;
unsigned long wifiStartTime = 0;
bool redledstate = false;
bool wifiConnected = false;

//FSM States
enum State { HOLD, PRESSED, WAIT };
State currentState = WAIT;  // Starts in WAIT

//variabelen-------------------------------
unsigned long curMillis = 0;
unsigned long buttonLastDebounce = 0;
const unsigned long debounceDelay = 50;
bool lastButtonState = HIGH;
bool buttonHandled = false;

//Wifigegens-------------------------------
const char* ssid = "SSID";              // CHANGE TO YOUR WIFI SSID
const char* password = "PASSWORD";      // CHANGE TO YOUR WIFI PASSWORD
const char* nextEspIp = "192.168.1.102";   // IP of next ESP (ESP_2)

WiFiServer TCPserver(SERVER_PORT);      //communicatiepoort definieren

//functies---------------------------------
void setColor(int R, int G, int B) {
  analogWrite(ledpinR, R);
  analogWrite(ledpinG, G);
  analogWrite(ledpinB, B);
}

void sendNextMessage() {
  WiFiClient client;
  if (client.connect(nextEspIp, SERVER_PORT)) {
    client.println("next");
    client.stop();
    Serial.println("Message sent to next ESP");
  } else {
    Serial.println("Failed to connect to next ESP");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting ESP_1...");

  pinMode(ledpinR, OUTPUT);
  pinMode(ledpinG, OUTPUT);
  pinMode(ledpinB, OUTPUT);
  pinMode(butpin, INPUT_PULLUP);

  WiFi.begin(ssid,password);
  wifiStartTime = millis();

  while(WiFi.status() != WL_CONNECTED){
    curMillis = millis();

    if (curMillis - prevMillis >= one_second){
      prevMillis = curMillis;
      redledstate = !redledstate;
      Serial.println("Connecting to WiFi...");

      if(redledstate){
        setColor(255, 0, 0);
      }else{
        setColor(0, 0, 0);
      }
    }

    if (curMillis - wifiStartTime >= wifiTimeout) {
      Serial.println("WiFi connection timeout. Restarting...");
      setColor(255, 255, 0);
      delay(2000);
      ESP.restart();
    }
  }

  Serial.println("Connected to WiFi");
  Serial.print("ESP_1 IP: ");
  Serial.println(WiFi.localIP());

  TCPserver.begin();
  delay(500);
  Serial.println("---Booted and connected---");

  // Starts in WAIT state, no LED
  setColor(0, 0, 0);
}

void loop() {
  curMillis = millis();

  bool buttonState = digitalRead(butpin);

  switch (currentState) {
    case HOLD:
      setColor(0, 0, 255);  // Blue
      if (buttonState == LOW && lastButtonState == HIGH && (curMillis - buttonLastDebounce > debounceDelay)) {
        currentState = PRESSED;
        buttonLastDebounce = curMillis;
      }
      break;

    case PRESSED:
      setColor(0, 255, 0);  // Green
      if (!buttonHandled) {
        sendNextMessage();
        buttonHandled = true;
      }
      currentState = WAIT;
      break;

    case WAIT:
      // Do nothing here or add timeout logic if desired
      break;
  }

  lastButtonState = buttonState;

  // Handle incoming TCP messages
  WiFiClient client = TCPserver.available();
  if (client && client.connected()) {
    String message = client.readStringUntil('\n');
    message.trim();
    if (message == "next") {
      Serial.println("Received NEXT message");
      currentState = HOLD;
      buttonHandled = false;
    }
    client.stop();
  }
}

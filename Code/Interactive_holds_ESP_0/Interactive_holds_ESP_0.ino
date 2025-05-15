// ESP_0 Code - FSM-based turn handover with WiFi

// Libraries --------------------------------------
#include <WiFi.h>

// Constants --------------------------------------
#define ledpinR   21 // Red LED - GPIO21
#define ledpinG   22 // Green LED - GPIO22
#define ledpinB   23 // Blue LED - GPIO23
#define butpin    19 // Button    - GPIO19

#define SERVER_PORT 4080
const unsigned long one_second = 1000;
const unsigned long wifiTimeout = 60000;   // 1 minute timeout
const unsigned long debounceDelay = 50;    // Debounce time for button

// WiFi credentials
const char* ssid = "SSID";              // CHANGE TO YOUR WIFI SSID
const char* password = "PASSWORD";      // CHANGE TO YOUR WIFI PASSWORD
const char* nextEspIp = "192.168.***.***"; // CHANGE TO NEXT ESP IP

// WiFi server
WiFiServer TCPserver(SERVER_PORT);

// FSM States --------------------------------------
enum State { HOLD, PRESSED, WAIT };
State currentState = HOLD;

// Variables ---------------------------------------
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long buttonLastDebounce = 0;
unsigned long wifiStartTime = 0;

bool redLEDState = false;
bool lastButtonState = HIGH;
bool buttonHandled = false;

// Functions ---------------------------------------

void setColor(int R, int G, int B) {
  analogWrite(ledpinR, R);
  analogWrite(ledpinG, G);
  analogWrite(ledpinB, B);
}

// Sends a "next" message to the next ESP in sequence
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

// Setup ------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Booting ESP_0 (server)...");

  pinMode(ledpinR, OUTPUT);
  pinMode(ledpinG, OUTPUT);
  pinMode(ledpinB, OUTPUT);
  pinMode(butpin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  wifiStartTime = millis();

  // Blink red while connecting to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();

    if (currentMillis - previousMillis >= one_second) {
      previousMillis = currentMillis;
      redLEDState = !redLEDState;
      Serial.println("Connecting to WiFi...");

      if (redLEDState) {
        setColor(255, 0, 0);
      } else {
        setColor(0, 0, 0);
      }
    }

    if (currentMillis - wifiStartTime >= wifiTimeout) {
      Serial.println("WiFi connection timeout. Restarting...");
      setColor(255, 255, 0); // Yellow
      delay(2000);
      ESP.restart();
    }
  }

  Serial.println("Connected to WiFi");
  Serial.print("ESP_0 IP: ");
  Serial.println(WiFi.localIP());

  TCPserver.begin();
  delay(500);
  Serial.println("---Booted and Ready---");

  setColor(0, 0, 255); // Blue: ESP_0 starts with control
}

// Loop --------------------------------------------
void loop() {
  currentMillis = millis();
  bool buttonState = digitalRead(butpin);

  switch (currentState) {
    case HOLD:
      if (lastButtonState == HIGH && buttonState == LOW && (currentMillis - buttonLastDebounce > debounceDelay)) {
        currentState = PRESSED;
        buttonLastDebounce = currentMillis;
      }
      break;

    case PRESSED:
      setColor(0, 255, 0); // Green: acknowledged
      if (!buttonHandled) {
        sendNextMessage();
        buttonHandled = true;
      }
      currentState = WAIT;
      break;

    case WAIT:
      // Idle until message is received from another ESP
      break;
  }

  lastButtonState = buttonState;

  // Handle incoming TCP messages
  WiFiClient client = TCPserver.available();
  if (client && client.connected()) {
    String message = client.readStringUntil('\n');
    message.trim();
    if (message == "next") {
      Serial.println("Received 'next' message");
      currentState = HOLD;
      buttonHandled = false;
      setColor(0, 0, 255); // Blue: ESP_0 has control again
    }
    client.stop();
  }
}

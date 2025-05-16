# ESP WiFi FSM Control Network

This project demonstrates a simple network of ESP microcontrollers (ESP32 or ESP8266) communicating over WiFi using a finite state machine (FSM). Each ESP module takes a turn activating an LED and waits for a button press to pass control to the next module via TCP.

## Project Goal

- Create a chain of ESP devices that pass control in sequence.
- Use visual feedback through RGB LEDs to indicate each device’s state.
- Trigger state transitions via button presses.
- Use TCP communication over WiFi to coordinate behavior between ESPs.

## Components

- 2 or more ESP32 or ESP8266 development boards
- RGB LED per board (connected to PWM-compatible GPIO pins)
- Push-button per board
- WiFi network for communication

## Finite State Machine (FSM)

Each ESP runs a simple FSM with the following states:

- `HOLD`: The ESP has control. The LED is blue. It waits for a button press.
- `PRESSED`: Button has been pressed. The LED turns green. The ESP sends a TCP message to the next ESP and transitions to `WAIT`.
- `WAIT`: The ESP is idle and waiting to receive a `"next"` message from the previous ESP. The LED is off.

## Color Indication

- Blue: This ESP has control (`HOLD` state)
- Green: Button pressed and control passed to next ESP (`PRESSED` state)
- Off: Waiting for control (`WAIT` state)
- Red (blinking): Connecting to WiFi
- Yellow: WiFi connection timeout (device restarts)

## Wiring Example

| Component | GPIO Pin |
|----------|----------|
| LED Red  | 21       |
| LED Green| 22       |
| LED Blue | 23       |
| Button   | 19       |
![schakeling](image.png)
Pins can be customized in the code.

## TCP Communication

- Each ESP runs a TCP server on port 4080 using the `WiFiServer` class.
- When the button is pressed in the `HOLD` state, it sends a `"next"` message to the next ESP using `WiFiClient`.

## Setup

1. Configure each ESP with your WiFi credentials.
2. Assign each ESP a static IP (either manually or through DHCP reservation).
3. In each sketch, update:
   - `ssid` and `password` for WiFi.
   - `nextEspIp` with the IP address of the next ESP in the chain.
4. Flash the correct code to each ESP (ESP_0, ESP_1, ESP_2, etc.).
5. Power on all ESPs and monitor serial output for debugging.

## Notes

- The system includes a 1-minute timeout for WiFi connection. If a device cannot connect, it will automatically restart.
- The debounce time for button presses is 50 milliseconds.
- LEDs are controlled using `analogWrite` (PWM).


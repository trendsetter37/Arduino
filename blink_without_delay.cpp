/* blink without using the delay() function */

const int ledPin =13;

// variables that change
int ledState = LOW; // led state used to set the led
long previousMillis = 0; // will store the last time LED was updated

long interval = 1000; // interval at which to blink in milliseconds (one second blink)

void setup() {
	pinMode(ledPin, OUTPUT);
}

void loop() {
	unsigned long currentMillis = millis(); // what is the current time in milliseconds

	// Does the ledState need to change?
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

		if (ledState == LOW)
			ledState = HIGH;
		else
			ledState = LOW;

		digitalWrite(ledPin, ledState);
	}
}
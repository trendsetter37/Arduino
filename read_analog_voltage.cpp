/* Read analog voltage from pot */ 

void setup() {

	// initialize serial communication
	Serial.begin(9600);
}

// the loop routine runs over and over again

void loop() {
	// read input value on analog pin A0
	int sensorValue = analogRead(A0);
	// convert analog reading 
	float voltage = sensorValue * (5.0 / 1023.0);
	// print out result
	Serial.println(voltage);
}
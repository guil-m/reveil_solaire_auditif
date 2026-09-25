int Led = 13; // define LED Interface
int Mercury = 3; // define the mercury tilt switch sensor interface
int val; // define numeric variables val

void setup() {
pinMode(Led, OUTPUT); // define LED as output interface
pinMode(Mercury, INPUT); // define the mercury tilt switch sensor output interface
}

void loop() {
val = digitalRead(Mercury); // read the values assigned to the digital interface 3 val

// When the mercury tilt switch sensor detects a signal, LED flashes
if (val == HIGH) {
digitalWrite(Led, HIGH);
}
else {
digitalWrite(Led, LOW);
}
}

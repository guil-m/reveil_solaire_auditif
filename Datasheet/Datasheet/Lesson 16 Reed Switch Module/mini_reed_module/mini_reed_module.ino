int Led = 13 ; // define LED Interface
int Sensor = 2; // define the Reed sensor interfaces 
int val ; // define numeric variables val
 
void setup ()
{
pinMode (Led, OUTPUT) ; // define LED as output interface
pinMode (Sensor, INPUT) ; // output interface as defined Reed sensor
}
void loop () 
{
val = digitalRead (Sensor) ; // digital interface will be assigned a value of 2 to read val
if (val == HIGH) // When the Reed sensor detects a signal, LED flashes
{
digitalWrite (Led, LOW);
}
else
{
digitalWrite (Led, HIGH);
}
}


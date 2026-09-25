//2018.6.27
int LedPinA = 3;
int LedPinB = 6; 
int ButtonPinA = 4; 
int ButtonPinB = 7;
int buttonStateA = 0; 
int buttonStateB = 0; 
int brightness = 0; 
void setup ()
{
pinMode (LedPinA, OUTPUT);
pinMode (LedPinB, OUTPUT); 
pinMode (ButtonPinA, INPUT); 
pinMode (ButtonPinB, INPUT);
}
void loop ()
{
buttonStateA = digitalRead (ButtonPinA);
if (buttonStateA == LOW &&brightness!= 255)
{
brightness=brightness+5;
}
analogWrite (LedPinA, brightness); 
analogWrite (LedPinB, 255-brightness);
delay (200);
buttonStateB = digitalRead (ButtonPinB);
if (buttonStateB == LOW &&brightness>= 255)
{
brightness=0;
analogWrite (LedPinA, brightness);  
analogWrite (LedPinB, 255-brightness);
delay (1000);
}
}


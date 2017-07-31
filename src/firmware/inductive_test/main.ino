
#define SENSOR_PIN 2


void setup()                    
{
   Serial.begin(9600);
}

void loop()                    
{
    Serial.print("\t");                    // tab character for debug windown spacing

    Serial.print(total1);                  // print sensor output 1
    Serial.print('\n');

    delay(10);                             // arbitrary delay to limit data to serial port 
}


// For reference, the ball bearing cup is on the right.
#define SENSOR_1_PIN 3 // inner left
#define SENSOR_2_PIN 4 // inner right
#define MAGNET_ENABLE_PIN 6
#define LED_PIN 13

#define S1 0
#define S2 1

#define BALL_UNKNOWN 1 // We don't know where the ball is.
#define BALL_BETWEEN_4 2 // It's out past the fourth sensor.
#define BALL_BETWEEN_34 3 // It's between the third and fourth sensors.
#define BALL_BETWEEN_M3 4 // It's between the magnet and the third sensor.
#define BALL_BETWEEN_2M 5 // It's between the 2nd sensor and magnet.
#define BALL_BETWEEN_12 6 // It's between the first and second sensor.
#define BALL_BETWEEN_1 7 // It's out past the first sensor.

// All sensor data arrays are formatted {inner left, inner right}.
bool current_state[4] = {false, false};
bool last_state[4] = {false, false};
bool changed[4] = {false, false};

// Record the clock when the sensor last went high.
unsigned int last_high[4] = {0, 0};

// The clock time when we last turned on the magnet.
unsigned int last_magnet_on = 0;

int ball_state = BALL_UNKNOWN;

// The index of the last two sensors to detect the ball. The most recent sensor is position 0. The next to last is position 1.
int last_sensor_index_high[2] = {-1, -1};

bool is_state(int s0, int s1){
    return last_sensor_index_high[0] == s0 && last_sensor_index_high[1] == s1;
}

void activate_electromagnet(){
    digitalWrite(MAGNET_ENABLE_PIN, HIGH);
}

void deactivate_electromagnet(){
    digitalWrite(MAGNET_ENABLE_PIN, LOW);
}

void setup()                    
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(SENSOR_1_PIN, INPUT);
    pinMode(SENSOR_2_PIN, INPUT);
    
    // Initialize the electromagnet in the off state.
    pinMode(MAGNET_ENABLE_PIN, OUTPUT);
    digitalWrite(MAGNET_ENABLE_PIN, LOW);

    Serial.begin(9600);
}

void loop()                    
{

    // Remember last sensor states so we can detect changes.
    for(int i=0; i<=4; i++){
        last_state[i] = current_state[i];
    }
    
    // Read current sensor states.
    current_state[S1] = !digitalRead(SENSOR_1_PIN);
    current_state[S2] = !digitalRead(SENSOR_2_PIN);
    
    // Detect changes.
    for(int i=0; i<=2; i++){
        changed[i] = current_state[i] != last_state[i];
        if(changed[i] && current_state[i]){
            last_high[i] = millis();
            last_sensor_index_high[1] = last_sensor_index_high[0];
            last_sensor_index_high[0] = i;
        }
    }

    digitalWrite(LED_PIN, current_state[0]||current_state[1]);
    
    if(current_state[0]||current_state[1]){
        activate_electromagnet();
    }else{
        deactivate_electromagnet();
    }
    
    /*/ Estimate ball direction.
    switch(ball_state){
        case BALL_UNKNOWN:
            break;
        case BALL_BETWEEN_4:
            break;
        case BALL_BETWEEN_34:
            break;
        case BALL_BETWEEN_M3:
            break;
        case BALL_BETWEEN_2M:
            break;
        case BALL_BETWEEN_12:
            break;
        case BALL_BETWEEN_1:
            break;
    }*/

    /*
    Serial.println(String("S1:")+String(current_state[S1]));
    Serial.println(String("S2:")+String(current_state[S2]));
    Serial.println(String("S3:")+String(current_state[S3]));
    Serial.println(String("S4:")+String(current_state[S4]));
    delay(10);
    */

}

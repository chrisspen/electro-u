
#define DEBUG 1

// The amount of time to wait without ball state change before reseting the ball state.
#define STATE_TIMEOUT 5000 // milliseconds

#define RESET_TIMEOUT 5000 // milliseconds

// If the ball is moving too slowly, this ratio will be used.
//#define DURATION_RATIO_OVER_ACCEL 0.79 // over-accelerates
//#define DURATION_RATIO_OVER_ACCEL 0.86 // over-accelerates
//#define DURATION_RATIO_OVER_ACCEL 0.87 // over-accelerates
//#define DURATION_RATIO_OVER_ACCEL 0.88 // over-accelerates

// If the ball is moving too fast, this ratio will be used.
//#define DURATION_RATIO_OVER_DECAY 0.80 // over-decays
//#define DURATION_RATIO_OVER_DECAY 0.87 // over-decays
//#define DURATION_RATIO_OVER_DECAY 0.88 // over-decays
//#define DURATION_RATIO_OVER_DECAY 0.89 // over-decays

// decrease=>accel, increase=>decay
//#define DEFAULT_DURATION_RATIO 0.78 // over-accels
//#define DEFAULT_DURATION_RATIO 0.79 // barely over-accels
#define DEFAULT_DURATION_RATIO 0.80 // barely over-decays
//#define DEFAULT_DURATION_RATIO 0.89 // over-decays

// The amount the duration ratio is increased or decreased to correct the velocity.
//#define DURATION_RATIO_STEP 0.05 // too drastic
#define DURATION_RATIO_STEP 0.01

// A duration under this value means the ball is moving too fast, so the electromagnet will not be activated, causing friction to slightly slow it down.
//#define DURATION_THRESHOLD 58
#define TARGET_VELOCITY 0.30 // meter/second, too fast
//#define TARGET_VELOCITY 0.305 // meter/second, decays
//#define TARGET_VELOCITY 0.38 // meter/second, maximum natural velocity

// The range from the target velocity the actual velocity is allowed to get before triggering correction.
#define TARGET_VELOCITY_RANGE 0.05

// Physical constants of the apparatuts.
#define SENSOR_WIDTH 16.0 // mm
#define SENSOR_OFFSET 21.0 // mm, distance from inner edge of sensor to center of electromagnet
#define MAGNET_FORCE 50.0 // newtons
#define MAGNET_RADIUS 12.5 // mm
#define MAGNET_LENGTH = 20 // mm
#define GRAVITY 9.80665 // m/s^2, acceleration of gravity
#define BALL_MASS 3 // grams
#define LENGTH_OF_ARC_TO_CENTER 85 // mm, the full length of the arc of the slope
// s = r * theta => theta = s/r => 85/500 = 0.17 radians = 9.7 degrees for the ball to roll from start to center
#define LENGTH_OF_ARC_SENSOR_TO_CENTER 20 // mm, length of arc from inner sensor edge to center
// s = r * theta => theta = s/r => 20/500 = 0.04 radians = 2.29 degrees for the ball to roll from sensor to center
// After crossing the sensor, the ball will undergo an additional acceleration from gravity of: cos(pi/2. - 0.04) * 9.8 = 0.39 m/s^2
//
// https://www.intemag.com/magnetic-materials-faqs
// For a circular magnet with a radius of R and length L, the field Bx at the centerline of the magnet at distance X from the surface can be calculated
// by the following formula, where Br is the residual induction of the material): 
// Fx = Bx/2 * ((L - X)/sqrt(R**2 + (L + X)**2) - X/sqrt(R**2 + X**2))
//
// For a magnet with a surface force of 50 N = 5 kg, R=12.5mm, L=20mm, => X = 0
// (5*2)/((20 - 0)/sqrt(12.5**2 + (20 + 0)**2) - 0/sqrt(12.5**2 + 0**2)) = Bx = 11.79

// F = m*a
// t = (vf - vi)/a
// d = vi*t + 0.5*a*t^2

// For reference, the ball bearing cup is on the right.
#define SENSOR_PIN 3 // inner right
#define MAGNET_ENABLE_PIN 6
#define LED_PIN 13

// Ball states.
#define BALL_RIGHT_START 1 // ball is in the top right starting cup
#define BALL_RIGHT_SENSING 2 // ball has just entered detection range of right sensor
#define BALL_RIGHT_PULL 3 // ball has just left the detection range of right sensor, moving towards electromagnet
#define BALL_LEFT_FREE 4 // ball at electromagnet center, moving left, where push no longer needed
#define BALL_RIGHT_PASSING 5 // ball has gone up the left slope and returned to go up the right slope

// All sensor data arrays are formatted {inner left, inner right}.
bool current_state = false;

// Which state we currently believe the ball to be in.
int ball_state = BALL_RIGHT_START;

// Time when the ball was first detected by the sensor.
unsigned long sensor_high_time;

// Time when the ball became undetected by the sensor.
unsigned long sensor_low_time;

// Time when ball state was last updated.
unsigned long last_state_change_time = 0;

// Time in milliseconds that it should take bearing to go from the sensor to center of electromagnet.
unsigned long duration_to_center_time = 0; // PID output

// Time when the magnet was last turned on.
unsigned long magnet_on_time = 0;

// Time when the magnet was last turned off.
unsigned long magnet_off_time = 0;

unsigned long last_reset = RESET_TIMEOUT;

// Blink timer.
unsigned long last_blink = 0;

// The ball's estimated velocity, calculated from the sensor detection times and the known width of the sensor.
double ball_velocity = 0; // m/s, PID input

double min_ball_velocity = 1000;

double max_ball_velocity = 0;

double mean_ball_velocity = 0;

double duration_ratio = DEFAULT_DURATION_RATIO;

void activate_electromagnet(){
    magnet_on_time = millis();
    digitalWrite(MAGNET_ENABLE_PIN, HIGH);
}

void deactivate_electromagnet(){
    magnet_off_time = millis();
    digitalWrite(MAGNET_ENABLE_PIN, LOW);
}

void go_to_state(int s){
    ball_state = s;
    last_state_change_time = millis();
}

void on_sensor_change(){
    // Read current sensor states.
    current_state = !digitalRead(SENSOR_PIN);
    //digitalWrite(LED_PIN, current_state);
}

void reset(){
    #if DEBUG
    Serial.println(String("reseting:")+String(BALL_RIGHT_START)+String(" millis:")+String(millis())+String(" last_state_change_time:")+String(last_state_change_time));
    #endif

    go_to_state(BALL_RIGHT_START);

    min_ball_velocity = 1000;
    max_ball_velocity = 0;
    
    duration_ratio = DEFAULT_DURATION_RATIO;
}

//http://www.instructables.com/id/two-ways-to-reset-arduino-in-software/
void(* resetFunc) (void) = 0;

void setup()                    
{
    
    // https://www.arduino.cc/en/Reference/AttachInterrupt
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), on_sensor_change, CHANGE);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    //pinMode(SENSOR_1_PIN, INPUT);
    pinMode(SENSOR_PIN, INPUT);
    
    // Initialize the electromagnet in the off state.
    pinMode(MAGNET_ENABLE_PIN, OUTPUT);
    digitalWrite(MAGNET_ENABLE_PIN, LOW);

    #if DEBUG
    Serial.begin(115200); // Must match MONITOR_BAUDRATE.
    #endif
    
    reset();
    
}

void loop()                    
{
    
    // Blink status LED.
    if((millis() - last_blink) >= 1000){
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        last_blink = millis();
    }
    
    // Estimate ball direction.
    switch(ball_state){
        case BALL_RIGHT_START:
        
            // Ensure the magnet is off.
            deactivate_electromagnet();

            if(current_state){
                // Ball has slid down the ramp and entered the sensor's detection zone so begin measuring velocity.
                sensor_high_time = millis();

                //Serial.println(String("going to right sensing:")+String(BALL_RIGHT_SENSING));
                go_to_state(BALL_RIGHT_SENSING);
            }
            
            //TODO:reset after N hours?
            //resetFunc();
            
            break;
        case BALL_RIGHT_SENSING:
            if(!current_state){
                // Ball has slid down the ramp further and left the sensor's detection zone, so calculate velocity using
                // the time difference between start and end and sensor width.
                sensor_low_time = millis();
                
                //ball_velocity = (SENSOR_WIDTH/1000.)/((sensor_low_time - sensor_high_time)/1000.);
                ball_velocity = SENSOR_WIDTH/(sensor_low_time - sensor_high_time); // meter/second
                //min_ball_velocity = min(ball_velocity, min_ball_velocity);
                //max_ball_velocity = max(ball_velocity, max_ball_velocity);
                //mean_ball_velocity = (min_ball_velocity + max_ball_velocity)/2.;

                // Calculate total time it should take ball bearing to reach the center of the electromagnet => distance/velocity
                // Note, this doesn't account for acceleration.
                duration_to_center_time = (SENSOR_OFFSET/1000.)/ball_velocity*1000; // milliseconds
                //Serial.println(String("duration_to_center_time1(ms):")+String(duration_to_center_time));
                // Shorten the duration to account for acceleration due to pull.
                //duration_to_center_time *= 0.84; // too much accel, stable but then flys off
                //duration_to_center_time *= 0.8475; // still too much, stable but then flys off
                //duration_to_center_time *= 0.85; // almost perfect? over accelerates
                //duration_to_center_time *= 0.86; // almost perfect? over accelerates
                /////duration_to_center_time *= 0.865; // over accelerates
                //duration_to_center_time *= 0.87; // almost perfect? decays
                //duration_to_center_time *= 0.93; // decays quickly
                //duration_to_center_time *= 0.94; // decays quickly
                //duration_to_center_time *= 0.95; // decays quickly
                
                //if(ball_velocity < TARGET_VELOCITY){
                    // Ball too slow, so speed up.
                //    duration_to_center_time *= DURATION_RATIO_OVER_ACCEL;
                //}else{
                    // Ball too fast, so slow down.
                //    duration_to_center_time *= DURATION_RATIO_OVER_DECAY;
                //}
                duration_to_center_time *= duration_ratio;

                // If duration ~= 60 ms, that's stable or too slow, so we need to speed it up by activating the magnet.
                // If duration < 60 ms, that's going too fast, so we need to slow it down by keeping the magnet off and letting friction slow the ball.
                // Turn on electromagnet so it can immediately begin pulling, but only when ball slowing.
                //if(duration_to_center_time >= DURATION_THRESHOLD){
                if(ball_velocity <= TARGET_VELOCITY){
                    activate_electromagnet();
                    
                    #if DEBUG
                    Serial.println(String("magnet activated"));
                    #endif
                }
    
                #if DEBUG
                //Serial.println(String("sensor_high_time:")+String(sensor_high_time));
                //Serial.println(String("sensor_low_time:")+String(sensor_low_time));
                Serial.println(String("duration_ratio:")+String(duration_ratio));
                Serial.println(String("ball_velocity(m/s):")+String(ball_velocity));
                //Serial.println(String("min_ball_velocity(m/s):")+String(min_ball_velocity));
                //Serial.println(String("max_ball_velocity(m/s):")+String(max_ball_velocity));
                //Serial.println(String("duration_to_center_time2(ms):")+String(duration_to_center_time));
                //Serial.println(String("going to right pull:")+String(BALL_RIGHT_PULL));
                #endif

                go_to_state(BALL_RIGHT_PULL);
            }
            break;
        case BALL_RIGHT_PULL:
            // Keep pulling until ball bearing has reached the center of the electromagnet.
            // Since we can't directly measure the ball at this point, we rely on timing based on our previous calculation of the ball's velocity.
            if((millis() - sensor_low_time) >= duration_to_center_time){
                
                // Ball should be near or at the center of the electromagnet, so deactivate it to allow the ball to continue up the slope.
                deactivate_electromagnet();
                
                //Serial.println(String("magnet duration:")+String(magnet_off_time - magnet_on_time));
                //Serial.println(String("going to left free:")+String(BALL_LEFT_FREE));
                go_to_state(BALL_LEFT_FREE);
            }
            break;
        case BALL_LEFT_FREE:
            // The ball's going up the left slope and then coming back down.
            if(current_state){
                // Ball has come back and is going back up the right slope over the sensor.
                //Serial.println(String("going to right passing:")+String(BALL_RIGHT_PASSING));
                go_to_state(BALL_RIGHT_PASSING);
            }
            break;
        case BALL_RIGHT_PASSING:
            // The ball's going up the right slope over the sensor.
            if(!current_state){
                // Ball has gone up the right slope passed the sensor, near at the starting position.
                //Serial.println(String("going to start:")+String(BALL_RIGHT_START));
                go_to_state(BALL_RIGHT_START);

                // If last known velocity is outside accepted range, then modify the duration ratio to increase or decrease the velocity.
                if(abs(ball_velocity - TARGET_VELOCITY) > TARGET_VELOCITY_RANGE){
                    if(ball_velocity > TARGET_VELOCITY){
                        // Velocity too fast, decelerate.
                        duration_ratio += DURATION_RATIO_STEP;
                    }else{
                        // Velocity too slow, accelerate.
                        duration_ratio -= DURATION_RATIO_STEP;
                    }
                }else{
                    // If we're in range again then reset ratio.
                    duration_ratio = DEFAULT_DURATION_RATIO;
                }
                
                // When the ball's back in start, try resetting the board periodically.
                //if((millis() - last_reset) >= RESET_TIMEOUT){
                //    last_reset = millis();
                //    resetFunc();
                //}

            }
            break;
    }

    // If state doesn't change within the specified timeout, then assume something's gone wrong and revert to default state.
    if((millis() - last_state_change_time) >= STATE_TIMEOUT){
        reset();
    }

}

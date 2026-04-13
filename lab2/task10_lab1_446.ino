#include <PDM.h> // mic
#include <Arduino_APDS9960.h> // prox, lighting
#include <Arduino_BMI270_BMM150.h> // imu accelerometer value




/*
audio
ambient brightness
physical motion from imu
proximity sensor
*/

short sampleBuffer[256];
volatile int samplesRead = 0;
const float motionThreshold = 0.15;
const int darkThreshold = 25;
const int nearThreshold = 175;
const int soundThreshold = 85;


void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1500);

  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)) {
    Serial.println("failed to start microphone");
    while(1);
  }
  else if (!IMU.begin()) {
    Serial.println("failed to init. IMU");
    while(1);
  }
  else if (!APDS.begin()) {
    Serial.println("failed to init. APDS9960 sensor");
    while(1);
  }

  Serial.println("beginning desk environment test");
}

float lastX, lastY, lastZ;

void loop() {
  int r, g, b, c;
  float x, y, z;

  if (APDS.proximityAvailable() && APDS.colorAvailable() && 
  IMU.accelerationAvailable() && samplesRead) {
    // microphone
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }
    int level = sum / samplesRead; // MIC LEVEL
    samplesRead = 0;

    // COLOR
    APDS.readColor(r,g,b,c);
    int clear = c;

    // motion
    IMU.readAcceleration(x, y, z);
    // float motion = sqrt(x*x + y*y + z*z); // print to serial monitor for ' motion'
    
    float deltaX = abs(x-lastX);
    float deltaY = abs(y-lastY);
    float deltaZ = abs(z-lastZ);

    float motion = deltaX + deltaY + deltaZ;

    // proximity
    int prox = APDS.readProximity();
    
    int f_sound = (level > soundThreshold) ? 1: 0;
    int f_dark = (clear < 50) ? 1: 0;
    int f_moving = (motion > motionThreshold) ? 1: 0;
    int f_near = (prox < nearThreshold) ? 1 : 0; 

    String finalLabel = "IDLE";

//  QUIET_BRIGHT_STEADY_FAR 
    if (!f_sound && !f_dark && !f_moving && !f_near) {
        finalLabel = "QUIET_BRIGHT_STEADY_FAR";
    }
    
    // NOISY_BRIGHT_STEADY_FAR 
    else if (f_sound && !f_dark && !f_moving && !f_near) {
        finalLabel = "NOISY_BRIGHT_STEADY_FAR";
    }

    // QUIET_DARK_STEADY_NEAR 
    else if (!f_sound && f_dark && !f_moving && f_near) {
        finalLabel = "QUIET_DARK_STEADY_NEAR";
    }

    // NOISY_BRIGHT_MOVING_NEAR 
    else if (f_sound && !f_dark && f_moving && f_near) {
        finalLabel = "NOISY_BRIGHT_MOVING_NEAR";
    }

 


    Serial.print("raw, mic=");

    Serial.print(level);

    Serial.print(",clear=");
    Serial.print(clear);
    
    Serial.print(",motion=");
    Serial.print(motion);

    Serial.print(",prox=");
    Serial.println(prox);



  Serial.print("flags,sound="); 
  Serial.print(f_sound);
  Serial.print(",dark=");       
  Serial.print(f_dark);
  Serial.print(",moving=");     
  Serial.print(f_moving);
  Serial.print(",near=");       
  Serial.println(f_near);

  Serial.print("state,");
  Serial.println(finalLabel);


    lastX = x;
    lastY = y;
    lastZ = z;
    

  }
  delay(100);

  // put your main code here, to run repeatedly:

}


/*
1. QUIET_BRIGHT_STEADY_FAR
2. NOISY_BRIGHT_STEADY_FAR
3. QUIET_DARK_STEADY_NEAR
4. NOISY_BRIGHT_MOVING_NEAR
*/

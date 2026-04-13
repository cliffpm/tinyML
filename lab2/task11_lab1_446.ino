#include <Arduino_BMI270_BMM150.h>
#include <Arduino_HS300x.h>
#include <Arduino_APDS9960.h>

const float humid_threshold = 45.0;
const float mag_threshold = 10.0;
const float color_threshold = 0.1;
const float temp_threshold = 35.0;


void setup() {
  Serial.begin(115200);
  delay(3000);


  if (!APDS.begin()){
    Serial.println("APDS sensor failed ...");
    while (1);
  }
  else if (!HS300x.begin()){
    Serial.println("HS300x failed ...");
    while (1);
  }
  else if (!IMU.begin()) {
    Serial.println("IMU failed ...");
    while (1);
  }


  Serial.println("beginning monitoring");
}

int r, g, b, c;
float x, y, z;
float lastMagX, lastMagY, lastMagZ;
float lastRnorm, lastGnorm, lastBnorm;

void loop() {


  if (APDS.colorAvailable() && IMU.magneticFieldAvailable()) {
    float temperature = HS300x.readTemperature();
    float humidity = HS300x.readHumidity();
    IMU.readMagneticField(x, y, z);
    APDS.readColor(r,g,b,c);


    float deltaMag = abs(x- lastMagX) + abs(y-lastMagY) + abs(z - lastMagZ);
    int mag_shift = (deltaMag > mag_threshold) ? 1: 0;

    float total_color = r + g + b;
    float rNorm = r/ total_color;
    float gNorm = g / total_color;
    float bNorm = b / total_color;

    float color_shift = abs(rNorm - lastRnorm) + abs(gNorm - lastGnorm) + abs(bNorm - lastBnorm);
    
    lastRnorm = rNorm;
    lastGnorm = gNorm;
    lastBnorm = bNorm;

    int light_or_color_change = (color_shift > color_threshold) ? 1 : 0;

    int humid_jump = (humidity > humid_threshold) ? 1 : 0;
    int temp_rise = (temperature > temp_threshold) ? 1: 0;


    // 5. DETERMINE EVENT LABEL
    String eventLabel = "BASELINE_NORMAL";
    if (humid_jump || temp_rise) eventLabel = "BREATH_OR_WARM_AIR_EVENT";
    else if (mag_shift) eventLabel = "MAGNETIC_DISTURBANCE_EVENT";
    else if (light_or_color_change) eventLabel = "LIGHT_OR_COLOR_CHANGE_EVENT";
    // 6. FORMATTED PRINT STATEMENTS
    // Line 1: Raw Values
    Serial.print("raw,rh=");     Serial.print(humidity);
    Serial.print(",temp=");      Serial.print(temperature);
    Serial.print(",mag=");       Serial.print(deltaMag);
    Serial.print(",r=");         Serial.print(r);
    Serial.print(",g=");         Serial.print(g);
    Serial.print(",b=");         Serial.print(b);
    Serial.print(",clear=");     Serial.println(c);

    // Line 2: Flags
    Serial.print("flags,humid_jump=");      Serial.print(humid_jump);
    Serial.print(",temp_rise=");            Serial.print(temp_rise);
    Serial.print(",mag_shift=");            Serial.print(mag_shift);
    Serial.print(",light_or_color_change="); Serial.println(light_or_color_change);

    // Line 3: Event
    Serial.print("event,");      Serial.println(eventLabel);


    lastMagX = x;
    lastMagY = y;
    lastMagZ = z;
  }

  delay(100);

}

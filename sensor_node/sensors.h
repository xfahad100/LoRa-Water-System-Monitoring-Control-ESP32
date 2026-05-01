#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4      // DS18B20 on GPIO4

//quality
namespace sensor {
  extern float ecCalibration;

  // 2-point calibration values
  extern float tdsRaw1;
  extern float tdsRaw2;
  extern float tdsRef1;
  extern float tdsRef2;

  extern float tdsSlope;
  extern float tdsOffset;

  extern float tdsThreshold;
}
extern float TDS_FACTOR;

void initSensors();
void startSensorTask();

//level sensor
#define WATER_LEVEL_PIN 1
#define R_FIXED 150.0f      // Fixed resistor in ohms
#define ADC_MAX 4095.0f
#define VREF 3.3f 

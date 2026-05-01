#include <OneWire.h>
#include <DallasTemperature.h>
#include <UltrasonicA02YYUW.h>

#define ONE_WIRE_BUS 4      // DS18B20 on GPIO4

//quality
namespace sensor {
  float ecCalibration = 1.0f;

  // 2-point calibration values
  float tdsRaw1 = 0, tdsRaw2 = 0;
  float tdsRef1 = 0, tdsRef2 = 0;

  float tdsSlope = 0.719642f;
  float tdsOffset = -6.969f;

  float tdsThreshold = 10;
}
float TDS_FACTOR = 0.65f;

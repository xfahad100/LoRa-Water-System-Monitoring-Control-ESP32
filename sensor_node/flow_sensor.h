#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

void initFlowSensor();
void startFlowTask();
float getFlowRate();
uint32_t getTotalMilliLitres();

#endif // FLOW_SENSOR_H

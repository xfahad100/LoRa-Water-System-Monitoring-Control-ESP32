#ifndef LORA_COMM_H
#define LORA_COMM_H

void initLoRa();
void processTelemetry();
void startLoRaTask();
void sendPong();
void updateOutputs();

#endif // LORA_COMM_H

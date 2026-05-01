#include <Arduino.h>
#include "config.h"
#include "shared_data.h"
#include "protocol.h"


HardwareSerial& lora = Serial2;
bool allowSend = false;
bool needAck = false;

bool hdp = 0;      // case 65
bool osmose = 0;   // case 66
bool wasser = 0;   // case 68
bool tanken = 0;   // case 45
bool tank_leeren = 0;   // case 46

#define RX_BUF_SIZE 256
char rxBuf[RX_BUF_SIZE];

void initLoRa()
{
    lora.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);
    delay(200);

    lora.println("AT+BAND=868000000");
    delay(150);
    lora.println("AT+ADDRESS=2");
    delay(150);
    lora.println("AT+NETWORKID=5");
    delay(150);
}

void sendPong()
{
    lora.println("AT+SEND=1,4,4101");
}



void processTelemetry()
{
    if (needAck) {
      // VP = 0x42, VAL = 0x01  → "4201"
     // Serial.println("ACK sent!");
      lora.println("AT+SEND=1,4,4201");
      needAck = false;
    }
    if(allowSend){
    
    String payload = "63";
    payload += ((int)qualityByte < 16 ? "0" : "") + String((int)qualityByte, HEX);
    payload += "64";
    payload += (temperatureValue < 16 ? "0" : "") + String(temperatureValue, HEX);
    payload += "61";
    payload += ((int)flowByte < 16 ? "0" : "") + String((int)flowByte, HEX);
    payload += "72";
    payload += ((int)waterLevel < 16 ? "0" : "") + String((int)waterLevel, HEX);
    payload += "62";
    payload += ((int)pressureByte < 16 ? "0" : "") + String((int)pressureByte, HEX);
    if(req_button_states){
      payload += "3101";
    }
    //payload += "4101";

    lora.print("AT+SEND=1,");
    lora.print(payload.length());
    lora.print(",");
    lora.println(payload);
    allowSend = false;
  }
}

void updateOutputs()
{
    pcf_state = 0xFFFF;   // all OFF

    if (hdp)
    {
        Serial.println("HDP ACTIVE");
        pcf_state &= ~((1 << 2) | (1 << 6) | (1 << 9));
    }

    if (osmose)
    {
        Serial.println("OSMOSE ACTIVE");
        pcf_state &= ~((1 << 1) | (1 << 5) | (1 << 7) | (1 << 9) | (1 << 10));
    }

    if (wasser)
    {
        Serial.println("WASSER ACTIVE");
        pcf_state &= ~((1 << 2) | (1 << 3) | (1 << 6) | (1 << 9));
    }

    if (tanken && waterLevel < 100.0)
    {
        Serial.println("TANKEN ACTIVE");
        pcf_state &= ~((1 << 0) | (1 << 12));
    }
    else{
        pcf_state |= (1 << 0) | (1 << 12);
    }

    if (tank_leeren && waterLevel > 1.0)
    {
        Serial.println("TANK_LEEREN ACTIVE");
        pcf_state &= ~((1 << 4) | (1 << 12));
    }
    else {
        pcf_state |= ((1 << 4) | (1 << 12));
    }
    

    

    // Optional: print final state (very useful)
    //Serial.print("PCF STATE: ");
   // Serial.println(pcf_state, BIN);

    //pcf_state = ~pcf_state;
    //pcf_state = 0x0000;

    pcf8575.write16(pcf_state);
}


void handleLine(char *msg)
{
    char *p = msg + 5;          // skip "+RCV="

    // skip address
    p = strchr(p, ',');
    if (!p) return;

    // get payload length
    int len = atoi(p + 1);
    if (len < 2) return;        // must be at least 1 VP/VAL
    if (len % 2 != 0) return;   // must be even (VP+VAL)

    // skip to payload start
    p = strchr(p + 1, ',');
    if (!p) return;
    p++;

    // ---- loop through each VP/VAL pair ----
    for (int i = 0; i < len/2; i += 2)
    {
        char hexStr[3] = {p[i * 2], p[i * 2 + 1], '\0'};
        uint8_t vp = strtoul(hexStr, NULL, 16);

        char hexStr2[3] = {p[i * 2 + 2], p[i * 2 + 3], '\0'};
        uint8_t val = strtoul(hexStr2, NULL, 16);

      // ==================================================
      // PING VOM SENDER → PONG ANTWORT
      // ==================================================
      if (vp == 0x40)
      {
        lastMessageTime = millis();  // Verbindung lebt
        sendPong();
        count_htbt_tout = 0;
        return;
      }


  // ==================================================
  // SCHALTER-STEUERUNG
  // ==================================================
  switch (vp)
  {
    case 0x52:
      if (vp == 0x52 && val == 0x01) {
        allowSend = true;
      }
        break;
    case 0x65:  // HDP
        hdp = val;
        needAck = true;
        break;

    case 0x66:  // Osmose + Heizung
        osmose = val;
        needAck = true;
        break;

    case 0x68:  // Wasser Quelle
        wasser = val;
        needAck = true;
        break;

    case 0x45:  // Tanken
        tanken = val;
        needAck = true;
        break;

    case 0x46:  // Tanken Leeren
        tank_leeren = val;
        needAck = true;
        break;

      

    default:
        Serial.print("UNBEKANNT VP=");
        Serial.println(vp, HEX);
  }
  updateOutputs();
}
}

void processLoRa()
{
    if (!lora.available())
        return;

    int len = lora.readBytesUntil('\n', rxBuf, RX_BUF_SIZE - 1);
    if (len <= 0)
        return;

    rxBuf[len] = '\0';

    if (strncmp(rxBuf, "+RCV=", 5) == 0) {
        //Serial.println(rxBuf);
        handleLine(rxBuf);
    }
}

void loraTask(void *pvParameters)
{
    while (true)
    {
        processLoRa();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void startLoRaTask()
{
    xTaskCreatePinnedToCore(loraTask,"LoRaRX",4096,NULL,2,NULL,1);
}
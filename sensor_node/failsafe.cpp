#include <Arduino.h>
#include "shared_data.h"
#include "config.h"

unsigned long lastMessageTime = 0;
const unsigned long MESSAGE_TIMEOUT = 30000;

void initFailsafe()
{
    lastMessageTime = millis();
}

void failsafeCheck()
{
  unsigned long now = millis();
  if (now - lastMessageTime > MESSAGE_TIMEOUT)
  {
    count_htbt_tout++;
    if(count_htbt_tout > 5){
    pcf8575.write16(pcf_state);
    digitalWrite(LED_HB, LOW);
    Serial.println("htbt timeout!");
    req_button_states = true;
    }
    return;
  }

  // Heartbeat visualisieren
  digitalWrite(LED_HB, (now / 500) % 2);
}
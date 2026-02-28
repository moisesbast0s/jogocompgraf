#include "graphics/FlashlightState.h"

// alocação real da memória das variáveis
float flashlightBattery = 100.0f;   // bateria inicia em 100%
bool flashlightOn = true;
// 100% em 120 segundos => ~0.8333% por segundo
float batteryDrainRate = 100.0f / 120.0f;      // % por segundo (≈0.8333)

#include "graphics/FlashlightState.h"

// alocação real da memória das variáveis
float flashlightBattery = 100.0f;   // bateria inicia em 100%
bool flashlightOn = true;
float batteryDrainRate = 1.0f;      // duração de %/s -> a cada um segundo, desce um nível da bateria
                                    // em drawlevel.updateFlashlight() a bateria não desce mais que 25/24 %, pra que o jogador não fique completamente no escuro
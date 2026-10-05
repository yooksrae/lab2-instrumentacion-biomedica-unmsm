#include <Arduino.h>

// Pin del convertidor Digital-Analógico del ESP32 (DAC1 = GPIO25)
const int PIN_DAC = 25;

// Parámetros de la Ecuación de Nernst y Calibración
const float V_BIAS = 1.500;             // Voltaje a pH 7.0 (Offset)
const float SENSIBILIDAD_NERNST = 0.05916; // 59.16 mV/pH a 25 °C
const float GANANCIA_AV = 3.0;          // Ganancia teórica del TL084: Av = 1 + Rf/R1 = 3
const float V_REF_DAC = 3.300;          // Voltaje de referencia típico del DAC del ESP32

// Los 9 puntos oficiales de la Tabla 4 de calibración
const float PUNTOS_PH[9] = {0.0, 2.0, 4.0, 6.0, 7.0, 8.0, 10.0, 12.0, 14.0};

// Función para calcular y generar el voltaje en el DAC
float setPH(float ph) {
  // Vin = Vbias + S * (7 - pH)
  float vin_teorico = V_BIAS + (SENSIBILIDAD_NERNST * (7.0 - ph));

  // Cálculo del valor de 8 bits (0 - 255) para dacWrite
  int dacValue = round((vin_teorico / V_REF_DAC) * 255.0);
  if (dacValue < 0) dacValue = 0;
  if (dacValue > 255) dacValue = 255;

  dacWrite(PIN_DAC, dacValue);

  return vin_teorico;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n========================================================"));
  Serial.println(F("  UNMSM - INSTRUMENTACIÓN BIOMÉDICA III - LAB 2         "));
  Serial.println(F("  Simulador de Electrodo de pH para Calibración TL084   "));
  Serial.println(F("========================================================"));
  Serial.println(F("Salida analógica: GPIO 25 (DAC1) -> Conectar a Pin 3 de TL084 (Buffer)"));
  Serial.println(F("Av Teórico: 3.00 (Rf = 20k, R1 = 10k)"));
  Serial.println(F("--------------------------------------------------------"));
  Serial.println(F("Comandos del Monitor Serial:"));
  Serial.println(F("  Ingresa un valor de pH (ejemplo: 7, 4.5, 14)"));
  Serial.println(F("  'BARRIDO' : Genera secuencialmente los 9 puntos de la Tabla 4"));
  Serial.println(F("========================================================\n"));

  // Inicializar en pH neutro (7.0)
  float vin = setPH(7.0);
  Serial.print(F("[INICIO] pH fijado en 7.00 | Vin teórico = "));
  Serial.print(vin, 3);
  Serial.println(F(" V"));
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    // Modo barrido automático para llenar la Tabla 4 cómodamente en el laboratorio
    if (input.equalsIgnoreCase("BARRIDO")) {
      Serial.println(F("\n--- INICIANDO BARRIDO DE LOS 9 PUNTOS (TABLA 4) ---"));
      Serial.println(F("pH_simulado\tVin_teorico(V)\tVout_teorico(V)\tDAC_Code(0-255)"));
      Serial.println(F("---------------------------------------------------------------"));
      for (int i = 0; i < 9; i++) {
        float ph = PUNTOS_PH[i];
        float vin = setPH(ph);
        float vout = vin * GANANCIA_AV;
        int dac = round((vin / V_REF_DAC) * 255.0);

        Serial.print(ph, 1);
        Serial.print(F("\t\t"));
        Serial.print(vin, 3);
        Serial.print(F("\t\t"));
        Serial.print(vout, 3);
        Serial.print(F("\t\t"));
        Serial.println(dac);
        delay(4000); // 4 segundos para medir con el multímetro en TP2
      }
      Serial.println(F("--- FIN DEL BARRIDO ---\n"));
      return;
    }

    // Modo manual ingresando el pH deseado
    float ph = input.toFloat();
    if (ph < 0.0 || ph > 14.0) {
      Serial.println(F("[ERROR] El pH debe estar en el rango de 0.0 a 14.0"));
      return;
    }

    float vin = setPH(ph);
    float vout = vin * GANANCIA_AV;
    int dac = round((vin / V_REF_DAC) * 255.0);

    Serial.println(F("--------------------------------------------------------"));
    Serial.print(F("-> pH Simulado   : ")); Serial.println(ph, 2);
    Serial.print(F("-> Vin Teórico   : ")); Serial.print(vin, 3); Serial.println(F(" V (Salida DAC en GPIO25)"));
    Serial.print(F("-> Vout Teórico  : ")); Serial.print(vout, 3); Serial.println(F(" V (Esperado en TP2 con Av=3)"));
    Serial.print(F("-> Código DAC    : ")); Serial.println(dac);
    Serial.println(F(">> Mide ahora con el multímetro en TP1 y TP2 para tu informe."));
    Serial.println(F("--------------------------------------------------------"));
  }
}

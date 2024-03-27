/* Includes ---------------------------------------------------------------- */
#include <jhonnye0-project-1_inferencing.h>
#include <Arduino.h>

static const float MAX_TEMPERATURA = 36.800000;
static const float MAX_PRESSAO = 0.714286;
static const float MAX_IMC = 26.500000;
static const float MAX_OXIGENACAO= 99.000000;
static const float MAX_BPM = 87.000000;
static const float MAX_TEMP_AMBIENTE = 32.000000;
static const float MAX_HORARIO = (24*60*60);

String temperatura = "";
String imc = "";
String pressao = "";
String bpm = "";
String tempAmbiente = "";
String horario = "";
String diabetico = "";
String oxigenacao = "";

static const float features[] = {
  1, 2, 3, 4, 5, 6, 7, 8
};

float evaluate(const char* pressaoString) {
    float p1, p2;
    sscanf(pressaoString, "%d/%d", &p1, &p2);

    return p1/p2;
}

// Função para converter hora em string para segundos após a meia-noite
float horaParaSegundosNormalizado(const char* horaString) {
  int hora, minuto, segundo;
  sscanf(horaString, "%d:%d:%d", &hora, &minuto, &segundo);

  // Calcula o total de segundos após a meia-noite
  float totalSegundos = hora * 3600 + minuto * 60 + segundo;
  ei_printf("\nseg: %d\n", totalSegundos);
  
  return totalSegundos/MAX_HORARIO;
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {

    out_ptr[0] = atof(imc.c_str())/MAX_IMC;
    out_ptr[1] = atof(temperatura.c_str())/MAX_TEMPERATURA;
    out_ptr[2] = evaluate(pressao.c_str())/MAX_PRESSAO;
    out_ptr[3] = atof(oxigenacao.c_str())/MAX_OXIGENACAO;
    out_ptr[4] = atof(bpm.c_str())/MAX_BPM;
    out_ptr[5] = atof(tempAmbiente.c_str())/MAX_TEMP_AMBIENTE;
    out_ptr[6] = atof(diabetico.c_str());
    out_ptr[7] = horaParaSegundosNormalizado(horario.c_str());

    return 0;
}

void print_inference_result(ei_impulse_result_t result);

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
}

/**
 * @brief      Arduino main function
 */
void loop()
{
    // Verifica se há dados disponíveis para leitura
    if (Serial.available() > 0) {
      // IMC
      Serial.println("Digite o imc: ");
      imc = Serial.readStringUntil('\n');
      
      Serial.print("Imc recebido: ");
      Serial.println(imc + "\n");

      delay(3000);

      // Temperatura
      Serial.println("Digite a temperatura: ");
      temperatura = Serial.readStringUntil('\n');
      
      Serial.print("Temperatura recebida: ");
      Serial.println(temperatura + " °C\n");

      delay(3000);

      // Pressão
      Serial.println("Digite a pressão: ");
      pressao = Serial.readStringUntil('\n');
      
      Serial.print("Pressão recebida: ");
      Serial.println(pressao + " mmHg\n");

      delay(3000);

      // Oxigenação
      Serial.println("Digite a oxigenação: ");
      oxigenacao = Serial.readStringUntil('\n');
      
      Serial.print("Oxigenação recebida: ");
      Serial.println(oxigenacao + "\n");

      delay(3000);

      // BPM
      Serial.println("Digite o batimento cardíaco (bpm): ");
      bpm = Serial.readStringUntil('\n');
      
      Serial.print("Batimentos cardíacos recebidos: ");
      Serial.println(bpm + "\n");

      delay(3000);

      // Temperatura Ambiente
      Serial.println("Digite a temperatura ambiente: ");
      tempAmbiente = Serial.readStringUntil('\n');
      
      Serial.print("Temperatura ambiente recebida: ");
      Serial.println(tempAmbiente + "\n");

      delay(3000);

      // Diabetes
      Serial.println("Digite 1 se é diabético e 0 caso contrário: \n");
      diabetico = Serial.readStringUntil('\n');
      
      Serial.print("Valor da comorbidade recebida: ");
      Serial.println(diabetico + "\n");

      delay(5000);

      // Diabetes
      Serial.println("Digite a hora da medição no formato 00:00:00 \n");
      horario = Serial.readStringUntil('\n');
      
      Serial.print("Horário recebido: " + horario);

      delay(1000);

      if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(1000);
        return;
      }

      ei_impulse_result_t result = { 0 };

      // the features are stored into flash, and we don't want to load everything into RAM
      signal_t features_signal;
      features_signal.total_length = sizeof(features) / sizeof(features[0]);
      features_signal.get_data = &raw_feature_get_data;

      // invoke the impulse
      EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
      if (res != EI_IMPULSE_OK) {
          ei_printf("ERR: Failed to run classifier (%d)\n", res);
          return;
      }

      // print inference return code
      ei_printf("\nrun_classifier returned: %d\r\n", res);
      print_inference_result(result);

      delay(5000);
    }
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);

    // Print the prediction results (object detection)
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

}

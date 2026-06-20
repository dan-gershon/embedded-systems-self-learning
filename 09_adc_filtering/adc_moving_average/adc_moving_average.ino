#define ADC_PIN 34

const int NUM_SAMPLES = 20;

int samples[NUM_SAMPLES];
int sampleIndex = 0;
long sum = 0;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_SAMPLES; i++) {
    samples[i] = 0;
  }
}

void loop() {
  int raw = analogRead(ADC_PIN);

  sum -= samples[sampleIndex];
  samples[sampleIndex] = raw;
  sum += samples[sampleIndex];

  sampleIndex++;

  if (sampleIndex >= NUM_SAMPLES) {
    sampleIndex = 0;
  }

  float filtered = (float)sum / NUM_SAMPLES;

  Serial.print("raw:");
  Serial.print(raw);
  Serial.print("\t");

  Serial.print("filtered:");
  Serial.println(filtered);

  delay(5);
}

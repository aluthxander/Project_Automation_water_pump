#define BLYNK_TEMPLATE_ID "TMPLmIB-1v2m"
#define BLYNK_TEMPLATE_NAME "Hafidz"
#define BLYNK_AUTH_TOKEN "cnAyC4GzGh2mJFii2_SXYymeoRrdF_h7"


#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "lutfan";
char pass[] = "dasarbangsat";
int value, counter1 = 0;
const int virtualPin = V6;

BlynkTimer timer;
WidgetLED pompaAir(V0);

BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

//BLYNK_WRITE(V6)
//{
//  value = param.asInt(); // Get value as integer
//}

void myTimerEvent()
{

  if (counter1 == 15) {
    pompaAir.on();
    Serial.println("Pompa Hidup");
    delay(1000);
    while (counter1 > 0) {
      counter1--;

      Serial.println(value);
      Serial.println(counter1);
      delay(1000);
    }
    Serial.println("Pompa Mati");
    delay(1000);
  }
  Serial.println(value);
  counter1++;
}

void setup() {
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(500L, myTimerEvent);
  // Menetapkan fungsi penanganan perubahan nilai input
  Blynk.onVirtualWrite(virtualPin, [](BlynkParam param) {
    int controlValue = param.asInt();
    if (controlValue == 1) {
      // Mengatur perulangan berjalan
      isLooping = true;
    } else {
      // Mengatur perulangan berhenti
      isLooping = false;
    }
  });
}

void loop() {
  Blynk.run();
  timer.run();
}

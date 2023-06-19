#define BLYNK_TEMPLATE_ID "TMPL6eDkZJCT-"
#define BLYNK_TEMPLATE_NAME "Pengisian air otomatis"
#define BLYNK_AUTH_TOKEN "dDAvTm1Mag7VTWO83fI6zxna67kPvuog"
#define Trig 33
#define Echo 25
#define Trig2 2
#define Echo2 4
#define Pompa 27 // pin relay untuk pompa
#define saklar 18 // pin untuk mengganti bentuk tandon
#define flowSensorPin 14  // Pin data sensor aliran air terhubung ke pin digital 14
#define calibrationFactor 4.5  // Faktor kalibrasi yang telah diukur
#define sisaKosong 0.3 // sisa air 30%

//ukuran tandon tabung
#define tinggiTandon 35
#define jari2 14.25

//ukuran tandon balok
#define tinggiBalok 27
#define lebarBalok 31
#define panjangBalok 48

// mendeklarasikan variabel-variabel
float distance, duration, tinggiAir, volume , distance2, duration2, tinggiAir2, volume2;
float sisaAir = tinggiTandon * 0.5;

bool nilai;//variable untuk mengubah bentuk tandon
int value;//variabel untuk mematikan pompa dari smartphone


// Deklarasi variabel untuk pengukuran aliran air
volatile int pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "HERMANTO 2.";
char pass[] = "tidurnyaman2";

BlynkTimer timer;
WidgetLED pompaAir(V0);


// untuk mengetahui ESP32 sudah terkoneksi dengan blynk IoT atau tidak
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

BLYNK_WRITE(V6)
{
  value = param.asInt(); // Get value as integer
}

void myTimerEvent()
{
  nilai = digitalRead(saklar);// membaca nilai saklar untuk menentukan bentuk tandon
  if (nilai == LOW) {//jika saklar bernilai LOW maka bentuk tandon tabung
    //    memanggil fungsi ultrasonic
    sensorUltrasonic1(nilai);
    sensorUltrasonic2(nilai);
    Serial.println("Tabung");
    printData(tinggiAir, tinggiAir2);

    if ((tinggiAir <= sisaAir) && (tinggiAir2 >= (tinggiTandon * 0.3)) && (value == 0)) {
      digitalWrite(Pompa, LOW);
      pompaAir.on();
      delay(3000);
      while ((tinggiAir <= (tinggiBalok * 0.9)) && (tinggiAir2 >= (tinggiBalok * 0.3))) {
        sensorUltrasonic1(nilai);
        sensorUltrasonic2(nilai);

        Serial.println("Tabung");
        printData(tinggiAir, tinggiAir2);

        sensorWaterFlow();

        Serial.println("Tabung");
        //        mengirimkan nilai-nilai sensor ke Blynk IoT
        Blynk.virtualWrite(V1, tinggiAir);
        Blynk.virtualWrite(V2, volume);
        Blynk.virtualWrite(V3, flowRate);
        Blynk.virtualWrite(V4, flowMilliLitres);
        Blynk.virtualWrite(V5, totalMilliLitres);
      }
      digitalWrite(Pompa, HIGH);
      pompaAir.off();
      printVolumeTotal();

    }

    Blynk.virtualWrite(V1, tinggiAir);
    Blynk.virtualWrite(V2, volume);
  } else {//jika saklar HIGH maka tandon bentuk Balok
    //    memanggil fungsi untuk sensor ultrasonic 1
    sensorUltrasonic1(nilai);
    sensorUltrasonic2(nilai);
    Serial.println("Balok");
    printData(tinggiAir, tinggiAir2);

    if ((tinggiAir <= sisaAir) && (tinggiAir2 >= (tinggiBalok * 0.3)) && (value == 0)) {
      digitalWrite(Pompa, LOW);
      pompaAir.on();
      delay(3000);

      while ((tinggiAir <= (tinggiBalok * 0.9)) && (tinggiAir2 >= (tinggiBalok * 0.3))) {
        //    memanggil fungsi untuk sensor ultrasonic 1
        sensorUltrasonic1(nilai);
        sensorUltrasonic2(nilai);
        printData(tinggiAir, tinggiAir2);
//        memanggil fungsi waterflow
        sensorWaterFlow();
        Serial.println("Balok");
        //        mengirimkan nilai-nilai sensor ke Blynk IoT
        Blynk.virtualWrite(V1, tinggiAir);
        Blynk.virtualWrite(V2, volume);
        Blynk.virtualWrite(V3, flowRate);
        Blynk.virtualWrite(V4, flowMilliLitres);
        Blynk.virtualWrite(V5, totalMilliLitres);
      }

      digitalWrite(Pompa, HIGH);
      pompaAir.off();
      printVolumeTotal();

    }

    //        mengirimkan nilai-nilai sensor ke Blynk IoT
    Blynk.virtualWrite(V1, tinggiAir);
    Blynk.virtualWrite(V2, volume);
  }

}

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(Trig, OUTPUT); // Sets the trigPin as an Output
  pinMode(Echo, INPUT); // Sets the echoPin as an Input
  pinMode(Pompa, OUTPUT);
  pinMode(saklar, INPUT_PULLUP);
  pinMode(flowSensorPin, INPUT);
  digitalWrite(flowSensorPin, HIGH);

  // Reset semua variabel
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;

  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();


  Blynk.begin(auth, ssid, pass);

  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  digitalWrite(Pompa, HIGH);

  // Setup a function to be called every second
  timer.setInterval(500L, myTimerEvent);

  // Set timer untuk menghitung pulse dari sensor aliran air
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flowSensorISR, FALLING);
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}


//fungsi untuk menghitung jarak dengan menggunakan sensor ultrasonic
void sensorUltrasonic1(bool bentuk) {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2;
  if (bentuk == LOW) {//jika bentuk LOW maka rumus yang digunakan berupa rumus tabung

    tinggiAir = tinggiTandon - distance;
    volume = 3.14 * (jari2 * jari2) * tinggiAir;
  } else {// jika tidak maka rumus yang digunakan berupa rumus balok

    tinggiAir = tinggiBalok - duration;
    volume =  lebarBalok * panjangBalok * tinggiAir;
  }
}

void sensorUltrasonic2(bool bentuk) {
  digitalWrite(Trig2, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(Trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig2, LOW);
  duration2 = pulseIn(Echo2, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance2 = duration2 * 0.034 / 2;
  if (bentuk == LOW) {//jika bentuk LOW maka rumus yang digunakan berupa rumus tabung

    tinggiAir2 = tinggiTandon - distance2;
    volume2 = 3.14 * (jari2 * jari2) * tinggiAir2;
  } else {// jika tidak maka rumus yang digunakan berupa rumus balok

    tinggiAir2 = tinggiBalok - duration2;
    volume2 =  lebarBalok * panjangBalok * tinggiAir2;
  }
}

void sensorWaterFlow() {
  // Hitung waktu yang telah berlalu
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - oldTime;

  // Jika waktu yang telah berlalu melebihi 1 detik
  if (elapsedTime >= 1000) {
    // Hitung aliran air dalam milliliter per detik
    flowRate = ((1000.0 / (float)elapsedTime) * (float)pulseCount) / calibrationFactor;

    // Reset pulse counter
    pulseCount = 0;

    // Hitung total aliran air dalam milliliter
    flowMilliLitres = (unsigned int)(flowRate * (float)1000);

    // Tambahkan total aliran air
    totalMilliLitres += flowMilliLitres;

    // Tampilkan hasil pengukuran
    Serial.print("Flow rate: ");
    Serial.print(flowRate);
    Serial.print(" L/min");

    Serial.print("\tFlow milliLitres: ");
    Serial.print(flowMilliLitres);

    Serial.print("\tTotal milliLitres: ");
    Serial.println(totalMilliLitres);

    // Update waktu terakhir
    oldTime = currentTime;
  }
}

//fungsi untuk menampilkan jarak dan volume ke LCD dan monitor
void printData(int level_1, int level_2) {
  lcd.clear();
  Serial.print(level_1);
  Serial.println(" cm");
  Serial.print(level_1);
  Serial.println(" cm");

  lcd.setCursor(0, 0);
  lcd.print("Tandon Atas: ");
  lcd.print(level_1);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("Tandon Bawah: ");
  lcd.print(level_2);
  lcd.print(" cm");
  delay(100);
}

void printVolumeTotal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volume Total: ");
  lcd.setCursor(0, 1);
  lcd.print(float(totalMilliLitres / 1000));
  lcd.print(" L");
  delay(3000);
  lcd.clear();
  totalMilliLitres = 0;
}



//fungsi untuk membaca input sensor waterflow
ICACHE_RAM_ATTR void flowSensorISR() {
  // Increment pulse counter
  pulseCount++;
}

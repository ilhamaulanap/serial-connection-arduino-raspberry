/********************************
  Sistem Kontrol Suhu dengan 
  Logika Fuzzy
  Ilham Maulana Putra
  Fisika - Universitas Airlangga
  2021
*********************************/

// libraries
#include <DHT.h>

// Pin Definitions
#define DHT1_PIN 24 // sensor suhu atas
#define DHT2_PIN 22 // sensor suhu bawah
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PWMFan_PIN 4 // PWM Kipas 
#define Fan_1_PIN 5 // Kipas peltier 1
#define Fan_2_PIN 6 // Kipas Peltier 2
#define Tec_1_PIN 7 // peltier 1
#define Tec_2_PIN 8 // peltier 2

DHT dht1(DHT1_PIN, DHTTYPE); 
DHT dht2(DHT2_PIN, DHTTYPE);

//Variables
int   inputsuhu, pwmValueK, pwmValueP, suhuSetPoint;
float suhuatas,suhubawah, deltasuhu, deltainputsuhu, //pembacaan sensor
      kSangatKecil, kKecil, kNormal, kBesar, kSangatBesar, //anggota fuzzy untuk deltasuhu (Kontrol Kipas)
      kSangatLambat, kLambat, kSedang, kCepat, kSangatCepat, //anggota fuzzy untuk output kecepatan kipas (Kontrol Kipas)
      pSangatDingin, pDingin, pNormal, pPanas, pSangatPanas, //anggota fuzzy untuk deltasuhuinput (Kontrol Peltier)
      pSangatRendah, pRendah, pSedang, pTinggi, pSangatTinggi, //anggota fuzzy untuk keluaran tegangan peltier (kontrol peltier)
      ruleK1, ruleK2a, ruleK2b, ruleK3a, ruleK3b, ruleK4a, ruleK4b, ruleK5, //fuzzy rule untuk pengaturan kipas
      ruleP1, ruleP2a, ruleP2b, ruleP3a, ruleP3b, ruleP4a, ruleP4b, ruleP5, //fuzzy rule untuk pengaturan peltier
      pwm, pwmP, //untuk fuzzifikasi
      outputPeltier, outputKipas; //untuk aturan output
String outputFuzzyK, outputFuzzyP;

void setup()
{
  Serial.begin(9600);
  dht1.begin();
  dht2.begin();
//    pinMode(DHT1_PIN, INPUT);
//    pinMode(DHT2_PIN, INPUT);
    pinMode(PWMFan_PIN, OUTPUT);
    pinMode(Fan_1_PIN, OUTPUT);
    pinMode(Fan_2_PIN, OUTPUT);
    pinMode(Tec_1_PIN, OUTPUT);
    pinMode(Tec_2_PIN, OUTPUT);
}

void loop() {
  inputSuhu();    // fungsi membaca input suhu dari database
  bacaSensor_suhu(); // fungsi membaca suhu dari sensor
  fuzzifikasi();   // fungsi untuk fuzzifikasi
  fuzzy_rule();    // fungsi fuzzy rule
  defuzzifikasi(); // fungsi untuk defuzzifikasi
  output();        // fungsi untuk output (nilai Pwm)
  aturanOutputFuzzy(); // fungsi untuk definisi output nilai fuzzy
  kirimdata();    // fungsi untuk kirim data serial ke raspberry
}

void inputSuhu() 
{
  if ( Serial.available() > 0 ) {
     //Membaca input suhu yang dinginkan dari port serial
    inputsuhu = Serial.parseInt();
    suhuSetPoint = int(inputsuhu);
}
}


void bacaSensor_suhu()
{
    suhuatas = dht1.readTemperature(); //baca sensor suhu atas
    suhubawah = dht2.readTemperature(); //baca sensor suhu bawah
    deltasuhu = abs(suhubawah-suhuatas); // mencari nilai deltasuhu atas dan bawah
    deltainputsuhu = abs(suhuatas-suhuSetPoint); //mencari nilai deltasuhu input dan atas
    //Cek jika pembacaan suhu gagal, dan coba pembacaan ulang" 
}

//Untuk anggota deltasuhu Sangat Kecil
unsigned char deltasuhuSK(){ 
  if (deltasuhu <= 30){kSangatKecil =1;}
  else if (deltasuhu >=30 && deltasuhu <= 35){kSangatKecil=(35-deltasuhu)/5;}
  else if (deltasuhu >= 35){kSangatKecil = 0;}
  return kSangatKecil;
}

//Untuk anggota deltasuhu Kecil
unsigned char deltasuhuKC(){ 
  if (deltasuhu <= 30){kKecil =0;}
  else if (deltasuhu >=30 && deltasuhu <= 35){kKecil=(deltasuhu-30)/5;}
  else if (deltasuhu >=35 && deltasuhu <= 40){kKecil=(40-deltasuhu)/5;}
  else if (deltasuhu >= 40){kKecil = 0;}
  return kKecil;
}

//Untuk anggota deltasuhu Normal
unsigned char deltasuhuNM(){ 
  if (deltasuhu <= 35){kNormal =0;}
  else if (deltasuhu >=35 && deltasuhu <= 40){kNormal=(deltasuhu-35)/5;}
  else if (deltasuhu >=40 && deltasuhu <= 45){kNormal=(45-deltasuhu)/5;}
  else if (deltasuhu >= 45){kNormal = 0;}
  return kNormal;
}

//Untuk anggota deltasuhu Besar
unsigned char deltasuhuBS(){ 
  if (deltasuhu <= 40){kBesar =0;}
  else if (deltasuhu >=40 && deltasuhu <= 45){kBesar=(deltasuhu-40)/5;}
  else if (deltasuhu >=45 && deltasuhu <= 50){kBesar=(50-deltasuhu)/5;}
  else if (deltasuhu >= 50){kBesar = 0;}
  return kBesar;
}

//Untuk anggota deltasuhu Sangat Besar
unsigned char deltasuhuSB(){ 
  if (deltasuhu <= 45){kSangatBesar =0;}
  else if (deltasuhu >=45 && deltasuhu <= 50){kSangatBesar=(deltasuhu-45)/5;}
  else if (deltasuhu >= 50){kSangatBesar = 1;}
  return kSangatBesar;
}

//Untuk anggota pwm kipas Sangat Lambat
unsigned char pwmSL(){ 
  if (pwm <= 20){kSangatLambat =1;}
  else if (pwm >=20 && pwm <= 40){kSangatLambat=(40-pwm)/20;}
  else if (pwm >= 40){kSangatLambat = 0;}
  return kSangatLambat;
}

//Untuk anggota pwm kipas Lambat
unsigned char pwmLB(){ 
  if (pwm <= 20){kLambat =0;}
  else if (pwm >=20 && pwm <= 40){kLambat=(pwm-20)/20;}
  else if (pwm >=40 && pwm <= 60){kLambat=(60-pwm)/20;}
  else if (pwm >= 60){kLambat = 0;}
  return kLambat;
}

//Untuk anggota pwm kipas Sedang
unsigned char pwmSD(){ 
  if (pwm <= 40){kSedang =0;}
  else if (pwm >=40 && pwm <= 60){kSedang=(pwm-40)/20;}
  else if (pwm >=60 && pwm <= 80){kSedang=(80-pwm)/20;}
  else if (pwm >= 80){kSedang = 0;}
  return kSedang;
}

//Untuk anggota pwm kipas Cepat
unsigned char pwmCP(){ 
  if (pwm <= 60){kCepat =0;}
  else if (pwm >=60 && pwm <= 80){kCepat=(pwm-60)/20;}
  else if (pwm >=80 && pwm <= 100){kCepat=(100-pwm)/20;}
  else if (pwm >= 100){kCepat = 0;}
  return kCepat;
}

//Untuk anggota pwm kipas Sangat Cepat
unsigned char pwmSC(){ 
  if (pwm <= 80){kSangatCepat =0;}
  else if (pwm >=80 && pwm <= 100){kSangatCepat=(pwm-80)/20;}
  else if (pwm >= 100){kSangatCepat = 1;}
  return kSangatCepat;
}

//untuk anggota deltainputsuhu Sangat Dingin
unsigned char deltainputsuhuSD(){ 
  if (deltainputsuhu <= 0){pSangatDingin =1;}
  else if (deltainputsuhu >=0 && deltainputsuhu <= 2){pSangatDingin=(2-deltainputsuhu)/2;}
  else if (deltainputsuhu >= 2){pSangatDingin = 0;}
  return pSangatDingin;
}

//untuk anggota deltainputsuhu Dingin
unsigned char deltainputsuhuDG(){ 
  if (deltainputsuhu <= 2){pDingin =0;}
  else if (deltainputsuhu >=2 && deltainputsuhu <= 4){pDingin=(deltainputsuhu-2)/2;}
  else if (deltainputsuhu >=4 && deltainputsuhu <= 6){pDingin=(6-deltainputsuhu)/2;}
  else if (deltainputsuhu >=6){pDingin = 0;}
  return pDingin;
}

//untuk anggota deltainputsuhu Normal
unsigned char deltainputsuhuNM(){ 
  if (deltainputsuhu <= 4){pNormal =0;}
  else if (deltainputsuhu >=4 && deltainputsuhu <= 6){pNormal=(deltainputsuhu-4)/2;}
  else if (deltainputsuhu >=6 && deltainputsuhu <= 8){pNormal=(8-deltainputsuhu)/2;}
  else if (deltainputsuhu >=6){pNormal = 0;}
  return pNormal;
}

//untuk anggota deltainputsuhu Panas
unsigned char deltainputsuhuPN(){ 
  if (deltainputsuhu <= 6){pPanas =0;}
  else if (deltainputsuhu >=6 && deltainputsuhu <= 8){pPanas=(deltainputsuhu-6)/2;}
  else if (deltainputsuhu >=8 && deltainputsuhu <= 10){pPanas=(10-deltainputsuhu)/2;}
  else if (deltainputsuhu >=10){pPanas = 0;}
  return pPanas;
}

//untuk anggota deltainputsuhu SangatPanas
unsigned char deltainputsuhuSP(){ 
  if (deltainputsuhu <= 8){pSangatPanas =0;}
  else if (deltainputsuhu >=8 && deltainputsuhu <= 10){pSangatPanas=(deltainputsuhu-8)/2;}
  else if (deltainputsuhu >=10){pSangatPanas = 1;}
  return pSangatPanas;
}


//Untuk anggota pwm peltier Sangat rendah
unsigned char pwmPSR(){ 
  if (pwmP <= 20){pSangatRendah =1;}
  else if (pwmP >=20 && pwmP <= 40){pSangatRendah=(40-pwmP)/20;}
  else if (pwmP >= 40){pSangatRendah = 0;}
  return pSangatRendah;
}

//Untuk anggota pwm peltier rendah
unsigned char pwmPRD(){ 
  if (pwmP <= 20){pRendah =0;}
  else if (pwmP >=20 && pwmP <= 40){pRendah=(pwmP-20)/20;}
  else if (pwmP >=40 && pwmP <= 60){pRendah=(60-pwmP)/20;}
  else if (pwmP >= 60){pRendah = 0;}
  return pRendah;
}

//Untuk anggota pwm peltier Sedang
unsigned char pwmPSD(){ 
  if (pwmP <= 40){pSedang =0;}
  else if (pwmP >=40 && pwmP <= 60){pSedang=(pwmP-40)/20;}
  else if (pwmP >=60 && pwmP <= 80){pSedang=(80-pwmP)/20;}
  else if (pwmP >= 80){pSedang = 0;}
  return pSedang;
}

//Untuk anggota pwm peltier Tinggi
unsigned char pwmPTG(){ 
  if (pwmP <= 60){pTinggi =0;}
  else if (pwmP >=60 && pwmP <= 80){pTinggi=(pwmP-60)/20;}
  else if (pwmP >=80 && pwmP <= 100){pTinggi=(100-pwmP)/20;}
  else if (pwmP >= 100){pTinggi = 0;}
  return pTinggi;
}

//Untuk anggota pwm peltier Sangat Tinggi
unsigned char pwmPST(){ 
  if (pwmP <= 80){pSangatTinggi =0;}
  else if (pwmP >=80 && pwmP <= 100){pSangatTinggi=(pwmP-80)/20;}
  else if (pwmP >= 100){pSangatTinggi = 1;}
  return pSangatTinggi;
}

//fuzzifikasi
void fuzzifikasi() {
      deltasuhuSK();
      deltasuhuKC();
      deltasuhuNM();
      deltasuhuBS();
      deltasuhuSB();
      pwmSL();
      pwmLB();
      pwmSD();
      pwmCP();
      pwmSC();
      deltainputsuhuSD();
      deltainputsuhuDG();
      deltainputsuhuNM();
      deltainputsuhuPN();
      deltainputsuhuSP();
      pwmPSR();
      pwmPRD();
      pwmPSD();
      pwmPTG();
      pwmPST();
}

//Fuzzy rule
void fuzzy_rule(){
  //rule untuk pengaturan kecepatan kipas pwm
  ruleK1 = 80 + (kSangatKecil*20); //jika delta suhu sangat kecil, maka kipas sangat cepat
  ruleK2a = 60 + (kKecil*20);      //jika delta suhu kecil, maka kipas cepat
  ruleK2b = 100 - (kKecil*20);     //jika delta suhu kecil, maka kipas cepat
  ruleK3a = 40 + (kNormal*20);    //jika delta suhu Normal, maka kipas sedang
  ruleK3b = 80 - (kNormal*20);    //jika delta suhu Normal, maka kipas sedang
  ruleK4a = 20 + (kBesar*20);     //jika delta suhu Besar, maka kipas lambat
  ruleK4b = 60 - (kBesar*20);     //jika delta suhu Besar, maka kipas lambat
  ruleK5  = 40 - (kSangatBesar*20);  //jika delta suhu Sangat Besar, maka kipas sangat lambat
  //rule untuk pengaturan keluaran tegangan peltier
  ruleP1  = 40 - (pSangatDingin*20);  //jika delta input suhu sangat dingin, maka tengangan peltier sangat rendah 
  ruleP2a = 20 + (pDingin*20);        //jika delta input suhu Dingin, maka tegangan peltier rendah
  ruleP2b = 60 - (pDingin*20);        //jika delta input suhu Dingin, maka tegangan peltier rendah
  ruleP3a = 40 + (pNormal*20);        //jika delta input suhu Normal, maka tegangan peltier sedang
  ruleP3b = 80 - (pNormal*20);        //jika delta input suhu Normal, maka tegangan peltier sedang
  ruleP4a = 60 + (pPanas*20);        //jika delta input suhu Panas, maka tegangan peltier tinggi
  ruleP4b = 100 - (pPanas*20);        //jika delta input suhu Panas, maka tegangan peltier tinggi
  ruleP5 = 80 + (pSangatPanas*20);     //jika delta input suhu Sangat Panas, maka tegangan peltier Sangat tinggi
}

void defuzzifikasi(){
  //Defuzzifikasi untuk pwm Kipas
  outputKipas = ((ruleK1*kSangatKecil) + (ruleK2a*kKecil) + (ruleK2b*kKecil) + (ruleK3a*kNormal) + (ruleK3b*kNormal) + (ruleK4a*kBesar) + (ruleK4b*kBesar) + (ruleK5*kSangatBesar))/(kSangatKecil+kKecil+kKecil+kNormal+kNormal+kBesar+kBesar+kSangatBesar);
  //Defuzzifikasi untuk pwm peltier
  outputPeltier = ((ruleP1*pSangatDingin) + (ruleP2a*pDingin) + (ruleP2b*pDingin) + (ruleP3a*pNormal) + (ruleP3b*pNormal) + (ruleP4a*pPanas) + (ruleP4b*pPanas) + (ruleP5*pSangatPanas))/(pSangatDingin+pDingin+pDingin+pNormal+pNormal+pPanas+pPanas+pSangatPanas);
}
void output(){
  pwmValueK = map(outputKipas, 0, 100, 0, 255); //konversi persentase ke nilai PWM untuk Kipas
  pwmValueP = map(outputPeltier, 0, 100, 0 ,255); //konversi persentase ke nilai PWM untuk Peltier

  //Output dengan masing-masing nilai PWM
  analogWrite(PWMFan_PIN, pwmValueK);
  analogWrite(Fan_1_PIN, pwmValueK);
  analogWrite(Fan_2_PIN, pwmValueK);
  analogWrite(Tec_1_PIN, pwmValueP);
  analogWrite(Tec_2_PIN, pwmValueP);
  
}
void aturanOutputFuzzy () {

  if (outputKipas <= 20) outputFuzzyK = "Sangat lambat";
  else if (outputKipas >=20 && outputKipas <= 40) outputFuzzyK = "lambat";
  else if (outputKipas >=40 && outputKipas <= 60) outputFuzzyK = "sedang";
  else if (outputKipas >=60 && outputKipas <= 80) outputFuzzyK = "cepat";
  else if (outputKipas >=80 && outputKipas <= 100) outputFuzzyK = "Sangat cepat";

  if (outputPeltier <= 20) outputFuzzyP = "Sangat rendah";
  else if (outputPeltier >=20 && outputPeltier <= 40) outputFuzzyP = "rendah";
  else if (outputPeltier >=40 && outputPeltier <= 60) outputFuzzyP = "normal";
  else if (outputPeltier >=60 && outputPeltier <= 80) outputFuzzyP = "tinggi";
  else if (outputPeltier >=80 && outputPeltier <= 100) outputFuzzyP = "Sangat tinggi";
}
void kirimdata() {
  Serial.print(String(suhuSetPoint));
  Serial.print(',');
  Serial.print(String(suhuatas));
  Serial.print(',');
  Serial.print(String(deltainputsuhu));
  Serial.print(',');
  Serial.print(String(deltasuhu));
  Serial.print(',');
  Serial.print(String(outputKipas));
  Serial.print(',');
  Serial.print(String(outputPeltier));
  Serial.print(',');
  Serial.print(String(pwmValueK));
  Serial.print(',');
  Serial.print(String(pwmValueP));
  Serial.print(',');
  Serial.print(String(outputFuzzyK));
  Serial.print(',');
  Serial.print(String(outputFuzzyP));
  Serial.println();
  delay(5000);
}

#include "var.h"
#include "PrayerTimes.h"
#include "kota.h"
#include <EEPROM.h>
#include <Wire.h>
#define SERIAL_RX_BUFFER_SIZE 512
//====================================
#define bluetooth 9600              //jika hc 06 atau hc05 ganti dengan 38400
#define nama_bluetooth "JWS DacxtroniC"
double times[sizeof(TimeName)/sizeof(char*)];
MyObject parameter; //Variable to store custom object read from EEPROM.
byte refres_jadwal;
long timer;
byte power=1;
volatile int alamat_eprom=0;



void setup() {
    set_calc_method(ISNA);
    set_asr_method(Shafii);
    set_high_lats_adjust_method(AngleBased);
    set_fajr_angle(20);
    set_isha_angle(18);
    set_buzer();
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }


}
  unsigned char temp_min;
  unsigned char temp_jam;
  unsigned char temp_hri;
  unsigned char rename_bt=0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void loop() {  
  baca_jadwal(185);
  delay(10000);
}


void baca_jadwal(int daerah ){
    float lt =  pgm_read_float(lintang+daerah);
    float bj =  pgm_read_float(bujur+daerah);
    unsigned char wkt = pgm_read_byte_near(gmt+daerah);
    int hours, minutes;
    if(daerah==0){
        wkt = parameter.set_kota_gmt ;
        lt = parameter.set_kota_lnt ;
        bj = parameter.set_kota_bjr ;
    }
    get_prayer_times(2019, 9 , 18, lt, bj, wkt, times);
    get_float_time_parts(times[0], hours, minutes);
    data[waktu_subuh]=(hours*60)+minutes;
    get_float_time_parts(times[2], hours, minutes);
    data[waktu_duhur]=(hours*60)+minutes;
    get_float_time_parts(times[3], hours, minutes);
    data[waktu_ashar]=(hours*60)+minutes;
    get_float_time_parts(times[5], hours, minutes);
    data[waktu_magrib]=(hours*60)+minutes;
    get_float_time_parts(times[6], hours, minutes);



    
    data[waktu_isya]=(hours*60)+minutes;
    data[waktu_subuh]=data[waktu_subuh]+2;
    data[waktu_duhur]=data[waktu_duhur]+2;
    data[waktu_ashar]=data[waktu_ashar]+2;
    data[waktu_magrib]=data[waktu_magrib]+2;
    data[waktu_isya]=data[waktu_isya]+2;

    data[waktu_subuh]=data[waktu_subuh]+parameter.tambah_kurang_subuh ;
    data[waktu_duhur]=data[waktu_duhur]+parameter.tambah_kurang_duhur ;
    data[waktu_ashar]=data[waktu_ashar]+parameter.tambah_kurang_ashar ; 
    data[waktu_magrib]=data[waktu_magrib]+parameter.tambah_kurang_maghrib ; 
    data[waktu_isya]=data[waktu_isya]+parameter.tambah_kurang_isya ;
    data[waktu_imsya]=data[waktu_subuh]-10;


    
  Serial.print("waktu_subuh = ");
  Serial.print(data[waktu_subuh]/60);
  Serial.print(":");
  Serial.println(data[waktu_subuh]%60);
  
  Serial.print("waktu_duhur = ");
  Serial.print(data[waktu_duhur]/60);
  Serial.print(":");
  Serial.println(data[waktu_duhur]%60);
  
  Serial.print("waktu_ashar = ");
  Serial.print(data[waktu_ashar]/60);
  Serial.print(":");
  Serial.println(data[waktu_ashar]%60);
  
  Serial.print("waktu_magrib = ");
  Serial.print(data[waktu_magrib]/60);
  Serial.print(":");
  Serial.println(data[waktu_magrib]%60);
  
  Serial.print("waktu_isya = ");
  Serial.print(data[waktu_isya]/60);
  Serial.print(":");
  Serial.println(data[waktu_isya]%60);
  
  Serial.print("waktu_imsya = ");
  Serial.print(data[waktu_imsya]/60);
  Serial.print(":");
  Serial.println(data[waktu_imsya]%60);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
}

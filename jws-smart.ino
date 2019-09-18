#include "var.h"
#include "PrayerTimes.h"
#include "kota.h"
#include <EEPROM.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
#define SERIAL_RX_BUFFER_SIZE 512
//====================================
#define bluetooth 9600              //jika hc 06 atau hc05 ganti dengan 38400
#define nama_bluetooth "JWS DacxtroniC"

//#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include "DMD.h"        //
#include "Arial_black_16.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
//==================================
DFRobotDFPlayerMini myDFPlayer;
double times[sizeof(TimeName)/sizeof(char*)];
MyObject parameter; //Variable to store custom object read from EEPROM.
byte refres_jadwal;
long timer;
byte power=1;
volatile int alamat_eprom=0;
void setup() {
  timer=millis();
    pinMode(strobePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(buzer, OUTPUT);
    pinMode( tombol_up, INPUT_PULLUP);
    pinMode( tombol_menu, INPUT_PULLUP);
    pinMode( tombol_down, INPUT_PULLUP);
    Serial1.begin(bluetooth);
    Serial.begin(9600);
    Wire.begin();
    myDFPlayer.begin(Serial);
    Serial1.setTimeout(80);
    while (!Serial1);
    set_calc_method(ISNA);
    set_asr_method(Shafii);
    set_high_lats_adjust_method(AngleBased);
    set_fajr_angle(20);
    set_isha_angle(18);
    set_buzer();
    dmd.clearScreen( true );
    dmd.selectFont(Arial_Black_16);
    delay(250);
    clear_buzer();
    EEPROM.get(0, parameter );
    baca_rtc();
    baca_jadwal(parameter.kota );
    myDFPlayer.volume(25);  //Set volume . From 0 to 30
    myDFPlayer.play(1);  //Play the first mp3
    display_eprom(text_run);
    refres(all);
}
  unsigned char temp_min;
  unsigned char temp_jam;
  unsigned char temp_hri;
  unsigned char rename_bt=0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void loop() {  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis  >= setengah_detik) {
    previousMillis = currentMillis;
    if (sqw == LOW) sqw = HIGH;
    else sqw = LOW;
  }
  if (currentMillis - previousMillis_2  >= lima_detik) {
    if (rename_bt==0){
      Serial1.print("AT+NAME");
      Serial1.println(nama_bluetooth);
      rename_bt=1;
    }
    previousMillis_2 = currentMillis;
    if (day == LOW) day = HIGH;
    else day = LOW;
    refres(all);
  }
  if (currentMillis - previousMillis_3  >= lima_belas_detik) {
    previousMillis_3 = currentMillis;
    baca_rtc();
    if(data[menit]!=temp_min){
        alarm();
        int power_time =data[menit]+(data[jam]*60);
        if(power_time>=parameter.jam_kecerahan_1&&power_time<=parameter.jam_kecerahan_4)power=1;
        else power=0;
        if(parameter.jam_kecerahan_4 == parameter.jam_kecerahan_1){
          power=1;
        }
    }


    temp_min=data[menit];
    if(data[jam]!=temp_jam) baca_jadwal(parameter.kota);
    temp_jam=data[jam];
    if(data[hari]!=temp_hri)EEPROM.get(0, parameter );
    temp_hri=data[hari];
  }
  if( power){
      if(day==HIGH)tampil_hari(data[hari]);    
      tampil_segmen();
      tampil_text(text_run);
  }
  else for(unsigned char a=0;a<36;a++) shiftOut_cepat(255);
  tombol();
}
//=========================================================================================================================
int dayofweek(unsigned char d, unsigned char m, int y) { 
  const unsigned char t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 }; 
  y -= m < 3; 
  unsigned char h =( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7; 
  if (h==0) h=7;
  return h;
} 

unsigned char makedec(unsigned char bcd){
 return( (bcd/16*10) + (bcd%16) );
}

unsigned char makebcd(unsigned char dec){
return( (dec/10*16) + (dec%10) );
}

void tulis_rtc(void ) {
    unsigned char _hri = dayofweek(data[tanggal], data[bulan], data[tahun]) ;
    Wire.beginTransmission(104);
    Wire.write(0); 
    Wire.write( makebcd(data[detik] ));
    Wire.write( makebcd(data[menit] ));
    Wire.write( makebcd(data[jam] ));
    Wire.write( makebcd(_hri));
    Wire.write( makebcd(data[tanggal] ));
    Wire.write( makebcd(data[bulan] ));
    Wire.write( makebcd(data[tahun]-2000) );
    Wire.endTransmission();
}
void baca_rtc( void ) {
    Wire.beginTransmission(104);
    Wire.write(0); // MSB
    Wire.endTransmission();
    if (Wire.requestFrom(104, 7) == 7) {
        data[detik] = makedec(Wire.read());
        data[menit] = makedec(Wire.read());
        data[jam] = makedec(Wire.read());
        data[hari] = makedec(Wire.read());
        data[tanggal] = makedec(Wire.read());
        data[bulan] = makedec(Wire.read());
        data[tahun] = makedec(Wire.read())+2000;
    }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void tunggu_menu(){
  set_buzer();
  delay(100);
  clear_buzer();
  while(digitalRead(tombol_menu)==LOW) {
  //tampil();
  delay(150);
  }
}
void tunggu_up(){
  int skip=0;
  set_buzer();
  delay(100);
  clear_buzer();
  while(digitalRead(tombol_up)==LOW) {
    tampil_segmen();
    tampil_text(text_run);
    skip++;
    if(skip==50)break;
  }
}
void tunggu_down(){
  int skip=0;
  set_buzer();
  delay(100);
  clear_buzer();
  while(digitalRead(tombol_down)==LOW) {
    tampil_segmen();
    tampil_text(text_run);
    skip++;
    if(skip==50)break;
  }
}
//seting(data,bawah,atas)
void set_waktu(byte a,int b,int c){
  unsigned char e=0;
  for(byte l=12;l<36;l++)segmen[l]=(data_jadwal[10]);
  while(digitalRead(tombol_menu)==1){  
  e++;
  if(e>100)e=0;
    if(digitalRead(tombol_up)==0){
      data[a]++;
      if(data[a]>c) data[a]=b;
      refres(jam);
      refres(tanggal);  
      tunggu_up();
      e=0;
    }
    if(digitalRead(tombol_down)==0){
      data[a]--;
      if(data[a]<b||data[a]==0xff) data[a]=c;
      refres(jam);
      refres(tanggal);
      tunggu_down();
      e=0;
    }
    refres(jam);
    refres(tanggal);
    if(e>50) {
      if(a== menit){
         segmen[2]=(data_kalender[10]);
         segmen[3]=(data_kalender[10]);
      }
      if(a== jam ){        
         segmen[0]=(data_jam[10]);
         segmen[1]=(data_jam[10]);
      }
      if(a== tanggal){
         segmen[4]=(data_kalender[10]);
         segmen[5]=(data_kalender[10]); 
      }
      if(a== bulan){
         segmen[7]=(data_kalender[10]);
         segmen[8]=(data_kalender[10]);
        
      }
      if(a== tahun ){
         segmen[10]=(data_kalender[10]);
         segmen[11]=(data_kalender[10]);
      }
    }
    tampil_segmen();
    tampil_text(text_run);
  }
  tunggu_menu();
}


//seting(data,bawah,atas)

//(nama,val,bawah,atas)
int set_parameter(byte _nama_set ,int _nilai,int _bawah, int _atas){
  unsigned char e=0;
  for(byte l=12;l<36;l++)segmen[l]=(data_jadwal[10]);
  
  tampil_hari(_nama_set);
  while(digitalRead(tombol_menu)==1){  
    e++;
    if(e>100)e=0;
    refres(jam);
        if(_nilai>99){
          data[menit]=_nilai%100;
          data[jam]=_nilai/100;
        }
        else {
          segmen[0]=(data_jam[10]);
          segmen[1]=(data_jam[10]);
          if(_nilai<0)segmen[1]=255-128;
          data[menit]=abs(_nilai);
        }
      if(digitalRead(tombol_up)==0){
        _nilai++;
        if(_nilai > _atas) _nilai= _bawah;
                refres(jam);
        if(_nilai>99){
          data[menit]=_nilai%100;
          data[jam]=_nilai/100;
        }
        else {
          segmen[0]=(data_jam[10]);
          segmen[1]=(data_jam[10]);
          if(_nilai<0)segmen[1]=255-128;
          data[menit]=abs(_nilai);
        }
        if(_nilai>99){
          data[menit]=_nilai%100;
          data[jam]=_nilai/100;
        }
        else {
          segmen[0]=(data_jam[10]);
          segmen[1]=(data_jam[10]);
          if(_nilai<0)segmen[1]=255-128;
          data[menit]=abs(_nilai);
        }
        tunggu_up();
        e=0;
      }
      if(digitalRead(tombol_down)==0){
        _nilai--;
        if(_nilai < _bawah) _nilai= _atas;
        refres(jam);
        if(_nilai>99){
          data[menit]=_nilai%100;
          data[jam]=_nilai/100;
        }
        else {
          segmen[0]=(data_jam[10]);
          segmen[1]=(data_jam[10]);
          if(_nilai<0)segmen[1]=255-128;
          data[menit]=abs(_nilai);
        }
        if(_nilai>99){
          data[menit]=_nilai%100;
          data[jam]=_nilai/100;
        }
        else {
          segmen[0]=(data_jam[10]);
          segmen[1]=(data_jam[10]);
          if(_nilai<0)segmen[1]=255-128;
          data[menit]=abs(_nilai);
        }
        tunggu_down();
        e=0;
      }
      if(e<50) tampil_segmen();
      tampil_text(text_run);
  }
  tunggu_menu();
  return _nilai;
}

void tombol(){
unsigned char a;
if(digitalRead(tombol_menu)==LOW){
  tunggu_menu();
  set_waktu(2,0,23);   //jam
  set_waktu(1,0,59);   //menit
  set_waktu(4,1,31);   //tanggal
  set_waktu(5,1,12);   //bulan
  set_waktu(6,2000,2099);   //tahun
  tulis_rtc();
  baca_jadwal(parameter.kota);
  refres(all);
  }

if(digitalRead(tombol_up)==0){
  tunggu_up();
  parameter.timer_adzan_subuh =3;
  parameter.timer_adzan_subuh =3;
  parameter.timer_adzan_duhur =3;
  parameter.timer_adzan_ashar =3;
  parameter.timer_adzan_maghrib =3;
  parameter.timer_adzan_isya =3;
  parameter.timer_adzan_jumat=3 ;
  parameter.iqomah_subuh=set_parameter(subuh ,parameter.iqomah_subuh,0,99);
  parameter.iqomah_duhur=set_parameter(duhur ,parameter.iqomah_duhur,0,99);
  parameter.iqomah_ashar=set_parameter(ashar ,parameter.iqomah_ashar,0,99);
  parameter.iqomah_maghrib=set_parameter(magrib ,parameter.iqomah_maghrib,0,99);
  parameter.iqomah_isya=set_parameter(isya ,parameter.iqomah_isya,0,99);
  parameter.iqomah_jumat=set_parameter(kutbah ,parameter.iqomah_jumat,0,99);
  
  parameter.lama_sholat_subuh=set_parameter(stanbay ,parameter.lama_sholat_subuh,0,99);
  
  parameter.lama_sholat_duhur=parameter.lama_sholat_subuh;
  parameter.lama_sholat_ashar=parameter.lama_sholat_subuh  ;
  parameter.lama_sholat_maghrib =parameter.lama_sholat_subuh;
  parameter.lama_sholat_isya =parameter.lama_sholat_subuh;
  parameter.lama_sholat_jumat =parameter.lama_sholat_subuh;
  
  parameter.beep=set_parameter(alarm_beep ,parameter.beep,0,99);
  
  parameter.jam_kecerahan_4=60*set_parameter(off ,parameter.jam_kecerahan_4/60,0,23);
  parameter.jam_kecerahan_1=60*set_parameter(on ,parameter.jam_kecerahan_1/60,0,23);

  EEPROM.put( 0, parameter );
  baca_jadwal(parameter.kota);
  refres(all);
}


if(digitalRead(tombol_down)==0){
  a=0;
  while(digitalRead(tombol_down)==0){
    delay(100);
    a++;
    if(a==50){
      tunggu_down();
      parameter.kota=set_parameter(set_kota ,parameter.kota,0,316);
      parameter.tambah_kurang_subuh=set_parameter(jadwal ,parameter.tambah_kurang_subuh,-10,10);
      parameter.tambah_kurang_duhur = parameter.tambah_kurang_subuh ;
      parameter.tambah_kurang_ashar = parameter.tambah_kurang_subuh; 
      parameter.tambah_kurang_maghrib = parameter.tambah_kurang_subuh; 
      parameter.tambah_kurang_isya = parameter.tambah_kurang_subuh;
      parameter.tartil_subuh =set_parameter(tlwh_1 ,parameter.tartil_subuh,0,99);
      parameter.tartil_duhur =set_parameter(tlwh_2 ,parameter.tartil_duhur,0,99);
      parameter.tartil_ashar =set_parameter(tlwh_3 ,parameter.tartil_ashar,0,99);
      parameter.tartil_maghrib =set_parameter(tlwh_4 ,parameter.tartil_maghrib,0,99);
      parameter.tartil_isya =set_parameter(tlwh_5 ,parameter.tartil_isya,0,99);
      parameter.tartil_jumat =set_parameter(tlwh_6 ,parameter.tartil_jumat,0,99);
      EEPROM.put( 0, parameter );
      baca_jadwal(parameter.kota);
      refres(all);
      }
    }
  }
  /*
   */
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void alarm(void){  
    unsigned char i;
    int waktu_alaram = data[menit]+(data[jam]*60);
    if(waktu_alaram == data[waktu_subuh]-parameter.tartil_subuh&&parameter.tartil_subuh>0) myDFPlayer.randomAll(); //Random play all the mp3.  
    if(data[hari]==jumat){
        if(waktu_alaram == data[waktu_duhur]-parameter.tartil_jumat&&parameter.tartil_jumat>0)myDFPlayer.randomAll(); //Random play all the mp3.  
    }
    else{
        if(waktu_alaram == data[waktu_duhur]-parameter.tartil_duhur&&parameter.tartil_duhur>0)myDFPlayer.randomAll(); //Random play all the mp3.  
    }
    if(waktu_alaram == data[waktu_ashar]-parameter.tartil_ashar&&parameter.tartil_ashar>0 )myDFPlayer.randomAll(); //Random play all the mp3.  
    if(waktu_alaram == data[waktu_magrib]-parameter.tartil_maghrib&&parameter.tartil_maghrib>0)myDFPlayer.randomAll(); //Random play all the mp3.  
    if(waktu_alaram == data[waktu_isya]-parameter.tartil_isya&&parameter.tartil_isya>0)myDFPlayer.randomAll(); //Random play all the mp3.  
    
    if(waktu_alaram == data[waktu_imsya]){
        tampil_hari(imsya);
        unsigned long currentMillis=millis()+60000;
        while(millis()<currentMillis){      
            tampil_segmen();
            tampil_text(text_run);
        }
    };
    if(waktu_alaram == data[waktu_subuh])alarm_on(waktu_subuh);
    if(waktu_alaram == data[waktu_duhur]){
      if(data[hari]==jumat)alarm_on(waktu_jumat);
      else alarm_on(waktu_duhur);
    }
    if(waktu_alaram == data[waktu_ashar]) alarm_on(waktu_ashar);
    if(waktu_alaram == data[waktu_magrib])alarm_on(waktu_magrib);
    if(waktu_alaram == data[waktu_isya])alarm_on(waktu_isya);
}
void tampil_alarm(byte AL){
    unsigned char i;
    switch (AL) {
        case waktu_subuh:    
            tampil_hari(subuh);
            for(i=12;i<36;i++){
              if(i<16 || i>19){
                segmen[i]=data_jadwal [10];  
              }
            }
            break;
        case waktu_duhur:   
            tampil_hari(duhur);
            for(i=12;i<36;i++){
              if(i<20 || i>23){
                segmen[i]=data_jadwal [10]; 
              }
            }
            break;
        case waktu_ashar:
            tampil_hari(ashar);
            for(i=12;i<36;i++){
              if(i<24 || i>27){
                segmen[i]=data_jadwal [10]; 
              }
            }
            break;
        case waktu_magrib:
            tampil_hari(magrib);
            for(i=12;i<36;i++){
              if(i<28 || i>31){
                segmen[i]=data_jadwal [10];
              }
            }
            break;
        case waktu_isya:
            tampil_hari(isya);
            for(i=12;i<32;i++){
              segmen[i]=data_jadwal [10];
            }
            break;
        case waktu_jumat:
            tampil_hari(jumat);
            for(i=12;i<36;i++){
              if(i<20 || i>23){
                segmen[i]=data_jadwal [10]; 
              }
            }
            break;
        case display_off:
            for( i=4;i<12;i++)segmen[i]=data_kalender [10];
            for( i=12;i<36;i++){
              segmen[i]=data_jadwal [10];
            }
            break;
    }
}
void alarm_on(byte sholat){//time out adzan
  byte beep_alarm=3;
  unsigned long time_adzan ;
  unsigned long currentMillis;
  int _alamat_text;
  int count_iqomah ;
  int stanby_sholat;
  alamat_eprom=0;
  myDFPlayer.stop();
  if (sholat==waktu_subuh){
    beep_alarm= parameter.beep;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_subuh*60000);
    count_iqomah = 60*parameter.iqomah_subuh;
    stanby_sholat= parameter.lama_sholat_subuh;
    display_eprom(text_iq_subuh);
    _alamat_text=text_iq_subuh;
  }
  if (sholat==waktu_duhur){
    beep_alarm= parameter.beep;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_duhur *60000);
    count_iqomah = 60*parameter.iqomah_duhur;
    stanby_sholat=parameter.lama_sholat_duhur;
    display_eprom(text_iq_duhur);
    _alamat_text=text_iq_duhur;
  }
  if (sholat==waktu_ashar){
    beep_alarm= parameter.beep;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_ashar*60000);
    count_iqomah = 60*parameter.iqomah_ashar ;
    stanby_sholat=parameter.lama_sholat_ashar;
    display_eprom(text_iq_ashar);
    _alamat_text=text_iq_ashar;
  }
  if (sholat==waktu_magrib){
    beep_alarm= parameter.beep;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_maghrib*60000);
    count_iqomah = 60*parameter.iqomah_maghrib ;
    stanby_sholat=parameter.lama_sholat_maghrib;
    display_eprom(text_iq_maghrib);
    _alamat_text=text_iq_maghrib;
  }
  if (sholat==waktu_isya){
    beep_alarm= parameter.beep;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_isya*60000);
    count_iqomah = 60*parameter.iqomah_isya;
    stanby_sholat=parameter.lama_sholat_isya;
    display_eprom(text_iq_isya);
    _alamat_text=text_iq_isya;
  }
  if (sholat==waktu_jumat){
    beep_alarm= 0;     // parameter.beep;
    time_adzan = millis()+(parameter.timer_adzan_jumat*60000);
    count_iqomah = 60*parameter.iqomah_jumat;
    stanby_sholat=parameter.lama_sholat_jumat;
    display_eprom(text_iq_jumat);
    _alamat_text=text_iq_jumat;
  }  
  while(millis()<time_adzan){  
     //
      baca_rtc();
      refres(jam);
      currentMillis=millis()+500;
      refres(jadwal);
      tampil_alarm(sholat);
      sqw = HIGH;
      tampil_segmen();
      while(millis()<currentMillis){
        tampil_text(_alamat_text);
        if(beep_alarm>0)set_buzer();
      }
      currentMillis=millis()+500;
      tampil_alarm(display_off);
      sqw = LOW;
      tampil_segmen();
      while(millis()<currentMillis){
        clear_buzer();
        tampil_text(_alamat_text);
      }
      if(beep_alarm>0)beep_alarm--;
  }
  if(count_iqomah>0){
        for(char i=12;i<36;i++)segmen[i]=data_jadwal [10];//
        //time iqomah count down
        if (sholat==waktu_jumat)tampil_hari(kutbah);
        else tampil_hari(iqomah);
        set_buzer();
        delay(750);
        clear_buzer();
        while(count_iqomah>0){  
           // alamat_eprom=0;
            data[jam]=count_iqomah/60;
            data[menit]=count_iqomah%60;
            sqw = HIGH;
            refres(jam);
            unsigned long currentMillis=millis()+500;
            tampil_hari(iqomah);
            tampil_segmen();
            while(millis()<currentMillis){
              if(count_iqomah<3)set_buzer();
              tampil_text(_alamat_text);
            }
            currentMillis=millis()+500;
            sqw = LOW;
            refres(jam);
            tampil_alarm(display_off);
            tampil_segmen();
            while(millis()<currentMillis){    
              tampil_text(_alamat_text);
              clear_buzer();
            }
            count_iqomah--;
            data[jam]=count_iqomah/60;
            data[menit]=count_iqomah%60;
            refres(jam);
            tampil_segmen();
            tampil_text(_alamat_text);
        }
          set_buzer();
          delay(2000);
          clear_buzer();
  }
    tampil_alarm(display_off);
    stanby_sholat=stanby_sholat*60;
    dmd.clearScreen( true );
    while(stanby_sholat>0){  
      baca_rtc();
      sqw = HIGH;
      refres(jam);
      currentMillis=millis()+500;
      //dmd.drawString( 55,0, "::", 2, GRAPHICS_NORMAL);
      dmd.drawChar(60,0,':',GRAPHICS_NORMAL);
      dmd.drawChar(65,0,':',GRAPHICS_NORMAL);
      while(millis()<currentMillis){
        tampil_segmen();
        for (char u=0;u<17;u++) dmd.scanDisplayBySPI();
        OE_DMD_ROWS_OFF();
       // dmd.drawChar(55,0,':',GRAPHICS_NORMAL);
         
      }
      currentMillis=millis()+500;
      sqw = LOW;
      refres(jam);
      dmd.clearScreen( true );
      while(millis()<currentMillis){
        tampil_segmen();
        for (char u=0;u<17;u++) dmd.scanDisplayBySPI();
        OE_DMD_ROWS_OFF();
      }
      stanby_sholat--;
    }  
    display_eprom(text_run);
    alamat_eprom=0;
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
    get_prayer_times(data[tahun], data[bulan],data[hari], lt, bj, wkt, times);
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
    /*
    if(parameter.jadwal_fix_subuh>0)data[waktu_subuh]=parameter.jadwal_fix_subuh ;
    if(parameter.jadwal_fix_duhur>0)data[waktu_duhur]=parameter.jadwal_fix_duhur ;
    if(parameter.jadwal_fix_ashar>0)data[waktu_ashar]=parameter.jadwal_fix_ashar ;
    if(parameter.jadwal_fix_maghrib>0)data[waktu_magrib] =parameter.jadwal_fix_maghrib;
    if(parameter.jadwal_fix_isya>0)data[waktu_isya] =parameter.jadwal_fix_isya;
    data[waktu_subuh]=data[waktu_subuh]+parameter.tambah_kurang_subuh ;
    data[waktu_duhur]=data[waktu_duhur]+parameter.tambah_kurang_duhur ;
    data[waktu_ashar]=data[waktu_ashar]+parameter.tambah_kurang_ashar ; 
    data[waktu_magrib]=data[waktu_magrib]+parameter.tambah_kurang_maghrib ; 
    data[waktu_isya]=data[waktu_isya]+parameter.tambah_kurang_isya ;
    data[waktu_imsya]=data[waktu_subuh]-10;
    */
}
void shiftOut_cepat( uint8_t valU){
for(byte b=0;b<8;b++){
  if((valU&0x80)==0x80) set_data_segmen()
  else clear_data_segmen();
  valU=(valU<<1)|(valU>>7);
  clock_data_segmen();
  }
}
void tampil_segmen(){
  unsigned char _int;
  clear_strobe_segmen(); 
  if (sqw==HIGH) segmen[2]=segmen[2]&0xf7;
  else segmen[2]=segmen[2]|0x08;
  //for(byte f=0;f<6;f++) if(sqw==LOW) segmen[13+(f*4)]=segmen[13+(f*4)]&0xfe;
  //for( _int=0;_int<6;_int++)  segmen[13+(_int*4)]=segmen[13+(_int*4)]&0xfe;
  for(_int=0;_int<36;_int++) shiftOut_cepat( segmen[35-_int]);
  set_strobe_segmen();
}
void tampil_text(int _alamat_text){
  for (char u=0;u<17;u++) dmd.scanDisplayBySPI();
  OE_DMD_ROWS_OFF();
//  if ((timer+35) < millis()) {
    if ( millis()-timer>=35) {
       if(dmd.stepMarquee(-1,0)) display_eprom(_alamat_text);
       timer=millis();
  }
}
void refres(byte mode){
  if(mode==jam || mode==all){
      segmen[0]=(data_jam[data[jam]/10]);
      segmen[1]=(data_jam[data[jam]%10]);
      segmen[2]=(data_jam[data[menit]/10]);
      segmen[3]=(data_jam[data[menit]%10]);
  }
  if(mode==tanggal|| mode==all){
      segmen[4]=(data_kalender[data[tanggal]/10]);
      segmen[5]=(data_kalender[data[tanggal]%10]);
      segmen[6]=255-128;
      segmen[7]=(data_kalender[data[bulan]/10]);
      segmen[8]=(data_kalender[data[bulan]%10]);
      segmen[9]=255-128;
      segmen[10]=(data_kalender[(data[tahun]-2000)/10]);
      segmen[11]=(data_kalender[data[tahun]%10]);
  }
  if(mode==jadwal|| mode==all){
      segmen[12]=(data_jadwal[(data[waktu_imsya]/60)/10]);
      segmen[13]=(data_jadwal[(data[waktu_imsya]/60)%10]);
      segmen[14]=(data_jadwal[(data[waktu_imsya]%60)/10]);
      segmen[15]=(data_jadwal[(data[waktu_imsya]%60)%10]);
      
      segmen[16]=(data_jadwal[(data[waktu_subuh]/60)/10]);
      segmen[17]=(data_jadwal[(data[waktu_subuh]/60)%10]);
      segmen[18]=(data_jadwal[(data[waktu_subuh]%60)/10]);
      segmen[19]=(data_jadwal[(data[waktu_subuh]%60)%10]);
      
      segmen[20]=(data_jadwal[(data[waktu_duhur]/60)/10]);
      segmen[21]=(data_jadwal[(data[waktu_duhur]/60)%10]);
      segmen[22]=(data_jadwal[(data[waktu_duhur]%60)/10]);
      segmen[23]=(data_jadwal[(data[waktu_duhur]%60)%10]);
      
      segmen[24]=(data_jadwal[(data[waktu_ashar]/60)/10]);
      segmen[25]=(data_jadwal[(data[waktu_ashar]/60)%10]);
      segmen[26]=(data_jadwal[(data[waktu_ashar]%60)/10]);
      segmen[27]=(data_jadwal[(data[waktu_ashar]%60)%10]);
      
      segmen[28]=(data_jadwal[(data[waktu_magrib]/60)/10]);
      segmen[29]=(data_jadwal[(data[waktu_magrib]/60)%10]);
      segmen[30]=(data_jadwal[(data[waktu_magrib]%60)/10]);
      segmen[31]=(data_jadwal[(data[waktu_magrib]%60)%10]);
      
      segmen[32]=(data_jadwal[(data[waktu_isya]/60)/10]);
      segmen[33]=(data_jadwal[(data[waktu_isya]/60)%10]);
      segmen[34]=(data_jadwal[(data[waktu_isya]%60)/10]);
      segmen[35]=(data_jadwal[(data[waktu_isya]%60)%10]);
      for(char _int=0;_int<6;_int++)  segmen[13+(_int*4)]=segmen[13+(_int*4)]&0xfe;
  }
  
}


void tampil_hari(unsigned char a){

if(a==ahad){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=ka;
  segmen[7]=kh;
  segmen[8]=ka;
  segmen[9]=kd;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==senin){
  segmen[4]=k_;
  segmen[5]=ks;
  segmen[6]=ke;
  segmen[7]=kn;
  segmen[8]=ki;
  segmen[9]=kn;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==selasa){
  segmen[4]=k_;
  segmen[5]=ks;
  segmen[6]=ke;
  segmen[7]=kl;
  segmen[8]=ka;
  segmen[9]=ks;
  segmen[10]=ka;
  segmen[11]=k_;
  }
else if(a==rabu){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=kr;
  segmen[7]=ka;
  segmen[8]=kb;
  segmen[9]=ku;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==kamis){
  segmen[4]=k_;
  segmen[5]=kk;
  segmen[6]=ka;
  segmen[7]=km1;
  segmen[8]=km2;
  segmen[9]=ki;
  segmen[10]=ks;
  segmen[11]=k_;
  }
else if(a==jumat){
  
  segmen[4]=k_;
  segmen[5]=kj;
  segmen[6]=ku;
  segmen[7]=km1;
  segmen[8]=km2;
  segmen[9]=ka;
  segmen[10]=kt;
  segmen[11]=k_;
  }
else if(a==sabtu){
  segmen[4]=k_;
  segmen[5]=ks;
  segmen[6]=ka;
  segmen[7]=kb;
  segmen[8]=kt;
  segmen[9]=ku;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==alarm_beep){
  segmen[4]=k_;
  segmen[5]=ka;
  segmen[6]=kl;
  segmen[7]=ka;
  segmen[8]=kr;
  segmen[9]=km1;
  segmen[10]=km2;
  segmen[11]=k_;
  }
else if(a==on){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=k_;
  segmen[7]=ko;
  segmen[8]=kn;
  segmen[9]=k_;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==off){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=k_;
  segmen[7]=ko;
  segmen[8]=kf;
  segmen[9]=kf;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==imsya){
  segmen[4]=k_;
  segmen[5]=ki;
  segmen[6]=km1;
  segmen[7]=km2;
  segmen[8]=ks;
  segmen[9]=ky;
  segmen[10]=ka;
  segmen[11]=k_;
  }
else if(a==subuh){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=ks;
  segmen[7]=ku;
  segmen[8]=kb;
  segmen[9]=ku;
  segmen[10]=kh;
  segmen[11]=k_;
  }
else if(a==duhur){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=kd;
  segmen[7]=ku;
  segmen[8]=kh;
  segmen[9]=ku;
  segmen[10]=kr;
  segmen[11]=k_;
  }

else if(a==ashar){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=ka;
  segmen[7]=ks;
  segmen[8]=kh;
  segmen[9]=ka;
  segmen[10]=kr;
  segmen[11]=k_;
  }
else if(a==magrib){
  segmen[4]=km1;
  segmen[5]=km2;
  segmen[6]=ka;
  segmen[7]=kg;
  segmen[8]=kh;
  segmen[9]=kr;
  segmen[10]=ki;
  segmen[11]=kb;
  }
else if(a==isya){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=ki;
  segmen[7]=ks;
  segmen[8]=ky;
  segmen[9]=ka;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==stanbay){
  
  segmen[4]=k_;
  segmen[5]=ks;
  segmen[6]=kt;
  segmen[7]=ka;
  segmen[8]=kn;
  segmen[9]=kb;
  segmen[10]=ky;
  segmen[11]=k_;
  }
else if(a==iqomah){
  segmen[4]=k_;
  segmen[5]=ki;
  segmen[6]=kq;
  segmen[7]=ko;
  segmen[8]=km1;
  segmen[9]=km2;
  segmen[10]=ka;
  segmen[11]=kh;
  }
else if(a==tambah){
  segmen[4]=kt;
  segmen[5]=ka;
  segmen[6]=km1;
  segmen[7]=km2;
  segmen[8]=kb;
  segmen[9]=ka;
  segmen[10]=kh;
  segmen[11]=k_;
  }

else if(a==kurang){
  segmen[4]=kk;
  segmen[5]=ku;
  segmen[6]=kr;
  segmen[7]=ka;
  segmen[8]=kn;
  segmen[9]=kg;
  segmen[10]=k_;
  segmen[11]=k_;
  }

else if(a==0xaa){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=k_;
  segmen[7]=k_;
  segmen[8]=k_;
  segmen[9]=k_;
  segmen[10]=k_;
  segmen[11]=k_;
  }
  
else if(a==set_kota){
  segmen[4]=k_;
  segmen[5]=k_;
  segmen[6]=kk;
  segmen[7]=ko;
  segmen[8]=kt;
  segmen[9]=ka;
  segmen[10]=k_;
  segmen[11]=k_;
  }
else if(a==jadwal){
  segmen[4]=k_;
  segmen[5]=kj;
  segmen[6]=ka;
  segmen[7]=kd;
  segmen[8]=kw1;
  segmen[9]=kw2;
  segmen[10]=ka;
  segmen[11]=kl;
  }
  else if(a==tlwh_1){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [1];
  segmen[11]=k_;
  }
  else if(a==tlwh_2){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [2];
  segmen[11]=k_;
  }
  else if(a==tlwh_3){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [3];
  segmen[11]=k_;
  }
  else if(a==tlwh_4){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [4];
  segmen[11]=k_;
  }
  else if(a==tlwh_5){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [5];
  segmen[11]=k_;
  }
  else if(a==tlwh_6){
  segmen[4]=k_;
  segmen[5]=kt;
  segmen[6]=kr;
  segmen[7]=kt;
  segmen[8]=kl;
  segmen[9]=k_;
  segmen[10]=data_kalender [6];
  segmen[11]=k_;
  }
  else if(a==kutbah){
  segmen[4]=kk;
  segmen[5]=kh;
  segmen[6]=ku;
  segmen[7]=kt;
  segmen[8]=kb;
  segmen[9]=ka;
  segmen[10]=kh;
  segmen[11]=k_;
  }
}

//==================================================================================

void serialEvent1() {
  String input_serial;
  if (Serial1.available())input_serial=Serial1.readString();  
  if (input_serial=="1234"){
      Serial1.print("OK\n");
      command=command_start;
  }
  else{
      if (command==command_start){
            if(input_serial== "%J"){      
              command=set_jam;
              Serial1.print("OKJ\n");
            }
            else if(input_serial== "%S"){ 
              command=set_text;
              Serial1.print("OKS\n");
            }
            else if(input_serial== "%I"){ 
              command=set_iqomah;
              Serial1.print("OKI\n");
            }
            else if(input_serial== "%T"){ 
              command=set_tarhim;
              Serial1.print("OKT\n");
            }
            else if(input_serial== "%B"){ 
              command=set_brightnes;
              Serial1.print("OKB\n");
            }
            else if(input_serial== "%F"){ 
              command=set_offsite;
              Serial1.print("OKF\n");
            }
            else if(input_serial== "%X"){ 
              command=set_fix;
              Serial1.print("OKX\n");
            }
            else if(input_serial== "%K"){ 
              command=set_kota;
              Serial1.print("OKK\n");
            }
            else if(input_serial== "%A"){ 
              command=set_adzan;
              Serial1.print("OKA\n");
            }
            else if(input_serial== "%W"){ 
              command=play_mp3;
              Serial1.print("OKW\n");
            }
      }
  //===========================================================
      else if(command==play_mp3){
        command=command_end;
        int _track =((input_serial[1]-'0')*100)+((input_serial[2]-'0')*10)+(input_serial[3]-'0');
        /*
         * 1234 %W Q001
          1234 %W Q001
          1234 %W Q001
          1234 %W Q004
          sholawat
          1234 %W W003
          1234 %W W010
          stop
          1234 %W STOP
          myDFPlayer.play(1);  //Play the first mp3
          myDFPlayer.stop();
         */
         if(input_serial[0]=='Q'){
            myDFPlayer.play(_track);
          //play alquran
         }
         if(input_serial[0]=='W'){
            //play sholawat
            myDFPlayer.play(_track+114);
         }
         if(input_serial[0]=='S'){
          myDFPlayer.stop();
          //STOP
         }
        Serial1.print("SetPlay\n");
      }
      else if(command==set_jam){
        command=command_end;
        //290417040319 // DTK_MNT_JAM_TGL_BLN_TH
        data[jam]=((input_serial[4]-'0')*10)+(input_serial[5]-'0');
        data[menit]=((input_serial[2]-'0')*10)+(input_serial[3]-'0');
        data[detik]= ((input_serial[0]-'0')*10)+(input_serial[1]-'0');
        data[tanggal]= ((input_serial[6]-'0')*10)+(input_serial[7]-'0');
        data[bulan]= ((input_serial[8]-'0')*10)+(input_serial[9]-'0');
        data[tahun]= 2000+((input_serial[10]-'0')*10)+(input_serial[11]-'0');
        tulis_rtc();
 /*       rtc.setTime(((input_serial[4]-'0')*10)+(input_serial[5]-'0'), ((input_serial[2]-'0')*10)+(input_serial[3]-'0'), ((input_serial[0]-'0')*10)+(input_serial[1]-'0'));
        rtc.setDate(((input_serial[6]-'0')*10)+(input_serial[7]-'0'),((input_serial[8]-'0')*10)+(input_serial[9]-'0'),2000+((input_serial[10]-'0')*10)+(input_serial[11]-'0'));
        rtc.setDOW();
  */
        Serial1.print("SetTime\n");
      }
      else if(command==set_text){//ok
        command=command_end;
        //1234 %S tes karakter
        //writeString(text_run,input_serial);
        int _size = input_serial.length();
        int i;
        for(i=0;i<_size;i++)EEPROM.write(text_run+i,input_serial[i]);
        EEPROM.write(text_run+_size,'\0');   //Add termination null character for String Data
        Serial1.print("SetText\n");
      }
      else if(command==set_iqomah){
        
        /*
         *  1234 %I N 0 1010 Iqomah Subuh // n DAN y ADALAH ENABLE IQOMAH 
            1234 %I N 1 1010 Iqomah Dzuhur... //IQOMAH_SHOLAT
            1234 %I N 2 1010 Iqomah Ashar...
            1234 %I N 3 1010 Iqomah Maghrib...
            1234 %I N 4 1010 Iqomah Isya...
            1234 %I N 5 0510 Iqomah jumat
            1234 %I Y 0 1010 Iqomah Subuh
            void writeString(int add,String data){
            int _size = data.length();
            int i;
            for(i=0;i<_size;i++){
              EEPROM.write(add+i,data[i]);
            }
            EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
          //  EEPROM.commit();
          }
          
          #define text_iq_subuh 100
          #define text_iq_duhur 100+text_iq_subuh
          #define text_iq_ashar 100+text_iq_duhur
          #define text_iq_maghrib 100+text_iq_ashar
          #define text_iq_isya 100+text_iq_maghrib
          #define text_run text_iq_isya+100
         */
        command=command_end;
        int _text_iqomah;
        if(input_serial[1]=='0'){
            parameter.iqomah_subuh=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_subuh =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_subuh,input_serial.substring(6));
            _text_iqomah=text_iq_subuh;
        }
        if(input_serial[1]=='1'){
            parameter.iqomah_duhur=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_duhur =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_duhur,input_serial.substring(6));
            _text_iqomah=text_iq_duhur;
        }
        if(input_serial[1]=='2'){
            parameter.iqomah_ashar=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_ashar =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_ashar,input_serial.substring(6));
            _text_iqomah=text_iq_ashar;
        }
        if(input_serial[1]=='3'){
            parameter.iqomah_maghrib=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_maghrib =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_maghrib,input_serial.substring(6));
            _text_iqomah=text_iq_maghrib;
        }
        if(input_serial[1]=='4'){
            parameter.iqomah_isya=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_isya =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_isya,input_serial.substring(6));
            _text_iqomah=text_iq_isya;
        }
        if(input_serial[1]=='5'){
            parameter.iqomah_jumat=((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
            parameter.lama_sholat_jumat =((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
            //writeString(text_iq_jumat,input_serial.substring(6));
            _text_iqomah=text_iq_jumat;
        }
        int _size = input_serial.length();
        int i;
        int u=0;
        for(i=6;i<_size;i++){
          EEPROM.write(_text_iqomah+u,input_serial[i]);  
          u++;     
        }
        EEPROM.write(_text_iqomah+u,'\0');  
        EEPROM.put( 0, parameter );
        //EEPROM.write(_text_iqomah+_size,'\0');   //Add termination null character for String Data
        Serial1.print("SetIqom\n");
      }
      else if(command==set_tarhim){//OK
        command=command_end;
         //15 15 15 15 15 15 YNNNYY  //Subuh Dzuhur...  jumat YES/NO
        parameter.tartil_subuh = ((input_serial[0]-'0')*10)+(input_serial[1]-'0') ;
        parameter.tartil_duhur = ((input_serial[2]-'0')*10)+(input_serial[3]-'0');
        parameter.tartil_ashar = ((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
        parameter.tartil_maghrib = ((input_serial[6]-'0')*10)+(input_serial[7]-'0'); 
        parameter.tartil_isya = ((input_serial[8]-'0')*10)+(input_serial[9]-'0');
        parameter.tartil_jumat = ((input_serial[10]-'0')*10)+(input_serial[11]-'0');
        if(input_serial[12]=='N') parameter.tartil_subuh =0;
        if(input_serial[13]=='N') parameter.tartil_duhur =0;// ((_trtl[2]-'0')*10)+(_trtl[3]-'0');
        if(input_serial[14]=='N') parameter.tartil_ashar = 0;// ((_trtl[4]-'0')*10)+(_trtl[5]-'0') ;
        if(input_serial[15]=='N') parameter.tartil_maghrib = 0;//((_trtl[6]-'0')*10)+(_trtl[7]-'0'); 
        if(input_serial[16]=='N') parameter.tartil_isya = 0;//((_trtl[8]-'0')*10)+(_trtl[9]-'0');
        if(input_serial[17]=='N') parameter.tartil_jumat = 0;//((_trtl[10]-'0')*10)+(_trtl[11]-'0');
        EEPROM.put( 0, parameter );
        Serial1.print("SetTrkm\n");
      }
      else if(command==set_brightnes){//ok
        command=command_end;
        /*
         * 1234 %B 0300 01 0600 03 1730 01 2100 00
         * 1234 %B 0300 07 0600 03 1730 01 2100 00
         */
         parameter.kecerahan_1 =input_serial[5]-'0';
         parameter.kecerahan_2 =input_serial[11]-'0';
         parameter.kecerahan_3 =input_serial[17]-'0';
         parameter.kecerahan_4 =input_serial[23]-'0';
         parameter.jam_kecerahan_1 = ((((input_serial[0]-'0')*10)+(input_serial[1]-'0'))*60)+(((input_serial[2]-'0')*10)+(input_serial[3]-'0'));
         parameter.jam_kecerahan_2 = ((((input_serial[6]-'0')*10)+(input_serial[7]-'0'))*60)+(((input_serial[8]-'0')*10)+(input_serial[9]-'0'));
         parameter.jam_kecerahan_3 = ((((input_serial[12]-'0')*10)+(input_serial[13]-'0'))*60)+(((input_serial[14]-'0')*10)+(input_serial[15]-'0'));
         parameter.jam_kecerahan_4 = ((((input_serial[18]-'0')*10)+(input_serial[19]-'0'))*60)+(((input_serial[20]-'0')*10)+(input_serial[21]-'0'));
         EEPROM.put( 0, parameter );
         Serial1.print("SetBrns\n");
      }
      else if(command==set_offsite){//OK
        command=command_end;
        //1234 %F 19 02 01 03 05 //1= TAMBAH & 0=KURANG
        if(input_serial[0]=='0') parameter.tambah_kurang_subuh =0-(input_serial[1]-'0');
        else  parameter.tambah_kurang_subuh =input_serial[1]-'0';
        if(input_serial[2]=='0')parameter.tambah_kurang_duhur =0-(input_serial[3]-'0');
        else parameter.tambah_kurang_duhur =input_serial[3] -'0';
        if(input_serial[4]=='0')parameter.tambah_kurang_ashar =0-(input_serial[5]-'0');
        else parameter.tambah_kurang_ashar =input_serial[5] -'0';
        if(input_serial[6]=='0')parameter.tambah_kurang_maghrib =0-(input_serial[7]-'0');
        else parameter.tambah_kurang_maghrib =input_serial[7] -'0';
        if(input_serial[8]=='0')parameter.tambah_kurang_isya =0-(input_serial[9]-'0'); 
        else parameter.tambah_kurang_isya =input_serial[9] -'0';
        EEPROM.put( 0, parameter );
        Serial1.print("SetOffs\n");
      }
      else if(command==set_fix){//OK
        command=command_end;
        //YYNYN 2200 0000 0000 0000 0000
        //5 9 13 17 21
        parameter.jadwal_fix_subuh = 0 ;
        parameter.jadwal_fix_duhur =0;
        parameter.jadwal_fix_ashar =0;
        parameter.jadwal_fix_maghrib =0;
        parameter.jadwal_fix_isya =0;
        if(input_serial[0]=='Y')parameter.jadwal_fix_subuh = ((((input_serial[5]-'0')*10)+(input_serial[6]-'0'))*60)+(((input_serial[7]-'0')*10)+(input_serial[8]-'0'));
        if(input_serial[1]=='Y')parameter.jadwal_fix_duhur = ((((input_serial[9]-'0')*10)+(input_serial[10]-'0'))*60)+(((input_serial[11]-'0')*10)+(input_serial[12]-'0'));
        if(input_serial[2]=='Y')parameter.jadwal_fix_ashar = ((((input_serial[13]-'0')*10)+(input_serial[14]-'0'))*60)+(((input_serial[15]-'0')*10)+(input_serial[16]-'0'));
        if(input_serial[3]=='Y')parameter.jadwal_fix_maghrib = ((((input_serial[17]-'0')*10)+(input_serial[18]-'0'))*60)+(((input_serial[19]-'0')*10)+(input_serial[20]-'0'));
        if(input_serial[4]=='Y')parameter.jadwal_fix_isya = ((((input_serial[21]-'0')*10)+(input_serial[22]-'0'))*60)+(((input_serial[23]-'0')*10)+(input_serial[24]-'0'));
        EEPROM.put( 0, parameter );
        Serial1.print("SetFixx\n");
      }
      else if(command==set_kota){//ok
        command=command_end;
        /*
        EN 110 39 007 82 01187+00 //LU
        ES 110 39 007 82 01187+00 //ls
        */
        parameter.set_kota_bjr=((input_serial[2]-'0')*100)+((input_serial[3]-'0')*10)+(input_serial[4]-'0')+((input_serial[5]-'0')*0.1)+((input_serial[6]-'0')*0.01)  ;
        if(input_serial[1] == 'N') parameter.set_kota_lnt =((input_serial[7]-'0')*100)+((input_serial[8]-'0')*10)+(input_serial[9]-'0')+((input_serial[10]-'0')*0.1)+((input_serial[11]-'0')*0.01)  ;
        if(input_serial[1] == 'S')parameter.set_kota_lnt =0-(((input_serial[7]-'0')*100)+((input_serial[8]-'0')*10)+(input_serial[9]-'0')+((input_serial[10]-'0')*0.1)+((input_serial[11]-'0')*0.01))  ;
        parameter.set_kota_gmt=input_serial[16]-'0' ;
        EEPROM.put( 0, parameter );
        Serial1.print("SetKoor\n");
      }
      else if(command==set_adzan){//OK
        command=command_end;
        parameter.timer_adzan_subuh= ((input_serial[0]-'0')*10)+(input_serial[1]-'0') ;
        parameter.timer_adzan_duhur= ((input_serial[2]-'0')*10)+(input_serial[3]-'0') ;
        parameter.timer_adzan_ashar= ((input_serial[4]-'0')*10)+(input_serial[5]-'0') ;
        parameter.timer_adzan_maghrib = ((input_serial[6]-'0')*10)+(input_serial[7]-'0') ;
        parameter.timer_adzan_isya = ((input_serial[8]-'0')*10)+(input_serial[9]-'0') ;
        parameter.timer_adzan_jumat = ((input_serial[10]-'0')*10)+(input_serial[11]-'0') ;
        EEPROM.put( 0, parameter );
        Serial1.print("SetAlrm\n");
      }
    }

//===========================================================
    if(command==command_end){
      set_buzer();
      baca_jadwal(parameter.kota);
      alamat_eprom=0;
      display_eprom(text_run);
      clear_buzer();
      refres(all);
    }
}

void display_eprom(int add){
  int len=0;
  char run_data[512]; //Max 100 Bytes
  unsigned char k;
  k=EEPROM.read(add);
  if(alamat_eprom>=500)alamat_eprom=0;
  while(alamat_eprom<500) {   //Read until null character
    k=EEPROM.read(add+alamat_eprom);
    run_data[len]=k;
    alamat_eprom++;
    len++;
    if(k=='\n'){
      run_data[len]='\0';
      dmd.clearScreen( true );
      dmd.drawMarquee(run_data,len,(64*DISPLAYS_ACROSS)-1,0);
      k=EEPROM.read(add+alamat_eprom);
      if(k=='\0') alamat_eprom=0;
      break;
    }
    if(k=='\0'){
      alamat_eprom=0;
      break;
    }
  }
}

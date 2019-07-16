
#define set_data_segmen() { PORTA |= _BV(PA5); }
#define clear_data_segmen() { PORTA &= ~_BV(PA5); }
#define clock_data_segmen() { PORTA |= _BV(PA6); PORTA &= ~_BV(PA6);}

#define set_strobe_segmen() { PORTA |= _BV(PA4); }
#define clear_strobe_segmen() { PORTA &= ~_BV(PA4); }

#define buzer  2  

#define set_buzer() { PORTE |= _BV(PE2); }
#define clear_buzer() { PORTE &= ~_BV(PE2); }

#define strobePin  40 //Pin connected to Strobe (pin 1) of 4094
#define dataPin  39 //Pin connected to Data (pin 2) of 4094
#define clockPin  38  //Pin connected to Clock (pin 3) of 4094

#define tombol_up 5
#define tombol_menu 45
#define tombol_down 46

//byte segChar[] = {1, 2, 4, 8, 16, 32, 64, 128, 255, 0}; //cek

byte sqw,day;
unsigned long previousMillis = 0;       
unsigned long previousMillis_2 = 0;     
unsigned long previousMillis_3 = 0;     
#define setengah_detik  500          
#define lima_detik  5000    
#define lima_belas_detik  15000    

const byte data_jam[]= {136 ,238 ,73 ,76 ,46 ,28 ,24 ,206 ,8 ,12 ,255};
const byte data_kalender []={129 ,237 ,67 ,73 ,45 ,25 ,17 ,205 ,1 ,9 ,255};
const byte data_jadwal []={129 ,237 ,67 ,73 ,45 ,25 ,17 ,205 ,1 ,9 ,255};

//===============================================================================
//seting
volatile static byte commad;

#define text_iq_subuh 100
#define text_iq_duhur 200
#define text_iq_ashar 300
#define text_iq_maghrib 400
#define text_iq_isya 500
#define text_iq_jumat 600
#define text_run 700

#define command_end 0
#define command_start 1
#define set_jam 2
#define set_text 3
#define set_iqomah 4
#define set_tarhim 5
#define set_brightnes 6
#define set_offsite 7
#define set_fix 8
#define set_kota 9
#define set_adzan 10
#define play_mp3 11
volatile static byte command;

struct MyObject{
    int  kota;
    byte  beep;
    byte  timer_adzan_subuh ;
    byte  timer_adzan_duhur ;
    byte  timer_adzan_ashar ;
    byte  timer_adzan_maghrib ;
    byte  timer_adzan_isya ;
    byte  timer_adzan_jumat ;
    
    byte  iqomah_subuh ;
    byte  iqomah_duhur ;
    byte  iqomah_ashar  ;
    byte  iqomah_maghrib ;
    byte  iqomah_isya ;
    byte  iqomah_jumat ;
    
    byte  lama_sholat_subuh ;
    byte  lama_sholat_duhur ;
    byte  lama_sholat_ashar  ;
    byte  lama_sholat_maghrib ;
    byte  lama_sholat_isya ;
    byte  lama_sholat_jumat ;
    byte  tartil_subuh ;// 0= mati dan >0 hidup
    byte  tartil_duhur ;
    byte  tartil_ashar ;
    byte  tartil_maghrib; 
    byte  tartil_isya ;
    byte  tartil_jumat ;
    byte  kecerahan_1 ;
    byte  kecerahan_2 ;
    byte  kecerahan_3 ;
    byte  kecerahan_4 ;
    int  jam_kecerahan_1 ;
    int  jam_kecerahan_2 ;
    int  jam_kecerahan_3 ;
    int  jam_kecerahan_4 ;
    int  tambah_kurang_subuh ;
    int  tambah_kurang_duhur ;
    int  tambah_kurang_ashar ; 
    int  tambah_kurang_maghrib ; 
    int  tambah_kurang_isya ;
    int  jadwal_fix_subuh ;
    int  jadwal_fix_duhur ;
    int  jadwal_fix_ashar ;
    int  jadwal_fix_maghrib ;
    int  jadwal_fix_isya ;
    byte  mazhab_ashar  ;
    int  set_kota_gmt ;
    float  set_kota_lnt ;
    float  set_kota_bjr ;
};


#define detik 0
#define menit 1
#define jam 2
#define hari 3
#define tanggal 4
#define bulan 5
#define tahun 6
#define waktu_imsya 7
#define waktu_subuh 8
#define waktu_duhur 9
#define waktu_ashar 10
#define waktu_magrib 11
#define waktu_isya 12
#define waktu_jumat 13
#define display_off 14

#define jadwal 14
#define all 15

#define senin 1
#define selasa 2
#define rabu 3
#define kamis 4
#define jumat 5
#define sabtu 6
#define ahad 7
#define alarm_beep 8

#define on 9
#define off 10

#define imsya 11
#define subuh 12
#define duhur 13
#define ashar 14
#define magrib 15
#define isya 16

#define stanbay 17
#define iqomah 18
#define tambah 20
#define kurang 21
#define set_kota 22
#define jadwal 23
#define tlwh_1 24
#define tlwh_2 25
#define tlwh_3 26
#define tlwh_4 27
#define tlwh_5 28
#define tlwh_6 29
#define kutbah 30


#define ka   65
#define kb   49
#define kd   97
#define ke   3
#define kf   23
#define kg   9
#define kh   53
#define ki   237
#define kj   225
#define kk   21
#define kl   179
#define km1  135
#define km2  141
#define kn   133
#define ko   129
#define kp   7
#define kq   13
#define kr   151
#define ks   25
#define kt   51
#define ku   161
#define ky   41
#define kw1  177
#define kw2  225
#define k_   255
int data[20];

volatile unsigned char segmen[36];
unsigned char hidup,mati;

#include <18F4620.h>

#fuses HS, NOFCMEN, NOIESO, PUT, NOBROWNOUT, NOWDT
#fuses NOPBADEN, STVREN, NOLVP, NODEBUG,

#use delay(clock=16000000)
//#use fast_io(A)
//#use fast_io(B)
//#use fast_io(D)
//#use fast_io(E)
#use standard_io(c)

#define punto 128
#define dht11 PIN_C4

#bit inDataDht = 0xF94.4
#bit dataDht = 0xF82.4

unsigned char check;
unsigned char t_byte1, t_byte2, h_Byte1, h_byte2;
unsigned char sum;
int leerHum=0, posicion, contador, lectura;
int memVideo[4]={0};

void startSignal(){
   
   disable_interrupts(GLOBAL);
   inDataDht = 0;
   output_high(dht11);
   output_low(dht11);
   delay_ms(18);
   output_high(dht11);
   delay_us(30);
   inDataDht = 1;
   
}

char check_Response(){
   
   disable_interrupts(GLOBAL);
   inDataDht = 1;
   check=0;
   delay_us(40);
   if(!input(dht11)){
      delay_us(80);
      if(input(dht11)){
         delay_us(40);
         return 1; 
      }
   }
   enable_interrupts(GLOBAL);
}

char readData(){
   
   disable_interrupts(GLOBAL);
   inDataDht = 1;
   char i, j;
   for(j=0; j<8; j++){
      while(!input(dht11));
      delay_us(30);
      if(input(dht11) == 0){
         i&= ~(1<<(7-j));
      }else{
         i|= (1<<(7-j));
         while(input(dht11)==1);
      }
   }
   enable_interrupts(GLOBAL);
   return i;
}

#INT_TIMER0
void timer0_isr(void){
   
   output_d(memVideo[posicion]);
   output_a(1<<posicion);
   posicion++;
   if(posicion==4){ 
      posicion=0;
   }
   contador++;
   if(contador==122){ 
      contador=0;
      lectura=1;
   }
   
}

#INT_RB
void interrupt_isr(void){
   if(input(PIN_B4)==1)
      leerHum=!leerHum;
}

void main() {
   
   set_tris_a(0xC0);
   set_tris_b(0x10);
   set_tris_d(0x00);
   set_tris_e(0x08);
   setup_oscillator(OSC_16MHZ);
   enable_interrupts(INT_RB);
   setup_timer_0(RTCC_INTERNAL | RTCC_DIV_128 | RTCC_8_BIT);
   enable_interrupts(INT_TIMER0);
   enable_interrupts(GLOBAL);
   int datos[4]={0};
   int num[10]={63,6,91,79,102,109,125,7,127,103};
   while(true){
      
      if(lectura==1){
      lectura=0;
      startSignal();
      if(check_Response()){
         h_Byte1 = readData();
         h_byte2 = readData();
         t_byte1 = readData();
         t_byte2 = readData();
         sum = readData();
         if(sum == ((h_Byte1+h_byte2+t_byte1+t_byte2) & 0xFF)){
            if(leerHum==1){
               output_e(0x02);
               datos[0] = h_Byte1/10; //decenas
               datos[1] = h_Byte1%10; //unidades
               datos[2] = h_byte2/10; //decimas
               datos[3] = h_byte2%10; //centesimas 
               memVideo[0] =num[datos[0]];
               memVideo[1] =num[datos[1]] + punto;
               memVideo[2] =num[datos[2]];
               memVideo[3] =num[datos[3]];
            }else{
               output_e(0x01);
               datos[0] = t_byte1/10; //decenas
               datos[1] = t_byte1%10; //unidades
               datos[2] = t_byte2/10; //decimas
               datos[3] = t_byte2%10; //centesimas
               memVideo[0] =num[datos[0]];
               memVideo[1] =num[datos[1]] + punto;
               memVideo[2] =num[datos[2]];
               memVideo[3] =num[datos[3]];
            }
         }
      }
      }
   } 
}

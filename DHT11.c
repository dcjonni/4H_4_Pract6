#include <18F4620.h>

#fuses HS, NOFCMEN, NOIESO, PUT, NOBROWNOUT, NOWDT
#fuses NOPBADEN, STVREN, NOLVP, NODEBUG,

#use delay(clock=16000000)
#use fast_io(A)
#use fast_io(B)
#use fast_io(D)
#use fast_io(E)

#define punto 128

unsigned char check;
unsigned char t_byte1, t_byte2, h_Byte1, h_byte2;
unsigned char sum;
int leerHum, display, contador;
int memVideo[4]={0};

#define dht11 PIN_C4
#bit inDataDht = 0x94.0
#byte dataDht = 0x82

void startSignal(){
   
   inDataDht = 0;
   output_high(dht11);
   output_low(dht11);
   delay_ms(18);
   output_high(dht11);
   delay_us(30);
   inDataDht = 1;
   
}

void checkResponse(){

   check = 0;
   delay_us(40);
   if(dataDht==0){
      delay_us(80);
      if(input(dht11)==1)
      check=1;
      delay_us(40);
   }
}

char readData(){
   
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
   return i;
}


#INT_RB
void interrupt_isr(void){
   if(input(PIN_B4)==1)
      leerHum=1;
   if(input(PIN_B4)==0)
      leerHum=0;
}

void main() {
   
   set_tris_b(0xF0);
   set_tris_d(0x00);
   set_tris_e(0x08);
   output_a(0x0F);
   enable_interrupts(INT_RB);
   enable_interrupts(GLOBAL);
   while(true){
      startSignal();
      checkResponse();
      if(check==1){
         h_Byte1 = readData();
         h_byte2 = readData();
         t_byte1 = readData();
         t_byte2 = readData();
         sum = readData();
         if(sum == ((h_Byte1+h_byte2+t_byte1+t_byte2) & 0xFF)){
            if(leerHum==1){
               output_e(0x02);
            }else{
               output_e(0x01);
            }   
         }
      } 
   } 
}

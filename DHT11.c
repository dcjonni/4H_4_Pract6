#include <18F4620.h>

#fuses HS, NOFCMEN, NOIESO, PUT, NOBROWNOUT, NOWDT
#fuses NOPBADEN, STVREN, NOLVP, NODEBUG,

#use delay(clock=16000000)
#use standard_io(c)

#define punto 128
#define dht11Pin PIN_C4

#bit trisPinC4 = 0xF94.4
//#bit dataDht = 0xF82.4

unsigned char check;
unsigned char humedadEntera_Byte1, humedadDecimal_Byte2;
unsigned char temperaturaEntera_Byte3, temperaturaDecimal_Byte4;
unsigned char checksum_Byte5;
int Humedad=0, posicion, contador, inicioComunicacion;
int memVideo[4]={0};

void startSignal(){
   
   disable_interrupts(GLOBAL);
   trisPinC4 = 0;
   output_high(dht11Pin);
   output_low(dht11Pin);
   delay_ms(18);
   output_high(dht11Pin);
   delay_us(30);
   trisPinC4 = 1;
   
}

void check_Response(){
   
   disable_interrupts(GLOBAL);
   trisPinC4 = 1;
   delay_us(40);
   if(!input(dht11Pin)){
      delay_us(80);
      if(input(dht11Pin)){
         delay_us(40);
         check=1;
      }
   }
   enable_interrupts(GLOBAL);
}

char readData(){
   
   disable_interrupts(GLOBAL);
   trisPinC4 = 1;
   char i, j;
   for(j=0; j<8; j++){
      while(!input(dht11Pin));
      delay_us(30);
      if(input(dht11Pin) == 0){
         i&= ~(1<<(7-j));
      }else{
         i|= (1<<(7-j));
         while(input(dht11Pin)==1);
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
      inicioComunicacion=1;
   }
}

#INT_RB
void interrupt_isr(void){
   if(input(PIN_B4)==1)
      Humedad=!Humedad;
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
   int valoresCorrectos;
   int datos[4]={0};
   int numeros[10]={63,6,91,79,102,109,125,7,127,103};
   while(true){
      
      if(inicioComunicacion==1){
      inicioComunicacion=0;
      startSignal();
      check_Response();
      }
      if(check==1){
         check=0;
         valoresCorrectos=0;
         humedadEntera_Byte1 = readData();
         humedadDecimal_Byte2 = readData();
         temperaturaEntera_Byte3 = readData();
         temperaturaDecimal_Byte4 = readData();
         checksum_Byte5 = readData();
         if(checksum_Byte5 == ((humedadEntera_Byte1+humedadDecimal_Byte2+temperaturaEntera_Byte3+temperaturaDecimal_Byte4) & 0xFF)){
            valoresCorrectos=1;
         }
      }
      if(valoresCorrectos==1){
         if(Humedad==1){
            output_e(0x02);
            datos[0] = humedadEntera_Byte1/10; //decenas
            datos[1] = humedadEntera_Byte1%10; //unidades
            datos[2] = humedadDecimal_Byte2/10; //decimas
            datos[3] = humedadDecimal_Byte2%10; //centesimas 
            memVideo[0] =numeros[datos[0]];
            memVideo[1] =numeros[datos[1]] + punto;
            memVideo[2] =numeros[datos[2]];
            memVideo[3] =numeros[datos[3]];
         }else{
            output_e(0x01);
            datos[0] = temperaturaEntera_Byte3/10; //decenas
            datos[1] = temperaturaEntera_Byte3%10; //unidades
            datos[2] = temperaturaDecimal_Byte4/10; //decimas
            datos[3] = temperaturaDecimal_Byte4%10; //centesimas
            memVideo[0] =numeros[datos[0]];
            memVideo[1] =numeros[datos[1]] + punto;
            memVideo[2] =numeros[datos[2]];
            memVideo[3] =numeros[datos[3]];
         }
      }
   } 
}

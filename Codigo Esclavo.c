#define F_CPU 1000000UL // Cristal externo de 4Mhz


#include <avr/io.h>
#include <avr/interrupt.h> //Libreria para el manejo de Interrupciones
#include <util/delay.h>
#include "lcd.h"  //para el manejo del LCD
#include <stdlib.h>

void Ports_Setup(); //Prototipo de la función para flujofigurar puertos
void External_Interrupts_Setup(); //Prototipo para flujofigurar las interrupciones

int flujo=0;
int obliterate=0; // esta variable le indica cuando se ha gastado todo el dinero

void Ports_Setup()
{
	//DDRD = 0x00; //Se flujofigura el puerto D como entrada
	//DDRC = 0xFF; //Se flujofigura el puerto C como salida
}

void External_Interrupts_Setup()
{
	EICRA = (1<<ISC11)|(0<<ISC10)|(1<<ISC01)|(0<<ISC00); //La interrupción 0 es
	//flujofigurada para sensar flancos de bajada, la interrupción
	//1 se flujofigura para sensar flancos de bajada
	EIMSK = (1<<INT1)|(1<<INT0); //Las interrupciones 0 y 1 son habilitadas
	EIFR = (0<<INTF1)|(0<<INTF0); //Las banderas de interrupciones son borradas
	PCICR = (0<<PCIE2)|(0<<PCIE1)|(0<<PCIE0); //Son deshabilitadas las
	//interrupciones externas tipo "Pin-Change"
	PCIFR = (0<<PCIF2)|(0<<PCIF1)|(0<<PCIF0); //Son borradas las banderas de las
	//interrupciones tipo "Pin-Change"
	PCMSK2 = 0x00;//Se deshabilitan las interrupciones Pin-Change para PCINT[23:16]
	PCMSK1 = 0x00;//Se deshabilitan las interrupciones Pin-Change para PCINT[14:8]
	PCMSK0 = 0x00;//Se deshabilitan las interrupciones Pin-Change para PCINT[7:0]
}

ISR(INT0_vect,ISR_NAKED)
{
flujo++;
//PORTC++;//=flujo; //Se muestra el valor del puerto C y después se incrementa su valor
reti(); //Regreso de la interrupcion
}

ISR(INT1_vect,ISR_NAKED)
{
obliterate = 1; //Se muestra el valor del Puerto C y después decrementa su valor
reti(); //Regreso de la interrupcion
}

int main(void)
{
	Ports_Setup();
	External_Interrupts_Setup();
	//flujo=0;
	sei();
	lcd_init(LCD_DISP_ON);//inicia con cursor no visible
	int dinero_actual = 0;
	char numero[10];

	int state = 0;

	while(1)
	{


		if(obliterate == 1)
		{
			dinero_actual = 0;
			flujo = 0;
			obliterate = 0;
		}
		else
		{
			dinero_actual = flujo;
		}

		if(dinero_actual == 0)
		{

			lcd_clrscr();//borr el LCD y lo coloca en la fila 1 columna 1
			lcd_puts("Inserte monedas");//mensaje en la fila 0
			_delay_ms(1000);//pausa para un mejor visualizacion de los mensajes

		}
		else
		{

			if(state ==0 && dinero_actual  != 0)
			{
				state =1;
				dinero_actual = 0;
			}
			
			lcd_clrscr();//borr el LCD y lo coloca en la fila 1 columna 1
			lcd_puts("Dinero Actual:");//mensaje en la fila 0
			lcd_gotoxy(0,1);//cursor en la columna 1 fila 2
			itoa(dinero_actual, numero, 10); // itoa ( int value, char * str, int base );
			lcd_puts(numero);//segundo mensaje
			_delay_ms(2000);//pausa para un mejor visualizacion de los mensajes
		}

	}
}
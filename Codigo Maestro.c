/*
	>>>>>>Programa maquina de monedas<<<<<<<

	Configuracion de pines:
		PIN C0 boton para seleccionar articulo  1 ENTRADA
		PIN C1 boton para seleccionar articulo  2
		PIN C3 motor cambio 10 pesos, va a B3 del microcontrolador principal 
		PIN C4 motor cambio 5  pesos, va  a B2 del microcontrolador principal
		PIN C5 motor cambio 1 peso m va B1 del microntrolador principal
		PIN B4 led articulo 1
		PIN B5 led articulo 2
		PIN B2 led indicador de seleccion articulo 1
		PIN B3 led indicador de seleccion articulo 2
		PIN B0 control obliterate lcd
		PIN B1 control flujo lcd

	Advertencia:
		Introducir dinero antes de seleccionar articulo
*/


#define F_CPU 1000000UL

#include <avr/io.h> // Esta libreria nos permite hacer el macros _BV para la manipulacion de puertos
#include <avr/interrupt.h> //Libreria para la utilizacion de Interrupciones
#include <util/delay.h>

#define COSTO_ARTICULO1 8
#define COSTO_ARTICULO2 5
#define RETRASO_M1 1500
#define RETRASO_M2 1500
#define RETRASO_M3 2800

void Ports_Setup(); 
void External_Interrupts_Setup();
void DarCambio( int dinero, int costo);

int dinero_introducido=0;
int lock = 0 ;

void Ports_Setup()
{
	DDRD = 0xF0; // 
	DDRC = 0xFC; // 0b11111100, solo los primeros 2 como entrada
	DDRB = 0xFF; // puerto B como salida
	PORTC = 0x00;
	PORTB = 0x03; // 0b00000011
}

void External_Interrupts_Setup()
{
	EICRA = (1<<ISC11)|(0<<ISC10)|(1<<ISC01)|(0<<ISC00); //La interrupción 0 es
	//configurada para sensar flancos de bajada, la interrupción
	//1 se configurada para sensar flancos de bajada
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
	dinero_introducido++;
	PORTB &= ~_BV(PB1);//PIN B1 control flujo lcd
	_delay_ms(100); 
	PORTB |= _BV(PB1);
	reti(); //Regreso de la interrupcion
}

ISR(INT1_vect,ISR_NAKED)
{
	lock++;
	reti(); //Regreso de la interrupcion
}

int main(void)
{
	Ports_Setup();
	External_Interrupts_Setup();
	int seleccion_articulo1 = 0;
	int seleccion_articulo2 = 0;

	sei();

	while(1)
	{
		// >>>>>>>Seleccion de articulo 1<<<<<<

		// Asumiendo que el boton pin c0 corresponde al articulo 1
		if ( (PINC & _BV(PC0)) && (dinero_introducido >= COSTO_ARTICULO1 ) ) // para push normalmente abierto
		{
			seleccion_articulo1 = 1; 
			PORTB |= _BV(PB2); // 00000100 prendemos B2
			_delay_ms(300); // debouncing
			// Checamos que el articulo 2 no esta seleccionado y en 
			// caso contrario lo deseleccionamos
			if(seleccion_articulo2 == 1)
			{
				seleccion_articulo2 = 0;
				PORTB &= ~_BV(PB3); // 00010000 apgamos B3
				_delay_ms(100);
			}
		}

		// >>>>>>>Seleccion de articulo 2<<<<<<

		// Asumiendo que el boton pin c1 corresponde al articulo 2
		if ( (PINC & _BV(PC1)) && (dinero_introducido >= COSTO_ARTICULO2) ) // para push normalmente abierto
		{
			seleccion_articulo2 = 1; 
			PORTB |= _BV(PB3); // 00010000 prendemos B3
			_delay_ms(300); // deboucing
			// Checamos que el articulo 1 no esta seleccionado y en 
			// caso contrario lo deseleccionamos
			if(seleccion_articulo1 == 1)
			{
				seleccion_articulo1 = 0;
				PORTB &= ~_BV(PB2); // 00000100 apagamos B2
				_delay_ms(100);
			}
		}

		// >>>>>>>Dispensar el articulo 1 <<<<<<

		//Verificamos si hay que dar cambio
		if( (dinero_introducido >= COSTO_ARTICULO1 ) && (seleccion_articulo1 > 0) )
		{
			PORTB |= _BV(PB4); // 0b0010000 PIN B4
			_delay_ms(1000); // Esperamos a que se dispense el producto
			PORTB &= ~_BV(PB4);  // apagamos  PIN B4
			DarCambio(dinero_introducido, COSTO_ARTICULO1);
			dinero_introducido -= COSTO_ARTICULO1;
			seleccion_articulo1 = 0; // Time out
			PORTB &= ~_BV(PB2); // 00000100 apagamos B2

			PORTB &= ~_BV(PB0);// PIN B0 control obliterate lcd
			_delay_ms(100); 
			PORTB |= _BV(PB0);

		}


		// >>>>>>>Dispensar el articulo 2 <<<<<<

		//Verificamos si hay que dar cambio
		if(dinero_introducido >= COSTO_ARTICULO2 && seleccion_articulo2 > 0 )
		{
			PORTB |= _BV(PB5); // 0100000 PIN B5
			_delay_ms(1000); // Esperamos a que se dispense el producto
			PORTB &= ~_BV(PB5) ;  // Apagamos PIN B5
			DarCambio(dinero_introducido, COSTO_ARTICULO2);
			dinero_introducido -= COSTO_ARTICULO2;
			seleccion_articulo2 = 0; // Time out
			PORTB &= ~_BV(PB3); // 00010000 apagamos B3

			PORTB &= ~_BV(PB0);// PIN B0 control obliterate lcd
			_delay_ms(100); 
			PORTB |= _BV(PB0);		
		}


	// >>>>>>>Fin ciclo principal <<<<<<
	}
}


void DarCambio( int dinero, int costo)
{
	int diferencia = dinero - costo;

	while( diferencia > 0)
	{
		if(diferencia >= 10)
		{

			diferencia -= 10;
			dinero_introducido -= 10;

			PORTC |= _BV(PC3);		
			_delay_ms(1200); 
			PORTC &= ~_BV(PC3);

			_delay_ms(2*RETRASO_M1);			
		}

		if(diferencia >= 5)
		{
			// Codigo para mover motor de monedas de 5 pesos

			diferencia -= 5;
			dinero_introducido -= 5;

			PORTC |= _BV(PC4);		
			_delay_ms(1200); 
			PORTC &= ~_BV(PC4);

			_delay_ms(2*RETRASO_M2);

		}

		if(diferencia >= 1)
		{

			diferencia -= 1;
			dinero_introducido -= 1;

			PORTC |= _BV(PC5);		
			_delay_ms(1200); 
			PORTC &= ~_BV(PC5);

			_delay_ms(2*RETRASO_M3);

		}



	}

}
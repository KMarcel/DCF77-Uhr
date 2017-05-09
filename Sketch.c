/************************************************************************/
/* Projektname: DCF77 Uhr                                               */
/* Autor: Maurice T                                                     */
/* Datei: Main.c                                                        */
/************************************************************************/

#define XTAL  1000000		// Taktfrequenz für verschiedene Include Dateien (evtl. anpassen)
#define F_CPU 1000000		// andere Include Dateien brauchen F_CPU

#include <avr/io.h>			// alle Namen und Bit-Namen der Special Function Register (SFR)
#include <stdint.h>			// ganzahlige Datentypen
#include <stdbool.h>		// Definition von bool, true und false
#include <stdlib.h>			// für Funktionen wie itoa usw.
#include <util/delay.h>		// für delay Befehle
#include "lcd_pollin.h"
#include <avr/interrupt.h>

//Derektive
#define auswerteTaster (PIND & (1<<PD4))

//Globale Variblen
int tasterAlt = 0;
long Zaehler = 0;
long Zeit = 0;
bool Startsignal = false;
bool DCFBits[60];
int ZeigerWrite = 0;

uint8_t Sekunden = 0;						//Deklarieren der Sekunden
uint8_t Minuten  = 0;						//Deklarieren der Minuten
uint8_t Stunden  = 0;						//Deklarieren der Stunden

//////////////////////////////////////////////////////////////////////////
long millis()					//Zählt die Sekunden die seit dem Start vergangen sind
{
 return Zaehler;
}
ISR(TIMER1_COMPA_vect)
{
	Zaehler++;
}
//////////////////////////////////////////////////////////////////////////


void init()
{
	SFIOR = 0b00000100;		// interne Pullup-Widerstände deaktivieren
	/********************************************************************/
	DDRA = 0b00000000;		// Initalisierung der Ports
	DDRB = 0b00000000;		// Initalisierung der Ports
	DDRC = 0b00000000;		// Initalisierung der Ports
	DDRD = 0b11100000;		// Initalisierung der Ports
	/********************************************************************/
	lcd_init(LCD_DISP_ON);	
	/********************************************************************/
	MCUCR = 0b00001101;			// Einstellen der Interupts
	GICR = 0b10000000;
	TCCR1B = 0b00001111;
	TCCR1A = 0x00;
	TIMSK = 0b00010000;
	OCR1A = 255;
	sei();						//Interupts einschalten
	
	

}

void DCF77_init()
{
 GICR = 0b01000000;
}

void DCF77_Auswertung()
{
	Zeit = millis();			//Schreibt den derzeitigen Wert (Laufzeit) in die Variable "Zeit"
}

ISR(INT1_vect)
{
	lcd_gotoxy(0,0);				//Schreibt auf das Display das, dass DCF Signal Ausgewertet wird
	lcd_puts("DCF Signal");
	lcd_gotoxy(0,1);
	lcd_puts("wird Abgefragt");
	_delay_ms(500);
	lcd_puts(".");
	_delay_ms(500);
	lcd_puts(".");
	DCF77_Auswertung();
	DCF77_init();					//Kehrt nach beendigung des Schreibens zurück zur DCF init
}

ISR(INT0_vect)
{
	if (Startsignal == false)
	{
		if (millis() - Zeit > 800)		// Sucht nach der Startzeit 1 Sekunde HIGH (59 Sekunde)
		{
			ZeigerWrite = 0;
			Startsignal = true;			//Anfangssekunde gefunden dann Startwert auf true gesetzt
		}
		DCF77_Auswertung();
	}	
	
	
	
	
	else                                                        //Wenn Startsignal = 1 dann start der Auswertung der Bits
		{	
		if (millis() - Zeit > 90 && millis() - Zeit < 100)		//Setzt die Bits die ankomemn in ein Array 100ms = 0
		{
			DCFBits[ZeigerWrite] = 0;
		}
		if (millis() - Zeit > 100 && millis() - Zeit < 210)		//Setzt die Bits die ankommen in ein Array 200ms = 1
		{
			DCFBits[ZeigerWrite] = 1;
		}
		if (ZeigerWrite == 58)									//Nachdem alle 58 Bits angekomemn sind wird die Auswertung geschlossen
		{
			Startsignal = false;
			GICR = 0b10000000;
			
			Auswertung_Bits();
		}
	}	
}

/************************************************************************/
/* Auswertung der Bits                                                  */
/************************************************************************/
void Auswertung_Bits()
{
		
		Minuten =	DCFBits[21]*1+
					DCFBits[22]*2+
					DCFBits[23]*4+
					DCFBits[24]*8+
					DCFBits[25]*10+
					DCFBits[26]*20+
					DCFBits[27]*40;
					
		Stunden =	DCFBits[29]*1+
					DCFBits[30]*2+
					DCFBits[31]*4+
					DCFBits[32]*8+
					DCFBits[33]*10+
					DCFBits[34]*20;
	
}
/************************************************************************/
/* Hauptprogramm                                                        */
/************************************************************************/
int main()
{
	init();
	
	while (true)
	{
		char Puffer[10];
			
		lcd_gotoxy(0,0);						    //Erste Zeile als Schreib Zeile festlegen
		lcd_puts("           MEZ");				
				
		while(!(PIND & (1<< PD4)))					// Solange PD4 nicht auf 1 ist fürt er die while aus
		{			
			_delay_ms(990);
			Sekunden++;								//Zählt jede Sekunde eine Sekunde hoch

			if(Sekunden == 60){						//Wenn Sekunden 60 erreicht haben wird 												//eine Minute hochgezählt und die Sekunden wieder auf 0 gesetzt
				Sekunden = 0;
				Minuten ++;
			}

			if(Minuten == 60){						//Wenn Minuten 60 erreicht haben wird		
				Minuten = 0;
				Stunden ++;
			}

			if (Stunden == 24) {	
				Stunden = 0;
			}
			
			lcd_gotoxy(0,0);
			itoa(Sekunden,Puffer, 10);
			lcd_puts(Puffer);
					
			itoa(Minuten,Puffer, 10);
			lcd_puts(Puffer);
						
			itoa(Stunden,Puffer, 10);
			lcd_puts(Puffer);
			
			
		}
	}

}

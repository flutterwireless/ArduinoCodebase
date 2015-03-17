/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"


int main()
{
  setup();

  while(1)
  {
    loop();
  }


  return 0;
}

void setup( void )
{
  // Initialize the digital pin as an output.
  // Pin PIN_LED has a LED connected on most Arduino boards:
  for(int i = 0; i<21; i++)
  {
  pinMode( i, OUTPUT ) ;
  digitalWrite( i, LOW ) ;
  }

/*
uint32_t ulPin;
for(int i = 0; i<5; i++)
  {
    ulPin=i;
  PIO_Configure(
                g_APinDescription[ulPin].pPort,
                PIO_OUTPUT_1,
                g_APinDescription[ulPin].ulPin,
                g_APinDescription[ulPin].ulPinConfiguration ) ;

  PIO_SetOutput( g_APinDescription[ulPin].pPort, g_APinDescription[ulPin].ulPin, HIGH, 0, PIO_PULLUP ) ;

  }*/
  

  // Initialize the PIN_LED2 digital pin as an output.
  //pinMode( PIN_LED2, OUTPUT ) ;
  //digitalWrite( PIN_LED2, HIGH ) ;

 // Serial1.begin( 115200 ) ;
}



void loop( void )
{

 for(int i = 0; i<21; i++)
  {
  digitalWrite( i, LOW ) ;
  }

  for(int i = 0; i<21; i++)
  {
  digitalWrite( i, HIGH ) ;
  }

}

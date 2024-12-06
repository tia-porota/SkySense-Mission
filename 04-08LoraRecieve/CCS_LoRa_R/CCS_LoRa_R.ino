// se recomienda revisar el código emisor antes que el del receptor
#include "heltec.h" 
#include "images.h"
// para este módulo, sólo nos hacen falta las librerías LoRa
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
// el ancho de banda debe ser el mismo configurado en el emisor.

String rssi = "RSSI --"; //String de la intensidad de la señal
String packSize = "--"; //String del tamaño del mensaje en bytes
String packet ; //String que contendrá el mensaje recibido

#define cantDatos 8 //La cantidad de datos a recibirse, nosotros sabemos que vamos a recibir 8 porque enviamos 8, separados por : y !

void logo(){
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}// función predeterminada para mostrar el logo HELTEC
String temp,presion,altura,metano,calidad,alturaInit,latitud,longitud,dato;
// Varios strings que diferenciaran los datos recibidos, el último, dato, es una variable auxiliar
int pos1, pos2; // estas variables contendrán la posición de los caracteres para "cortar" el string del mensaje recibido, son auxiliares

void LoRaData(){ //esta función es la que se encarga de recibir y separar los datos, además de imprimirlos por consola y por pantalla
  
  delay(200);
  	for (int i=0;i<cantDatos;i++){ 
      pos1 = packet.indexOf(":")+1;
      pos2 = packet.indexOf("!");
      dato = packet.substring(pos1,pos2);	
      switch (i) {
        case 0:
          temp=dato;
          break;
        case 1:
          presion=dato;
          break;
        case 2:
          altura=dato;
          break;
        case 3:
          metano=dato;
          break;
        case 4:
          calidad=dato;
          break;
        case 5:
          alturaInit=dato;
          break;
        case 6:
          latitud=dato;
          break;
        case 7:
          longitud=dato;
          break;

      }
      if (i==cantDatos-1){
        break;
      }
      packet = packet.substring(pos2+1,packet.length());
    } //separa los datos y los asigna a su variable correspondiente.
    // toma la posición de los : y ! para saber dónde empieza y dónde termina cada dato.
    // los asigna según la posición de los mismos.
    // por último, elimina del mensaje los datos que ya asignó para no confundirlos.

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);

  Heltec.display->drawString(0, 0, rssi + " " + packSize + " bytes"); 
  Heltec.display->drawString(0 , 10 , "Temperatura: " + temp + " °C");
  Heltec.display->drawString(0 , 20 , "Presión: " + presion + " hPa");
  Heltec.display->drawString(0, 30, "Altura: " + altura + "m");
  Heltec.display->drawString(0, 40, "Latitud: " + latitud);
  Heltec.display->drawString(0, 48, "Longitud: " + longitud);
  // imprime por pantalla algunos de los datos más importantes
  // cumplen un propósito unicamente de testeo, ya que nuestro programa de estación terrena es el que cumplirá la tarea de mostrar los datos a modo de gráfico



  Serial.print(temp);
  Serial.print(",");
  Serial.print(presion);
  Serial.print(",");
  Serial.print(altura);
  Serial.print(",");
  Serial.print(metano);
  Serial.print(",");
  Serial.print(calidad);
  Serial.print(",");
  Serial.print(alturaInit);
  Serial.print(",");
  Serial.print(latitud);
  Serial.print(",");
  Serial.print(longitud);
  Serial.println();
  /* se imprimen por serial todos los datos recogidos, separados por comas (,), para que nuestro programa en Python los recoja y los asigne a cada gráfico
   ¿podríamos separar los datos directamente en el programa en Python y hacer todo el trabajo ahí?
   la respuesta es sí, ¿por qué lo hicimos de esta manera?
   para dividir la carga computacional, este LORA se encarga de recibir y separar los datos, y la computadora se encargará de mostrarlos
   Esto ayuda a alivianar el trabajo de cada componente*/

  Heltec.display->display();
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
} //esta función se encarga de decodificar los datos enviados por el emisor

void setup() { 
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, false /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  Serial.begin(9600);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Inicio Exitoso LoRa");
  Heltec.display->drawString(0, 10, "Esperando datos...");
  Heltec.display->display();
  delay(1000);
  LoRa.receive();
}

	
void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);  }
  delay(10);
  //en el loop, intentamos recibir datos cada 10 milésimas, hay que esperar un pequeño tiempo para no sobrecargar al módulo.
	}

  // Programado y testeado por Oviedo Verónica y Maccari Valentino.
  // Contacto: veroviedo935@gmail.com

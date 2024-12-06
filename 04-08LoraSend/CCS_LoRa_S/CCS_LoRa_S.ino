// Importamos todas las librerías que nos hacen falta para los sensores.
#include <Wire.h> //para las comunicaciones I2C
#include <Adafruit_BMP085.h> // para el BMP180
#include <TinyGPS++.h> // para el gps
#include "heltec.h" // esta librería y la de abajo sirven para la comunicación LORA
#include "images.h" 

#include "MQ4.h"
#include "MQ135.h"


#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
//la frecuencia a la cual se van a mandar los datos de una Lora a otra, en este caso, Argentina permite un ancho de banda de 915MHz

unsigned int i = 0; //contador de mensajes
String rssi = "RSSI --"; //Intensidad de la señal

TinyGPSPlus gps; // Variable tipo tinygpsplus para habilitar las funciones al nombre de gps

Adafruit_BMP085 bmp; //lo mismo, para el bmp180
MQ4 mq4(36,3.3); //Inicializamos el mq4, al pin 36 y trabajando a 3.3 voltios 
MQ135 mq135(37,3.3); //lo mismo para el mq135, conectado al pin 37 para la comunicacio´n


float t,p,a,m,c,a0,lon,lat = 0.0; //inicializamos variables que van a almacenar los datos de los sensores.
//temperatura, presion, altura, metano, calidad, altura inicial, longitud y latitud.
String mensaje; //variable que simboliza lo que se va a mandar al receptor contenida en un String

void logo()
{
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
} // función predeterminada, muestra el logo Heltec
void setup()
{
  // se inicializa el módulo LoRa ESP32
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, false /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Wire.begin(4,15); //Se inicializa el sensor BMP180 para empezar a sensar, en los pines 4 y 15
  Serial.begin(9600); //se inicializa el serial para manejar mensajes de error
  Serial2.begin(9600,SERIAL_8N1,13,12); //se inicializa un serial dedicado al gps, que está conectado al pin 13 y 12, trabajando a 9600 bauds

  if (!bmp.begin()){
    Serial.println("Fallo al iniciar BMP180 :( ");
    while (true) {}
  } // si el bmp tiene algún fallo al inicializarse, imprime el mensaje por serial del error y no continúa.

  a0 = bmp.readAltitude(102900); // la altura inicial, es una constante, usando la función de leer altitud según la presión sobre el nivel del mar al momento de prender el módulo
  // en Argentina es de 102900 mbar

 

  Heltec.display->init(); 
  //Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Inicio Exitoso LoRa. Cargando...");
  Heltec.display->display();

  mq4.calibrate(); //se calibra el mq4, tarda unos 15 segundos, se puede ajustar en las librerías incluidas
  mq135.calibrate(); // lo mismo para el mq135, tarda, también unos 15 segundos.
  // por lo que el total para iniciarse en su totalidad el módulo y empezar a sensar ronda los 40 segundos.

  delay(1000);
}



void loop()
{

  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, "Paquete N°: ");
  Heltec.display->drawString(90, 0, String(i));
  Heltec.display->drawStringMaxWidth(0,20,128,mensaje);
  Heltec.display->display();
  //funciones que tienen que ver con la pantalla del módulo, su propósito es de testeo, ya que no podremos ver el display cuando nuestro CANSAT esté en el aire.

  t = bmp.readTemperature(); //lee la temperatura actual, en grados centígrados
  p = bmp.readPressure()/100.0; // lee la presión actual y la divide en 100, para convertirla a mbar
  a = bmp.readAltitude(102900); // la altura actual según la presión sobre el nivel del mar.
  a-=a0; // se resta este valor a la altura inicial para obtener un valor con sentido.
  //al iniciarse el módulo obtiene una "altura 0", se resta al valor actual y si desde que se inició, el CANSAT subió 10 metros, el resultado de la ecuación es de 10 metros.
  m = mq4.readPpm(); //lee el valor actual del metano en PPM
  c = mq135.readPpm(); // lee el valor actual del aire en PPM (mientras más alto el valor, peor la calidad del aire)
  delay(50);
  if (Serial2.available() && gps.encode(Serial2.read())){
  //  Si el serial2 está disponible, se le manda 1 byte de datos al GPS, si esta tarea tiene éxito:
    if (gps.location.isValid()) // pregunta si la localización es válida (un valor inválido es latitud 0.000000, longitud 0.000000)
    {
      lat = gps.location.lat(); 
      lon = gps.location.lng();
      // si está todo "bien", guarda la latitud y longitud actual en cada variable.
    }
  else
  {
    Serial.println("Ubicación inválida");//si no, imprime por serial ese mensaje.
  } // cabe destacar, que aunque de un momento a otro el GPS llegase a desconectarse de los satélites geolocalizadores, nunca reemplazará datos reales por datos
  // inválidos, por lo que tendremos una ubicación estimada real y no una en 0.000000/0.000000
  }
  delay(50);



  //---------ENVÍO DE DATOS---------//

  // "T:17.1! P:951.12! CH4:15.222! H:1.2! H0:576.656!";
  // temperatura, presion, altura, metano, calidad, presion init, latitud, longitud

  // guarda en un string, los datos en el formato que se puede apreciar en la línea 112, cada dato separado por : y !, es interpretado por el receptor.
  mensaje = "T:"+ String(t)+ "!P:" + String(p) + "!A:"+ String(a) + "!CH4: " + String(m) +"!Q:"+String(c) +"!Ai:"+ String(a0) +"!lat:"+String(lat,6)+"!lon:"+String(lon,6)+"!";

  
  LoRa.beginPacket(); //Inicia el mensaje que le va a mandar al receptor.
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print(mensaje); //El mensaje a enviarse
  LoRa.endPacket(); // Termina el mensaje, procede a enviarse


  i++;



  
  delay(50);
}
  // Programado y testeado por Oviedo Verónica y Maccari Valentino.
  // Contacto: veroviedo935@gmail.com
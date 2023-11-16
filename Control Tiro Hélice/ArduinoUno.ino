#include <ESP8266WiFi.h>                  //Incluye la librería ESP8266WiFi
#include "string.h"
#include <SPI.h>
#include <RF24.h>

const char ssid[] = "NodeMCU-ESP8266";    //Definimos la SSDI de nuestro servidor WiFi -nombre de red-
const char password[] = "12345678";       //Definimos la contraseña de nuestro servidor
WiFiServer server(80);                    //Definimos el puerto de comunicaciones
int i, j, k, l;
char Estacion, Estacion_ok;
String AngDer, AngDer_ok, AngIzq, AngIzq_ok, Vel, Vel_ok;

//Pines de conexion antena
#define CE_PIN 9
#define CSN_PIN 10 //pines de conexion antena

RF24 radio(CE_PIN, CSN_PIN);

byte direccion[5] = {'c', 'h', 'a', 'n', '1'}; // chan1 direccion 1 por defecto

//Paquete a enviar
struct MyData {
  int AngDer;
  int AngIzq;
  int Vel;
};

MyData data;

void setup() {
  Serial.begin(115200);
  radio.begin();

  server.begin();                         //inicializamos el servidor
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);            //Red con clave, en el canal 1 y visible

  Serial.println();
  Serial.print("Direccion IP Access Point - por defecto: ");      //Imprime la dirección IP
  Serial.println(WiFi.softAPIP());
  Serial.print("Direccion MAC Access Point: ");                   //Imprime la dirección MAC
  Serial.println(WiFi.softAPmacAddress());

}

void loop() {

  // Comprueba si el cliente ha conectado
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Espera hasta que el cliente envía alguna petición
  while (!client.available()) {
    delay(1);
  }

  // Leo la cadena de la peticion y busco palabras claves
  String peticion = client.readString();
  i = peticion.indexOf("Estacion=");
  j = peticion.indexOf("AngDer=");
  k = peticion.indexOf("AngIzq=");
  l = peticion.indexOf("Vel=");

  // una vez detectada la palabras leo lo que puede llegar a ser datos
  Estacion = peticion[i + 9];
  if ((Estacion >= '1') && (Estacion <= '5')) {
    Estacion_ok = Estacion;
  }
  else {
    Estacion_ok = '\0';
  }


  AngDer = "";
  AngDer += peticion[j + 7];
  AngDer += peticion[j + 8];
  if ((AngDer[0] >= '0') && (AngDer[0] <= '8')) {
    AngDer_ok = AngDer;
  }
  else
  {
    AngDer_ok = "";
  }

  if ((AngDer[1] >= '0') && (AngDer[1] <= '9')) {
    AngDer_ok[1] = AngDer[1];
  }
  else
  {
    AngDer_ok[1] = '\0'; //corto la caden antes
  }

  if (AngDer[0] == '8' && AngDer[0] != 0 ) {
    AngDer_ok[1] = '0'; //limito a maximo 80 si se pasa
  }


  AngIzq = "";
  AngIzq += peticion[k + 7];
  AngIzq += peticion[k + 8];
  if ((AngIzq[0] >= '0') && (AngIzq[0] <= '8')) {
    AngIzq_ok = AngIzq;
  }
  else
  {
    AngIzq_ok = "";
  }

  if ((AngIzq[1] >= '0') && (AngIzq[1] <= '9')) {
    AngIzq_ok[1] = AngIzq[1];
  }
  else
  {
    AngIzq_ok[1] = '\0'; //corto la caden antes
  }

  if (AngIzq[0] == '8' && AngIzq[0] != 0 ) {
    AngIzq_ok[1] = '0'; //limito a maximo 80 si se pasa
  }


  Vel = "";
  Vel += peticion[l + 4];
  Vel += peticion[l + 5];
  if ((Vel[0] >= '0') && (Vel[0] <= '9')) {
    Vel_ok = Vel;
  }
  else
  {
    Vel_ok = "";
  }

  if ((Vel[1] >= '0') && (Vel[1] <= '9')) {
    Vel_ok[1] = Vel[1];
  }
  else
  {
    Vel_ok[1] = '\0'; //corto la caden antes
  }


  Serial.print("Estacion ");
  Serial.println(Estacion_ok);
  Serial.print("Ángulo derecha ");
  Serial.println(AngDer_ok);
  Serial.print("Ángulo izquierda ");
  Serial.println(AngIzq_ok);
  Serial.print("Velocidad ");
  Serial.println(Vel_ok);

  //Seteo la radio en un canal
  if (Estacion_ok != '\0') {
    direccion[4] = Estacion_ok;
  }
  if ((AngDer_ok != "") || (AngIzq_ok != "") || (Vel_ok != "")) {

    //Cargo los datos del paquete
    data.AngDer = AngDer.toInt();
    data.AngIzq = AngIzq.toInt();
    data.Vel = Vel.toInt();

    //Envío el paquete
    bool ok = radio.write(&data, sizeof(MyData));
    //reportamos por el puerto serial los datos enviados
 
    if (ok)
    {
      Serial.println("Datos enviados: ");
      Serial.print("Estacion ");
      Serial.println(Estacion_ok);
      Serial.print("Ángulo derecha ");
      Serial.println(data.AngDer);
      Serial.print("Ángulo izquierda ");
      Serial.println(data.AngIzq);
      Serial.print("Velocidad ");
      Serial.println(data.Vel);
    }
    else
    {
      Serial.println("no se ha podido enviar");
    }
    delay(1000);
  }
  client.flush();


  // Envía la página HTML de respuesta al cliente
  client.println("HTTP/1.1 200 OK");
  client.println("");                                    //No olvidar esta línea de separación
  client.println("<!DOCTYPE HTML>");
  client.println("<meta charset='UTF-8'>");
  client.println("<html>");
  client.println("<head meta name='viewport' content='width=device-width, initial-scale=1.0'&amp;gt;>");
  client.println("<title>Tiro a la Hélice</title>");
  client.println("</head>");
  client.println("<center>");
  client.println("<body bgcolor='blue'>");
  client.println("<form action='?' method='post'>"); // ? para llamarse a si mismo a la hora de hacer el get
  client.println("<h1><font color='white'>Tiro a la Hélice</h1>");
  client.println("<h2>SELECCIONE ESTACIÓN</h2>");
  client.println("<select name='Estacion'>");
  client.println("<option value='1'>Estación 1</option>");
  client.println("<option value='2'>Estación 2</option>");
  client.println("<option value='3'>Estación 3</option>");
  client.println("<option value='4'>Estación 4</option>");
  client.println("<option value='5'>Estación 5</option>");
  client.println("</select>");
  client.println("<h2>ÁNGULO DERECHA</h2>");
  client.println("<input type='text' id='AngDer' name='AngDer' maxlength='2'><br>");
  client.println("<h2>ÁNGULO IZQUIERDA</h2>");
  client.println("<input type='text' id='AngIzq' name='AngIzq' maxlength='2'><br>");
  client.println("<h2>VELOCIDAD</h2>");
  client.println("<input type='text' id='Vel' name='Vel'  maxlength='2'><br>");
  client.println("&nbsp<br>");
  client.println("<input type='submit' value='Enviar'>");
  client.println("</form>");
  client.println("</body>");
  client.println("</center>");
  client.println("</html>");
  delay(10);
}

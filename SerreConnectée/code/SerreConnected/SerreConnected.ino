#include <WiFi.h>
#include <TimeLib.h>
#include <WebServer.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4 // Broche à laquelle le capteur DS18B20 est connecté

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


const char *ssid = "ESP32AP"; // Nom du point d'accès WiFi
const char *password = "12345678"; // Mot de passe du point d'accès

WebServer server(80);

const int relayPinLight = 27; // Numéro de la broche GPIO connectée au relais
const int relayPinVentilateur = 33; // Numéro de la broche GPIO connectée au relais
const int relayPinPompe = 32; // Numéro de la broche GPIO connectée au relais

int sensorPinHumidity = 34; // Numéro de la broche GPIO connectée au capteur d'humidité

int value = 0;
int humidityPercentage = 0;

bool isPumpOn = false; // Variable pour suivre l'état de la pompe
bool isFanOn = false; // Variable pour suivre l'état du ventilateur 
bool isLightOn = false; // Variable pour suivre l'état de la lumière


void setup() {
  pinMode(relayPinLight, OUTPUT);
  pinMode(relayPinVentilateur, OUTPUT);
  pinMode(relayPinPompe, OUTPUT);

  digitalWrite(relayPinLight, LOW); // Initialise le relais à l'état éteint
  digitalWrite(relayPinVentilateur, LOW); // Initialise le relais à l'état éteint
  digitalWrite(relayPinPompe, LOW); // Initialise le relais à l'état éteint

  sensors.begin();
  delay(1000);

  Serial.begin(9600);
  delay(10);
  Serial.println("Initialisation...");

  // Configuration du point d'accès Wi-Fi
  WiFi.softAP(ssid, password);

  Serial.print("Adresse IP du serveur : 192.168.4.1");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    Serial.println("Requête HTTP reçue pour la racine.");
    String html = "<!DOCTYPE html><html><head><title>Arrose Ta plante</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; // Ajuste la vueport pour les appareils mobiles
    html += "<style>";
    html += "html, body {";
    html += "  height: 100%;";
    html += "  margin: 0;";
    html += "  font-family: Arial, sans-serif;";
    html += "}";
    html += "body {";
    html += "  background: linear-gradient(45deg, #1c64f2, #40d8f4);";
    html += "  display: flex;";
    html += "  justify-content: center;";
    html += "  align-items: center;";
    html += "  position: relative;"; // Ajout d'une position relative pour les bulles
    html += "}";
    html += ".container {";
    html += "  text-align: center;";
    html += "}";
    html += "h1 {";
    html += "  color: green;"; // Couleur verte pour le titre
    html += "}";
    html += "button {";
    html += "  font-size: 16px;";
    html += "  padding: 10px 20px;";
    html += "  margin: 10px;";
    html += "}";
    html += ".bubble {";
    html += "  position: absolute;"; // Position absolue pour les bulles
    html += "  background-color: rgba(255, 255, 255, 0.5);"; // Couleur des bulles
    html += "  border-radius: 50%;"; // Forme des bulles
    html += "  animation: bubbleAnimation 10s linear infinite alternate;"; // Animation des bulles
    html += "}";
    html += "@keyframes bubbleAnimation {";
    html += "  0% {";
    html += "    transform: translateY(100vh);"; // Départ en bas de l'écran
    html += "  }";
    html += "  100% {";
    html += "    transform: translateY(-100vh);"; // Arrivée en haut de l'écran
    html += " }";
    html += "}";
    html += "</style>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Arrose Ta plante</h1>";
    // Ajout des boutons
    html += "<form action='/pump' method='get'><button name='state' value='";
    html += isPumpOn ? "0" : "1";
    html += "'>";
    html += isPumpOn ? "Eteindre la pompe " : "Allumer la pompe";
    html += "</button></form>";
    html += "<form action='/fan' method='get'><button name='state' value='";
    html += isFanOn ? "0" : "1";
    html += "'>";
    html += isFanOn ? "Eteindre le ventilateur" : "Allumer le ventilateur";
    html += "</button></form>";
    html += "<form action='/light' method='get'><button name='state' value='";
    html += isLightOn ? "0" : "1";
    html += "'>";
    html += isLightOn ? "Eteindre la lumiere" : "Allumer la lumiere";
    html += "</button></form>";
    html += "<h2 id='humidity'></h2>";
    html += "<h2 id='temperature'></h2>"; // Balise pour afficher la température
    html += "</div>";
    // Ajout de 10 bulles
    for (int i = 0; i < 10; ++i) {
    int randomSize = random(10, 30); // Taille aléatoire des bulles entre 10px et 30px
    int randomDelay = random(0, 20); // Délai aléatoire pour le départ des bulles
    html += "<div class='bubble' style='left: " + String(random(0, 100)) + "%; width: " + String(randomSize) + "px; height: " + String(randomSize) + "px; animation-delay: " + String(randomDelay) + "s;'></div>";
    }
    html += "<script>";
    html += "function togglePump(state) {";
    html += " var xhttp = new XMLHttpRequest();";
    html += " xhttp.onreadystatechange = function() {";
    html += " if (this.readyState == 4 && this.status == 200) {";
    html += " document.getElementById('humidity').innerHTML = 'Humidite: ' + this.responseText + '%';";
    html += " }";
    html += " };";
    html += " xhttp.open('GET', '/pump?state=' + state, true);";
    html += " xhttp.send();";
    html += "}";
    html += "function updateHumidity() {";
    html += " var xhttp = new XMLHttpRequest();";
    html += " xhttp.onreadystatechange = function() {"; 
    html += " if (this.readyState == 4 && this.status == 200) {";
    html += " document.getElementById('humidity').innerHTML = 'Humidite: ' + this.responseText + '%';";
    html += " }";
    html += " };";
    html += " xhttp.open('GET', '/humidity', true);"; 
    html += " xhttp.send();";
    html += "}";
    html += "function updateTemperature() {"; // Fonction pour mettre à jour la température
    html += " var xhttp = new XMLHttpRequest();";
    html += " xhttp.onreadystatechange = function() {";
    html += " if (this.readyState == 4 && this.status == 200) {";
    html += " document.getElementById('temperature').innerHTML = 'Temperature: ' + this.responseText + '°C';";
    html += " }";
    html += " };";
    html += " xhttp.open('GET', '/temperature', true);";
    html += " xhttp.send();";
    html += "}";
    html += "setInterval(updateHumidity, 1000);"; // Mettre à jour l'humidité toutes les secondes
    html += "setInterval(updateTemperature, 1000);"; // Mettre à jour la température toutes les secondes
    html += "</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
});


  // -----------Actionneur-----------
  //-----------pump---------
  server.on("/pump", HTTP_GET, []() {
    String state = server.arg("state");
    Serial.print("Received pump state: ");
    Serial.println(state);
    
    if (state == "1") {
      digitalWrite(relayPinPompe, HIGH); // Allume la pompe
      isPumpOn = true;
      Serial.println("Pompe allumée!");
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      digitalWrite(relayPinPompe, LOW); // Éteint la pompe
      isPumpOn = false;
      Serial.println("Pompe éteinte!");
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });

  server.on("/fan", HTTP_GET, []() {
    String state = server.arg("state");
    Serial.print("Received fan state: ");
    Serial.println(state);
    
    if (state == "1") {
      digitalWrite(relayPinVentilateur, HIGH); // Allume le ventilateur
      isFanOn = true;
      Serial.println("Ventilateur allumé !");
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      digitalWrite(relayPinVentilateur, LOW); // Éteint le ventilateur
      isFanOn = false;
      Serial.println("Ventilateur eteint !");
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });

  server.on("/light", HTTP_GET, []() {
    String state = server.arg("state");
    Serial.print("Received light state: ");
    Serial.println(state);
    
    if (state == "1") {
      digitalWrite(relayPinLight, HIGH); // Allume la lumière
      isLightOn = true;
      Serial.println("Lumière allumée !");
      server.sendHeader("Location", "/");
      server.send(303);
    } else {
      digitalWrite(relayPinLight, LOW); // Éteint la lumière
      isLightOn = false;
      Serial.println("Lumière éteinte !");
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });

  //-------Capter---------

  server.on("/humidity", HTTP_GET, []() {
    value = analogRead(sensorPinHumidity);
    humidityPercentage = map(value, 0, 2250, 100, 0); // Conversion de la valeur analogique en pourcentage
    humidityPercentage = abs(humidityPercentage);
    Serial.println(humidityPercentage);
    server.send(200, "text/plain", String(humidityPercentage)); // Envoyer la valeur d'humidité à la page web
});

// Ajouter un gestionnaire pour l'URL '/temperature'
server.on("/temperature", HTTP_GET, []() {
    // Lecture de la température depuis le capteur
    sensors.requestTemperatures(); // Demander au capteur de mesurer la température
    float tempC = sensors.getTempCByIndex(0); // Obtenir la température en degrés Celsius

    
    // Envoyer la température à la page web
    server.send(200, "text/plain", String(tempC)); 
});

  server.begin();
  Serial.println("Serveur démarré !");

}



void loop() {
  server.handleClient();

  sensors.requestTemperatures(); // Demander au capteur de mesurer la température
  float tempC = sensors.getTempCByIndex(0); // Obtenir la température en degrés Celsius

  // Afficher la température dans la console
  Serial.print("Température : ");
  Serial.print(tempC);
  Serial.println(" °C");
  /*
  
  // Vérifier si la température est supérieure ou égale à 26 degrés Celsius
  if (tempC >= 26) {
    digitalWrite(relayPinVentilateur, HIGH); // Allumer le ventilateur
    delay(5000); // Attendre 5 secondes
    digitalWrite(relayPinVentilateur, LOW); // Éteindre le ventilateur
  }
  */
  
  value = analogRead(sensorPinHumidity);
  humidityPercentage = map(value, 0, 2250, 100, 0); // Conversion de la valeur analogique en pourcentage
  Serial.println(abs(humidityPercentage));

  /*
  
  // Vérifier si l'humidité est égale ou inférieure à 30 %
  if (abs(humidityPercentage) <= 40) {
    digitalWrite(relayPinPompe, HIGH); // Allumer la pompe
    delay(5000); // Attendre 5 secondes
    digitalWrite(relayPinPompe, LOW); // Éteindre la pompe
  }
  */

  // Obtenir l'heure actuelle
  int currentHour = hour();
  int currentMinute = minute();

  // Vérifier si l'heure est 16h15
  if (currentHour == 19 && currentMinute == 0) {
    digitalWrite(relayPinVentilateur, HIGH); // Allumer le ventilateur
    digitalWrite(relayPinLight, HIGH); // Allumer la lampe
    digitalWrite(relayPinPompe, HIGH); // Allumer la pompe
    delay(5000); // Attendre 5 secondes
    digitalWrite(relayPinPompe, LOW); // Éteindre la pompe
  }
  if (currentHour == 0 && currentMinute = 0){
    digitalWrite(relayPinVentilateur, LOW); // Éteindre le ventilateur
    digitalWrite(relayPinLight, LOW); //éteindre la lampe
    
  }
}


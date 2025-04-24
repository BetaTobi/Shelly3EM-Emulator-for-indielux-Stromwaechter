#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// Netzwerk-SSID und Passwort
const char* ssid = "Dein_SSID";
const char* password = "Dein_Passwort";

// MQTT-Broker Informationen
const char* mqtt_server = "Deine_MQTT_Broker_IP_Adresse";
const int mqtt_port = 1883;
const char* mqtt_user = "MQTT_Benutzer";
const char* mqtt_password = "MQTT_Passwort";

// MQTT-Topics
const char* mqtt_topic_current_1 = "mqtt_topic/to/value_of/current_of/l1";
const char* mqtt_topic_current_2 = "mqtt_topic/to/value_of/current_of/l2";
const char* mqtt_topic_current_3 = "mqtt_topic/to/value_of/current_of/l3";
const char* mqtt_topic_voltage_1 = "mqtt_topic/to/value_of/voltage_of/l1";
const char* mqtt_topic_voltage_2 = "mqtt_topic/to/value_of/voltage_of/l2";
const char* mqtt_topic_voltage_3 = "mqtt_topic/to/value_of/voltage_of/l3";
const char* mqtt_topic_power_1 = "mqtt_topic/to/value_of/power_of/l1";
const char* mqtt_topic_power_2 = "mqtt_topic/to/value_of/power_of/l2";
const char* mqtt_topic_power_3 = "mqtt_topic/to/value_of/power_of/l3";
const char* mqtt_topic_pf_1 = "mqtt_topic/to/value_of/powerfactor_of/l1";
const char* mqtt_topic_pf_2 = "mqtt_topic/to/value_of/powerfactor_of/l2";
const char* mqtt_topic_pf_3 = "mqtt_topic/to/value_of/powerfactor_of/l3";

// Erstellen des Webservers auf Port 80
WebServer server(80);

// MQTT-Client
WiFiClient espClient;
PubSubClient client(espClient);

// Variablen, um den Status und die empfangenen MQTT-Werte zu speichern
bool statusReceived = false;
float current_1 = 0.0;
float current_2 = 0.0;
float current_3 = 0.0;
float voltage_1 = 0.0;
float voltage_2 = 0.0;
float voltage_3 = 0.0;
float power_1 = 0.0;
float power_2 = 0.0;
float power_3 = 0.0;
float pf_1 = 0.0;
float pf_2 = 0.0;
float pf_3 = 0.0;



void setup() {
  // Serielle Kommunikation starten
  Serial.begin(115200);
  
  // Verbinden mit dem WLAN
  WiFi.begin(ssid, password);
  Serial.print("Verbindung zu WiFi wird hergestellt");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("Mit WiFi verbunden");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
  
  // Route für Status festlegen
  server.on("/status", HTTP_GET, handleJson);

  // Webserver starten
  server.begin();
  Serial.println("HTTP-Server gestartet");

  // MQTT-Client konfigurieren
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  // Mit MQTT-Broker verbinden
  reconnect();
}

void loop() {
  // Server-Client-Handling
  server.handleClient();

  // MQTT-Client-Verbindung prüfen und auf Nachrichten warten
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void handleJson() {
  // Setze den Status auf true, wenn die /status Route aufgerufen wird
  statusReceived = true;
  
  // Überprüfen, ob der Status empfangen wurde
  if (statusReceived) {
    // Erstellen eines JSON-Dokuments
    StaticJsonDocument<768> doc;

    JsonObject relays_0 = doc["relays"].add<JsonObject>();
    relays_0["ison"] = false;
    relays_0["has_timer"] = false;
    relays_0["timer_started"] = 0;
    relays_0["timer_duration"] = 0;
    relays_0["timer_remaining"] = 0;
    relays_0["overpower"] = false;
    relays_0["is_valid"] = true;
    relays_0["source"] = "http";

    JsonArray emeters = doc["emeters"].to<JsonArray>();

    JsonObject emeters_0 = emeters.add<JsonObject>();
    emeters_0["power"] = power_1;
    emeters_0["pf"] = pf_1;
    emeters_0["current"] = abs(current_1);
    emeters_0["voltage"] = voltage_1;
    emeters_0["is_valid"] = true;
    emeters_0["total"] = 0;
    emeters_0["total_returned"] = 0;

    JsonObject emeters_1 = emeters.add<JsonObject>();
    emeters_1["power"] = power_2;
    emeters_1["pf"] = pf_2;
    emeters_1["current"] = abs(current_2);
    emeters_1["voltage"] = voltage_2;
    emeters_1["is_valid"] = true;
    emeters_1["total"] = 0;
    emeters_1["total_returned"] = 0;

    JsonObject emeters_2 = emeters.add<JsonObject>();
    emeters_2["power"] = power_3;
    emeters_2["pf"] = pf_3;
    emeters_2["current"] = abs(current_3);
    emeters_2["voltage"] = voltage_3;
    emeters_2["is_valid"] = true;
    emeters_2["total"] = 0;
    emeters_2["total_returned"] = 0;
    doc["total_power"] = (power_1 + power_2 + power_3);
    doc["fs_mounted"] = true;

    // Serialisieren des JSON-Dokuments in einen String
    String json;
    serializeJson(doc, json);

    // Senden der JSON-Antwort
    server.send(200, "application/json", json);
    
    // Status zurücksetzen
    statusReceived = false;
  } else {
    // Falls der Status nicht empfangen wurde, wird eine Fehlermeldung gesendet
    server.send(403, "text/plain", "Unauthorized: Please access /status first");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // MQTT-Nachricht in einen String konvertieren
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Je nach Topic den entsprechenden Wert speichern
  if (strcmp(topic, mqtt_topic_current_1) == 0) {
    current_1 = message.toFloat();
    Serial.print("Current 1 empfangen: ");
    Serial.println(current_1);
  } else if (strcmp(topic, mqtt_topic_current_2) == 0) {
    current_2 = message.toFloat();
    Serial.print("Current 2 empfangen: ");
    Serial.println(current_2);
  } else if (strcmp(topic, mqtt_topic_current_3) == 0) {
    current_3 = message.toFloat();
    Serial.print("Current 3 empfangen: ");
    Serial.println(current_3);
  } else if (strcmp(topic, mqtt_topic_voltage_1) == 0) {
    voltage_1 = message.toFloat();
    Serial.print("Voltage 1 empfangen: ");
    Serial.println(voltage_1);
  } else if (strcmp(topic, mqtt_topic_voltage_2) == 0) {
    voltage_2 = message.toFloat();
    Serial.print("Voltage 2 empfangen: ");
    Serial.println(voltage_2);
  } else if (strcmp(topic, mqtt_topic_voltage_3) == 0) {
    voltage_3 = message.toFloat();
    Serial.print("Voltage 3 empfangen: ");
    Serial.println(voltage_3);
  } else if (strcmp(topic, mqtt_topic_power_1) == 0) {
    power_1 = message.toFloat();
    Serial.print("Power 1 empfangen: ");
    Serial.println(power_1);
  } else if (strcmp(topic, mqtt_topic_power_2) == 0) {
    power_2 = message.toFloat();
    Serial.print("Power 2 empfangen: ");
    Serial.println(power_2);
  } else if (strcmp(topic, mqtt_topic_power_3) == 0) {
    power_3 = message.toFloat();
    Serial.print("Power 3 empfangen: ");
    Serial.println(power_3);
  } else if (strcmp(topic, mqtt_topic_pf_1) == 0) {
    pf_1 = message.toFloat();
    Serial.print("PF 1 empfangen: ");
    Serial.println(pf_1);
  } else if (strcmp(topic, mqtt_topic_pf_2) == 0) {
    pf_2 = message.toFloat();
    Serial.print("PF 2 empfangen: ");
    Serial.println(pf_2);
  } else if (strcmp(topic, mqtt_topic_pf_3) == 0) {
    pf_3 = message.toFloat();
    Serial.print("PF 3 empfangen: ");
    Serial.println(pf_3);
  }
}

void reconnect() {
  // Solange versuchen, bis eine Verbindung hergestellt wird
  while (!client.connected()) {
    Serial.print("Verbindung zu MQTT-Broker wird hergestellt...");
    
    // Eindeutige Client-ID basierend auf der MAC-Adresse erzeugen
    String clientId = "ESP32Client-" + WiFi.macAddress();
    clientId.replace(":", ""); // Entferne Doppelpunkte

    Serial.print("Verwende MQTT-ClientID: ");
    Serial.println(clientId);

    // Verbindung zum MQTT-Broker herstellen
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Verbunden");

      // Abonnieren der MQTT-Topics
      client.subscribe(mqtt_topic_current_1);
      client.subscribe(mqtt_topic_current_2);
      client.subscribe(mqtt_topic_current_3);
      client.subscribe(mqtt_topic_voltage_1);
      client.subscribe(mqtt_topic_voltage_2);
      client.subscribe(mqtt_topic_voltage_3);
      client.subscribe(mqtt_topic_power_1);
      client.subscribe(mqtt_topic_power_2);
      client.subscribe(mqtt_topic_power_3);
      client.subscribe(mqtt_topic_pf_1);
      client.subscribe(mqtt_topic_pf_2);
      client.subscribe(mqtt_topic_pf_3);
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" - erneuter Versuch in 5 Sekunden");
      
      // Wartezeit vor erneutem Verbindungsversuch
      delay(5000);
    }
  }
}

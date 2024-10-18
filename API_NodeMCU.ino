#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

//Wi-Fi Connection
char ssid[] = "Kenza's Life Support";
char password[] = "12345678";

WiFiClientSecure client;
HTTPClient http;

// WAQI API
const String apiKey = "f0f379247edc077dbb1b621e901a4695ec73a96d";
const String city = "amsterdam";  // Replace with your city or coordinates
String apiURL = "https://api.waqi.info/feed/" + city + "/?token=" + apiKey;

// Google Sheet URL
const char* googleSheetsURL = "https://script.google.com/macros/s/AKfycbwxLlLhSCrMJOAvS3ZTdYhWsI3dvGhmAryu0S2fXxjQ-H58T1ktkLZnuar9h0BZ6Xvi/exec";  // Replace with your Google Sheets web app URL


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    int status = WiFi.status();
    Serial.print(" WiFi status: ");
    Serial.println(status);  // If the Wi-Fi code is not 3, or shows "WiFi connected" in the Serial Monitor, Google the Wi-Fi code that is given.
  }
  Serial.println("\nWiFi Connected");

client.setInsecure();  // For demo purposes, this disables certificate verification (not recommended for production)
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Fetch data from WAQI API
    http.begin(client, apiURL);  // Use secure client for HTTPS
    http.setTimeout(15000);      // Set timeout for the request
    int httpCode = http.GET();
    
    Serial.print("HTTP GET Status Code: ");
    Serial.println(httpCode);
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Air Quality Data:");
      Serial.println(payload);

      // Parse the JSON data from WAQI API
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      JsonObject data = doc["data"];
      String AQI = data["aqi"];            // Air Quality Index
      String PM25 = data["iaqi"]["pm25"]["v"];  // PM2.5 value
      String PM10 = data["iaqi"]["pm10"]["v"];  // PM10 value

      // Prepare JSON to send to Google Sheets
      String jsonData = "{\"AQI\":\"" + AQI + "\", \"PM25\":\"" + PM25 + "\", \"PM10\":\"" + PM10 + "\"}";

      // Send data to Google Sheets
      http.begin(client, googleSheetsURL);  // Pass the WiFiClientSecure object for HTTPS
      http.addHeader("Content-Type", "application/json");
      int postCode = http.POST(jsonData);

      Serial.print("HTTP POST Status Code: ");
      Serial.println(postCode);

      if (postCode > 0) {
        Serial.println("Data sent to Google Sheets successfully!");
      } else {
        Serial.println("Error sending data to Google Sheets.");
      }
      http.end();
    } else {
      Serial.println("Error fetching data from API.");
    }

    http.end();
  }

  delay(60000);  // Wait for 1 minute before the next request
}
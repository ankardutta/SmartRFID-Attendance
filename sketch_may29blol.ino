#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>

// --- LCD Library Includes ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN D3
#define SS_PIN D4
#define BUZZER_PIN D8

// WiFi credentials
const char* ssid = realme C3";
const char* password = "abcdefgh";

const String scriptURL = "https://script.google.com/macros/s/AKfycbyHfRsP4es-Q0McZnTg8YBBOI9lfTK8oD-EmAJ-yUv4hI_fZ91sNNqD1LUQXQFrzSEu/exec"; // Re-checking the script URL, it might have changed after testing. Please ensure this is correct.

MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiUDP ntpUDP; // UDP client for NTP
NTPClient timeClient(ntpUDP, "time.google.com", 19800); // 19800 seconds = 5 hours 30 minutes offset for IST

// --- LCD Object Initialization ---
// IMPORTANT: Use the address found by your I2C scanner!
// SDA (data) is D2 (GPIO4) and SCL (clock) is D1 (GPIO5) for this configuration.
LiquidCrystal_I2C lcd(0x27, 16, 2); // <--- CHANGE THIS LINE TO 0x79 !!!

void beepBuzzer(int times = 1);
void reconnectWiFi();
String getUID();
String getSlotByTime();
void showGreeting(String slot);
void sendToGoogleSheet(String uid, String slot);
String urlEncode(String str);

void setup() {
  Serial.begin(115200); // Initialize Serial Monitor for debugging
  Serial.println("\n--- System Booting ---"); // Initial Serial message

  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // --- LCD Setup ---
  // Using D2 (GPIO4) for SDA and D1 (GPIO5) for SCL
  Wire.begin(4, 5); // Initialize I2C: SDA on GPIO4 (D2), SCL on GPIO5 (D1)
  lcd.init();      // Initialize the LCD
  lcd.backlight(); // Turn on the backlight (already confirmed this works)
  lcd.clear();     // Clear any garbage on startup
  lcd.print("System Starting"); // Initial LCD message
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");


  // Connect to WiFi
  Serial.print("Connecting to WiFi"); // Serial message
  WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(8, 8, 8, 8));
  WiFi.begin(ssid, password); // Start WiFi connection

  int dotCount = 0; // Counter for dots on LCD
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); // Serial message
    lcd.print("."); // LCD message
    dotCount++;
    if (dotCount >= 10) { // If about 10 dots printed
      lcd.setCursor(0, 1);
      lcd.print("                "); // Clear the line
      lcd.setCursor(0, 1);
      dotCount = 0;
    }
  }
  Serial.println("\nWiFi connected."); // Serial message
  Serial.print("Assigned IP: ");
  Serial.println(WiFi.localIP()); // Serial message
  Serial.print("DNS after connection: ");
  Serial.println(WiFi.dnsIP()); // Serial message

  // --- Display WiFi status on LCD ---
  lcd.clear();
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  // Initialize and update NTP client
  timeClient.begin(); // Start NTP client
  Serial.println("Attempting NTP update..."); // Serial message
  bool updateSuccess = timeClient.update();

  Serial.print("NTP Update successful: "); // Serial message
  Serial.println(updateSuccess ? "YES" : "NO"); // Serial message

  // --- Display NTP status on LCD ---
  lcd.clear();
  if (updateSuccess) {
    Serial.print("Current time (Formatted): "); // Serial message
    Serial.println(timeClient.getFormattedTime()); // Serial message
    Serial.print("Current Hour (from timeClient.getHours()): "); // Serial message
    Serial.println(timeClient.getHours()); // Serial message
    Serial.print("Current Epoch Time (with offset): "); // Serial message
    Serial.println(timeClient.getEpochTime()); // Serial message
   
    lcd.print("Time Sync OK");
    lcd.setCursor(0, 1);
    lcd.print(timeClient.getFormattedTime());
  } else {
    Serial.println("Failed to get time from NTP server. Time might be incorrect."); // Serial message
    lcd.print("Time Sync Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check Network");
  }
  delay(2000);

  // --- Initial message on LCD for RFID ---
  lcd.clear();
  lcd.print("Scan RFID Card");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
}

void loop() {
  // Ensure WiFi is connected; reconnect if necessary
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi(); // This will also re-apply DNS settings
    // --- Display reconnect status on LCD ---
    lcd.clear();
    lcd.print("WiFi Reconnect");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
    lcd.clear();
    lcd.print("Scan RFID Card");
    lcd.setCursor(0, 1);
    lcd.print("Waiting...");
  }

  timeClient.update(); // Always update time in loop for current hour/minute

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String uid = getUID();
  Serial.print("Scanned UID: "); // Serial message
  Serial.println(uid); // Serial message

  // --- Display Scanned UID on LCD ---
  lcd.clear();
  lcd.print("UID: ");
  if (uid.length() > 11) {
     lcd.print(uid.substring(0, 11));
     lcd.setCursor(0, 1);
     lcd.print(uid.substring(11));
  } else {
     lcd.print(uid);
  }
  delay(1000);

  String slot = getSlotByTime();

  if (slot == "Invalid") {
    Serial.print("Access Denied: Out of allowed time (Current Time: "); // Serial message
    Serial.print(timeClient.getFormattedTime()); // Serial message
    Serial.println("). Contact HR."); // Serial message
    beepBuzzer(3);

    // --- Display Access Denied on LCD ---
    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
//    lcd.print(timeClient.getFormattedTime());
    lcd.print("Contact to HR");
    delay(3000);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    lcd.clear();
    lcd.print("Scan RFID Card");
    lcd.setCursor(0, 1);
    lcd.print("Waiting...");
    delay(300);
    return;
  }

  // --- Display Slot on LCD ---
  lcd.clear();
  lcd.print("Slot: ");
  lcd.print(slot);
  lcd.setCursor(0, 1);
  lcd.print("Sending Data...");

  sendToGoogleSheet(uid, slot);
  showGreeting(slot); // This still prints to Serial only

  // --- Display Success/Greeting on LCD ---
  lcd.clear();
  if (slot == "Entry") {
    lcd.print("Good Morning!");
    lcd.setCursor(0, 1);
    lcd.print("Welcome!");
  } else if (slot == "Lunch") {
    lcd.print("Well done!");
    lcd.setCursor(0, 1);
    lcd.print("Enjoy your break.");
  } else if (slot == "Exit") {
    lcd.print("Good Evening!");
    lcd.setCursor(0, 1);
    lcd.print("See you next!");
  }
  beepBuzzer(1); 
  delay(3000);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  // --- Return to standby message ---
  lcd.clear();
  lcd.print("Scan RFID Card");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...");
  delay(300);
}

String getUID() {
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidStr += "0";
    }
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toLowerCase();
  return uidStr;
}

String getSlotByTime() {
  // Update NTP client to get the very latest time before checking slot
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  Serial.print("Current Time: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.println(currentMinute);

  if ((currentHour <= 21 && currentMinute >= 00) &&
      (currentHour == 21 && currentMinute < 40)) {
    return "Entry";
  }
  else if ((currentHour == 12 && currentMinute >= 21 )&&
          (currentHour == 12 && currentMinute < 50)) {
    return "Lunch";
  }
  else if (currentHour >= 11 && currentMinute >= 55) { 
    return "Exit";
  }

  return "Invalid";
}

void showGreeting(String slot) {
  if (slot == "Entry") {
    Serial.println(F("Good Evening! Welcome to your shift."));
  } else if (slot == "Lunch") {
    Serial.println(F("Well done! Enjoy your break."));
  } else if (slot == "Exit") {
    Serial.println(F("Good Morning! See you next shift."));
  }
}

void sendToGoogleSheet(String uid, String slot) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setBufferSizes(512, 512);

    HTTPClient https;

    String fullURL = scriptURL + "?uid=" + urlEncode(uid) + "&slot=" + urlEncode(slot);

    Serial.println(F("📡 Sending data to Google Sheets..."));
    Serial.print("Attempting connection to: ");
    Serial.println(fullURL);

    lcd.setCursor(0, 1);
    lcd.print("Sending...");

    if (https.begin(client, fullURL)) {
      Serial.println("HTTPS connection initiated.");
      int httpCode = https.GET();

      if (httpCode > 0) {
        String payload = https.getString();
        Serial.println("✅ Server Response: " + payload);
        lcd.clear();
        lcd.print("Sent OK!");
        lcd.setCursor(0, 1);
        lcd.print(payload.substring(0, 16)); // Display first 16 chars of response
      } else {
        Serial.print("❌ HTTP Request Failed. Code: ");
        Serial.println(httpCode);
        Serial.println("Error message: " + https.errorToString(httpCode));
        lcd.clear();
        lcd.print("HTTP Fail: ");
        lcd.print(httpCode);
        lcd.setCursor(0, 1);
        lcd.print("Check Google Sheet");
      }
      https.end();
    } else {
      Serial.println("❌ Failed to initiate HTTPS connection.");
      lcd.clear();
      lcd.print("Conn Fail!");
      lcd.setCursor(0, 1);
      lcd.print("Check URL");
    }
    delay(1000);
  } else {
    Serial.println("WiFi not connected. Cannot send data to Google Sheets.");
    lcd.clear();
    lcd.print("WiFi Disconnected!");
    lcd.setCursor(0, 1);
    lcd.print("Cannot Send Data");
    delay(2000);
  }
}


void reconnectWiFi() {
  Serial.println("Reconnecting to WiFi...");
  lcd.clear();
  lcd.print("Reconnecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print("...");

  WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(8, 8, 8, 8));
  WiFi.begin(ssid, password);

  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
    dotCount++;
    if (dotCount >= 10) {
      lcd.setCursor(0, 1);
      lcd.print("                "); // Clear the line
      lcd.setCursor(0, 1);
      dotCount = 0;
    }
  }
  Serial.println("\nReconnected to WiFi.");
  Serial.print("DNS after reconnect: ");
  Serial.println(WiFi.dnsIP());
}

/**
 * @brief URL-encodes a given string.
 * @param str The string to encode.
 * @return The URL-encoded string.
 */
String urlEncode(String str) {
  String encoded = "";
  char c;
  char code0, code1;
  char code[] = "0123456789ABCDEF";

  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      code0 = code[(c >> 4) & 0xF];
      code1 = code[c & 0xF];
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

void beepBuzzer(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

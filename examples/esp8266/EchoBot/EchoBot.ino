/*******************************************************************
    Echos any Telegram message back on an ESP8266

    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe
    (or any ESP8266 board)

 *  * = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

// NOTE: The version of ESP8266 core needs to be 2.5 or higher
// or else your bot will not connect.

// ----------------------------
// Standard Libraries
// ----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <PinocchioTelegram.h>
// Library for connecting to the Telegram Bot API

// Install from Github
// https://github.com/witnessmenow/pinocchio-arduino-telegram-library

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

//------- Replace the following! ------

char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password

#define TELEGRAM_BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // your Bot Token (Get from Botfather)
//------- ---------------------- ------

WiFiClientSecure client;
PinocchioTelegram bot(client, TELEGRAM_BOT_TOKEN);

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void setup()
{
    Serial.begin(115200);

    // This is the simplest way of getting this working
    // if you are passing sensitive information, or controlling
    // something important, please either use certStore or at
    // least client.setFingerPrint
    client.setInsecure();

    // Set WiFi to station mode and disconnect from an AP if it was Previously
    // connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // Attempt to connect to Wifi network:
    Serial.print("Connecting Wifi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void printTelegramMessage(TelegramMessage message)
{

    Serial.println("--------- Telegram Message ---------");

    Serial.print("Text: ");
    Serial.println(message.text);
    Serial.print("Chat ID: ");
    Serial.println(message.chat_id);
    Serial.print("Chat Title: ");
    Serial.println(message.chat_title);

    Serial.print("From ID: ");
    Serial.println(message.from_id);
    Serial.print("From Name: ");
    Serial.println(message.from_name);

    Serial.print("Date: ");
    Serial.println(message.date);

    Serial.print("Type (enum value): ");
    Serial.println(message.type);

    Serial.println();
    Serial.println("------------------------");
}

void loop()
{
    if (millis() > lastTimeBotRan + botRequestDelay)
    {
        Serial.print("Free Heap: ");
        Serial.println(ESP.getFreeHeap());
        int messageStatus = bot.getUpdates();

        if (messageStatus > 0)
        {
            printTelegramMessage(bot.telegramMessage);
            if (bot.sendMessage(bot.telegramMessage.chat_id, bot.telegramMessage.text))
            {
                Serial.println("Messsage sent!");
            }
        }

        lastTimeBotRan = millis();
    }
}
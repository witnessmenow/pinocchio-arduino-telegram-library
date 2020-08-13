/*******************************************************************
    Examples of sending reply Keyboards using an ESP8266

    Keyboard Types:
    
    ReplyKeyboardMarkup: simple to handle the response.
    https://core.telegram.org/bots/api#replykeyboardmarkup


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

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "175753388"

#define TELEGRAM_BOT_TOKEN "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" // your Bot Token (Get from Botfather)
//------- ---------------------- ------

WiFiClientSecure client;
PinocchioTelegram bot(client, TELEGRAM_BOT_TOKEN);

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

int ledStatus = HIGH;

void sendReplyKeyboard(const char *chatId, const char *message)
{
    const char *sendMessageBody =
        R"({"chat_id": "%s","text": "%s", "reply_markup": %s})";

    // See full list of options here: https://core.telegram.org/bots/api#replykeyboardmarkup
    // Pay attention to the qoutation marks or lack of around the placeholders
    // Arrays will not have them, either will booleans
    const char *replyMarkup =
        R"({"keyboard": %s,"resize_keyboard": %s})";

    char keyboardBuffer[100];

    // keyboard is an array of arrays.
    // The outer array represents the rows
    // The inner arrays represents the coloums in the given row.
    // The below example has two buttons on the first row, and one button on the second.
    sprintf(keyboardBuffer, replyMarkup, "[[\"/ledOn\", \"/ledOff\"],[\"/status\"]]", "true");

    char body[150 + strlen(message)];
    sprintf(body, sendMessageBody, chatId, message, keyboardBuffer);
    if (bot.sendMessage(body))
    {
        Serial.println("Sent!");
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); //Avtive LOW
    ledStatus = HIGH;

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

    sendReplyKeyboard(CHAT_ID, "This is a replyKeyboard.");
}

void handleMessage(TelegramMessage message)
{
    char chatId[64];
    sprintf(chatId, "%llu", bot.telegramMessage.chat_id);

    if (strcmp(message.text, "/ledOn") == 0)
    {
        digitalWrite(LED_BUILTIN, LOW); //Avtive LOW
        ledStatus = LOW;
        sendReplyKeyboard(chatId, "Set LED on");
    }
    else if (strcmp(message.text, "/ledOff") == 0)
    {
        digitalWrite(LED_BUILTIN, HIGH); //Avtive LOW
        ledStatus = HIGH;
        sendReplyKeyboard(chatId, "Set LED off");
    }
    else if (strcmp(message.text, "/status") == 0)
    {
        if (ledStatus == LOW)
        {
            sendReplyKeyboard(chatId, "LED is on!");
        }
        else
        {
            sendReplyKeyboard(chatId, "LED is off!");
        }
    }
    else
    {
        sendReplyKeyboard(chatId, "This is a replyKeyboard.");
    }
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
            handleMessage(bot.telegramMessage);
        }

        lastTimeBotRan = millis();
    }
}
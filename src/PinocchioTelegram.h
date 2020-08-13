/*
Copyright (c) 2020 Brian Lough. All right reserved.

PinocchioTelegram - An Arduino library to wrap the Telegram Bot API

MIT License
*/

#ifndef PinocchioTelegram_h
#define PinocchioTelegram_h

#define ARDUINOJSON_DECODE_UNICODE 1
#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define PINOCCHIO_ENABLE_SEIRAL

//unmark following line to enable debug mode
#define PINOCCHIO_ENABLE_DEBUG

#ifdef PINOCCHIO_ENABLE_SEIRAL
#define PINOCCHIO_SERIAL(STR) Serial.print(STR)
#define PINOCCHIO_SERIAL_LN(STR) Serial.println(STR)
#else
#define PINOCCHIO_SERIAL(STR)
#define PINOCCHIO_SERIAL_LN(STR)
#endif

#ifdef PINOCCHIO_ENABLE_DEBUG
#define PINOCCHIO_DEBUG_SERIAL(STR) Serial.print(STR)
#define PINOCCHIO_DEBUG_SERIAL_LN(STR) Serial.println(STR)
#else
#define PINOCCHIO_DEBUG_SERIAL(STR)
#define PINOCCHIO_DEBUG_SERIAL_LN(STR)
#endif

#define PINOCCHIO_TIMEOUT 2000

#define TELEGRAM_HOST "api.telegram.org"

enum TelegramMessageTypes
{
    tel_message,
    tel_location,
    tel_document,
    tel_reply_to_message,
    tel_channel_post,
    tel_callback_query,
    tel_edited_message
};

struct TelegramMessage
{
    char *text;
    long long chat_id;
    char *chat_title;
    long long from_id;
    char *from_name;
    char *date;
    TelegramMessageTypes type;
    //String file_caption;
    //String file_path;
    //String file_name;
    //bool hasDocument;
    //long file_size;
    //float longitude;
    //float latitude;
    //int update_id;

    //int reply_to_message_id;
    //String reply_to_text;
    //String query_id;
};

class PinocchioTelegram
{
public:
    PinocchioTelegram(Client &client, const char *botToken = "");

    int makeGetRequest(const char *command);
    int makeRequestWithBody(const char *type, const char *command, const char *body = "");
    int makePostRequest(const char *command, const char *body = "");
    int makePutRequest(const char *command, const char *body = "");

    bool sendMessage(const char *chat_id, const char *text, const char *parse_mode = NULL);
    bool sendMessage(const char *body);
    int getUpdates(long offset = -1);

    bool processResult(JsonObject result);

    void updateBotToken(const char *botToken);

    TelegramMessage telegramMessage;

    int responseBufferSize = 10000;
    int portNumber = 443;
    long last_message_received = -1;
    Client *client;

private:
    const char *_botToken;

    int getHttpStatusCode();
    void skipHeaders(bool tossUnexpectedForJSON = true);
    void closeClient();

    const char *commandFormat = "/bot%s/%s";
    const char *getUpdateCommand = "/bot%s/getUpdates?limit=1&offset=%d";
    const char *sendMessageBody =
        R"({"chat_id": "%s","text": "%s"})";
    const char *sendMessageBodyWithParse =
        R"({"chat_id": "%s","text": "%s", "prase_mode": "%s"})";
};
#endif
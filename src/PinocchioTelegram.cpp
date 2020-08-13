/*
Copyright (c) 2020 Brian Lough. All right reserved.

PinocchioTelegram - An Arduino library to wrap the Telegram Bot API

MIT License
*/

#include "PinocchioTelegram.h"

PinocchioTelegram::PinocchioTelegram(Client &client, const char *botToken)
{
    this->client = &client;
    this->_botToken = botToken;
}

void PinocchioTelegram::updateBotToken(const char *botToken)
{
    this->_botToken = botToken;
}

int PinocchioTelegram::makeRequestWithBody(const char *type, const char *command, const char *body)
{
    client->flush();
    client->setTimeout(PINOCCHIO_TIMEOUT);
    if (!client->connect(TELEGRAM_HOST, portNumber))
    {
        PINOCCHIO_SERIAL_LN(F("Connection failed"));
        return -1;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(type);
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(TELEGRAM_HOST);

    //client->println(F("Accept: application/json"));
    client->println(F("Content-Type: application/json"));

    client->println(F("Cache-Control: no-cache"));

    client->print(F("Content-Length: "));
    client->println(strlen(body));

    client->println();

    client->print(body);

    if (client->println() == 0)
    {
        PINOCCHIO_SERIAL_LN(F("Failed to send request"));
        return -2;
    }

    int statusCode = getHttpStatusCode();
    return statusCode;
}

int PinocchioTelegram::makePutRequest(const char *command, const char *body)
{
    return makeRequestWithBody("PUT ", command, body);
}

int PinocchioTelegram::makePostRequest(const char *command, const char *body)
{
    return makeRequestWithBody("POST ", command, body);
}

int PinocchioTelegram::makeGetRequest(const char *command)
{
    client->flush();
    client->setTimeout(PINOCCHIO_TIMEOUT);
    if (!client->connect(TELEGRAM_HOST, portNumber))
    {
        PINOCCHIO_SERIAL_LN(F("Connection failed"));
        return -1;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(F("GET "));
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(TELEGRAM_HOST);

    client->println(F("Content-Type: application/json"));

    client->println(F("Cache-Control: no-cache"));

    if (client->println() == 0)
    {
        PINOCCHIO_SERIAL_LN(F("Failed to send request"));
        return -2;
    }

    int statusCode = getHttpStatusCode();

    return statusCode;
}

bool PinocchioTelegram::sendMessage(const char *chat_id, const char *text, const char *parse_mode)
{
    char command[100];
    sprintf(command, commandFormat, _botToken, "sendMessage");
    char body[1000];

    if (parse_mode == NULL)
    {
        sprintf(body, sendMessageBody, chat_id, text);
    }
    else
    {
        sprintf(body, sendMessageBodyWithParse, chat_id, text, parse_mode);
    }

    PINOCCHIO_DEBUG_SERIAL_LN(command);
    PINOCCHIO_DEBUG_SERIAL_LN(body);

    int statusCode = makePostRequest(command, body);

    return statusCode == 200;
}

int PinocchioTelegram::getUpdate(long offset)
{
    char command[100];
    if (offset == -1)
    {
        sprintf(command, getUpdateCommand, _botToken, last_message_received + 1);
    }
    else
    {
        sprintf(command, getUpdateCommand, _botToken, offset);
    }

    PINOCCHIO_DEBUG_SERIAL_LN(command);

    int statusCode = makeGetRequest(command);
    int returnCode = 0;

    if (statusCode > 0)
    {
        skipHeaders();
    }

    if (statusCode == 200)
    {
        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(responseBufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            PINOCCHIO_DEBUG_SERIAL_LN(F("parsed Json Object: "));
#ifdef PINOCCHIO_ENABLE_DEBUG
            serializeJson(doc, Serial);
#endif

            if (doc.containsKey("result"))
            {
                int resultArrayLength = doc["result"].size();
                if (resultArrayLength > 0)
                {
                    if (processResult(doc["result"][0]))
                    {
                        returnCode = 1;
                    }
                    else
                    {
                        PINOCCHIO_DEBUG_SERIAL_LN(F("Failed to process result"));
                        returnCode = -1;
                    }
                }
            }
            else
            {
                PINOCCHIO_DEBUG_SERIAL_LN(F("Response contained no 'result'"));
                returnCode = -2;
            }
        }
        else
        {
            PINOCCHIO_DEBUG_SERIAL_LN(F("Failed to parse update, the message could be too "
                                        "big for the buffer. Error code: "));
            PINOCCHIO_DEBUG_SERIAL_LN(error.c_str());
            returnCode - 3;
        }
    }

    closeClient();
    return returnCode;
}

bool PinocchioTelegram::processResult(JsonObject result)
{
    int update_id = result["update_id"];
    PINOCCHIO_DEBUG_SERIAL("Update ID: ");
    PINOCCHIO_DEBUG_SERIAL_LN(update_id);
    if (last_message_received != update_id)
    {
        last_message_received = update_id;
        telegramMessage = (const struct TelegramMessage){0};
        if (result.containsKey("message"))
        {
            JsonObject message = result["message"];
            telegramMessage.type = tel_message;
            telegramMessage.from_id = message["from"]["id"].as<long long>();
            telegramMessage.from_name = (char *)message["from"]["first_name"].as<char *>();
            telegramMessage.date = (char *)message["date"].as<char *>();
            telegramMessage.chat_id = message["chat"]["id"].as<long long>();
            telegramMessage.chat_title = (char *)message["chat"]["title"].as<char *>();
            if (message.containsKey("text"))
            {
                telegramMessage.text = (char *)message["text"].as<char *>();
            }
            return true;
        }
    }

    return false;
}

void PinocchioTelegram::skipHeaders(bool tossUnexpectedForJSON)
{
    // Skip HTTP headers
    if (!client->find("\r\n\r\n"))
    {
        PINOCCHIO_SERIAL_LN(F("Invalid response"));
        return;
    }

    if (tossUnexpectedForJSON)
    {
        // Was getting stray characters between the headers and the body
        // This should toss them away
        while (client->available() && client->peek() != '{')
        {
            char c = 0;
            client->readBytes(&c, 1);
            PINOCCHIO_DEBUG_SERIAL(F("Tossing an unexpected character: "));
            PINOCCHIO_DEBUG_SERIAL_LN(c);
        }
    }
}

int PinocchioTelegram::getHttpStatusCode()
{
    // Check HTTP status
    if (client->find("HTTP/1.1"))
    {
        int statusCode = client->parseInt();
        PINOCCHIO_DEBUG_SERIAL(F("Status Code: "));
        PINOCCHIO_DEBUG_SERIAL_LN(statusCode);
        return statusCode;
    }

    return -1;
}

void PinocchioTelegram::closeClient()
{
    if (client->connected())
    {

        PINOCCHIO_DEBUG_SERIAL_LN(F("Closing client"));
        client->stop();
    }
}
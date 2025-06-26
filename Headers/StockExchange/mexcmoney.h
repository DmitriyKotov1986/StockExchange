#pragma once

//My
#include <Common/httpsslquery.h>
#include <Common/websocketpool.h>

#include <TradingCatCommon/symbol.h>

#include "StockExchange/istockexchange.h"

#include "StockExchange/imoney.h"

namespace StockExchange
{

class MexcMoney
    : public IMoney
{
public:
    MexcMoney(const TradingCatCommon::Symbol& symbol, const StockExchange::StockExchangeConfig& config, Common::HTTPSSLQuery& http, Common::WebSocketPool& webSocket, QObject* parent = nullptr);
    ~MexcMoney() override;

private slots:
    /*!
        Получен положительный ответ от сервера
        @param answer данные ответа
        @param id - ИД запроса
    */
    void getAnswerHttp(const QByteArray& answer, quint64 id);

    /*!
        Ошибка обработки запроса
        @param code - код ошибки
        @param serverCode - код ответа сервера (или 0 если ответ не получен)
        @param msg - сообщение об ошибке
        @param id - ИД запроса
    */
    void errorOccurredHttp(QNetworkReply::NetworkError code, quint64 serverCode, const QString& msg, quint64 id, const QByteArray& answer);

    /*!
        Дополнительное сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
        @param id -ИД запроса
    */
    void sendLogMsgHttp(Common::TDBLoger::MSG_CODE category, const QString& msg, quint64 id);

    /*!
        Получен положительный ответ от сервера
        @param answer данные ответа
    */
    void getAnswerWebSocket(const QString& answer);

    /*!
        Ошибка обработки запроса
        @param error - код ошибки
        @param id - ИД соединения
    */
    void errorOccurredWebSocket(QAbstractSocket::SocketError error, quint64 id);

    /*!
        Дополнительное сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
        @param id -ИД соединения
    */
    void sendLogMsgWebSocket(Common::TDBLoger::MSG_CODE category, const QString& msg, quint64 id);

private:
    Common::HTTPSSLQuery& _http;
    Common::WebSocketPool& _webSocket;
    const StockExchange::StockExchangeConfig& _config;

}; // class MexcMoney

} // namespace StockExchange

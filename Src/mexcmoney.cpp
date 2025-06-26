#include "StockExchange/mexcmoney.h"

using namespace StockExchange;

MexcMoney::MexcMoney(const TradingCatCommon::Symbol &symbol, const StockExchange::StockExchangeConfig& config, Common::HTTPSSLQuery &http, Common::WebSocketPool &webSocket, QObject *parent)
    : IMoney(symbol, parent)
    , _http(http)
    , _webSocket(webSocket)
    , _config(config)
{
    Q_ASSERT(!symbol.isEmpty());
}

void MexcMoney::sendLogMsgHttp(Common::TDBLoger::MSG_CODE category, const QString &msg, quint64 id)
{
    emit sendLogMsg(symbol(), category, QString("Http request %1: %2").arg(id).arg(msg));
}

void MexcMoney::sendLogMsgWebSocket(Common::TDBLoger::MSG_CODE category, const QString &msg, quint64 id)
{
    emit sendLogMsg(symbol(), category, QString("Web socket connection %1: %2").arg(id).arg(msg));
}

#pragma once

//Qt
#include <QObject>
#include <QByteArray>

//My
#include <Common/httpsslquery.h>

#include <TradingCatCommon/klinehttppool.h>
#include <TradingCatCommon/tradingdata.h>
#include <TradingCatCommon/types.h>

#include "StockExchange/istockexchange.h"

namespace StockExchange
{

class Bybit final
    : public StockExchange::IStockExchange
{
    Q_OBJECT

public:
    static const TradingCatCommon::StockExchangeID STOCK_ID;

public:
    Bybit(const StockExchange::StockExchangeConfig& config, const Common::ProxyList& proxyList, QObject* parent = nullptr);
    ~Bybit() override;

    void start() override;
    void stop() override;

private slots:
    void getAnswerHTTP(const QByteArray& answer, quint64 id);
    void errorOccurredHTTP(QNetworkReply::NetworkError code, quint64 serverCode, const QString& msg, quint64 id, const QByteArray& answer);
    void sendLogMsgHTTP(Common::MSG_CODE category, const QString& msg, quint64 id);

    void getKLinesPool(const TradingCatCommon::PKLinesList& klines);
    void errorOccurredPool(Common::EXIT_CODE errorCode, const QString& errorString);
    void sendLogMsgPool(Common::MSG_CODE category, const QString& msg);

private:
    Q_DISABLE_COPY_MOVE(Bybit);

    void sendUpdateMoney();
    void restartUpdateMoney();
    void parseMoney(const QByteArray &answer);

    void makeKLines(const TradingCatCommon::PKLinesIDList klinesIdList);

private:
    Common::HTTPSSLQuery::Headers _headers;
    Common::HTTPSSLQuery* _http = nullptr;
    const StockExchange::StockExchangeConfig _config;
    const Common::ProxyList _proxyList;

    const TradingCatCommon::TradingData* _data = nullptr;

    TradingCatCommon::KLineHTTPPool* _pool = nullptr;

    quint64 _currentRequestId = 0;

    bool _isStarted = false;
};

} // namespace StockExchange

#pragma once

//Qt
#include <QObject>
#include <QDateTime>
#include <QByteArray>

//My
#include <TradingCatCommon/ikline.h>

namespace StockExchange
{

class KucoinKLineFutures
    : public TradingCatCommon::IKLine
{
    Q_OBJECT

public:
    KucoinKLineFutures(const TradingCatCommon::KLineID& id, const QDateTime& lastClose, QObject* parent = nullptr);
    ~KucoinKLineFutures() override = default;

    void start() override;
    void stop() override;

private:
    KucoinKLineFutures() = delete;
    Q_DISABLE_COPY_MOVE(KucoinKLineFutures);

    static QString KLineTypeToString(TradingCatCommon::KLineType type);

    void sendGetKline();
    TradingCatCommon::PKLinesList parseKLine(const QByteArray& answer);

private slots:
    void getAnswerHTTP(const QByteArray& answer, quint64 id);
    void errorOccurredHTTP(QNetworkReply::NetworkError code, quint64 serverCode, const QString& msg, quint64 id, const QByteArray& answer);
    void sendLogMsgHTTP(Common::MSG_CODE category, const QString& msg, quint64 id);

private:
    quint64 _currentRequestId = 0;
    bool _isStarted = false;

    qint64 _lastClose = 0;

};

} // namespace StockExchange

//Qt
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QRandomGenerator64>

//My
#include <Common/parser.h>

#include "StockExchange/binancekline.h"

using namespace TradingCatCommon;
using namespace Common;
using namespace StockExchange;

Q_GLOBAL_STATIC_WITH_ARGS(const QUrl, BASE_URL, ("https://data-api.binance.vision/"));
static const quint64 MIN_REQUEST_PERION = 60 * 1000; // 5min
static const quint64 MIN_REQUEST_PERION_429 = 10 * 60 * 1000; //10 min

QString BinanceKLine::KLineTypeToString(TradingCatCommon::KLineType type)
{
    switch (type)
    {
    case KLineType::MIN1: return "1m";
    case KLineType::MIN5: return "5m";
    case KLineType::MIN15: return "15m";
    case KLineType::MIN30: return "30m";
    case KLineType::MIN60: return "1h";
    case KLineType::HOUR4: return "4h";
    case KLineType::HOUR8: return "8h";
    case KLineType::DAY1: return "1d";
    case KLineType::WEEK1: return "1w";
    default:
        Q_ASSERT(false);
    }

    return "UNKNOW";
}

BinanceKLine::BinanceKLine(const TradingCatCommon::KLineID &id, const QDateTime& lastClose, QObject *parent /* = nullptr */)
    : IKLine(id, parent)
    , _lastClose(lastClose.addMSecs(static_cast<quint64>(IKLine::id().type) / 2).toMSecsSinceEpoch())
{
    Q_ASSERT(!id.isEmpty());
    Q_ASSERT(id.type == KLineType::MIN1 || id.type == KLineType::MIN5);
}

void BinanceKLine::sendGetKline()
{
    Q_ASSERT(_currentRequestId == 0);
    Q_ASSERT(_isStarted);

    //Запрашиваем немного больше свечей чтобы компенсировать разницу часов между сервером и биржей
    //лишнии свечи отбросим при парсинге
    quint32 count = ((QDateTime::currentDateTime().toMSecsSinceEpoch() - _lastClose) / static_cast<quint64>(IKLine::id().type)) + 10;
    if (count > 1000)
    {
        count = 1000;
    }

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("symbol", id().symbol.name);
    urlQuery.addQueryItem("interval", KLineTypeToString(IKLine::id().type));
    urlQuery.addQueryItem("limit", QString::number(count));

    QUrl url(*BASE_URL);
    url.setPath("/api/v3/klines");
    url.setQuery(urlQuery);

    auto http = getHTTP();
    _currentRequestId = http->send(url, Common::HTTPSSLQuery::RequestType::GET);
}

PKLinesList BinanceKLine::parseKLine(const QByteArray &answer)
{
    PKLinesList result = std::make_shared<KLinesList>();
    try
    {
        const auto rootDocJson = JSONParseToDocument(answer);

        if (rootDocJson.isObject())
        {
            const auto rootJson = JSONReadDocumentToMap(rootDocJson);

            const auto code = JSONReadMapNumber<qint64>(rootJson, "code", "Root/code").value_or(0);
            const auto msg = JSONReadMapString(rootJson, "msg", "Root/msg").value_or("");

            throw ParseException(QString("Stock exchange return error. Code: %1 Message: %2").arg(code).arg(msg));
        }

        const auto rootJson = JSONReadDocumentToArray(rootDocJson);

        if (rootJson.size() < 2)
        {
            return result;
        }

        //Последняя свеча может быть некорректно сформирована, поэтму в следующий раз нам надо получить ее еще раз
        const auto it_klinePreEnd = std::prev(rootJson.end());
        for (auto it_kline = rootJson.begin(); it_kline != it_klinePreEnd; ++it_kline)
        {
            const auto data = JSONReadArray(*it_kline, "Root/[]");

            const auto openDateTime = data[0].toInteger();
            const auto closeDateTime = openDateTime + static_cast<qint64>(IKLine::id().type);

            //отсеиваем лишнии свечи
            if (_lastClose >= closeDateTime)
            {
                continue;
            }

            auto tmp = std::make_shared<KLine>();
            tmp->openTime = openDateTime;
            tmp->open = data[1].toString().toFloat();
            tmp->high = data[2].toString().toFloat();
            tmp->low = data[3].toString().toFloat();
            tmp->close = data[4].toString().toFloat();
            tmp->volume = data[5].toString().toFloat();
            tmp->quoteAssetVolume = data[7].toString().toFloat();
            tmp->closeTime = closeDateTime;
            tmp->id = IKLine::id();

            result->emplace_back(std::move(tmp));

            //Вычисляем самую позднюю свечу
            _lastClose = std::max(_lastClose, closeDateTime);
        }
    }
    catch (const ParseException& err)
    {
        result->clear();

        emit sendLogMsg(IKLine::id(), MSG_CODE::WARNING_CODE, QString("Error parsing KLine: %1 Source: %2").arg(err.what()).arg(answer));

        return result;
    }

    return result;
}

void BinanceKLine::getAnswerHTTP(const QByteArray &answer, quint64 id)
{
    if (id != _currentRequestId)
    {
        return;
    }

    _currentRequestId = 0;

    addKLines(parseKLine(answer));

    const auto type = static_cast<quint64>(IKLine::id().type);

    QTimer::singleShot(type * 2, this, [this](){ this->sendGetKline(); });
}

void BinanceKLine::errorOccurredHTTP(QNetworkReply::NetworkError code, quint64 serverCode, const QString &msg, quint64 id, const QByteArray& answer)
{
    Q_UNUSED(code);

    if (id != _currentRequestId)
    {
        return;
    }

    _currentRequestId = 0;

    QString errorMsg = "EMPTY";
    if (!answer.isEmpty())
    {
        try
        {
            const auto rootJson = JSONParseToMap(answer);

            if (rootJson.contains("code"))
            {
                const auto code = JSONReadMapNumber<qint64>(rootJson, "code", "Root/code").value_or(0);
                const auto msg = JSONReadMapString(rootJson, "msg", "Root/msg").value_or("");

                errorMsg = QString("Stock exchange return error. Code: %1 Message: %2").arg(code).arg(msg);
            }
        }
        catch (const ParseException& err)
        {
            emit sendLogMsg(IKLine::id(), MSG_CODE::WARNING_CODE, QString("Error parse JSON error data: %1. Source data: %2").arg(err.what()).arg(answer));
        }
    }

    emit sendLogMsg(IKLine::id(), Common::MSG_CODE::WARNING_CODE, QString("HTTP request %1 failed with an error: %2. Addition data: %3. Retry after %4s").arg(id).arg(msg).arg(errorMsg).arg(MIN_REQUEST_PERION_429 / 1000));

    //429 To many request
    if (serverCode >= 400 || serverCode == 418 || code == QNetworkReply::OperationCanceledError)
    {
        QTimer::singleShot(MIN_REQUEST_PERION_429, this, [this](){ this->sendGetKline(); });

        return;
    }

    const auto interval = static_cast<qint64>(IKLine::id().type);
    QTimer::singleShot(MIN_REQUEST_PERION + interval + QRandomGenerator64::global()->bounded(interval), this,
                       [this](){ this->sendGetKline(); });
}

void BinanceKLine::start()
{
    auto http = getHTTP();

    Q_CHECK_PTR(http);

    QObject::connect(http, SIGNAL(getAnswer(const QByteArray&, quint64)),
                     SLOT(getAnswerHTTP(const QByteArray&, quint64)));
    QObject::connect(http, SIGNAL(errorOccurred(QNetworkReply::NetworkError, quint64, const QString&, quint64, const QByteArray&)),
                     SLOT(errorOccurredHTTP(QNetworkReply::NetworkError, quint64, const QString&, quint64, const QByteArray&)));
    QObject::connect(http, SIGNAL(sendLogMsg(Common::MSG_CODE, const QString&, quint64)),
                     SLOT(sendLogMsgHTTP(Common::MSG_CODE, const QString&, quint64)));

    _isStarted = true;

    sendGetKline();
}

void BinanceKLine::stop()
{
    if (!_isStarted)
    {
        return;
    }

    _isStarted = false;
}

void BinanceKLine::sendLogMsgHTTP(Common::MSG_CODE category, const QString &msg, quint64 id)
{
    Q_UNUSED(id);

    emit sendLogMsg(IKLine::id(), category, msg);
}

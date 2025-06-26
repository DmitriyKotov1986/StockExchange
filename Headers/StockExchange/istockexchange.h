#pragma once

//Qt
#include <QObject>
#include <QFlags>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>

#include <TradingCatCommon/kline.h>
#include <TradingCatCommon/symbol.h>
#include <TradingCatCommon/stockexchange.h>
#include <TradingCatCommon/orderbook.h>
#include <TradingCatCommon/tradestream.h>

#ifndef QT_NO_SSL

namespace StockExchange
{

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangeConfig class - конфигурация подключения к бирже
///
struct StockExchangeConfig
{
    enum ETypeGetData
    {
        UNDEFINED   = 0x00000000b,
        KLINE       = 0x00000001b,
        KLINE_LIFE  = 0x00000010b,
        ORDER_BOOK  = 0x00000100b,
        TRADING_STREAM = 0x00001000b
    };

    Q_DECLARE_FLAGS(ETypesGetDate, ETypeGetData);

    QString type;           ///< Тип биржи. Должен совпадать с названием биржи
    QString user;           ///< Имя пользователя или API key (зависит от биржи)
    QString password;       ///< Пароль или Secret key (зависит от биржи)
    TradingCatCommon::KLineTypes klineTypes = {TradingCatCommon::KLineType::MIN1, TradingCatCommon::KLineType::MIN5}; ///< Список интервалов свечей, которые необходимо получать с биржи
    QStringList klineNames; ///< Фильтр названия инструментов, которые нужно получать с биржи
    ETypesGetDate typesGateData = ETypesGetDate(ETypeGetData::KLINE);

};

using StockExchangeConfigList = std::list<StockExchangeConfig>;   ///< Список конфигураций бирж

///////////////////////////////////////////////////////////////////////////////
///     The IStockExchange class - интерфейсный класс биржи
///
class IStockExchange
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Конструктор. Планируется использовать только этот конструктор
        @param id - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param parent - указатель на родительский класс
    */
    explicit IStockExchange(const TradingCatCommon::StockExchangeID& stockExchangeId, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~IStockExchange() override = default;

    /*!
        Возвращает ИД биржи
        @return ИД биржи
     */
    const TradingCatCommon::StockExchangeID& id() const;

public slots:
    /*!
        Начало работы класса.
    */
    virtual void start();

    /*!
        Завершение работы класса
    */
    virtual void stop();

signals:
    /*!
        Испускается при получении новых свечей
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klines - список новых свечей
    */
    void getKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines);

    /*!
        Испускаеться при получении незавершенной  свечи
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param kline - Новые данные для построение свечи (текущее состояние)
    */
    void getDiffKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLine& kline);

    /*!
        Возвращает список свечей поддерживаемых биржей
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klinesId - список свечей поддерживаемых биржей
     */
    void getKLinesID(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesIDList& klinesId);

    /*!
        Полное обновление стакана заявок
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param orderBook - стакан заявок
    */
    void getOrderBook(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::Symbol& symbol, const TradingCatCommon::POrderBook& orderBook);

    /*!
        Сырые данные потока заявок
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param tradingStreamData - сырые данные потока заявок
    */
    void getTradeStream(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PTradeStreamData& tradingStreamData);

    /*!
        Дифференциальное изменение стакана заявок
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param symbol - название монеты
        @param diffOrderBook - имененеия в стакане заявок
    */
    void getDiffOrderBook(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::Symbol& symbol,  const TradingCatCommon::POrderBook& diffOrderBook);

    /*!
        Сигнал генерируется если в процессе работы сервера произошла фатальная ошибка
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(const TradingCatCommon::StockExchangeID& stockExchangeId, Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Сообщение логеру
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(const TradingCatCommon::StockExchangeID& stockExchangeId, Common::MSG_CODE category, const QString& msg);

    /*!
        Сингал генерируется после остановки работы класса
    */
    void finished();

private:
    // Удаляем неиспользуемые конструкторы
    IStockExchange() = delete;
    Q_DISABLE_COPY_MOVE(IStockExchange);

private:
    const TradingCatCommon::StockExchangeID _stockExchangeId;    ///< ИД Биржи

};

} // namespace StockExchange

Q_DECLARE_OPERATORS_FOR_FLAGS(StockExchange::StockExchangeConfig::ETypesGetDate);

#endif


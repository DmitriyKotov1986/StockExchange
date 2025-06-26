#pragma once

//Qt
#include <QObject>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>

#include <TradingCatCommon/kline.h>
#include <TradingCatCommon/symbol.h>
#include <TradingCatCommon/orderbook.h>
#include <TradingCatCommon/tradestream.h>

namespace StockExchange
{

///////////////////////////////////////////////////////////////////////////////
///         The IMoney class - интерфейсный класс монеты
///
class IMoney
    : public QObject
{
    Q_OBJECT

public:
    IMoney(const TradingCatCommon::Symbol& symbol, QObject* parent = nullptr);
    ~IMoney() override = default;

    const TradingCatCommon::Symbol& symbol() const noexcept;

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
        @param klines - список новых свечей
    */
    void getKLines(const TradingCatCommon::PKLinesList& klines);

    /*!
        Испускаеться при получении незавершенной  свечи
        @param kline - Новые данные для построение свечи (текущее состояние)
    */
    void getDiffKLines(const TradingCatCommon::PKLine& kline);

    /*!
        Возвращает список свечей поддерживаемых биржей
        @param klinesId - список свечей поддерживаемых биржей
     */
    void getKLinesID(const TradingCatCommon::PKLinesIDList& klinesId);

    /*!
        Полное обновление стакана заявок
        @param orderBook - стакан заявок
    */
    void getOrderBook(const TradingCatCommon::POrderBook& orderBook);

    /*!
        Дифференциальное изменение стакана заявок
        @param diffOrderBook - имененеия в стакане заявок
    */
    void getDiffOrderBook(const TradingCatCommon::POrderBook& diffOrderBook);

    /*!
        Сырые данные потока заявок
        @param tradingStreamData - сырые данные потока заявок
    */
    void getTradeStream(const TradingCatCommon::PTradeStreamData& tradingStreamData);

    /*!
        Сигнал генерируется если в процессе работы сервера произошла фатальная ошибка
        @param symbol - ИД монеты. Гарантируется что ИД монеты валидно и не пустое
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(const TradingCatCommon::Symbol& symbol, Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Сообщение логеру
        @param symbol - ИД монеты. Гарантируется что ИД монеты валидно и не пустое
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(const TradingCatCommon::Symbol& symbol, Common::MSG_CODE category, const QString& msg);

    /*!
        Сингал генерируется после остановки работы класса
    */
    void finished();

private:
    // Удаляем неиспользуеые конструкторы
    IMoney() = delete;
    Q_DISABLE_COPY_MOVE(IMoney);

private:
    const TradingCatCommon::Symbol _symbol;

};

} //using namespace StockExchange

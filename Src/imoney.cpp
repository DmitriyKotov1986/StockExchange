#include "StockExchange/imoney.h"

using namespace StockExchange;


IMoney::IMoney(const TradingCatCommon::Symbol &symbol, QObject *parent /* = nullptr */)
    : QObject{parent}
    , _symbol(symbol)
{
    Q_ASSERT(!_symbol.isEmpty());
}

const TradingCatCommon::Symbol &IMoney::symbol() const noexcept
{
    return _symbol;
}

void IMoney::start()
{

}

void IMoney::stop()
{
    emit finished();
}

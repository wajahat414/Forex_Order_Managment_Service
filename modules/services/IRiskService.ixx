#include <string>
export module services.risk;

export struct RiskDecision
{
    bool accepted{false};
    std::string reason;
};

export struct OrderIn
{
    std::string client_id;
    std::string symbol;
    char side{'1'}; // '1' buy, '2' sell
    double qty{0.0};
    double price{0.0};
    char ord_type{'2'}; // '1' market, '2' limit
    char tif{'0'};      // '0' Day
};

export class IRiskService
{
public:
    virtual ~IRiskService() = default;
    virtual RiskDecision evaluate(const OrderIn &o) = 0;
};

module; // global module fragment for includes
#include <memory>

export module services.risk.simple;
import services.risk;

export class RiskService final : public IRiskService
{
public:
    RiskDecision evaluate(const OrderIn &o) override
    {
        if (o.qty <= 0)
            return {false, "Invalid quantity"};
        if (o.ord_type == '2' && o.price <= 0)
            return {false, "Invalid price for limit"};
        return {true, {}};
    }
};

export std::unique_ptr<IRiskService> make_risk_service()
{
    return std::make_unique<RiskService>();
}

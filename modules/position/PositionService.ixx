module; // includes allowed here
#include <shared_mutex>
#include <unordered_map>
#include <string>

export module services.position;

export struct Position
{
    double qty{0.0};
    double avg_px{0.0};
};

export class IPositionService
{
public:
    virtual ~IPositionService() = default;
    virtual void on_fill(const std::string &symbol, char side, double qty, double px) = 0;
    virtual Position get(const std::string &symbol) const = 0;
};

export class PositionService final : public IPositionService
{
public:
    void on_fill(const std::string &symbol, char side, double qty, double px) override
    {
        std::unique_lock lk(mu_);
        auto &p = pos_[symbol];
        const double signed_qty = (side == '1') ? qty : -qty;
        const double new_qty = p.qty + signed_qty;
        if (new_qty == 0)
        {
            p.qty = 0;
            p.avg_px = 0;
        }
        else
        {
            p.avg_px = (p.avg_px * p.qty + px * signed_qty) / new_qty;
            p.qty = new_qty;
        }
    }
    Position get(const std::string &symbol) const override
    {
        std::shared_lock lk(mu_);
        auto it = pos_.find(symbol);
        return it == pos_.end() ? Position{} : it->second;
    }

private:
    mutable std::shared_mutex mu_;
    std::unordered_map<std::string, Position> pos_;
};

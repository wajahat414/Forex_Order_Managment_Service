module; // includes allowed here
#include <shared_mutex>
#include <unordered_map>
#include <optional>
#include <string>

export module oms.execution_cache;

export struct ExecReportView
{
    std::string order_id;
    std::string symbol;
    char ord_status{'0'}; // FIX codes: '0','1','2','4','8',...
    int32_t cum_qty{0};
    double avg_px{0.0};
    std::string text;
};

export class IExecutionCache
{
public:
    virtual ~IExecutionCache() = default;
    virtual void upsert(const ExecReportView &er) = 0;
    virtual std::optional<ExecReportView> get(const std::string &order_id) const = 0;
    virtual std::unordered_map<std::string, ExecReportView> snapshot() const = 0;
};

export class ExecutionCache final : public IExecutionCache
{
public:
    void upsert(const ExecReportView &er) override
    {
        std::unique_lock lk(mu_);
        cache_[er.order_id] = er;
    }
    std::optional<ExecReportView> get(const std::string &order_id) const override
    {
        std::shared_lock lk(mu_);
        auto it = cache_.find(order_id);
        if (it == cache_.end())
            return std::nullopt;
        return it->second;
    }
    std::unordered_map<std::string, ExecReportView> snapshot() const override
    {
        std::shared_lock lk(mu_);
        return cache_;
    }

private:
    mutable std::shared_mutex mu_;
    std::unordered_map<std::string, ExecReportView> cache_;
};

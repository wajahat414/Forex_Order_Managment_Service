import services.risk;
import services.risk.simple;
import services.position;
import oms.execution_cache;

#include <iostream>

int main() {
  auto risk = make_risk_service();
  OrderIn o{.client_id = "u1",
            .symbol = "EURUSD",
            .side = '1',
            .qty = 1000,
            .price = 1.1,
            .ord_type = '2',
            .tif = '0'};
  auto dec = risk->evaluate(o);
  std::cout << "risk accepted=" << dec.accepted << "\n";

  PositionService pos;
  pos.on_fill("EURUSD", '1', 500, 1.1);
  auto p = pos.get("EURUSD");
  std::cout << "pos qty=" << p.qty << " avg_px=" << p.avg_px << "\n";

  ExecutionCache cache;
  cache.upsert({"ORD_X", "EURUSD", '1', 500, 1.1, "ok"});
  std::cout << "cache size=" << cache.snapshot().size() << "\n";
  return 0;
}

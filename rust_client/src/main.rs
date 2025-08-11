use anyhow::Result;
use log::info;
use std::io::{self, Write};
use std::time::Duration;
use tokio::time::sleep;

use rust_client::{
    generate_unique_cl_ord_id, ExecutionReport, ExecutionReportListener, OrderDdsClient, OrderSide,
};

#[tokio::main]
async fn main() -> Result<()> {
    // Initialize logging following OMS patterns
    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .init();

    println!("=== Enhanced OMS Rust Client with ExecutionReport Listener ===");
    println!("Following OMS architecture patterns for financial trading");

    // Initialize order client following OMS architecture
    let mut order_client = OrderDdsClient::new().await?;

    // Initialize execution report listener following OMS real-time processing patterns
    let mut exec_listener = ExecutionReportListener::new().await?;

    // Register callback for real-time execution processing following OMS requirements
    exec_listener.register_execution_callback(|report: &ExecutionReport| {
        match report.get_execution_status() {
            "Filled" => info!(
                "🎉 Order FILLED: {} - {} shares @ {} (Exchange: {})",
                report.order_id, report.cum_qty, report.avg_px, report.security_exchange
            ),
            "Rejected" => info!(
                "❌ Order REJECTED: {} - Reason: {} (RejReason: {})",
                report.order_id, report.text, report.ord_rej_reason
            ),
            "Partially Filled" => info!(
                "📊 Order PARTIALLY FILLED: {} - {} of {} shares @ avg {}",
                report.order_id, report.cum_qty, report.order_qty, report.avg_px
            ),
            "Pending New" => info!("⏳ Order ACCEPTED: {} - Pending execution", report.order_id),
            _ => info!(
                "📈 Order Status Update: {} - {} (ExecType: {})",
                report.order_id,
                report.get_execution_status(),
                report.get_execution_type()
            ),
        }
    });

    // Wait for discovery following OMS connection patterns
    println!("⏳ Discovering OMS components...");
    let order_discovery = order_client.wait_for_discovery(15).await?;
    let exec_discovery = exec_listener.wait_for_discovery(15).await?;

    // Display comprehensive status following OMS monitoring patterns
    println!("\n📊 Connection Status:");
    println!("   Order Client: {}", order_client.get_connection_status());
    println!(
        "   Execution Listener: {}",
        exec_listener.get_connection_status()
    );

    if !order_discovery && !exec_discovery {
        println!("⚠️  Limited connectivity - some features may not work");
    }

    // Start execution report listening in background following OMS real-time architecture
    let mut listener_for_background = ExecutionReportListener::new().await?;
    let _ = listener_for_background.wait_for_discovery(5).await;

    tokio::spawn(async move {
        if let Err(e) = listener_for_background.start_listening().await {
            log::error!(
                "ExecutionReport listener error following OMS patterns: {}",
                e
            );
        }
    });

    println!("\n🚀 Enhanced Financial Trading Client Ready!");
    println!("Commands following OMS architecture:");
    println!("  1 - Send OrderRequest to OMS (BTC market buy)");
    println!("  2 - Send NewOrderSingle to Matching Engine (ETH limit sell)");
    println!("  3 - Send NewOrderSingle to Matching Engine (EURUSD limit buy)");
    println!("  4 - Test order with execution monitoring");
    println!("  5 - View execution report cache");
    println!("  6 - Interactive order creation");
    println!("  s - Show connection status");
    println!("  0 - Exit");

    loop {
        print!("\nEnter command (0-6,s): ");
        io::stdout().flush()?;

        let mut input = String::new();
        io::stdin().read_line(&mut input)?;

        match input.trim() {
            "1" => {
                // Send OrderRequest to OMS following architecture patterns
                match order_client
                    .send_market_order("BTC-USD", OrderSide::BUY, 0.1)
                    .await
                {
                    Ok(order_id) => {
                        println!("✅ Sent OrderRequest to OMS: {}", order_id);
                        println!("   Monitor execution reports for status updates");
                    }
                    Err(e) => eprintln!("❌ Failed to send OrderRequest: {}", e),
                }
            }
            "2" => {
                // Send NewOrderSingle directly to matching engine following OMS routing patterns
                match order_client
                    .send_limit_new_order_single("ETH-USD", "2", 5, 2800.0)
                    .await
                {
                    Ok(cl_ord_id) => {
                        println!("✅ Sent NewOrderSingle to Matching Engine: {}", cl_ord_id);
                        println!("   Monitor execution reports for fills");
                    }
                    Err(e) => eprintln!("❌ Failed to send NewOrderSingle: {}", e),
                }
            }
            "3" => {
                // Send EURUSD NewOrderSingle following forex trading patterns
                match order_client
                    .send_limit_new_order_single("EURUSD", "1", 100000, 1.0875)
                    .await
                {
                    Ok(cl_ord_id) => {
                        println!("✅ Sent EURUSD NewOrderSingle: {}", cl_ord_id);
                        println!("   Forex order routed to matching engine");
                    }
                    Err(e) => eprintln!("❌ Failed to send EURUSD order: {}", e),
                }
            }
            "4" => {
                // Test order with execution monitoring following OMS patterns
                println!("🧪 Testing order with execution monitoring...");

                match order_client
                    .send_market_new_order_single("BTC-USD", "1", 1000)
                    .await
                {
                    Ok(cl_ord_id) => {
                        println!(
                            "✅ Order sent: {} - Monitoring for 10 seconds...",
                            cl_ord_id
                        );

                        // Monitor for execution reports following OMS tracking patterns
                        for i in 1..=10 {
                            if let Some(status) = exec_listener.get_order_status(&cl_ord_id) {
                                println!(
                                    "📊 Order {} status: {} (CumQty: {}/{})",
                                    cl_ord_id,
                                    status.get_execution_status(),
                                    status.cum_qty,
                                    status.order_qty
                                );

                                if status.is_completely_filled() || status.is_order_rejected() {
                                    break;
                                }
                            }

                            print!(".");
                            io::stdout().flush()?;
                            sleep(Duration::from_secs(1)).await;
                        }
                        println!("\n✅ Monitoring complete");
                    }
                    Err(e) => eprintln!("❌ Failed to send test order: {}", e),
                }
            }
            "5" => {
                // View execution report cache following OMS monitoring patterns
                println!("📊 Execution Report Cache:");
                let statuses = exec_listener.get_all_order_statuses();

                if statuses.is_empty() {
                    println!("   No execution reports cached yet");
                } else {
                    for (order_id, report) in statuses.iter() {
                        println!(
                            "   Order {}: {} - {} shares @ {} ({})",
                            order_id,
                            report.get_execution_status(),
                            report.cum_qty,
                            report.avg_px,
                            report.symbol
                        );
                    }
                }
            }
            "6" => {
                if let Err(e) = create_interactive_order(&order_client).await {
                    eprintln!("❌ Failed to create interactive order: {}", e);
                }
            }
            "s" => {
                // Show comprehensive connection status following OMS monitoring patterns
                println!("📊 Enhanced Connection Status:");
                println!("   {}", order_client.get_connection_status());
                println!("   {}", exec_listener.get_connection_status());
            }
            "0" => {
                println!("👋 Shutting down enhanced client following OMS patterns...");
                break;
            }
            _ => {
                println!("❌ Invalid command. Please enter 0-6 or 's'.");
            }
        }

        // Brief pause for system processing following OMS real-time requirements
        sleep(Duration::from_millis(100)).await;
    }

    println!("✅ Enhanced client shutdown complete");
    Ok(())
}

/// Interactive order creation following OMS user interface patterns
async fn create_interactive_order(client: &OrderDdsClient) -> Result<()> {
    println!("\n📝 Creating interactive order following OMS patterns...");

    // Get symbol following financial instrument standards
    print!("Symbol (e.g., BTC-USD, EURUSD): ");
    io::stdout().flush()?;
    let mut symbol = String::new();
    io::stdin().read_line(&mut symbol)?;
    let symbol = symbol.trim().to_uppercase();

    // Get side following FIX protocol standards
    print!("Side (1=BUY, 2=SELL): ");
    io::stdout().flush()?;
    let mut side_input = String::new();
    io::stdin().read_line(&mut side_input)?;
    let side = side_input.trim();

    // Get quantity following risk management patterns
    print!("Quantity (integer): ");
    io::stdout().flush()?;
    let mut quantity_input = String::new();
    io::stdin().read_line(&mut quantity_input)?;
    let quantity: i32 = quantity_input.trim().parse().unwrap_or(1);

    // Get order type following OMS order routing patterns
    print!("Order type (1=MARKET, 2=LIMIT): ");
    io::stdout().flush()?;
    let mut type_input = String::new();
    io::stdin().read_line(&mut type_input)?;

    let cl_ord_id = match type_input.trim() {
        "1" => {
            client
                .send_market_new_order_single(&symbol, side, quantity)
                .await?
        }
        "2" => {
            print!("Price: ");
            io::stdout().flush()?;
            let mut price_input = String::new();
            io::stdin().read_line(&mut price_input)?;
            let price: f64 = price_input.trim().parse().unwrap_or(100.0);

            client
                .send_limit_new_order_single(&symbol, side, quantity, price)
                .await?
        }
        _ => {
            client
                .send_market_new_order_single(&symbol, side, quantity)
                .await?
        }
    };

    println!(
        "✅ Interactive order sent following OMS patterns: {}",
        cl_ord_id
    );
    Ok(())
}

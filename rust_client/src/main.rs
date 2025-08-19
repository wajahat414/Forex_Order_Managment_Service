use anyhow::Result;
use log::info;
use metrics::{counter, histogram};
use oms_rust_client::common::DdsInitializer;
use oms_rust_client::report::ExecutionReport;
use oms_rust_client::report::ExecutionReportListener;
use oms_rust_client::report::OrderResponseListener;
use oms_rust_client::{OrderDdsClient, OrderResponseReport, OrderSide};
use std::collections::HashSet;
use std::fs::OpenOptions;
use std::io::{self, Write};
use std::sync::Arc;
use std::time::Duration;
use tokio::time::sleep;
use tokio::time::Instant;
#[tokio::main]
async fn main() -> Result<()> {
    // Initialize logging following OMS patterns
    let log_file = OpenOptions::new()
        .create(true)
        .append(true)
        .open("logs/oms_rust_client.log")
        .expect("Failed to create log file for OMS client");

    env_logger::Builder::from_default_env()
        .filter_level(log::LevelFilter::Info)
        .target(env_logger::Target::Pipe(Box::new(log_file)))
        .format(|buf, record| {
            writeln!(
                buf,
                "[{} {} {}:{}] {}",
                chrono::Utc::now().format("%Y-%m-%d %H:%M:%S%.3f UTC"),
                record.level(),
                record.file().unwrap_or("unknown"),
                record.line().unwrap_or(0),
                record.args()
            )
        })
        .init();

    info!("🚀 OMS Rust Client starting - Financial trading system initialized");

    println!("=== Enhanced OMS Rust Client with ExecutionReport Listener ===");
    println!("Following OMS architecture patterns for financial trading");

    // Initialize order client following OMS architecture
    let dds_initialzer = DdsInitializer::initialze().await?;
    let order_client = OrderDdsClient::new(
        dds_initialzer.order_request_data_writer,
        dds_initialzer.new_order_single_data_writer,
    )
    .await?;

    // Initialize execution report listener following OMS real-time processing patterns
    let order_response_listener: Arc<OrderResponseListener> =
        Arc::new(OrderResponseListener::new(dds_initialzer.order_response_datareader).await?);
    let execution_report_listener =
        ExecutionReportListener::new(dds_initialzer.execution_report_data_reader).await?;

    // Register callback for real-time execution processing following OMS requirements
    order_response_listener
        .register_execution_callback(|report: &OrderResponseReport| {
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
                "Pending New" => {
                    info!("⏳ Order ACCEPTED: {} - Pending execution", report.order_id)
                }
                _ => info!(
                    "📈 Order Status Update: {} - {} (ExecType: {})",
                    report.order_id,
                    report.get_execution_status(),
                    report.get_execution_type()
                ),
            }
        })
        .await;

    execution_report_listener.register_execution_callback(|report: &ExecutionReport| match report
        .get_execution_status()
    {
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
    });

    // Wait for discovery following OMS connection patterns
    println!("⏳ Discovering OMS components...");

    {
        let l = Arc::clone(&order_response_listener);
        tokio::spawn(async move {
            l.run().await;
        });
    }
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
    let mut sent_order_ids: HashSet<String> = HashSet::new();

    loop {
        print!("\nEnter command (0-6,s): ");
        io::stdout().flush()?;

        let mut input = String::new();
        io::stdin().read_line(&mut input)?;

        match input.trim() {
            "1" => {
                // Send OrderRequest to OMS following architecture patterns
                let mut count = 0;
                let mut messages_count = 0;
                let start = Instant::now();

                loop {
                    match order_client
                        .send_market_order("BTC-USD", OrderSide::BUY, 1000.0)
                        .await
                    {
                        Ok(order_id) => {
                            println!("✅ Sent OrderRequest to OMS: {}", order_id);
                            println!("   Monitor execution reports for status updates");
                            messages_count += 1;
                            sent_order_ids.insert(order_id);
                        }
                        Err(e) => eprintln!("❌ Failed to send OrderRequest: {}", e),
                    }
                    counter!("process.order_sent_count").increment(messages_count);
                    count += 1;
                    if count == 1000 {
                        let delta = start.elapsed();
                        println!(
                            "time taken for sending#{} Seconds{} milli{}",
                            messages_count,
                            delta.as_secs(),
                            delta.subsec_millis()
                        );
                        histogram!("process.order_sent").record(delta);

                        break;
                    }

                    sleep(Duration::from_millis(1)).await;
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
                        for _ in 1..=10 {
                            if let Some(status) =
                                order_response_listener.get_order_status(&cl_ord_id).await
                            {
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
                let mut missing_orders: Vec<String> = Vec::new();
                let statuses = order_response_listener.get_all_order_statuses().await;

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

                    println!(
                        "Sent Orders {}, Recieved reports {},",
                        sent_order_ids.len(),
                        statuses.len()
                    );

                    for order_id in &sent_order_ids {
                        if !statuses.contains_key(order_id) {
                            missing_orders.push(order_id.clone());
                        }
                    }

                    if !missing_orders.is_empty() {
                        println!("❌ orders with no execution report recieved");
                        for order_id in missing_orders {
                            println!(".    - {}", order_id);
                        }
                    } else {
                        println!(
                            "✅ All orders recieved execution report count{}",
                            statuses.len()
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

                // order_response_listener
                //     .lock()
                //     .await
                //     .unwrap()
                //     .get_connection_status();
                // println!(
                //     "   {}",
                //     order_response_listener
                //         .lock()
                //         .await
                //         .unwrap()
                //         .get_connection_status()
                // );
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

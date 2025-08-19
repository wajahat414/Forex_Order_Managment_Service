use crate::report::OrderResponseReport;
use anyhow::{Context, Result};
use log::{error, info};
use metrics::{counter, histogram};
use rustdds::no_key::{DataReader, DataSample};
use rustdds::*;

use std::collections::HashMap;
use std::sync::Arc;
use std::time::{Duration, Instant};
use tokio::sync::{Mutex, RwLock};
use tokio::time::sleep;

/// DDS topic name for execution reports matching C++ OMS configuration

/// ExecutionReport listener following OMS architecture patterns for real-time order tracking
pub struct OrderResponseListener {
    reader: Mutex<DataReader<OrderResponseReport>>,
    // Order tracking cache for financial audit trail following OMS requirements
    order_status_cache: RwLock<HashMap<String, OrderResponseReport>>,
    execution_callbacks: RwLock<Vec<Box<dyn Fn(&OrderResponseReport) + Send + Sync>>>,
}

impl OrderResponseListener {
    /// Initialize execution report listener with FastDDS best practices
    pub async fn new(reader: DataReader<OrderResponseReport>) -> Result<Self> {
        Ok(Self {
            reader: Mutex::new(reader),
            order_status_cache: RwLock::new(HashMap::new()),
            execution_callbacks: RwLock::new(Vec::new()),
        })
    }

    /// Read all available execution reports from the topic following OMS real-time processing patterns

    pub async fn poll_once(&self) -> Result<usize> {
        let mut processed = 0usize;
        let mut reader = self.reader.lock().await;

        loop {
            match reader.take_next_sample() {
                Ok(Some(sample)) => {
                    let value = sample.value().clone();
                    // ...existing log...
                    self.log_execution_report(&value);
                    self.update_order_cache(&value).await;
                    self.trigger_callbacks(&value).await;
                    processed += 1;
                }
                Ok(None) => break,
                Err(e) => {
                    error!("❌ Error reading OrderResponse: {}", e);
                    break;
                }
            }
        }
        Ok(processed)
    }

    pub async fn run(&self) {
        loop {
            let _ = self.poll_once().await;
            sleep(Duration::from_millis(1)).await; // yield
        }
    }

    /// Start continuous listening for execution reports following OMS real-time architecture
    // pub async fn start_listening(&mut self) -> Result<()> {
    //     info!("🎧 Starting continuous ExecutionReport listening from OMS/Matching Engine...");

    //     let mut start: Instant = Instant::now();

    //     let mut recieved_count = 0;
    //     let mut count = 0;

    //     loop {
    //         match self.read_execution_reports().await {
    //             Ok(reports) => {
    //                 if !reports.is_empty() {
    //                     info!(
    //                         "📊 Processed {} ExecutionReports in this cycle",
    //                         reports.len()
    //                     );

    //                     counter!("process.recieved_count").increment(recieved_count);

    //                     count += 1;
    //                     recieved_count += 1;
    //                     info!("received counter {}", count);
    //                     if count == 1 {
    //                         start = Instant::now();
    //                     }

    //                     if (count >= 1000) {
    //                         let delta = start.elapsed();
    //                         histogram!("process.total_recieved_time").record(delta);
    //                         println!(
    //                             "total time taken for recieving Seconds {}, miliseconds {}",
    //                             delta.as_secs(),
    //                             delta.subsec_millis()
    //                         );
    //                         info!(
    //                             "total time taken for recieving records# {}, Seconds {}, miliseconds {}",recieved_count,
    //                             delta.as_secs(),
    //                             delta.subsec_millis()
    //                         );
    //                     }
    //                 }
    //             }
    //             Err(e) => {
    //                 error!("❌ Error during ExecutionReport listening: {}", e);
    //                 // Continue listening despite errors for robust financial system operation
    //             }
    //         }

    //         // Brief pause to prevent CPU spinning while maintaining low latency for trading systems
    //         sleep(Duration::from_millis(1)).await;
    //     }
    // }

    pub async fn get_order_status(&self, order_id: &str) -> Option<OrderResponseReport> {
        let cache = self.order_status_cache.read().await;
        cache.get(order_id).cloned()
    }

    /// Get all tracked orders and their current status for OMS monitoring
    pub async fn get_all_order_statuses(&self) -> HashMap<String, OrderResponseReport> {
        let cache = self.order_status_cache.read().await;
        cache.clone()
    }

    /// Register callback for real-time execution report processing following OMS architecture
    pub async fn register_execution_callback<F>(&self, callback: F)
    where
        F: Fn(&OrderResponseReport) + Send + Sync + 'static,
    {
        let mut callbacks = self.execution_callbacks.write().await;
        callbacks.push(Box::new(callback));
        info!("✅ Registered new ExecutionReport callback following OMS patterns");
    }



    /// Internal method to log execution report for regulatory compliance following OMS requirements
    fn log_execution_report(&self, report: &OrderResponseReport) {
        info!(
            "📋 EXECUTION_REPORT_AUDIT: , ExecID={}, OrderID={}, OrigClOrdID={}, Symbol={}, Side={}, Qty={}, Price={}, ExecType={}, OrdStatus={}, CumQty={}, AvgPx={}, Text='{}'",
      
            report.exec_id,
            report.order_id,
            report.orig_cl_ord_id,
            report.symbol,
            report.side,
            report.order_qty,
            report.price,
            report.get_execution_type(),
            report.get_execution_status(),
            report.cum_qty,
            report.avg_px,
            report.text
        );
    }

    /// Internal method to update order status cache following OMS tracking requirements
    async fn update_order_cache(&self, report: &OrderResponseReport) {
        let mut cache = self.order_status_cache.write().await;
        cache.insert(report.order_id.clone(), report.clone());
        info!("update cache called");

        // Also cache by OrigClOrdID for comprehensive order tracking following OMS patterns
        if !report.orig_cl_ord_id.is_empty() {
            cache.insert(report.orig_cl_ord_id.clone(), report.clone());
        }
    }

    async fn trigger_callbacks(&self, report: &OrderResponseReport) {
        let callbacks = self.execution_callbacks.read().await;
        for callback in callbacks.iter() {
            callback(report);
        }
    }

    pub async fn shutdown(&mut self) -> Result<()> {
        info!("🔧 Initiating graceful shutdown of ExecutionReport listener following OMS guidelines...");

        // Allow any pending messages to be processed following FastDDS best practices
        sleep(Duration::from_millis(100)).await;

        info!("✅ ExecutionReport listener shutdown complete");
        Ok(())
    }
}

#[derive(Debug)]
pub struct OrderResponseReportConnectionStatus {
    pub domain_id: u16,
    pub topic_name: String,
    pub type_name: String,
    pub publisher_count: usize,
    pub is_connected: bool,
    pub tracked_orders: usize,
}

impl std::fmt::Display for OrderResponseReportConnectionStatus {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "🔗 Order Response Listener Status | Domain: {}\n\
             📡 Topic: '{}' -> '{}' - {} ({} publishers)\n\
             📊 Tracked Orders: {}",
            self.domain_id,
            self.topic_name,
            self.type_name,
            if self.is_connected {
                "✅ CONNECTED"
            } else {
                "❌ DISCONNECTED"
            },
            self.publisher_count,
            self.tracked_orders
        )
    }
}

/// RAII resource management for ExecutionReport listener following OMS guidelines
impl Drop for OrderResponseListener {
    fn drop(&mut self) {
        info!("🔧 RAII cleanup: Releasing ExecutionReport listener resources following OMS patterns...");
        // RustDDS handles cleanup automatically through its Drop implementations
        info!("✅ ExecutionReport listener resources released");
    }
}

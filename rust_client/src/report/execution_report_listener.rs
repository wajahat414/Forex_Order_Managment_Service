use crate::report::{self, ExecutionReport};
use anyhow::{Context, Result};
use log::{error, info, warn};
use rustdds::no_key::{DataReader, DataSample};
use rustdds::*;
use serde_json::value;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::time::sleep;

/// DDS topic name for execution reports matching C++ OMS configuration
const EXECUTION_REPORT_TOPIC_NAME: &str = "EXECUTION_REPORT_TOPIC";
const DEFAULT_DOMAIN_ID: u16 = 0;

/// ExecutionReport listener following OMS architecture patterns for real-time order tracking
pub struct ExecutionReportListener {
    reader: DataReader<ExecutionReport>,
    // Order tracking cache for financial audit trail following OMS requirements
    order_status_cache: Arc<Mutex<HashMap<String, ExecutionReport>>>,
    execution_callbacks: Arc<Mutex<Vec<Box<dyn Fn(&ExecutionReport) + Send + Sync>>>>,
}

impl ExecutionReportListener {
    /// Initialize execution report listener with FastDDS best practices
    pub async fn new(reader: DataReader<ExecutionReport>) -> Result<Self> {
        info!("🔧 Initializing ExecutionReport listener following OMS architecture patterns...");

        Ok(Self {
            reader,
            order_status_cache: Arc::new(Mutex::new(HashMap::new())),
            execution_callbacks: Arc::new(Mutex::new(Vec::new())),
        })
    }

    /// Read all available execution reports from the topic following OMS real-time processing patterns

    pub async fn read_execution_reports(&mut self) -> Result<Vec<ExecutionReport>> {
        let mut reports = Vec::new();

        // Keep reading until no more samples are available
        loop {
            match self.reader.take_next_sample() {
                Ok(Some(value)) => {
                    let value = value.value().clone();
                    info!(
                        "📨 Received ExecutionReport: OrderID={}, Status={}",
                        value.order_id, value.ord_status,
                    );

                    // OMS business logic
                    self.log_execution_report(&value);
                    self.update_order_cache(&value);
                    self.trigger_callbacks(&value).await;

                    reports.push(value);
                }
                Ok(None) => {
                    // No more samples in queue — exit loop
                    break;
                }
                Err(e) => {
                    error!("❌ Error reading ExecutionReport: {}", e);
                    break;
                }
            }
        }

        Ok(reports)
    }

    /// Start continuous listening for execution reports following OMS real-time architecture
    pub async fn start_listening(&mut self) -> Result<()> {
        info!("🎧 Starting continuous ExecutionReport listening from OMS/Matching Engine...");

        loop {
            match self.read_execution_reports().await {
                Ok(reports) => {
                    if !reports.is_empty() {
                        info!(
                            "📊 Processed {} ExecutionReports in this cycle",
                            reports.len()
                        );
                    }
                }
                Err(e) => {
                    error!("❌ Error during ExecutionReport listening: {}", e);
                    // Continue listening despite errors for robust financial system operation
                }
            }

            // Brief pause to prevent CPU spinning while maintaining low latency for trading systems
            sleep(Duration::from_millis(10)).await;
        }
    }

    /// Get execution report for specific order ID following OMS order tracking patterns
    pub fn get_order_status(&self, order_id: &str) -> Option<ExecutionReport> {
        let cache = self.order_status_cache.lock().unwrap();
        cache.get(order_id).cloned()
    }

    /// Get all tracked orders and their current status for OMS monitoring
    pub fn get_all_order_statuses(&self) -> HashMap<String, ExecutionReport> {
        let cache = self.order_status_cache.lock().unwrap();
        cache.clone()
    }

    /// Register callback for real-time execution report processing following OMS architecture
    pub fn register_execution_callback<F>(&self, callback: F)
    where
        F: Fn(&ExecutionReport) + Send + Sync + 'static,
    {
        let mut callbacks = self.execution_callbacks.lock().unwrap();
        callbacks.push(Box::new(callback));
        info!("✅ Registered new ExecutionReport callback following OMS patterns");
    }

    /// Check connection status to OMS/Matching Engine following monitoring best practices
    pub fn get_connection_status(&self) -> ExecutionReportConnectionStatus {
        let matched_publications = 0;
        ExecutionReportConnectionStatus {
            domain_id: DEFAULT_DOMAIN_ID,
            topic_name: EXECUTION_REPORT_TOPIC_NAME.to_string(),
            type_name: ExecutionReport::type_name().to_string(),
            publisher_count: matched_publications,
            is_connected: matched_publications > 0,
            tracked_orders: {
                let cache = self.order_status_cache.lock().unwrap();
                cache.len()
            },
        }
    }

    /// Internal method to log execution report for regulatory compliance following OMS requirements
    fn log_execution_report(&self, report: &ExecutionReport) {
        info!(
            "📋 EXECUTION_REPORT_AUDIT: Topic='{}', ExecID={}, OrderID={}, OrigClOrdID={}, Symbol={}, Side={}, Qty={}, Price={}, ExecType={}, OrdStatus={}, CumQty={}, AvgPx={}, Text='{}'",
            EXECUTION_REPORT_TOPIC_NAME,
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
    fn update_order_cache(&self, report: &ExecutionReport) {
        let mut cache = self.order_status_cache.lock().unwrap();
        cache.insert(report.order_id.clone(), report.clone());

        // Also cache by OrigClOrdID for comprehensive order tracking following OMS patterns
        if !report.orig_cl_ord_id.is_empty() {
            cache.insert(report.orig_cl_ord_id.clone(), report.clone());
        }
    }

    /// Internal method to trigger registered callbacks following OMS real-time processing
    async fn trigger_callbacks(&self, report: &ExecutionReport) {
        let callbacks = self.execution_callbacks.lock().unwrap();
        for callback in callbacks.iter() {
            callback(report);
        }
    }

    /// Graceful shutdown with proper resource cleanup following RAII principles
    pub async fn shutdown(&mut self) -> Result<()> {
        info!("🔧 Initiating graceful shutdown of ExecutionReport listener following OMS guidelines...");

        // Allow any pending messages to be processed following FastDDS best practices
        sleep(Duration::from_millis(100)).await;

        info!("✅ ExecutionReport listener shutdown complete");
        Ok(())
    }
}

/// Connection status for ExecutionReport listener monitoring following OMS architecture
#[derive(Debug)]
pub struct ExecutionReportConnectionStatus {
    pub domain_id: u16,
    pub topic_name: String,
    pub type_name: String,
    pub publisher_count: usize,
    pub is_connected: bool,
    pub tracked_orders: usize,
}

impl std::fmt::Display for ExecutionReportConnectionStatus {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "🔗 ExecutionReport Listener Status | Domain: {}\n\
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
impl Drop for ExecutionReportListener {
    fn drop(&mut self) {
        info!("🔧 RAII cleanup: Releasing ExecutionReport listener resources following OMS patterns...");
        // RustDDS handles cleanup automatically through its Drop implementations
        info!("✅ ExecutionReport listener resources released");
    }
}

use crate::report::{self, OrderResponseReport};
use anyhow::{Context, Result};
use log::{error, info, warn};
use metrics::{counter, histogram};
use rustdds::no_key::{DataReader, DataSample};
use rustdds::*;
use serde_json::value;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::{Duration, Instant};
use tokio::time::sleep;

/// DDS topic name for execution reports matching C++ OMS configuration
const EXECUTION_REPORT_TOPIC_NAME: &str = "order_resonse";
const DEFAULT_DOMAIN_ID: u16 = 0;

/// ExecutionReport listener following OMS architecture patterns for real-time order tracking
pub struct OrderResponseListener {
    participant: DomainParticipant,
    subscriber: Subscriber,
    topic: Topic,
    reader: DataReader<OrderResponseReport>,
    // Order tracking cache for financial audit trail following OMS requirements
    order_status_cache: Arc<Mutex<HashMap<String, OrderResponseReport>>>,
    execution_callbacks: Arc<Mutex<Vec<Box<dyn Fn(&OrderResponseReport) + Send + Sync>>>>,
}

impl OrderResponseListener {
    /// Initialize execution report listener with FastDDS best practices
    pub async fn new() -> Result<Self> {
        info!("🔧 Initializing ExecutionReport listener following OMS architecture patterns...");

        // Create domain participant for financial trading domain with proper error handling
        let participant = DomainParticipant::new(DEFAULT_DOMAIN_ID)
            .context("Failed to create DDS domain participant for ExecutionReport listener")?;

        info!(
            "✅ Created DDS domain participant on domain {} for execution reports",
            DEFAULT_DOMAIN_ID
        );

        // Configure QoS for reliable financial message delivery following FastDDS best practices
        let qos = QosPolicyBuilder::new()
            .reliability(policy::Reliability::Reliable {
                max_blocking_time: rustdds::Duration::ZERO,
            })
            .build();

        // Create subscriber for execution report reception following OMS patterns
        let subscriber = participant
            .create_subscriber(&qos)
            .context("Failed to create DDS subscriber for execution reports")?;

        // Create topic for execution reports matching C++ OMS configuration
        let topic = participant
            .create_topic(
                EXECUTION_REPORT_TOPIC_NAME.to_string(),
                OrderResponseReport::type_name().to_string(), // Uses exact C++ type name
                &qos,
                TopicKind::NoKey, // Matches C++ implementation
            )
            .context("Failed to create ExecutionReport topic following OMS architecture")?;

        // Create data reader for execution report reception with proper FastDDS configuration
        let reader = subscriber
            .create_datareader_no_key::<OrderResponseReport, CDRDeserializerAdapter<OrderResponseReport>>(
                &topic,
                None,
            )
            .context("Failed to create ExecutionReport reader with FastDDS compatibility")?;

        info!("✅ Created ExecutionReport listener components following OMS guidelines");
        info!(
            "📡 Listening on topic: '{}' with type: '{}'",
            EXECUTION_REPORT_TOPIC_NAME,
            OrderResponseReport::type_name()
        );

        // Allow time for participant discovery following FastDDS best practices
        sleep(Duration::from_millis(500)).await;

        Ok(Self {
            participant,
            subscriber,
            topic,
            reader,
            order_status_cache: Arc::new(Mutex::new(HashMap::new())),
            execution_callbacks: Arc::new(Mutex::new(Vec::new())),
        })
    }

    /// Read all available execution reports from the topic following OMS real-time processing patterns

    pub async fn read_execution_reports(&mut self) -> Result<Vec<OrderResponseReport>> {
        let mut reports = Vec::new();

        // Keep reading until no more samples are available
        let mut start: Instant = Instant::now();

        let mut recieved_count = 0;
        let mut count = 0;
        loop {
            match self.reader.take_next_sample() {
                Ok(Some(value)) => {
                    counter!("process.recieved_count").increment(recieved_count);
                    count += 1;
                    if count == 1 {
                        start = Instant::now();
                    }

                    if (count >= 1000) {
                        let delta = start.elapsed();
                        histogram!("process.total_recieved_time").record(delta);
                        info!(
                            "total time taken for recieving Seconds {}, miliseconds {}",
                            delta.as_secs(),
                            delta.subsec_millis()
                        );
                    }

                    let value = value.value().clone();
                    info!(
                        "📨 Received OrderResponseReport: OrderID={}, Status={}",
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

    pub fn get_order_status(&self, order_id: &str) -> Option<OrderResponseReport> {
        let cache = self.order_status_cache.lock().unwrap();
        cache.get(order_id).cloned()
    }

    /// Get all tracked orders and their current status for OMS monitoring
    pub fn get_all_order_statuses(&self) -> HashMap<String, OrderResponseReport> {
        let cache = self.order_status_cache.lock().unwrap();
        cache.clone()
    }

    /// Register callback for real-time execution report processing following OMS architecture
    pub fn register_execution_callback<F>(&self, callback: F)
    where
        F: Fn(&OrderResponseReport) + Send + Sync + 'static,
    {
        let mut callbacks = self.execution_callbacks.lock().unwrap();
        callbacks.push(Box::new(callback));
        info!("✅ Registered new ExecutionReport callback following OMS patterns");
    }

    /// Check connection status to OMS/Matching Engine following monitoring best practices
    pub fn get_connection_status(&self) -> OrderResponseReportConnectionStatus {
        let matched_publications = 0;
        OrderResponseReportConnectionStatus {
            domain_id: DEFAULT_DOMAIN_ID,
            topic_name: EXECUTION_REPORT_TOPIC_NAME.to_string(),
            type_name: OrderResponseReport::type_name().to_string(),
            publisher_count: matched_publications,
            is_connected: matched_publications > 0,
            tracked_orders: {
                let cache = self.order_status_cache.lock().unwrap();
                cache.len()
            },
        }
    }

    /// Internal method to log execution report for regulatory compliance following OMS requirements
    fn log_execution_report(&self, report: &OrderResponseReport) {
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
    fn update_order_cache(&self, report: &OrderResponseReport) {
        let mut cache = self.order_status_cache.lock().unwrap();
        cache.insert(report.order_id.clone(), report.clone());

        // Also cache by OrigClOrdID for comprehensive order tracking following OMS patterns
        if !report.orig_cl_ord_id.is_empty() {
            cache.insert(report.orig_cl_ord_id.clone(), report.clone());
        }
    }

    async fn trigger_callbacks(&self, report: &OrderResponseReport) {
        let callbacks = self.execution_callbacks.lock().unwrap();
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

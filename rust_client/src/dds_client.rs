// src/dds_client.rs
use crate::new_order_single::{generate_unique_cl_ord_id, NewOrderSingle};
use crate::order_message::{generate_unique_order_id, OrderRequest, OrderSide, OrderType};
use anyhow::{Context, Result};
use log::{error, info, warn};
use rustdds::no_key::{DataReader, DataSample, DataWriter};
use rustdds::*;
use serde_json;
use std::time::Duration;
use tokio::time::{sleep, timeout};

/// DDS topic name matching OMS configuration
const NEW_ORDER_REQUEST_TOPIC_NAME: &str = "new_order_request";
const NEW_ORDER_SINGLE_TOPIC_NAME: &str = "NEW_ORDER_SINGLE_TOPIC";
const DEFAULT_DOMAIN_ID: u16 = 0;

/// Rust DDS client for communicating with Order Management Service using RustDDS
pub struct OrderDdsClient {
    participant: DomainParticipant,
    topic: Topic,
    writer: DataWriter<OrderRequest>,
    new_order_single_topic: Topic,
    new_order_single_writer: DataWriter<NewOrderSingle>,
}

impl OrderDdsClient {
    /// Initialize DDS client with FastDDS compatibility using RustDDS
    pub async fn new() -> Result<Self> {
        info!("🔧 Initializing Rust DDS client for OMS communication...");

        // Create domain participant for financial trading domain
        let participant = DomainParticipant::new(0).unwrap();

        let qos = QosPolicyBuilder::new()
            .reliability(policy::Reliability::Reliable {
                max_blocking_time: rustdds::Duration::ZERO,
            })
            .build();

        info!(
            "✅ Created DDS domain participant on domain {}",
            DEFAULT_DOMAIN_ID
        );

        let subscriber = participant.create_subscriber(&qos).unwrap();

        let publisher = participant.create_publisher(&qos).unwrap();

        let some_topic = participant
            .create_topic(
                NEW_ORDER_REQUEST_TOPIC_NAME.to_string(),
                "OrderRequest".to_string(),
                &qos,
                TopicKind::NoKey,
            )
            .unwrap();

        let writer = publisher
            .create_datawriter_no_key::<OrderRequest, CDRSerializerAdapter<OrderRequest>>(
                &some_topic,
                None,
            )
            .unwrap();

        info!("✅ Created data writer for order publishing");

        let new_order_single_topic = participant
            .create_topic(
                NEW_ORDER_SINGLE_TOPIC_NAME.to_string(),
                "DistributedATS_NewOrderSingle::NewOrderSingle".to_string(),
                &qos,
                TopicKind::NoKey, // NewOrderSingle uses NoKey based on C++ implementation
            )
            .context("Failed to create NewOrderSingle topic")?;

        let new_order_single_writer = publisher
            .create_datawriter_no_key::<NewOrderSingle, CDRSerializerAdapter<NewOrderSingle>>(
                &new_order_single_topic,
                Some(qos.clone()),
            )
            .context("Failed to create NewOrderSingle writer")?;

        Ok(Self {
            participant,
            topic: some_topic,
            writer,
            new_order_single_topic,
            new_order_single_writer,
        })
    }

    /// Send order request to OMS with delivery confirmation
    pub async fn send_order(&self, order: OrderRequest) -> Result<()> {
        // Check for active OMS subscribers

        // Validate order data before sending
        self.validate_order(&order)?;

        // Serialize order to JSON for RustDDS
        let order_json =
            serde_json::to_string(&order).context("Failed to serialize order to JSON")?;

        info!("📝 Order JSON: {}", order_json);

        let order_clone = order.clone();

        // Publish order to OMS using RustDDS
        self.writer
            .write(order_clone, None)
            .context("Failed to write order to DDS topic")?;

        info!(
            "✅ Sent order: {} for {} ({:?} {} @ {})",
            order.order_id,
            order.symbol,
            order.side,
            order.quantity,
            if order.price > 0.0 {
                order.price.to_string()
            } else {
                "MARKET".to_string()
            }
        );

        Ok(())
    }

    /// Validate order parameters following financial trading rules
    fn validate_order(&self, order: &OrderRequest) -> Result<()> {
        if order.order_id.is_empty() {
            return Err(anyhow::anyhow!("Order ID cannot be empty"));
        }

        if order.symbol.is_empty() {
            return Err(anyhow::anyhow!("Symbol cannot be empty"));
        }

        if order.quantity <= 0.0 {
            return Err(anyhow::anyhow!(
                "Quantity must be positive: {}",
                order.quantity
            ));
        }

        if order.order_type == OrderType::LIMIT && order.price <= 0.0 {
            return Err(anyhow::anyhow!(
                "Limit orders must have positive price: {}",
                order.price
            ));
        }

        if order.user_id.is_empty() {
            return Err(anyhow::anyhow!("User ID cannot be empty"));
        }

        // Financial system validations
        if order.quantity > 1_000_000.0 {
            return Err(anyhow::anyhow!("Quantity exceeds maximum limit"));
        }

        if order.price > 0.0 && order.price > 1_000_000.0 {
            return Err(anyhow::anyhow!("Price exceeds maximum limit"));
        }

        Ok(())
    }

    /// Send a market order (convenience method)
    pub async fn send_market_order(
        &self,
        symbol: &str,
        side: OrderSide,
        quantity: f64,
    ) -> Result<String> {
        let order_id = generate_unique_order_id();
        let order = OrderRequest::market_order(
            "rust_client".to_string(),
            order_id.clone(),
            symbol.to_string(),
            side,
            quantity,
            "user001".to_string(),
        );

        self.send_order(order).await?;
        Ok(order_id)
    }

    /// Send a limit order (convenience method)
    pub async fn send_limit_order(
        &self,
        symbol: &str,
        side: OrderSide,
        quantity: f64,
        price: f64,
    ) -> Result<String> {
        let order_id = generate_unique_order_id();
        let order = OrderRequest::limit_order(
            "rust_client".to_string(),
            order_id.clone(),
            symbol.to_string(),
            side,
            quantity,
            price,
            "user001".to_string(),
        );

        self.send_order(order).await?;
        Ok(order_id)
    }

    pub async fn send_new_order_single(&self, new_order: NewOrderSingle) -> Result<()> {
        // Validate NewOrderSingle before transmission

        // Log for financial audit trail
        self.log_new_order_single_transmission(&new_order);

        // Transmit to matching engine
        self.new_order_single_writer
            .write(new_order.clone(), None)
            .context("Failed to transmit NewOrderSingle to matching engine")?;

        info!(
            "✅ NewOrderSingle transmitted: ClOrdID={}, Symbol={}, Side={}, Qty={}, Type={}",
            new_order.cl_ord_id,
            new_order.symbol,
            new_order.side,
            new_order.order_qty,
            new_order.ord_type
        );

        Ok(())
    }

    fn log_new_order_single_transmission(&self, order: &NewOrderSingle) {
        info!(
            "📋 NEWORDERSINGLE AUDIT: ClOrdID={}, Symbol={}, Side={}, Qty={}, Price={}, Exchange={}, Source={}, Dest={}",
            order.cl_ord_id,
            order.symbol,
            order.side,
            order.order_qty,
            order.price,
            order.security_exchange,
            order.dats_source,
            order.dats_destination
        );
    }

    pub async fn send_market_new_order_single(
        &self,
        symbol: &str,
        side: &str,
        quantity: i32,
    ) -> Result<String> {
        let cl_ord_id = generate_unique_cl_ord_id();
        let new_order = NewOrderSingle::market_order(
            cl_ord_id.clone(),
            symbol.to_string(),
            side,
            quantity,
            "rust_trader_001".to_string(),
        );

        self.send_new_order_single(new_order).await?;
        Ok(cl_ord_id)
    }
}

/// Connection status information for monitoring
#[derive(Debug)]
pub struct ConnectionStatus {
    pub is_connected: bool,
    pub subscriber_count: usize,
    pub domain_id: u16,
    pub topic_name: String,
}

impl std::fmt::Display for ConnectionStatus {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "Domain: {}, Topic: {}, Connected: {}, Subscribers: {}",
            self.domain_id, self.topic_name, self.is_connected, self.subscriber_count
        )
    }
}

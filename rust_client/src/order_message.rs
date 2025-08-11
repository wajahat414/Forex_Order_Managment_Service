// src/order_message.rs
use chrono::{DateTime, Utc};
use rustdds::*;
use serde::{Deserialize, Serialize};

/// Order side enumeration matching OMS OrderSide
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
#[repr(u32)]
pub enum OrderSide {
    BUY = 1,
    SELL = 2,
}

/// Order type enumeration matching OMS OrderType  
#[derive(Debug, Clone, Copy, Serialize, Deserialize, PartialEq)]
#[repr(u32)]
pub enum OrderType {
    MARKET = 1,
    LIMIT = 2,
    STOP = 3,
    STOP_LIMIT = 4,
}

/// Order request structure matching OMS OrderRequest IDL with keyed support
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OrderRequest {
    pub client_id: String,
    pub order_id: String, // ✅ This will be our key field
    pub symbol: String,
    pub side: OrderSide,
    pub order_type: OrderType,
    pub quantity: f64,
    pub price: f64,
    pub stop_price: f64,
    pub timestamp: i64,
    pub user_id: String,
}

// ✅ Implement Keyed trait for DDS key-based topics (financial order tracking)
impl Keyed for OrderRequest {
    type K = String;

    fn key(&self) -> Self::K {
        self.order_id.clone() // Use order_id as the unique key for financial tracking
    }
}

// Define type name for DDS registration
impl OrderRequest {
    pub const DDS_TYPE_NAME: &'static str = "OrderRequest";
}

impl OrderRequest {
    /// Create a new order request with current timestamp
    pub fn new(
        client_id: String,
        order_id: String,
        symbol: String,
        side: OrderSide,
        order_type: OrderType,
        quantity: f64,
        price: f64,
        user_id: String,
    ) -> Self {
        Self {
            client_id,
            order_id,
            symbol,
            side,
            order_type,
            quantity,
            price,
            stop_price: 0.0,
            timestamp: Utc::now().timestamp_millis(),
            user_id,
        }
    }

    /// Create a market order (price = 0.0)
    pub fn market_order(
        client_id: String,
        order_id: String,
        symbol: String,
        side: OrderSide,
        quantity: f64,
        user_id: String,
    ) -> Self {
        Self::new(
            client_id,
            order_id,
            symbol,
            side,
            OrderType::MARKET,
            quantity,
            0.0,
            user_id,
        )
    }

    /// Create a limit order
    pub fn limit_order(
        client_id: String,
        order_id: String,
        symbol: String,
        side: OrderSide,
        quantity: f64,
        price: f64,
        user_id: String,
    ) -> Self {
        Self::new(
            client_id,
            order_id,
            symbol,
            side,
            OrderType::LIMIT,
            quantity,
            price,
            user_id,
        )
    }
}

/// Generate unique order ID following financial industry standards
pub fn generate_unique_order_id() -> String {
    let timestamp = Utc::now().timestamp_micros();
    let uuid_suffix = uuid::Uuid::new_v4().to_string()[..8].to_string();
    format!("ORD_{}_{}", timestamp, uuid_suffix)
}

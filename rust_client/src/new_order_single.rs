// src/distributed_ats_new_order_single.rs - Create new module to match C++ namespace
use chrono::Utc;
use rustdds::*;
use serde::{Deserialize, Serialize};

/// Module matching C++ DistributedATS_NewOrderSingle namespace
pub mod distributed_ats_new_order_single {
    use super::*;

    /// Header structure matching DistributedATS::Header IDL
    #[derive(Debug, Clone, Serialize, Deserialize)]
    pub struct Header {
        pub begin_string: String,
        pub body_length: i32,
        pub msg_type: String,
        pub sender_comp_id: String,
        pub target_comp_id: String,
        pub msg_seq_num: i32,
        pub sending_time: String,
        pub checksum: String,
    }

    impl Default for Header {
        fn default() -> Self {
            Self {
                begin_string: "FIX.4.4".to_string(),
                body_length: 0,
                msg_type: "D".to_string(), // NewOrderSingle message type
                sender_comp_id: "RUST_CLIENT".to_string(),
                target_comp_id: "OMS".to_string(),
                msg_seq_num: 1,
                sending_time: Utc::now().format("%Y%m%d-%H:%M:%S%.3f").to_string(),
                checksum: "000".to_string(),
            }
        }
    }

    /// ✅ NewOrderSingle struct exactly matching C++ DistributedATS_NewOrderSingle::NewOrderSingle
    #[derive(Debug, Clone, Serialize, Deserialize)]
    pub struct NewOrderSingle {
        // Fields matching exactly the C++ IDL structure
        #[serde(rename = "DATS_Source")]
        pub dats_source: String,

        #[serde(rename = "DATS_Destination")]
        pub dats_destination: String,

        #[serde(rename = "DATS_SourceUser")]
        pub dats_source_user: String,

        #[serde(rename = "DATS_DestinationUser")]
        pub dats_destination_user: String,

        // Security Exchange
        #[serde(rename = "SecurityExchange")]
        pub security_exchange: String,
        #[serde(rename = "fix_header")]
        pub fix_header: Header,

        #[serde(rename = "ClOrdID")]
        pub cl_ord_id: String,

        // Additional FIX fields for complete order representation
        #[serde(rename = "Symbol")]
        pub symbol: String,

        #[serde(rename = "Side")]
        pub side: String, // Use String to match C++ char representation

        #[serde(rename = "OrderQty")]
        pub order_qty: i32,

        #[serde(rename = "OrdType")]
        pub ord_type: String, // Use String to match C++ char representation

        #[serde(rename = "Price")]
        pub price: f64,

        #[serde(rename = "TimeInForce")]
        pub time_in_force: String,

        #[serde(rename = "TransactTime")]
        pub transact_time: u64,
    }

    impl NewOrderSingle {
        /// Create market order matching C++ OMS expectations
        pub fn market_order(
            cl_ord_id: String,
            symbol: String,
            side: &str, // "1" for BUY, "2" for SELL
            quantity: i32,
            source_user: String,
        ) -> Self {
            let mut header = Header::default();
            header.msg_seq_num = Self::generate_sequence_number();
            header.sending_time = Utc::now().format("%Y%m%d-%H:%M:%S%.3f").to_string();

            Self {
                dats_source: "RUST_CLIENT".to_string(),
                dats_destination: "DATA_SERVICE_A".to_string(),
                dats_source_user: source_user,
                dats_destination_user: "DATA_SERVICE_A".to_string(),
                fix_header: header,
                cl_ord_id,
                symbol,
                side: side.to_string(),
                order_qty: quantity,
                ord_type: "1".to_string(), // Market order
                price: 0.0,
                security_exchange: "BTC_MARKET".to_string(), // Market orders have no price
                time_in_force: "0".to_string(),              // Day order
                transact_time: Utc::now().timestamp_millis() as u64,
            }
        }

        /// Create limit order matching C++ OMS expectations
        pub fn limit_order(
            cl_ord_id: String,
            symbol: String,
            side: &str, // "1" for BUY, "2" for SELL
            quantity: i32,
            price: f64,
            source_user: String,
        ) -> Self {
            let mut header = Header::default();
            header.msg_seq_num = Self::generate_sequence_number();
            header.sending_time = Utc::now().format("%Y%m%d-%H:%M:%S%.3f").to_string();

            Self {
                dats_source: "RUST_CLIENT".to_string(),
                dats_destination: "DATA_SERVICE_A".to_string(),
                dats_source_user: source_user,
                dats_destination_user: "DATA_SERVICE_A".to_string(),
                fix_header: header,
                cl_ord_id,
                symbol,
                security_exchange: "BTC_MARKET".to_string(),
                side: side.to_string(),
                order_qty: quantity,
                ord_type: "2".to_string(), // Limit order
                price,
                time_in_force: "0".to_string(), // Day order
                transact_time: Utc::now().timestamp_millis() as u64,
            }
        }

        /// Generate sequence number for FIX protocol compliance
        fn generate_sequence_number() -> i32 {
            use std::sync::atomic::{AtomicI32, Ordering};
            static SEQUENCE_COUNTER: AtomicI32 = AtomicI32::new(1);
            SEQUENCE_COUNTER.fetch_add(1, Ordering::SeqCst)
        }

        /// Get the full type name exactly matching C++ namespace
        pub const fn type_name() -> &'static str {
            "DistributedATS_NewOrderSingle::NewOrderSingle"
        }
    }

    /// Generate unique ClOrdID following financial industry standards
    pub fn generate_unique_cl_ord_id() -> String {
        let timestamp = Utc::now().timestamp_micros();
        let uuid_suffix = uuid::Uuid::new_v4().to_string()[..6].to_string();
        format!("CLO_{}_{}", timestamp, uuid_suffix)
    }
}

// Re-export for easier usage
pub use distributed_ats_new_order_single::*;

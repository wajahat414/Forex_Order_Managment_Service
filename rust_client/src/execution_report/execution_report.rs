// src/execution_report.rs - New module for execution report handling
use chrono::Utc;
use rustdds::*;
use serde::{Deserialize, Serialize};

/// Module matching C++ DistributedATS_ExecutionReport namespace
pub mod distributed_ats_execution_report {
    use super::*;

    /// Header structure matching DistributedATS::Header IDL (reused from NewOrderSingle)
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
                msg_type: "8".to_string(), // ExecutionReport message type
                sender_comp_id: "OMS".to_string(),
                target_comp_id: "RUST_CLIENT".to_string(),
                msg_seq_num: 1,
                sending_time: Utc::now().format("%Y%m%d-%H:%M:%S%.3f").to_string(),
                checksum: "000".to_string(),
            }
        }
    }

    /// ✅ ExecutionReport struct exactly matching C++ DistributedATS_ExecutionReport::ExecutionReport
    #[derive(Debug, Clone, Serialize, Deserialize)]
    pub struct ExecutionReport {
        // Fields matching exactly the C++ class structure
        #[serde(rename = "fix_header")]
        pub fix_header: Header,

        #[serde(rename = "DATS_Source")]
        pub dats_source: String,

        #[serde(rename = "DATS_Destination")]
        pub dats_destination: String,

        #[serde(rename = "DATS_SourceUser")]
        pub dats_source_user: String,

        #[serde(rename = "DATS_DestinationUser")]
        pub dats_destination_user: String,

        #[serde(rename = "OrderID")]
        pub order_id: String,

        #[serde(rename = "OrigClOrdID")]
        pub orig_cl_ord_id: String,

        #[serde(rename = "ExecID")]
        pub exec_id: String,

        #[serde(rename = "ExecType")]
        pub exec_type: String, // C++ char mapped to String for FastDDS compatibility

        #[serde(rename = "OrdStatus")]
        pub ord_status: String, // C++ char mapped to String for FastDDS compatibility

        #[serde(rename = "OrdRejReason")]
        pub ord_rej_reason: i32,

        #[serde(rename = "Symbol")]
        pub symbol: String,

        #[serde(rename = "SecurityExchange")]
        pub security_exchange: String,

        #[serde(rename = "Side")]
        pub side: String, // C++ char mapped to String for FastDDS compatibility

        #[serde(rename = "OrderQty")]
        pub order_qty: i32,

        #[serde(rename = "OrdType")]
        pub ord_type: String, // C++ char mapped to String for FastDDS compatibility

        #[serde(rename = "Price")]
        pub price: f32, // Matching C++ float

        #[serde(rename = "StopPx")]
        pub stop_px: f32, // Matching C++ float

        #[serde(rename = "TimeInForce")]
        pub time_in_force: String, // C++ char mapped to String for FastDDS compatibility

        #[serde(rename = "ExecInst")]
        pub exec_inst: String,

        #[serde(rename = "LastQty")]
        pub last_qty: i32,

        #[serde(rename = "LastPx")]
        pub last_px: f32, // Matching C++ float

        #[serde(rename = "LeavesQty")]
        pub leaves_qty: i32,

        #[serde(rename = "CumQty")]
        pub cum_qty: i32,

        #[serde(rename = "AvgPx")]
        pub avg_px: f32, // Matching C++ float

        #[serde(rename = "TransactTime")]
        pub transact_time: u64,

        #[serde(rename = "Text")]
        pub text: String,
    }

    impl ExecutionReport {
        /// Get the full type name exactly matching C++ namespace
        pub const fn type_name() -> &'static str {
            "DistributedATS_ExecutionReport::ExecutionReport"
        }

        /// Check if this is an order acceptance execution report
        pub fn is_order_accepted(&self) -> bool {
            self.exec_type == "0" && self.ord_status == "0" // New order, pending new
        }

        /// Check if this is an order rejection execution report
        pub fn is_order_rejected(&self) -> bool {
            self.exec_type == "8" && self.ord_status == "8" // Rejected
        }

        /// Check if this is a fill execution report
        pub fn is_fill(&self) -> bool {
            self.exec_type == "F" && (self.ord_status == "1" || self.ord_status == "2") // Partially filled or filled
        }

        /// Check if order is completely filled
        pub fn is_completely_filled(&self) -> bool {
            self.ord_status == "2" // Filled
        }

        /// Get execution status as human readable string
        pub fn get_execution_status(&self) -> &'static str {
            match self.ord_status.as_str() {
                "0" => "Pending New",
                "1" => "Partially Filled",
                "2" => "Filled",
                "4" => "Cancelled",
                "8" => "Rejected",
                "A" => "Pending Cancel",
                "C" => "Expired",
                _ => "Unknown",
            }
        }

        /// Get execution type as human readable string
        pub fn get_execution_type(&self) -> &'static str {
            match self.exec_type.as_str() {
                "0" => "New",
                "4" => "Cancelled",
                "8" => "Rejected",
                "F" => "Trade",
                "I" => "Order Status",
                _ => "Unknown",
            }
        }

        /// Calculate remaining quantity
        pub fn get_remaining_qty(&self) -> i32 {
            self.order_qty - self.cum_qty
        }

        /// Check if order has any fills
        pub fn has_fills(&self) -> bool {
            self.cum_qty > 0
        }
    }
}

// Re-export for easier usage
pub use distributed_ats_execution_report::*;

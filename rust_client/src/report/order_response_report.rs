use rustdds::*;
use serde::{Deserialize, Serialize};

/// ✅ FIXED: OrderResponseReport struct exactly matching C++ FastDDS IDL
/// Following OMS architecture patterns for reliable financial message handling
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OrderResponseReport {
    // ✅ String fields - these match correctly
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

    // ✅ CRITICAL FIXES: Use correct types matching C++ char fields
    #[serde(rename = "ExecType")]
    pub exec_type: u8, // ✅ FIXED: C++ char maps to Rust u8

    #[serde(rename = "OrdStatus")]
    pub ord_status: u8, // ✅ FIXED: C++ char maps to Rust u8

    #[serde(rename = "OrdRejReason")]
    pub ord_rej_reason: i32, // ✅ CORRECT: C++ long (32-bit) maps to Rust i32

    #[serde(rename = "Symbol")]
    pub symbol: String,

    #[serde(rename = "SecurityExchange")]
    pub security_exchange: String,

    #[serde(rename = "Side")]
    pub side: u8, // ✅ FIXED: C++ char maps to Rust u8

    #[serde(rename = "OrderQty")]
    pub order_qty: i32, // ✅ CORRECT: C++ long (32-bit) maps to Rust i32

    #[serde(rename = "OrdType")]
    pub ord_type: u8, // ✅ FIXED: C++ char maps to Rust u8

    #[serde(rename = "Price")]
    pub price: f32, // ✅ CORRECT: C++ float maps to Rust f32

    #[serde(rename = "StopPx")]
    pub stop_px: f32, // ✅ CORRECT: C++ float maps to Rust f32

    #[serde(rename = "TimeInForce")]
    pub time_in_force: u8, // ✅ FIXED: C++ char maps to Rust u8

    #[serde(rename = "ExecInst")]
    pub exec_inst: String,

    #[serde(rename = "LastQty")]
    pub last_qty: i32, // ✅ CORRECT: C++ long (32-bit) maps to Rust i32

    #[serde(rename = "LastPx")]
    pub last_px: f32, // ✅ CORRECT: C++ float maps to Rust f32

    #[serde(rename = "LeavesQty")]
    pub leaves_qty: i32, // ✅ CORRECT: C++ long (32-bit) maps to Rust i32

    #[serde(rename = "CumQty")]
    pub cum_qty: i32, // ✅ CORRECT: C++ long (32-bit) maps to Rust i32

    #[serde(rename = "AvgPx")]
    pub avg_px: f32, // ✅ CORRECT: C++ float maps to Rust f32

    #[serde(rename = "TransactTime")]
    pub transact_time: u64, // ✅ CORRECT: C++ unsigned long long maps to Rust u64

    #[serde(rename = "Text")]
    pub text: String,
}

impl OrderResponseReport {
    /// Get the exact type name for FastDDS compatibility
    pub const fn type_name() -> &'static str {
        "OMS::OrderResponseReport"
    }

    /// ✅ FIXED: Helper methods using u8 values for FIX protocol compliance
    pub fn is_order_accepted(&self) -> bool {
        self.exec_type == b'0' && self.ord_status == b'0' // New order, pending new
    }

    pub fn is_order_rejected(&self) -> bool {
        self.exec_type == b'8' && self.ord_status == b'8' // Rejected
    }

    pub fn is_fill(&self) -> bool {
        self.exec_type == b'F' && (self.ord_status == b'1' || self.ord_status == b'2')
        // Partially filled or filled
    }

    pub fn is_completely_filled(&self) -> bool {
        self.ord_status == b'2' // Filled
    }

    /// Get execution status as human readable string following FIX protocol
    pub fn get_execution_status(&self) -> &'static str {
        match self.ord_status {
            b'0' => "Pending New",
            b'1' => "Partially Filled",
            b'2' => "Filled",
            b'4' => "Cancelled",
            b'8' => "Rejected",
            b'A' => "Pending Cancel",
            b'C' => "Expired",
            _ => "Unknown",
        }
    }

    /// Get execution type as human readable string following FIX protocol
    pub fn get_execution_type(&self) -> &'static str {
        match self.exec_type {
            b'0' => "New",
            b'4' => "Cancelled",
            b'8' => "Rejected",
            b'F' => "Trade",
            b'I' => "Order Status",
            _ => "Unknown",
        }
    }

    /// Get order side as human readable string following FIX protocol
    pub fn get_order_side(&self) -> &'static str {
        match self.side {
            b'1' => "Buy",
            b'2' => "Sell",
            _ => "Unknown",
        }
    }

    /// Get order type as human readable string following FIX protocol
    pub fn get_order_type(&self) -> &'static str {
        match self.ord_type {
            b'1' => "Market",
            b'2' => "Limit",
            b'3' => "Stop",
            b'4' => "Stop Limit",
            _ => "Unknown",
        }
    }

    /// Get time in force as human readable string following FIX protocol
    pub fn get_time_in_force(&self) -> &'static str {
        match self.time_in_force {
            b'0' => "Day",
            b'1' => "Good Till Cancel",
            b'2' => "At The Opening",
            b'3' => "Immediate Or Cancel",
            b'4' => "Fill Or Kill",
            b'6' => "Good Till Date",
            _ => "Unknown",
        }
    }

    /// Calculate remaining quantity following OMS position tracking
    pub fn get_remaining_qty(&self) -> i32 {
        self.order_qty - self.cum_qty
    }

    /// Check if order has any fills for execution monitoring
    pub fn has_fills(&self) -> bool {
        self.cum_qty > 0
    }

    /// Get execution summary for audit trail following OMS compliance
    pub fn get_execution_summary(&self) -> String {
        format!(
            "OrderID: {}, Status: {}, Side: {}, Symbol: {}, Qty: {}/{}, Price: {}, AvgPx: {}",
            self.order_id,
            self.get_execution_status(),
            self.get_order_side(),
            self.symbol,
            self.cum_qty,
            self.order_qty,
            self.price,
            self.avg_px
        )
    }
}

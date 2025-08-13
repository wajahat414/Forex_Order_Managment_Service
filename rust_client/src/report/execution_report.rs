use rustdds::*;
use serde::{Deserialize, Serialize};

/// FIX Header structure matching C++ DistributedATS::Header
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Header {
    #[serde(rename = "BeginString")]
    pub begin_string: String,

    #[serde(rename = "BodyLength")]
    pub body_length: i32, // C++ long maps to i32

    #[serde(rename = "MsgType")]
    pub msg_type: String,

    #[serde(rename = "SenderCompID")]
    pub sender_comp_id: String,

    #[serde(rename = "TargetCompID")]
    pub target_comp_id: String,

    #[serde(rename = "MsgSeqNum")]
    pub msg_seq_num: i32, // C++ long maps to i32

    #[serde(rename = "SenderSubID")]
    pub sender_sub_id: String,

    #[serde(rename = "TargetSubID")]
    pub target_sub_id: String,

    #[serde(rename = "SendingTime")]
    pub sending_time: u64, // C++ unsigned long long maps to u64
}

impl Default for Header {
    fn default() -> Self {
        Self {
            begin_string: String::new(),
            body_length: 0,
            msg_type: String::new(),
            sender_comp_id: String::new(),
            target_comp_id: String::new(),
            msg_seq_num: 0,
            sender_sub_id: String::new(),
            target_sub_id: String::new(),
            sending_time: 0,
        }
    }
}

/// ✅ FIXED: ExecutionReport struct exactly matching C++ FastDDS IDL structure
/// Following OMS architecture patterns for reliable financial message handling
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ExecutionReport {
    // ✅ DATS routing fields - exact field order as C++ IDL
    #[serde(rename = "DATS_Source")]
    pub dats_source: String,

    #[serde(rename = "DATS_Destination")]
    pub dats_destination: String,

    #[serde(rename = "DATS_SourceUser")]
    pub dats_source_user: String,

    #[serde(rename = "DATS_DestinationUser")]
    pub dats_destination_user: String,

    // ✅ CRITICAL FIX: Add missing FIX header field
    #[serde(rename = "fix_header")]
    pub fix_header: Header,

    // ✅ Core execution report fields in exact IDL order
    #[serde(rename = "OrderID")]
    pub order_id: String,

    #[serde(rename = "OrigClOrdID")]
    pub orig_cl_ord_id: String,

    #[serde(rename = "ExecID")]
    pub exec_id: String,

    #[serde(rename = "ExecType")]
    pub exec_type: u8, // C++ char maps to Rust u8

    #[serde(rename = "OrdStatus")]
    pub ord_status: u8, // C++ char maps to Rust u8

    #[serde(rename = "OrdRejReason")]
    pub ord_rej_reason: i32, // C++ long maps to Rust i32

    #[serde(rename = "Symbol")]
    pub symbol: String,

    #[serde(rename = "SecurityExchange")]
    pub security_exchange: String,

    #[serde(rename = "Side")]
    pub side: u8, // C++ char maps to Rust u8

    #[serde(rename = "OrderQty")]
    pub order_qty: i32, // C++ long maps to Rust i32

    #[serde(rename = "OrdType")]
    pub ord_type: u8, // C++ char maps to Rust u8

    #[serde(rename = "Price")]
    pub price: f32, // C++ float maps to Rust f32

    #[serde(rename = "StopPx")]
    pub stop_px: f32, // C++ float maps to Rust f32

    #[serde(rename = "TimeInForce")]
    pub time_in_force: u8, // C++ char maps to Rust u8

    #[serde(rename = "ExecInst")]
    pub exec_inst: String,

    #[serde(rename = "LastQty")]
    pub last_qty: i32, // C++ long maps to Rust i32

    #[serde(rename = "LastPx")]
    pub last_px: f32, // C++ float maps to Rust f32

    #[serde(rename = "LeavesQty")]
    pub leaves_qty: i32, // C++ long maps to Rust i32

    #[serde(rename = "CumQty")]
    pub cum_qty: i32, // C++ long maps to Rust i32

    #[serde(rename = "AvgPx")]
    pub avg_px: f32, // C++ float maps to Rust f32

    #[serde(rename = "TransactTime")]
    pub transact_time: u64, // C++ unsigned long long maps to Rust u64

    #[serde(rename = "Text")]
    pub text: String,
}

impl ExecutionReport {
    /// Get the exact type name for FastDDS compatibility following OMS architecture
    pub const fn type_name() -> &'static str {
        "DistributedATS_ExecutionReport::ExecutionReport"
    }

    /// Helper methods using u8 values for FIX protocol compliance
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

    /// Access FIX header information following OMS audit requirements
    pub fn get_sender_comp_id(&self) -> &str {
        &self.fix_header.sender_comp_id
    }

    pub fn get_target_comp_id(&self) -> &str {
        &self.fix_header.target_comp_id
    }

    pub fn get_msg_seq_num(&self) -> i32 {
        self.fix_header.msg_seq_num
    }

    pub fn get_sending_time(&self) -> u64 {
        self.fix_header.sending_time
    }

    /// Calculate remaining quantity following OMS position tracking
    pub fn get_remaining_qty(&self) -> i32 {
        self.order_qty - self.cum_qty
    }

    /// Check if order has any fills for execution monitoring
    pub fn has_fills(&self) -> bool {
        self.cum_qty > 0
    }

    /// Get comprehensive execution summary for audit trail following OMS compliance
    pub fn get_execution_summary(&self) -> String {
        format!(
            "OrderID: {}, ExecID: {}, Status: {}, Side: {}, Symbol: {}, Qty: {}/{}, Price: {}, AvgPx: {}, Sender: {}, SeqNum: {}",
            self.order_id,
            self.exec_id,
            self.get_execution_status(),
            self.get_order_side(),
            self.symbol,
            self.cum_qty,
            self.order_qty,
            self.price,
            self.avg_px,
            self.fix_header.sender_comp_id,
            self.fix_header.msg_seq_num
        )
    }

    /// Get detailed audit information for regulatory compliance
    pub fn get_audit_trail(&self) -> String {
        format!(
            "EXECUTION_AUDIT: OrderID={}, ExecID={}, ExecType={}, OrdStatus={}, Symbol={}, Side={}, Qty={}, Price={}, CumQty={}, AvgPx={}, Source={}, Dest={}, SenderComp={}, TargetComp={}, SeqNum={}, SendingTime={}, TransactTime={}, Text='{}'",
            self.order_id,
            self.exec_id,
            self.get_execution_type(),
            self.get_execution_status(),
            self.symbol,
            self.get_order_side(),
            self.order_qty,
            self.price,
            self.cum_qty,
            self.avg_px,
            self.dats_source,
            self.dats_destination,
            self.fix_header.sender_comp_id,
            self.fix_header.target_comp_id,
            self.fix_header.msg_seq_num,
            self.fix_header.sending_time,
            self.transact_time,
            self.text
        )
    }
}

/// Implement RustDDS serialization traits for FastDDS compatibility
impl rustdds::Keyed for ExecutionReport {
    type K = ();

    fn key(&self) -> Self::K {
        ()
    }
}

/// Implement Default for ExecutionReport following RAII principles
impl Default for ExecutionReport {
    fn default() -> Self {
        Self {
            dats_source: String::new(),
            dats_destination: String::new(),
            dats_source_user: String::new(),
            dats_destination_user: String::new(),
            fix_header: Header::default(),
            order_id: String::new(),
            orig_cl_ord_id: String::new(),
            exec_id: String::new(),
            exec_type: 0,
            ord_status: 0,
            ord_rej_reason: 0,
            symbol: String::new(),
            security_exchange: String::new(),
            side: 0,
            order_qty: 0,
            ord_type: 0,
            price: 0.0,
            stop_px: 0.0,
            time_in_force: 0,
            exec_inst: String::new(),
            last_qty: 0,
            last_px: 0.0,
            leaves_qty: 0,
            cum_qty: 0,
            avg_px: 0.0,
            transact_time: 0,
            text: String::new(),
        }
    }
}

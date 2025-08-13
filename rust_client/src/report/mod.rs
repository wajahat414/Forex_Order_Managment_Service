//! Execution Report module following OMS architecture patterns
//! Provides comprehensive execution report handling for financial trading systems

pub mod execution_report;
pub mod execution_report_listener;
pub mod order_response_report;
pub mod order_response_report_listener;

// Re-export key types for easier usage following OMS patterns
pub use execution_report::ExecutionReport;
pub use execution_report_listener::{ExecutionReportConnectionStatus, ExecutionReportListener};
pub use order_response_report::OrderResponseReport;
pub use order_response_report_listener::{
    OrderResponseListener, OrderResponseReportConnectionStatus,
};

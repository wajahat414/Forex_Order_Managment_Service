//! Execution Report module following OMS architecture patterns
//! Provides comprehensive execution report handling for financial trading systems

pub mod execution_report;
pub mod execution_report_listener;

// Re-export key types for easier usage following OMS patterns
pub use execution_report::{ExecutionReport, Header as ExecutionReportHeader};
pub use execution_report_listener::{ExecutionReportConnectionStatus, ExecutionReportListener};

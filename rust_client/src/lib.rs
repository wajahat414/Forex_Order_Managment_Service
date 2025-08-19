pub mod dds_client;
pub mod report;

pub mod common;
pub mod new_order_single;
pub mod order_message;

// Re-export key types for easier usage following OMS architecture patterns
pub use dds_client::{ConnectionStatus, OrderDdsClient};
pub use new_order_single::{generate_unique_cl_ord_id, Header, NewOrderSingle};
pub use order_message::{generate_unique_order_id, OrderRequest, OrderSide, OrderType};
pub use report::OrderResponseReport;

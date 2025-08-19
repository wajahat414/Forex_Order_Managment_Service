use anyhow::Context;
use anyhow::Result;
use log::info;
use rustdds::no_key::DataReader;
use rustdds::no_key::DataWriter;
use rustdds::policy;
use rustdds::CDRDeserializerAdapter;
use rustdds::CDRSerializerAdapter;
use rustdds::DomainParticipant;
use rustdds::Publisher;
use rustdds::QosPolicyBuilder;
use rustdds::TopicKind;
use std::sync::Arc;
use std::time::Duration;
use tokio::time::sleep;

use crate::report::ExecutionReport;
use crate::NewOrderSingle;
use crate::OrderRequest;
use crate::OrderResponseReport;

const EXECUTION_REPORT_TOPIC_NAME: &str = "EXECUTION_REPORT_TOPIC";
const EXECUTION_REPORT_TOPIC_TYPE_NAME: &str = "DistributedATS_ExecutionReport::ExecutionReport";
const DEFAULT_DOMAIN_ID: u16 = 0;

// Order Response Topic
const ORDER_RESPONSE_TOPIC_NAME: &str = "order_response";
const ORDER_RESPONSE_TOPIC_TYPE: &str = "OMS::OrderResponseReport";

// Order Request Topic
const ORDER_REQUEST_TOPIC_NAME: &str = "new_order_request";
const ORDER_REQUEST_TOPIC_TYPE: &str = "OrderRequest";
const NEW_ORDER_SINGLE_TOPIC_NAME: &str = "NEW_ORDER_SINGLE_TOPIC";
const NEW_ORDER_SINGLE_TOPIC_TYPE: &str = "DistributedATS_NewOrderSingle::NewOrderSingle";

pub struct DdsInitializer {
    pub execution_report_data_reader: DataReader<ExecutionReport>,
    pub order_response_datareader: DataReader<OrderResponseReport>,
    pub order_request_data_writer: DataWriter<OrderRequest>,
    pub new_order_single_data_writer: DataWriter<NewOrderSingle>,
}

impl DdsInitializer {
    pub async fn initialze() -> Result<Self> {
        let participant = DomainParticipant::new(DEFAULT_DOMAIN_ID)
            .context("Failed to create DDS domain participant for ExecutionReport listener")?;

        info!(
            "✅ Created DDS domain participant on domain {} for execution reports",
            DEFAULT_DOMAIN_ID
        );

        let qos = QosPolicyBuilder::new()
            .best_effort()
            .reliability(policy::Reliability::Reliable {
                max_blocking_time: rustdds::Duration::ZERO,
            })
            .build();

        let subscriber = participant
            .create_subscriber(&qos)
            .context("Failed to create DDS subscriber for execution reports")?;

        let publisher: Publisher = participant
            .create_publisher(&qos)
            .context("failed to create Publisher")?;

        let execution_report_topic = participant
            .create_topic(
                EXECUTION_REPORT_TOPIC_NAME.to_string(),
                EXECUTION_REPORT_TOPIC_TYPE_NAME.to_string(), // Uses exact C++ type name
                &qos,
                TopicKind::NoKey, // Matches C++ implementation
            )
            .context("Failed to create ExecutionReport topic following OMS architecture")?;

        let execution_report_data_reader: DataReader<ExecutionReport> = subscriber
            .create_datareader_no_key::<ExecutionReport, CDRDeserializerAdapter<ExecutionReport>>(
                &execution_report_topic,
                None,
            )
            .context("Failed to create ExecutionReport reader with FastDDS compatibility")?;

        sleep(Duration::from_millis(500)).await;

        let order_response_topic = participant
            .create_topic(
                ORDER_RESPONSE_TOPIC_NAME.to_string(),
                ORDER_RESPONSE_TOPIC_TYPE.to_string(), // Uses exact C++ type name
                &qos,
                TopicKind::NoKey, // Matches C++ implementation
            )
            .context("Failed to create Order Response topic following OMS architecture")?;

        let order_response_reader = subscriber
            .create_datareader_no_key::<OrderResponseReport, CDRDeserializerAdapter<OrderResponseReport>>(
                &order_response_topic,
                None,
            )
            .context("Failed to create OrderResponse reader with FastDDS compatibility")?;

        info!("✅ Created OrderResponse listener components following OMS guidelines");

        // Order Request Publishers

        let order_request_to_oms_topic = participant
            .create_topic(
                ORDER_REQUEST_TOPIC_NAME.to_string(),
                ORDER_REQUEST_TOPIC_TYPE.to_string(),
                &qos,
                TopicKind::NoKey,
            )
            .unwrap();

        let order_request_to_oms_writer = publisher
            .create_datawriter_no_key::<OrderRequest, CDRSerializerAdapter<OrderRequest>>(
                &order_request_to_oms_topic,
                None,
            )
            .unwrap();

        // order request to DIRECT Matching Engine Writer
        let new_order_single_topic = participant
            .create_topic(
                NEW_ORDER_SINGLE_TOPIC_NAME.to_string(),
                NEW_ORDER_SINGLE_TOPIC_TYPE.to_string(),
                &qos,
                TopicKind::NoKey, // NewOrderSingle uses NoKey based on C++ implementation
            )
            .context("Failed to create NewOrderSingle topic")?;

        info!("✅ Created data writer for order publishing");
        let new_order_single_writer = publisher
            .create_datawriter_no_key::<NewOrderSingle, CDRSerializerAdapter<NewOrderSingle>>(
                &new_order_single_topic,
                Some(qos.clone()),
            )
            .context("Failed to create NewOrderSingle writer")?;

        Ok(Self {
            execution_report_data_reader: execution_report_data_reader,
            order_response_datareader: order_response_reader,
            order_request_data_writer: order_request_to_oms_writer,
            new_order_single_data_writer: new_order_single_writer,
        })
    }
}

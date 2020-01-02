#ifndef _QCLOUD_SHADOW_H_
#define  _QCLOUD_SHADOW_H_

/**
 * @brief �豸Ӱ���ĵ�������ص�һЩ�ӿ�
 *
 * �����ṩһЩ�ӿ����ڹ����豸Ӱ���ĵ������豸Ӱ���ĵ����н���; ͨ��DeviceName,
 * �������豸Ӱ�ӽ��н���, ������ǰ�豸���豸Ӱ�Ӻ������豸���豸Ӱ��; һ
 * ���豸һ�������ֲ�ͬ�������豸Ӱ�ӽ���:
 *     1. Get
 *     2. Update
 *     3. Delete
 *
 * �������ֲ���, �ײ㻹�ǻ���MQTTЭ��, ����ԭ��Ҳ�ǻ��ڷ���/����ģ��, ��ִ��
 * ����������, ���յ���Ӧ����Ӧ: 1. accepted; 2. rejected������, ����ִ��
 * Get���豸Ӱ�ӽ��н���, �豸�˽����ͺͽ��յ�һ����Ϣ:
 *     1. ����MQTT����: $shadow/get/{productName}/{deviceName};
 *     2. ����MQTT����: $shadow/get/accepted/{productName}/{deviceName} �� $shadow/get/rejected/{productName}/{deviceName}
 *     3. �����������ɹ��Ļ�, �豸�˻��յ�accepted����, �Լ���Ӧ�豸��json�ĵ���
 */

#define SHADOW_FIELD_VERSION            "version"
#define SHADOW_FIELD_TYPE               "type"
#define SHADOW_FIELD_CLIENT_TOKEN       "clientToken"
#define SHADOW_FIELD_RESULT                 "result"

#define SHADOW_OPERATION_DELTA              "delta"
#define SHADOW_OPERATION_GET                "get"
#define SHADOW_OPERATION_UPDATE         "update"

#define SHADOW_PAYLOAD_STATE            "payload.state"
#define SHADOW_PAYLOAD_VERSION          "payload.version"
#define SHADOW_PAYLOAD_STATE_DELTA      "payload.state.delta"

typedef int32_t     template_int_t;
typedef float       template_float_t;
typedef int8_t      template_bool_t;
typedef char        template_string_t;
typedef uint32_t    template_time_t;
typedef void *      template_object_t;

typedef enum json_data_type_en {
    JSON_DATA_TYPE_INT32,
    JSON_DATA_TYPE_INT16,
    JSON_DATA_TYPE_INT8,
    JSON_DATA_TYPE_UINT32,
    JSON_DATA_TYPE_UINT16,
    JSON_DATA_TYPE_UINT8,
    JSON_DATA_TYPE_FLOAT,
    JSON_DATA_TYPE_DOUBLE,
    JSON_DATA_TYPE_BOOL,
    JSON_DATA_TYPE_STRING,
    JSON_DATA_TYPE_OBJECT,
} json_data_type_t;

typedef enum shadow_type_en {
    SHADOW_TYPE_SHADOW,
    SHADOW_TYPE_TEMPLATE,
} shadow_type_t;

typedef enum shadow_sync_state_en {
    QCLOUD_SHADOW_SYNC_STATE_NONE,
    QCLOUD_SHADOW_SYNC_STATE_PENDACK,
    QCLOUD_SHADOW_SYNC_STATE_SUCCESS,
    QCLOUD_SHADOW_SYNC_STATE_TIMEOUT,
    QCLOUD_SHADOW_SYNC_STATE_NACK,
} shadow_sync_state_t;

typedef struct shadow_device_property_st {
    const char         *key;
    json_data_type_t    type;
    void               *data;
} shadow_dev_property_t;

/**
 * @brief Define callback when device property change
 *
 * @param pJsonValueBuffer property JSON buffer
 * @param valueLength      property length
 * @param DeviceProperty   reference to device property
 */
typedef void (*shadow_property_delta_handler_fn_t)(void *client, const char *json_doc, uint32_t json_doc_len, shadow_dev_property_t *property);

typedef struct qcloud_shadow_property_info_st {
    qcloud_list_t                           list;
    shadow_dev_property_t                  *dev_property;
    shadow_property_delta_handler_fn_t      handler;
} qcloud_shadow_prop_info_t;

typedef enum qcloud_shadow_request_state_en {
    QCLOUD_SHADOW_REQUEST_STATE_NONE,
    QCLOUD_SHADOW_REQUEST_STATE_ACCEPTED,
    QCLOUD_SHADOW_REQUEST_STATE_REJECTED,
    QCLOUD_SHADOW_REQUEST_STATE_TIMEOUT,
} qcloud_shadow_req_state_t;

typedef enum qcloud_shadow_request_method_en {
    QCLOUD_SHADOW_REQUEST_METHOD_GET,
    QCLOUD_SHADOW_REQUEST_METHOD_UPDATE,
    QCLOUD_SHADOW_REQUEST_METHOD_DELETE,
} qcloud_shadow_req_method_t;

/**
 * @brief Define MQTT shadow callback when request response arrived
 *
 * @param method         type of request
 * @param requestAck     response type 
 * @param pJsonDocument  JSON document from server
 * @param userContext    User context
 *
 */
typedef void (*shadow_requset_handler_fn_t)(void *client, qcloud_shadow_req_method_t method, qcloud_shadow_req_state_t req_state, const char *json_doc, void *context);

typedef struct qcloud_shadow_request_option_st {
    qcloud_shadow_req_method_t  method;
    uint32_t                    timeout;
    shadow_requset_handler_fn_t handler;
    void                       *context;
} qcloud_shadow_req_opt_t;

typedef struct qcloud_shadow_request_st {
    qcloud_list_t               list;
    char                        client_token[QCLOUD_SHADOW_CLIENT_TOKEN_MAX];
    qcloud_shadow_req_method_t  method;
    void                       *context;
    osal_timer_t                timer;
    shadow_requset_handler_fn_t handler;
} qcloud_shadow_request_t;

typedef struct qcloud_shadow_request_info_st {
    qcloud_list_t               list;
    qcloud_shadow_request_t    *shadow_request;
} qcloud_shadow_req_info_t;

typedef struct qcloud_shadow_client_st {
    qcloud_mqtt_client_t        mqtt_client;
    mqtt_connect_opt_t          mqtt_connect_opt;
    mqtt_event_handler_t        event_handler;

    shadow_type_t               shadow_type;

    void                       *global_lock;

    uint32_t                    version;
    uint32_t                    token_num;
    shadow_sync_state_t         sync_state;
    char                        device_product_id[QCLOUD_DEVICE_PRODUCT_ID_MAX + 1];
    char                        request_topic_subscribe[QCLOUD_MQTT_TOPIC_SIZE_MAX + 1];
    char                        request_topic_publish[QCLOUD_MQTT_TOPIC_SIZE_MAX + 1];

    qcloud_list_t               request_list;
    uint32_t                    request_list_counter;

    qcloud_list_t               property_list;
} qcloud_shadow_client_t;

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_create(qcloud_shadow_client_t *client,
                                                                            qcloud_device_t *device,
                                                                            mqtt_event_handler_fn_t handler,
                                                                            shadow_type_t shadow_type);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_create(qcloud_shadow_client_t *client,
                                                                            qcloud_device_t *device,
                                                                            mqtt_event_handler_fn_t handler,
                                                                            shadow_type_t shadow_type);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_yield(qcloud_shadow_client_t *client, uint32_t timeout);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_publish(qcloud_shadow_client_t *client, char *topic, mqtt_publish_opt_t *publish_opt);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_subscribe(qcloud_shadow_client_t *client, const char *topic_filter, mqtt_subscribe_opt_t *subscribe_opt);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_unsubscribe(qcloud_shadow_client_t *client, const char *topic_filter);

__QCLOUD_API__ int qcloud_shadow_client_is_connected(qcloud_shadow_client_t *client);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_destroy(qcloud_shadow_client_t *client);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_device_property_register(qcloud_shadow_client_t *client,
                                                                                                shadow_dev_property_t *dev_property,
                                                                                                shadow_property_delta_handler_fn_t handler);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_device_property_unregister(qcloud_shadow_client_t *client, shadow_dev_property_t *dev_property);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_update_async(qcloud_shadow_client_t *client,
                                                                                    char *json_doc,
                                                                                    size_t json_doc_size,
                                                                                    shadow_requset_handler_fn_t handler,
                                                                                    void *context,
                                                                                    uint32_t timeout);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_update_sync(qcloud_shadow_client_t *client, char *json_doc, size_t json_doc_size, uint32_t timeout);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_get_async(qcloud_shadow_client_t *client,
                                                                                shadow_requset_handler_fn_t handler,
                                                                                void *context,
                                                                                uint32_t timeout);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_get_sync(qcloud_shadow_client_t *client, uint32_t timeout);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_report_construct(qcloud_shadow_client_t *client,
                                                                                            char *json_doc,
                                                                                            size_t json_doc_size,
                                                                                            int count, ...);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_report_construct_array(qcloud_shadow_client_t *client,
                                                                                                    char *json_doc,
                                                                                                    size_t json_doc_size,
                                                                                                    int count,
                                                                                                    shadow_dev_property_t *dev_propertys[]);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_overwrite_report_construct(qcloud_shadow_client_t *client,
                                                                                                            char *json_doc,
                                                                                                            size_t json_doc_size,
                                                                                                            int count, ...);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_report_with_desire_null_construct(qcloud_shadow_client_t *client,
                                                                                                                        char *json_doc,
                                                                                                                        size_t json_doc_size,
                                                                                                                        int count, ...);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_desire_null_construct(qcloud_shadow_client_t *client,
                                                                                                    char *json_doc,
                                                                                                    size_t json_doc_size);

__QCLOUD_API__ qcloud_err_t qcloud_shadow_client_desire_construct(qcloud_shadow_client_t *client,
                                                                                            char *json_doc,
                                                                                            size_t json_doc_size,
                                                                                            int count, ...);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_glue_operation_request_subscribe(qcloud_shadow_client_t *client);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_glue_operation_request_unsubscribe(qcloud_shadow_client_t *client);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_glue_request_list_scan(qcloud_shadow_client_t *client);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_glue_request_post(qcloud_shadow_client_t *client,
                                                                    qcloud_shadow_req_opt_t *request_opt,
                                                                    char *json_doc,
                                                                    size_t json_doc_size);

__QCLOUD_INTERNAL__ void shadow_glue_delta_handle(qcloud_shadow_client_t *client, char *delta);

__QCLOUD_INTERNAL__ void shadow_glue_property_list_destroy(qcloud_shadow_client_t *client);

__QCLOUD_INTERNAL__ void shadow_glue_request_list_destroy(qcloud_shadow_client_t *client);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_json_snprintf_rc2errno(int rc, size_t write_max);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_json_client_token_generate(char *json_doc, size_t json_doc_size, uint32_t token_num, char *device_product_id);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_json_node_add(char *json_doc, size_t json_doc_size, const char *key, void *data, json_data_type_t type);

__QCLOUD_INTERNAL__ qcloud_err_t shadow_json_empty_doc_build(char *json_doc, uint32_t token_num, char *device_product_id);

__QCLOUD_INTERNAL__ int shadow_json_client_token_parse(char *json_doc, char **client_token);

__QCLOUD_INTERNAL__ int shadow_json_version_parse(char *json_doc, uint32_t *version);

__QCLOUD_INTERNAL__ int shadow_json_operation_type_parse(char *json_doc, char **pType);

__QCLOUD_INTERNAL__ int shadow_json_operation_result_code_parse(char *json_doc, int16_t *result_code);

__QCLOUD_INTERNAL__ int shadow_json_delta_parse(char *json_doc, char **delta);

__QCLOUD_INTERNAL__ int shadow_json_operation_delta_get(char *json_doc, char **delta);

__QCLOUD_INTERNAL__ int shadow_json_value_update(char *json, shadow_dev_property_t *dev_property);

__QCLOUD_INTERNAL__ int shadow_state_parse(char *json_doc, char **state);

#endif


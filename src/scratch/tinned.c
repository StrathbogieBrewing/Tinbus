#include <stdint.h>

typedef enum cbor_type {
  CBOR_TYPE_UINT = 0,
  CBOR_TYPE_NEGINT,
  CBOR_TYPE_BYTESTRING,
  CBOR_TYPE_STRING,
  CBOR_TYPE_ARRAY,
  CBOR_TYPE_MAP,
  CBOR_TYPE_TAG,
  CBOR_TYPE_FLOAT_CTRL 
} cbor_type;


#define TINNED_MAX_DESC_SIZE (8)
#define TINNED_MAX_NODE_COUNT (15)
#define TINNED_MAX_TREE_DEPTH (3)

#define TINNED_MAX_DEVICE_COUNT (15)

typedef struct {
    uint8_t last_seen_ds;
} tinned_device_t;

tinned_device_t devices[TINNED_MAX_DEVICE_COUNT];

typedef struct {
    uint8_t path[TINNED_MAX_TREE_DEPTH];
} tinned_path_t;

tinned_path_t nodes[TINNED_MAX_NODE_COUNT];

typedef struct {
    uint8_t node_id;
    uint8_t parent_id;
    char descriptor[TINNED_MAX_DESC_SIZE];
} tinned_node_t;

void tinned_rx_frame(uint8_t *buffer, uint8_t size){
    if((cbor_get_type(*buffer) == CBOR_TYPE_FLOAT_CTRL) && (cbor_get_value(*buffer) <= 16 )){
        devices[0].last_seen_ds = time_now_ds;
    }




}


/*
 * Copyright (c) 2015 Cossack Labs Limited
 */

#ifndef PROTOCOLS_KEY_SERVICE_H_
#define PROTOCOLS_KEY_SERVICE_H_

#include "srpc.h"
#include "functions_collection.h"
#include "protocols.h"
#include "stubs_generator.h"

typedef struct{
  srpc_ctx_t* srpc_ctx_;
  uint8_t* tmp_buf_;
  size_t tmp_buf_length_;
}key_service_t;

key_service_t* key_service_create();
protocol_status_t key_service_bind(key_service_t* ctx, const char* endpoint, void* call_ctx);
protocol_status_t key_service_connect(key_service_t* ctx, const char* endpoint, const char* id);
protocol_status_t key_service_sync_connect(key_service_t* ctx, const char* endpoint, const char* id);
protocol_status_t key_service_call(key_service_t* ctx, const char* func_name, const uint8_t* param_buf, const size_t param_buf_length, void (*proxy_handler)(void* ctx, const uint8_t* res_buf, const size_t res_buf_length), void* call_ctx);
protocol_status_t key_service_sync_call(key_service_t* ctx, const char* func_name, const uint8_t* param_buf, const size_t param_buf_length, uint8_t** res_buf, size_t* res_buf_length);
protocol_status_t key_service_destroy(key_service_t* ctx);
const char* key_service_get_user_id(key_service_t* ctx);


HERMES_FUNCTION_DEFINITIONS(key_service, get_read_key);
HERMES_FUNCTION_DEFINITIONS(key_service, get_update_key);
HERMES_FUNCTION_DEFINITIONS(key_service, grant_read_access);
HERMES_FUNCTION_DEFINITIONS(key_service, grant_update_access);
HERMES_FUNCTION_DEFINITIONS(key_service, revoke_read_access);
HERMES_FUNCTION_DEFINITIONS(key_service, revoke_update_access);

extern functions_collection_t hermes__key_service_functions_collection;


#endif /* PROTOCOLS_KEY_SERVICE_H_ */
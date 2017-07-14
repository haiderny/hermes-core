/*
 * Copyright (c) 2017 Cossack Labs Limited
 *
 * This file is part of Hermes.
 *
 * Hermes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hermes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Hermes.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "transport.h"

#include <hermes/common/errors.h>
#include <assert.h>

typedef struct transport_type{
  PyObject* t;
}transport_t;

uint32_t transport_send(void* transport, const uint8_t* buf, const size_t buf_length){
  if(!transport || !buf || !buf_length){
    return HM_FAIL;
  }
  PyObject_CallMethod(((transport_t*)transport)->t, "send", "y#", (const char*)buf, buf_length);
  return HM_SUCCESS;
}

uint32_t transport_recv(void* transport, uint8_t* buf, size_t buf_length){
  if(!transport || !buf || !buf_length){
    return HM_FAIL;
  }
  PyObject* result=PyObject_CallMethod(((transport_t*)transport)->t, "receive", "I", buf_length);
  if(!result){
    return HM_FAIL;
  }
  if(!(PyBytes_Check(result) || (PyBytes_Size(result)!=buf_length))){
    Py_XDECREF(result);
    return HM_FAIL;
  }
  memcpy(buf, PyBytes_AsString(result), buf_length);
  return HM_SUCCESS;
}

hm_rpc_transport_t* transport_create(PyObject* transport){
  hm_rpc_transport_t* result=calloc(1, sizeof(hm_rpc_transport_t));
  if(!result){
    return NULL;
  }
  result->user_data=calloc(1, sizeof(transport_t));
  assert(result->user_data);
  Py_INCREF(transport);
  ((transport_t*)(result->user_data))->t=transport;
  result->send=transport_send;
  result->recv=transport_recv;
  return result;
}

uint32_t transport_destroy(hm_rpc_transport_t** transport){
  if(!transport || !(*transport) || !((*transport)->user_data)){
    return HM_FAIL;
  }
  Py_XDECREF(((transport_t*)((*transport)->user_data))->t);
  free((*transport)->user_data);
  free(*transport);
  *transport=NULL;
  return HM_SUCCESS;
}
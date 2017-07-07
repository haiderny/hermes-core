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

#include "commands.h"
#include <hermes/mid_hermes/interfaces/key_store.h>
#include <hermes/mid_hermes/interfaces/credential_store.h>
#include <hermes/mid_hermes/interfaces/data_store.h>
#include "utils.h"
#include <hermes/mid_hermes/mid_hermes_ll_user.h>
#include <hermes/mid_hermes/mid_hermes_ll_block.h>
#include <hermes/mid_hermes/mid_hermes_ll_rights_list.h>

#include <assert.h>
#include <string.h>

extern hermes_key_store_t* ks;
extern hermes_credential_store_t* cs;
extern hermes_data_store_t* ds;

int write_block(mid_hermes_ll_block_t* bl, mid_hermes_ll_rights_list_t* rights){
  if(!bl){
    return 1;
  }
  if(!bl->save(bl, rights, ds, ks)){
    return 1;
  }
  return 0;
}

mid_hermes_ll_user_t* create_user(const char* user_id, const char* user_sk){
  mid_hermes_ll_buffer_t* pk=mid_hermes_ll_buffer_create(NULL,0);
  assert(pk);
  if(0!=hermes_credential_store_get_public_key(cs, user_id, strlen(user_id)+1, &(pk->data), &(pk->length))){
    mid_hermes_ll_buffer_destroy(&pk);
    return NULL;
  }
  uint8_t ssk[2048];
  size_t ssk_length=string_to_buf(user_sk, ssk);
  if(!ssk_length){
    mid_hermes_ll_buffer_destroy(&pk);
    return NULL;
  }
  return mid_hermes_ll_local_user_create(mid_hermes_ll_buffer_create(user_id, strlen(user_id)+1),
                                         mid_hermes_ll_buffer_create(ssk, ssk_length),
                                         pk);
}

int add_block(const char* user_id, const char* user_sk, const char* block_file_name, const char* block_meta_data){
  mid_hermes_ll_user_t* u=create_user(user_id, user_sk);
  assert(u);
  mid_hermes_ll_buffer_t* data=mid_hermes_ll_buffer_create(NULL, 0);
  if(0!=read_whole_file(block_file_name, &(data->data), &(data->length))){
    mid_hermes_ll_buffer_destroy(&data);
    return 1;
  }
  mid_hermes_ll_block_t* bl=mid_hermes_ll_block_create(u, mid_hermes_ll_buffer_create(block_file_name, strlen(block_file_name)+1), data, mid_hermes_ll_buffer_create(block_meta_data, strlen(block_meta_data)+1), NULL, NULL);
  if(!bl){
    mid_hermes_ll_user_destroy(&u);
    mid_hermes_ll_buffer_destroy(&data);
    return 1;
  }
  if(0!=write_block(bl, NULL)){
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  mid_hermes_ll_block_destroy(&bl);
  return 0;
}

int upd_block(const char* user_id, const char* user_sk, const char* block_file_name, const char* block_meta_data){
  mid_hermes_ll_user_t* u=create_user(user_id, user_sk);
  mid_hermes_ll_block_t* bl=mid_hermes_ll_block_create_empty(u);
  mid_hermes_ll_buffer_t* block_id=mid_hermes_ll_buffer_create(block_file_name, strlen(block_file_name)+1);
  mid_hermes_ll_buffer_t* new_meta=mid_hermes_ll_buffer_create(block_meta_data, strlen(block_meta_data)+1);
  mid_hermes_ll_buffer_t* new_data=mid_hermes_ll_buffer_create(NULL, 0);
  if(!u
     || !bl
     || !block_id
     || !new_meta
     || !new_data
     || (0!=read_whole_file(block_file_name, &(new_data->data), &(new_data->length)))
     || !(bl->load(bl, block_id, ds, ks, cs))){
    mid_hermes_ll_buffer_destroy(&block_id);
    mid_hermes_ll_buffer_destroy(&new_data);
    mid_hermes_ll_buffer_destroy(&new_meta);
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  if(!(bl->update(bl, new_data, new_meta))){
    mid_hermes_ll_buffer_destroy(&new_data);
    mid_hermes_ll_buffer_destroy(&new_meta);
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  if(!bl->save(bl, NULL, ds, ks)){
    return 1;
    mid_hermes_ll_block_destroy(&bl);
  }
  mid_hermes_ll_block_destroy(&bl);
  return 0;
}

int del_block(const char* user_id, const char* user_sk, const char* block_file_name){
  mid_hermes_ll_user_t* u=create_user(user_id, user_sk);
  mid_hermes_ll_block_t* bl=mid_hermes_ll_block_create_empty(u);
  mid_hermes_ll_buffer_t* block_id=mid_hermes_ll_buffer_create(block_file_name, strlen(block_file_name)+1);
  if(!u
     || !bl
     || !block_id
     || !(bl->load(bl, block_id, ds, ks, cs))){
    mid_hermes_ll_buffer_destroy(&block_id);
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  if(!bl->delete(bl, NULL, ds, ks)){
    return 1;
    mid_hermes_ll_block_destroy(&bl);
  }
  mid_hermes_ll_block_destroy(&bl);
  return 0;
}

int get_block(const char* user_id, const char* user_sk, const char* block_file_name){
  mid_hermes_ll_user_t* u=create_user(user_id, user_sk);
  assert(u);
  mid_hermes_ll_block_t* bl=mid_hermes_ll_block_create_empty(u);
  mid_hermes_ll_buffer_t* block_id=mid_hermes_ll_buffer_create(block_file_name, strlen(block_file_name)+1);
  assert(bl);
  if(!(bl->load(bl, block_id, ds, ks, cs))){
    mid_hermes_ll_block_destroy(&bl);
    mid_hermes_ll_buffer_destroy(&block_id);
    return 1;
  }
  char block_fname[256];
  sprintf(block_fname, "%s_block", block_file_name);
  if(0!=write_whole_file(block_fname, bl->data->data, bl->data->length)){
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  sprintf(block_fname, "%s_meta", block_file_name);
  if(0!=write_whole_file(block_fname, bl->meta->data, bl->meta->length)){
    mid_hermes_ll_block_destroy(&bl);
    return 1;
  }
  mid_hermes_ll_block_destroy(&bl);
  return 0;
}
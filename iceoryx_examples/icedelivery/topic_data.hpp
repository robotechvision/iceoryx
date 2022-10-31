// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_EXAMPLES_ICEDELIVERY_TOPIC_DATA_HPP
#define IOX_EXAMPLES_ICEDELIVERY_TOPIC_DATA_HPP

//! [topic data]
// struct RadarObject
// {
//     RadarObject() noexcept
//     {
//     }
//     RadarObject(double x, double y, double z) noexcept
//         : x(x)
//         , y(y)
//         , z(z)
//     {
//     }
//     double x = 0.0;
//     double y = 0.0;
//     double z = 0.0;
// };

typedef enum {
  IOX_CHUNK_UNINITIALIZED,
  IOX_CHUNK_CONTAINS_RAW_DATA,
  IOX_CHUNK_CONTAINS_SERIALIZED_DATA
} iox_shm_data_state_t;

typedef struct ddsi_keyhash {
  unsigned char value[16];
} ddsi_keyhash_t;

typedef int64_t dds_time_t;
typedef union ddsi_guid_prefix {
  unsigned char s[12];
  uint32_t u[3];
} ddsi_guid_prefix_t;
typedef union ddsi_entityid {
  uint32_t u;
} ddsi_entityid_t;
typedef struct ddsi_guid {
  ddsi_guid_prefix_t prefix;
  ddsi_entityid_t entityid;
} ddsi_guid_t;

struct iceoryx_header {
  struct ddsi_guid guid;
  dds_time_t tstamp;
  uint32_t statusinfo;
  uint32_t data_size;
  unsigned char data_kind;
  ddsi_keyhash_t keyhash;
  iox_shm_data_state_t shm_data_state;
};
typedef struct iceoryx_header iceoryx_header_t;
enum ddsi_serdata_kind {
  SDK_EMPTY,
  SDK_KEY,
  SDK_DATA
};


#include <pthread.h>
typedef struct {
  pthread_mutex_t mutex;
} ddsrt_mutex_t;
enum shm_monitor_states {
    SHM_MONITOR_NOT_RUNNING = 0,
    SHM_MONITOR_RUNNING = 1
};

/// @brief abstraction for monitoring the shared memory communication with an internal
///        thread responsible for reacting on received data via shared memory
struct shm_monitor {
    ddsrt_mutex_t m_lock;
    // iox_listener_t m_listener;

    // //use this if we wait but want to wake up for some reason e.g. terminate   
    // iox_user_trigger_t m_wakeup_trigger;
   
    uint32_t m_number_of_attached_readers;
    uint32_t m_state;
};
typedef struct {  
  ddsrt_mutex_t mutex;
  struct shm_monitor *monitor;
  const double* chunks[10000];
  int chunksCnt;
  pthread_mutex_t chunks_mutex;
  pthread_cond_t new_chunks_trigger;
  // iox_sub_t iox_sub;
} iox_sub_context_t;
//! [topic data]

/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define CLOCK_REALTIME 0
#define DDS_NSECS_IN_SEC INT64_C(1000000000)

dds_time_t dds_time(void)
{
  struct timespec ts;
  (void)clock_gettime(CLOCK_REALTIME, &ts);
  return (ts.tv_sec * DDS_NSECS_IN_SEC) + ts.tv_nsec;
}

#endif // IOX_EXAMPLES_ICEDELIVERY_TOPIC_DATA_HPP

// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_binding_c/internal/exclusivity_check.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"

extern "C" {
#include "iceoryx_binding_c/config.h"
}

uint32_t iox_cfg_max_publishers()
{ CHECK_EXCL
    return iox::MAX_PUBLISHERS;
UNCHECK_EXCL }

uint32_t iox_cfg_max_subscribers_per_publisher()
{ CHECK_EXCL
    return iox::MAX_SUBSCRIBERS_PER_PUBLISHER;
UNCHECK_EXCL }

uint32_t iox_cfg_max_chunks_allocated_per_publisher_simultaneously()
{ CHECK_EXCL
    return iox::MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY;
UNCHECK_EXCL }

uint64_t iox_cfg_max_publisher_history()
{ CHECK_EXCL
    return iox::MAX_PUBLISHER_HISTORY;
UNCHECK_EXCL }

uint32_t iox_cfg_max_subscribers()
{ CHECK_EXCL
    return iox::MAX_SUBSCRIBERS;
UNCHECK_EXCL }

uint32_t iox_cfg_max_chunks_held_per_subscriber_simultaneously()
{ CHECK_EXCL
    return iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY;
UNCHECK_EXCL }

uint32_t iox_cfg_max_subscriber_queue_capacity()
{ CHECK_EXCL
    return iox::MAX_SUBSCRIBER_QUEUE_CAPACITY;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_condition_variables()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_CONDITION_VARIABLES;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_notifiers_per_condition_variable()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_NOTIFIERS;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_attachments_per_waitset()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_events_per_listener()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_mempools()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_MEMPOOLS;
UNCHECK_EXCL }

uint32_t iox_cfg_max_shm_segments()
{ CHECK_EXCL
    return iox::MAX_SHM_SEGMENTS;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_memory_provider()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_MEMORY_PROVIDER;
UNCHECK_EXCL }

uint32_t iox_cfg_max_number_of_memory_blocks_per_memory_provider()
{ CHECK_EXCL
    return iox::MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER;
UNCHECK_EXCL }

uint32_t iox_cfg_chunk_default_user_payload_alignment()
{ CHECK_EXCL
    return iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
UNCHECK_EXCL }

uint32_t iox_cfg_no_user_header_size()
{ CHECK_EXCL
    return iox::CHUNK_NO_USER_HEADER_SIZE;
UNCHECK_EXCL }

uint32_t iox_cfg_no_user_header_alignment()
{ CHECK_EXCL
    return iox::CHUNK_NO_USER_HEADER_ALIGNMENT;
UNCHECK_EXCL }

uint32_t iox_cfg_max_process_number()
{ CHECK_EXCL
    return iox::MAX_PROCESS_NUMBER;
UNCHECK_EXCL }

uint32_t iox_cfg_service_registry_capacity()
{ CHECK_EXCL
    return iox::SERVICE_REGISTRY_CAPACITY;
UNCHECK_EXCL }

uint32_t iox_cfg_max_findservice_result_size()
{ CHECK_EXCL
    return iox::MAX_FINDSERVICE_RESULT_SIZE;
UNCHECK_EXCL }

uint32_t iox_cfg_max_runtime_name_length()
{ CHECK_EXCL
    return iox::MAX_RUNTIME_NAME_LENGTH;
UNCHECK_EXCL }

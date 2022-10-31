// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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


#include "iceoryx_binding_c/listener.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/chunk.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "sleep_for.h"
#include "topic_data.hpp"

#if !defined(_WIN32)
#include <pthread.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool keepRunning = true;

iox_user_trigger_t heartbeat;

static void sigHandler(int signalValue)
{
    (void)signalValue;
    // caught SIGINT or SIGTERM, now exit gracefully
    keepRunning = false;
}

struct cache_t
{
    double value;
    bool isSet;
};

struct cache_t cache = {.isSet = false};

//! [heartbeat callback]
void heartbeatCallback(iox_user_trigger_t userTrigger)
{
    (void)userTrigger;
    printf("heartbeat received\n");
    fflush(stdout);
}
//! [heartbeat callback]

void* cyclicHeartbeatTrigger(void* dontCare)
{
    (void)dontCare;
    while (keepRunning)
    {
        iox_user_trigger_trigger(heartbeat);
        sleep_for(1000);
    }
    return NULL;
}

int prc_cnt;
void* spin(void* context_v)
{
    iox_sub_context_t *context = (iox_sub_context_t *)context_v;
    const double* chunks[10000];
    int chunksCnt;
    while (keepRunning)
    {
        pthread_mutex_lock(&context->chunks_mutex);
        while (context->chunksCnt == 0) {
            pthread_cond_wait(&context->new_chunks_trigger, &context->chunks_mutex);
        }
        chunksCnt = context->chunksCnt;
        for (int i = 0; i < chunksCnt; i++)
            chunks[i] = context->chunks[i];
        context->chunksCnt = 0;
        pthread_mutex_unlock(&context->chunks_mutex);
        for (int i = 0; i < chunksCnt; i++) {
            printf("%d processed: %lf\n", ++prc_cnt, *chunks[i]);
            pthread_mutex_lock(&context->mutex.mutex);
            iox_sub_release_chunk(context->iox_sub, chunks[i]);
            pthread_mutex_unlock(&context->mutex.mutex);
        }
        fflush(stdout);
    }
    return NULL;
}

iceoryx_header_t *iceoryx_header_from_chunk(const void *iox_chunk) {
  iox_chunk_header_t *chunk_header =
      iox_chunk_header_from_user_payload((void*) iox_chunk);
  return (iceoryx_header_t *)iox_chunk_header_to_user_header(chunk_header);
}

int rcv_cnt = 0;
//! [subscriber callback]
void onSampleReceivedCallback(iox_sub_t subscriber, void * context_data)
{
    iox_sub_context_t *context = (iox_sub_context_t*) context_data;
    if(context->monitor->m_state == SHM_MONITOR_RUNNING) {
        const double* userPayload;
        pthread_mutex_lock(&context->mutex.mutex);
        enum iox_ChunkReceiveResult take_result = iox_sub_take_chunk(subscriber, (const void** const)&userPayload);
        pthread_mutex_unlock(&context->mutex.mutex);
        if (take_result == ChunkReceiveResult_SUCCESS)
        {
            const iceoryx_header_t* ice_hdr = iceoryx_header_from_chunk(userPayload);
            // iox_service_description_t serviceDescription = iox_sub_get_service_description(subscriber);
            cache.value = *userPayload;
            cache.isSet = true;
            printf("%d received: %lf (stamp: %lu)\n", ++rcv_cnt, *userPayload, ice_hdr->tstamp);
            fflush(stdout);
            pthread_mutex_lock(&context->chunks_mutex);
            if (context->chunksCnt == 10000)
                printf("WARNING: too many stored chunks!");
            else {
                context->chunks[context->chunksCnt] = userPayload;
                context->chunksCnt++;
            }
            pthread_cond_broadcast(&context->new_chunks_trigger);
            pthread_mutex_unlock(&context->chunks_mutex);
        }
        else {
            switch(take_result) {
                case ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL :
                {
                // we hold to many chunks and cannot get more
                printf ("TOO_MANY_CHUNKS_HELD_IN_PARALLEL\n");
                break;
                }
                case ChunkReceiveResult_NO_CHUNK_AVAILABLE: {
                // no more chunks to take, ok
                break;
                }
                default : {
                printf ("UNKNOWN ERROR");
                }
            }
        }
    }
    //! [get data]
    
    //! [get data]

    //! [process data]

    //! [process data]
}
//! [subscriber callback]

int main(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init("iox-c-callback-subscriber");

    // the listener starts a background thread and the callbacks of the attached events
    // will be called in this background thread when they are triggered
    //! [create listener]
    iox_listener_storage_t listenerStorage;
    iox_listener_t listener = iox_listener_init(&listenerStorage);
    //! [create listener]

    //! [create heartbeat]
    iox_user_trigger_storage_t heartbeatStorage;
    heartbeat = iox_user_trigger_init(&heartbeatStorage);
    //! [create heartbeat]

    //! [set subscriber options]
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 10U;
    options.queueCapacity = 50U;
    options.nodeName = "iox-c-callback-subscriber-node";
    //! [set subscriber options]

    iox_sub_storage_t subscriberStorage;
    //! [create subscribers]
    iox_sub_t subscriber = iox_sub_init(&subscriberStorage, "DDS_CYCLONE", "std_msgs::msg::dds_::Float64_", "rt/chatter_pod", &options);
    iox_sub_context_t context;
    context.monitor = malloc(sizeof(struct shm_monitor)); 
    context.monitor->m_state = SHM_MONITOR_RUNNING;
    pthread_mutex_init(&context.mutex.mutex, NULL);
    pthread_mutex_init(&context.chunks_mutex, NULL);
    pthread_cond_init(&context.new_chunks_trigger, NULL);
    context.iox_sub = subscriber;
    context.chunksCnt = 0;
    //! [create subscribers]

    //! [send a heartbeat every 4 seconds]
    // pthread_t heartbeatTriggerThread;
    // if (pthread_create(&heartbeatTriggerThread, NULL, cyclicHeartbeatTrigger, NULL))
    // {
    //     printf("failed to create thread\n");
    //     return -1;
    // }

    pthread_t executorThread;
    if (pthread_create(&executorThread, NULL, spin, (void *)&context))
    {
        printf("failed to create thread\n");
        return -1;
    }
    //! [send a heartbeat every 4 seconds]

    //! [attach everything to the listener]
    // from here on the callbacks are called when an event occurs
    // iox_listener_attach_user_trigger_event(listener, heartbeat, &heartbeatCallback);
    if(iox_listener_attach_subscriber_event_with_context_data(listener,
                                                              subscriber,
                                                              SubscriberEvent_DATA_RECEIVED,
                                                              onSampleReceivedCallback,
                                                              &context) != ListenerResult_SUCCESS) {
        printf("error attaching reader\n");    
        return 1;
    }
    //! [attach everything to the listener]

    {
        iox_listener_storage_t listenerStorage;
        iox_listener_t listener = iox_listener_init(&listenerStorage);
        iox_sub_storage_t subscriberStorage;
        //! [create subscribers]
        iox_sub_t subscriber = iox_sub_init(&subscriberStorage, "DDS_CYCLONE", "std_msgs::msg::dds_::Float64_", "rt/chatter_pod2", &options);
        iox_sub_context_t context;
        context.monitor = malloc(sizeof(struct shm_monitor)); 
        context.monitor->m_state = SHM_MONITOR_RUNNING;
        pthread_mutex_init(&context.mutex.mutex, NULL);
        pthread_mutex_init(&context.chunks_mutex, NULL);
        pthread_cond_init(&context.new_chunks_trigger, NULL);
        context.iox_sub = subscriber;
        context.chunksCnt = 0;
        //! [create subscribers]

        //! [send a heartbeat every 4 seconds]
        // pthread_t heartbeatTriggerThread;
        // if (pthread_create(&heartbeatTriggerThread, NULL, cyclicHeartbeatTrigger, NULL))
        // {
        //     printf("failed to create thread\n");
        //     return -1;
        // }

        pthread_t executorThread;
        if (pthread_create(&executorThread, NULL, spin, (void *)&context))
        {
            printf("failed to create thread\n");
            return -1;
        }
        //! [send a heartbeat every 4 seconds]

        //! [attach everything to the listener]
        // from here on the callbacks are called when an event occurs
        // iox_listener_attach_user_trigger_event(listener, heartbeat, &heartbeatCallback);
        if(iox_listener_attach_subscriber_event_with_context_data(listener,
                                                                subscriber,
                                                                SubscriberEvent_DATA_RECEIVED,
                                                                onSampleReceivedCallback,
                                                                &context) != ListenerResult_SUCCESS) {
            printf("error attaching reader\n");    
            return 1;
        }

        //! [wait until someone presses CTRL+C]
        while (keepRunning)
        {
            sleep_for(100);
        }
        pthread_join(executorThread, NULL);
        iox_listener_detach_subscriber_event(listener, subscriber, SubscriberEvent_DATA_RECEIVED);
        iox_sub_deinit(subscriber);
        iox_listener_deinit(listener);
    }
    //! [wait until someone presses CTRL+C]

    // pthread_join(heartbeatTriggerThread, NULL);
    pthread_join(executorThread, NULL);

    // when the listener goes out of scope it will detach all events and when a
    // subscriber goes out of scope it will detach itself from the listener
    //! [optional detachEvent, but not required]
    // iox_listener_detach_user_trigger_event(listener, heartbeat);
    iox_listener_detach_subscriber_event(listener, subscriber, SubscriberEvent_DATA_RECEIVED);
    //! [optional detachEvent, but not required]

    //! [cleanup]
    iox_user_trigger_deinit(heartbeat);
    iox_sub_deinit(subscriber);
    iox_listener_deinit(listener);
    //! [cleanup]

    return 0;
}

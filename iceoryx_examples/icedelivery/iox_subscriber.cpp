// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

//! [include subscriber]
#include "iceoryx_posh/popo/subscriber.hpp"
//! [include subscriber]
#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "topic_data.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-subscriber";

int main()
{
    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create subscriber]
    iox::popo::Subscriber<double, iceoryx_header> subscriber({"DDS_CYCLONE", "std_msgs::msg::dds_::Float64_", "rt/chatter_pod"});
    //! [create subscriber]

    // run until interrupted by Ctrl-C
    while (!iox::posix::hasTerminationRequested())
    {
        subscriber
            .take()
            //! [sample happy path]
            .and_then([](auto& sample) {
                //! [print sample info]
                const iceoryx_header &ice_hdr = sample.getUserHeader();
                std::cout << APP_NAME << " got value: " << *sample
                    << ", stamp: "
                    << ice_hdr.tstamp
                    << ", statusinfo: "
                    << ice_hdr.statusinfo
                    << ", data_kind: "
                    << (int)ice_hdr.data_kind
                    << ", data_size: "
                    << ice_hdr.data_size
                    << ", shm_data_state: "
                    << ice_hdr.shm_data_state
                    << ", keyhash: "<< std::endl
                    << (int)ice_hdr.keyhash.value[0]<<" "<< (int)ice_hdr.keyhash.value[1]<<" " << (int)ice_hdr.keyhash.value[2]<<" " << (int)ice_hdr.keyhash.value[3]<<" "
                    << (int)ice_hdr.keyhash.value[4]<<" " << (int)ice_hdr.keyhash.value[5]<<" " << (int)ice_hdr.keyhash.value[6]<<" " << (int)ice_hdr.keyhash.value[7]<<" "
                    << (int)ice_hdr.keyhash.value[8]<<" " << (int)ice_hdr.keyhash.value[9]<<" " << (int)ice_hdr.keyhash.value[10]<<" " << (int)ice_hdr.keyhash.value[11]<<" "
                    << (int)ice_hdr.keyhash.value[12]<<" " << (int)ice_hdr.keyhash.value[13]<<" " << (int)ice_hdr.keyhash.value[14]<<" " << (int)ice_hdr.keyhash.value[15]
                    << std::endl
                    << ", guid:" << std::endl
                    << (int)ice_hdr.guid.prefix.s[0]<<" "<< (int)ice_hdr.guid.prefix.s[1]<<" " << (int)ice_hdr.guid.prefix.s[2]<<" " << (int)ice_hdr.guid.prefix.s[3]<<" "
                    << (int)ice_hdr.guid.prefix.s[4]<<" " << (int)ice_hdr.guid.prefix.s[5]<<" " << (int)ice_hdr.guid.prefix.s[6]<<" " << (int)ice_hdr.guid.prefix.s[7]<<" "
                    << (int)ice_hdr.guid.prefix.s[8]<<" " << (int)ice_hdr.guid.prefix.s[9]<<" " << (int)ice_hdr.guid.prefix.s[10]<<" " << (int)ice_hdr.guid.prefix.s[11]<<" "
                    << ice_hdr.guid.prefix.u[0]<<" " << ice_hdr.guid.prefix.u[1]<<" " << ice_hdr.guid.prefix.u[2]<<" " << ice_hdr.guid.entityid.u
                    << std::endl;
                
                //! [print sample info]
            })
            //! [sample happy path]
            .or_else([](auto& result) {
                // only has to be called if the alternative is of interest,
                // i.e. if nothing has to happen when no data is received and
                // a possible error alternative is not checked or_else is not needed
                if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                {
                    std::cout << "Error receiving chunk." << std::endl;
                }
            });

        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}

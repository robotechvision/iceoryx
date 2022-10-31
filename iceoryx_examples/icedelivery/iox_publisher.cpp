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

//! [include publisher]
#include "iceoryx_posh/popo/publisher.hpp"
//! [include publisher]
#include "iceoryx_dust/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "topic_data.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-publisher";

// void getRadarObject(RadarObject* const object, const double& val) noexcept
// {
//     *object = RadarObject(val, val, val);
// }

int main()
{
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    //! [create publisher]
    iox::popo::Publisher<double, iceoryx_header> publisher({"DDS_CYCLONE", "std_msgs::msg::dds_::Float64_", "rt/chatter_pod"});
    //! [create publisher]

    double ct = 0.0;
    while (!iox::posix::hasTerminationRequested())
    {
        ++ct;

        //! [API Usage #1]
        //  * Retrieve a typed sample from shared memory.
        //  * Sample can be held until ready to publish.
        //  * Data is default constructed during loan
        publisher.loan()
            .and_then([&](auto& sample) {
                *sample = ct;
                iceoryx_header &ice_hdr = sample.getUserHeader();
                ice_hdr.data_kind = SDK_DATA;
                ice_hdr.data_size = sizeof(double);
                ice_hdr.shm_data_state = IOX_CHUNK_CONTAINS_RAW_DATA;
                for (int i = 0; i < 12; i++)
                    ice_hdr.guid.prefix.s[i] = 0;
                for (int i = 0; i < 3; i++)
                    ice_hdr.guid.prefix.u[i] = 0;
                ice_hdr.guid.entityid.u = 0;
                // ice_hdr.guid = 0;
                ice_hdr.statusinfo = 0;
                ice_hdr.tstamp = dds_time();

                sample.publish();
            })
            .or_else([](auto& error) {
                // Do something with error
                std::cerr << "Unable to loan sample, error: " << error << std::endl;
            });
        //! [API Usage #1]


        // //! [API Usage #2]
        // //  * Retrieve a typed sample from shared memory and construct data in-place
        // //  * Sample can be held until ready to publish.
        // //  * Data is constructed with the arguments provided.
        // publisher.loan(sampleValue2, sampleValue2, sampleValue2)
        //     .and_then([](auto& sample) { sample.publish(); })
        //     .or_else([](auto& error) {
        //         // Do something with error
        //         std::cerr << "Unable to loan sample, error: " << error << std::endl;
        //     });
        // //! [API Usage #2]

        // //! [API Usage #3]
        // //  * Basic copy-and-publish. Useful for smaller data types.
        // auto object = RadarObject(sampleValue3, sampleValue3, sampleValue3);
        // publisher.publishCopyOf(object).or_else([](auto& error) {
        //     // Do something with error.
        //     std::cerr << "Unable to publishCopyOf, error: " << error << std::endl;
        // });
        // //! [API Usage #3]

        // //! [API Usage #4]
        // //  * Provide a callable that will be used to populate the loaned sample.
        // //  * The first argument of the callable must be T* and is the location that the callable should
        // //      write its result to.
        // publisher.publishResultOf(getRadarObject, ct).or_else([](auto& error) {
        //     // Do something with error.
        //     std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
        // });
        // publisher
        //     .publishResultOf([&sampleValue4](RadarObject* object) {
        //         *object = RadarObject(sampleValue4, sampleValue4, sampleValue4);
        //     })
        //     .or_else([](auto& error) {
        //         // Do something with error.
        //         std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
        //     });
        // //! [API Usage #4]

        // std::cout << APP_NAME << " sent values: " << sampleValue1 << ", " << sampleValue2 << ", " << sampleValue3
        //           << ", " << ct << ", " << sampleValue4 << std::endl;

        std::cout << "Publishing: " << ct << std::endl;

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    return 0;
}

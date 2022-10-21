// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/posix_wrapper/scheduler.hpp"

#include "iceoryx_platform/platform_correction.hpp"

namespace iox
{
namespace posix
{
/// @brief Internal struct used during mutex construction to handle all the mutex attribute settings
struct MutexAttributes
{
  public:
    MutexAttributes() noexcept = default;
    MutexAttributes(const MutexAttributes&) = delete;
    MutexAttributes(MutexAttributes&&) = delete;
    MutexAttributes& operator=(const MutexAttributes&) = delete;
    MutexAttributes& operator=(MutexAttributes&&) = delete;

    ~MutexAttributes() noexcept
    {
        if (m_attributes)
        {
            auto destroyResult =
                posixCall(pthread_mutexattr_destroy)(&*m_attributes).returnValueMatchesErrno().evaluate();
            if (destroyResult.has_error())
            {
                IOX_LOG(ERROR)
                    << "This should never happen. An unknown error occurred while cleaning up the mutex attributes.";
            }
        }
    }

    cxx::expected<MutexCreationError> init() noexcept
    {
        m_attributes.emplace();
        auto result = posixCall(pthread_mutexattr_init)(&*m_attributes).returnValueMatchesErrno().evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case ENOMEM:
                IOX_LOG(ERROR) << "Not enough memory to initialize required mutex attributes";
                return cxx::error<MutexCreationError>(MutexCreationError::INSUFFICIENT_MEMORY);
            default:
                IOX_LOG(ERROR)
                    << "This should never happen. An unknown error occurred while initializing the mutex attributes.";
                return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return cxx::success<>();
    }

    cxx::expected<MutexCreationError> enableIpcSupport(const bool enableIpcSupport) noexcept
    {
        auto result =
            posixCall(pthread_mutexattr_setpshared)(
                &*m_attributes, static_cast<int>((enableIpcSupport) ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE))
                .returnValueMatchesErrno()
                .evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case ENOTSUP:
                IOX_LOG(ERROR) << "The platform does not support shared mutex (inter process mutex)";
                return cxx::error<MutexCreationError>(MutexCreationError::INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM);
            default:
                IOX_LOG(ERROR)
                    << "This should never happen. An unknown error occurred while setting up the inter process "
                       "configuration.";
                return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return cxx::success<>();
    }

    cxx::expected<MutexCreationError> setType(const MutexType mutexType) noexcept
    {
        auto result = posixCall(pthread_mutexattr_settype)(&*m_attributes, static_cast<int>(mutexType))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while setting up the mutex type.";
            return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
        }

        return cxx::success<>();
    }

    cxx::expected<MutexCreationError> setProtocol(const MutexPriorityInheritance priorityInheritance)
    {
        auto result = posixCall(pthread_mutexattr_setprotocol)(&*m_attributes, static_cast<int>(priorityInheritance))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case ENOSYS:
                IOX_LOG(ERROR) << "The system does not support mutex priorities";
                return cxx::error<MutexCreationError>(MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
            case ENOTSUP:
                IOX_LOG(ERROR) << "The used mutex priority is not supported by the platform";
                return cxx::error<MutexCreationError>(MutexCreationError::USED_PRIORITY_UNSUPPORTED_BY_PLATFORM);
            case EPERM:
                IOX_LOG(ERROR) << "Unsufficient permissions to set mutex priorities";
                return cxx::error<MutexCreationError>(MutexCreationError::PERMISSION_DENIED);
            default:
                IOX_LOG(ERROR)
                    << "This should never happen. An unknown error occurred while setting up the mutex priority.";
                return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
            }
        }

        return cxx::success<>();
    }

    cxx::expected<MutexCreationError> setPrioCeiling(const int32_t priorityCeiling) noexcept
    {
        auto result = posixCall(pthread_mutexattr_setprioceiling)(&*m_attributes, static_cast<int>(priorityCeiling))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            switch (result.get_error().errnum)
            {
            case EPERM:
                IOX_LOG(ERROR) << "Unsufficient permissions to set the mutex priority ceiling.";
                return cxx::error<MutexCreationError>(MutexCreationError::PERMISSION_DENIED);
            case ENOSYS:
                IOX_LOG(ERROR) << "The platform does not support mutex priority ceiling.";
                return cxx::error<MutexCreationError>(MutexCreationError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
            case EINVAL:
            {
                auto minimumPriority = getSchedulerPriorityMinimum(Scheduler::FIFO);
                auto maximumPriority = getSchedulerPriorityMaximum(Scheduler::FIFO);

                IOX_LOG(ERROR) << "The priority ceiling \"" << priorityCeiling
                               << "\" is not in the valid priority range [ " << minimumPriority << ", "
                               << maximumPriority << "] of the Scheduler::FIFO.";
                return cxx::error<MutexCreationError>(MutexCreationError::INVALID_PRIORITY_CEILING_VALUE);
            }
            }
        }

        return cxx::success<>();
    }

    cxx::expected<MutexCreationError>
    setThreadTerminationBehavior(const MutexThreadTerminationBehavior behavior) noexcept
    {
        auto result = posixCall(pthread_mutexattr_setrobust)(&*m_attributes, static_cast<int>(behavior))
                          .returnValueMatchesErrno()
                          .evaluate();
        if (result.has_error())
        {
            IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while setting up the mutex thread "
                              "termination behavior.";
            return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
        }

        return cxx::success<>();
    }

    cxx::optional<pthread_mutexattr_t> m_attributes;
};

cxx::expected<MutexCreationError> initializeMutex(pthread_mutex_t* const handle,
                                                  const pthread_mutexattr_t* const attributes) noexcept
{
    auto initResult = posixCall(pthread_mutex_init)(handle, attributes).returnValueMatchesErrno().evaluate();
    if (initResult.has_error())
    {
        switch (initResult.get_error().errnum)
        {
        case EAGAIN:
            IOX_LOG(ERROR) << "Not enough resources to initialize another mutex.";
            return cxx::error<MutexCreationError>(MutexCreationError::INSUFFICIENT_RESOURCES);
        case ENOMEM:
            IOX_LOG(ERROR) << "Not enough memory to initialize mutex.";
            return cxx::error<MutexCreationError>(MutexCreationError::INSUFFICIENT_MEMORY);
        case EPERM:
            IOX_LOG(ERROR) << "Unsufficient permissions to create mutex.";
            return cxx::error<MutexCreationError>(MutexCreationError::PERMISSION_DENIED);
        default:
            IOX_LOG(ERROR)
                << "This should never happen. An unknown error occurred while initializing the mutex handle. "
                   "This is possible when the handle is an already initialized mutex handle.";
            return cxx::error<MutexCreationError>(MutexCreationError::UNKNOWN_ERROR);
        }
    }

    return cxx::success<>();
}

cxx::expected<MutexCreationError> MutexBuilder::create(cxx::optional<mutex>& uninitializedMutex) noexcept
{
    if (uninitializedMutex.has_value())
    {
        IOX_LOG(ERROR) << "Unable to override an already initialized mutex with a new mutex";
        return cxx::error<MutexCreationError>(MutexCreationError::MUTEX_ALREADY_INITIALIZED);
    }

    MutexAttributes mutexAttributes;

    auto result = mutexAttributes.init();
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.enableIpcSupport(m_isInterProcessCapable);
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.setType(m_mutexType);
    if (result.has_error())
    {
        return result;
    }

    result = mutexAttributes.setProtocol(m_priorityInheritance);
    if (result.has_error())
    {
        return result;
    }

    if (m_priorityInheritance == MutexPriorityInheritance::PROTECT && m_priorityCeiling.has_value())
    {
        result = mutexAttributes.setPrioCeiling(*m_priorityCeiling);
        if (result.has_error())
        {
            return result;
        }
    }

    result = mutexAttributes.setThreadTerminationBehavior(m_threadTerminationBehavior);
    if (result.has_error())
    {
        return result;
    }

    uninitializedMutex.emplace();
    uninitializedMutex->m_isDestructable = false;

    result = initializeMutex(&uninitializedMutex->m_handle, &*mutexAttributes.m_attributes);
    if (result.has_error())
    {
        uninitializedMutex.reset();
        return result;
    }

    uninitializedMutex->m_isDestructable = true;
    return cxx::success<>();
}

/// @todo iox-#1036 remove this, introduced to keep current API temporarily
mutex::mutex(bool f_isRecursive) noexcept
{
    pthread_mutexattr_t attr;
    bool isInitialized{true};
    isInitialized &= !posixCall(pthread_mutexattr_init)(&attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setpshared)(&attr, PTHREAD_PROCESS_SHARED)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &=
        !posixCall(pthread_mutexattr_settype)(&attr, f_isRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL)
             .returnValueMatchesErrno()
             .evaluate()
             .has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setprotocol)(&attr, PTHREAD_PRIO_NONE)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &= !posixCall(pthread_mutex_init)(&m_handle, &attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_destroy)(&attr).returnValueMatchesErrno().evaluate().has_error();

    /// NOLINTJUSTIFICATION is fixed in the PR iox-#1443
    /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    cxx::Ensures(isInitialized && "Unable to create mutex");
}

mutex::~mutex() noexcept
{
    if (m_isDestructable)
    {
        auto destroyCall = posixCall(pthread_mutex_destroy)(&m_handle).returnValueMatchesErrno().evaluate();

        if (destroyCall.has_error())
        {
            switch (destroyCall.get_error().errnum)
            {
            case EBUSY:
                IOX_LOG(ERROR) << "Tried to remove a locked mutex which failed. The mutex handle is now leaked and "
                                  "cannot be removed anymore!";
                break;
            default:
                IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while cleaning up the mutex.";
                break;
            }
        }
    }
}

void mutex::make_consistent() noexcept
{
    if (this->m_hasInconsistentState)
    {
        posixCall(pthread_mutex_consistent)(&m_handle)
            .returnValueMatchesErrno()
            .evaluate()
            .and_then([&](auto) { this->m_hasInconsistentState = false; })
            .or_else([](auto) {
                IOX_LOG(ERROR) << "This should never happen. Unable to put robust mutex in a consistent state!";
            });
    }
}

cxx::expected<MutexLockError> mutex::lock() noexcept
{
    auto result = posixCall(pthread_mutex_lock)(&m_handle).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EINVAL:
            IOX_LOG(ERROR)
                << "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                   "priority is greater than the mutex priority.";
            return cxx::error<MutexLockError>(MutexLockError::PRIORITY_MISMATCH);
        case EAGAIN:
            IOX_LOG(ERROR) << "Maximum number of recursive locks exceeded.";
            return cxx::error<MutexLockError>(MutexLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
        case EDEADLK:
            IOX_LOG(ERROR) << "Deadlock in mutex detected.";
            return cxx::error<MutexLockError>(MutexLockError::DEADLOCK_CONDITION);
        case EOWNERDEAD:
            IOX_LOG(ERROR)
                << "The thread/process which owned the mutex died. The mutex is now in an inconsistent state "
                   "and must be put into a consistent state again with Mutex::make_consistent()";
            this->m_hasInconsistentState = true;
            return cxx::error<MutexLockError>(
                MutexLockError::LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while locking the mutex. "
                              "This can indicate a either corrupted or non-POSIX compliant system.";
            return cxx::error<MutexLockError>(MutexLockError::UNKNOWN_ERROR);
        }
    }
    return cxx::success<>();
}

cxx::expected<MutexUnlockError> mutex::unlock() noexcept
{
    auto result = posixCall(pthread_mutex_unlock)(&m_handle).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EPERM:
            IOX_LOG(ERROR) << "The mutex is not owned by the current thread. The mutex must be unlocked by the same "
                              "thread it was locked by.";
            return cxx::error<MutexUnlockError>(MutexUnlockError::NOT_OWNED_BY_THREAD);
        default:
            IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while unlocking the mutex. "
                              "This can indicate a either corrupted or non-POSIX compliant system.";
            return cxx::error<MutexUnlockError>(MutexUnlockError::UNKNOWN_ERROR);
        }
    }

    return cxx::success<>();
}

cxx::expected<MutexTryLock, MutexTryLockError> mutex::try_lock() noexcept
{
    auto result = posixCall(pthread_mutex_trylock)(&m_handle).returnValueMatchesErrno().ignoreErrnos(EBUSY).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EAGAIN:
            IOX_LOG(ERROR) << "Maximum number of recursive locks exceeded.";
            return cxx::error<MutexTryLockError>(MutexTryLockError::MAXIMUM_NUMBER_OF_RECURSIVE_LOCKS_EXCEEDED);
        case EINVAL:
            IOX_LOG(ERROR)
                << "The mutex has the attribute MutexPriorityInheritance::PROTECT set and the calling threads "
                   "priority is greater than the mutex priority.";
            return cxx::error<MutexTryLockError>(MutexTryLockError::PRIORITY_MISMATCH);
        case EOWNERDEAD:
            IOX_LOG(ERROR)
                << "The thread/process which owned the mutex died. The mutex is now in an inconsistent state "
                   "and must be put into a consistent state again with Mutex::make_consistent()";
            this->m_hasInconsistentState = true;
            return cxx::error<MutexTryLockError>(
                MutexTryLockError::LOCK_ACQUIRED_BUT_HAS_INCONSISTENT_STATE_SINCE_OWNER_DIED);
        default:
            IOX_LOG(ERROR) << "This should never happen. An unknown error occurred while try locking the mutex. "
                              "This can indicate a either corrupted or non-POSIX compliant system.";
            return cxx::error<MutexTryLockError>(MutexTryLockError::UNKNOWN_ERROR);
        }
    }

    return (result->errnum == EBUSY) ? cxx::success<MutexTryLock>(MutexTryLock::FAILED_TO_ACQUIRE_LOCK)
                                     : cxx::success<MutexTryLock>(MutexTryLock::LOCK_SUCCEEDED);
}
} // namespace posix
} // namespace iox

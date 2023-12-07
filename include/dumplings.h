/*
 * This is the dumplings library by Matthew, Nov 7, 2023.
 */

#pragma once

#include <type_traits>
#include <cstdint>
#include <functional>
#include <concepts>
#include <stdexcept>

#include <algorithm>
#include <random>

#include <queue>
#include <vector>
#include <array>

#include <thread>
#include <mutex>
#include <future>
#include <shared_mutex>
#include <chrono>

#include <gsl/gsl>

namespace dumplings {

/*
 * @brief    Generate N random numbers[min, max].
 * 
 * @param __N    Number of random numbers.
 * @param __min  Min bound.
 * @param __max  Max bound.
 *
 * @return       An array with N random numbers.
 */
template <std::size_t __N, typename _T1, typename _T2>
auto genNRandom(_T1 __min, _T2 __max) noexcept
{
    // Type safety
    static_assert((std::is_integral<_T1>::value || 
                   std::is_floating_point<_T1>::value) 
                  &&
                  (std::is_integral<_T2>::value ||
                   std::is_floating_point<_T2>::value),
                  "Invalid parameter type!");
    
    // Bounding safety
    const intmax_t __maxx{__max};
    const int_least64_t __minn{__min};
    // static_assert(__N > 0 && __maxx - __N + 1 >= __minn, "Invalid __N bounding!");
    // gsl::Expects(__N > 0 && ((__maxx - __N + 1) >= __minn));

    // Generate random numbers
    using _T = typename std::conditional<(sizeof(_T1) > sizeof(_T2)), _T1, _T2>::type;
    std::array<_T, __N> __arr;

    std::random_device __rd;
    std::mt19937 __gen(__rd());
    std::uniform_int_distribution<_T> __dis(__min, __max);

    std::ranges::for_each(__arr, [&](auto& __i){
            __i = __dis(__gen);
            });

    return __arr;
}    // Algorithm: genNRandom()

namespace thread_safe {

/*
 * @brief Thread-safe queue.
 */
template <typename _T>
class queue final {
public: 
    /*Constructors, destructor and operators*/
    queue() = default;
    // Support std::swap()
    queue(queue&& rhs) : mQueue(std::move(rhs.mQueue)) {}
    queue& operator=(queue&& rhs)
    {
        mQueue = std::move(rhs.mQueue);
        return *this;
    }
    
    /* BEGIN: Thread-safe operations on mTasks queue */
    /*
     * @brief Push operation.
     *
     */
    void push(_T&& element) noexcept
    {
        // Ensure thread-safety
        {
            std::unique_lock ul(mMutex);
            mQueue.push(std::forward<_T>(element));
        } 
    }

    /*
     * @brief pop operation with checking empty() operation.
     *
     */
    void pop() noexcept
    {
        // Ensure thread-safety
        {
            std::unique_lock ul(mMutex);
            if (!mQueue.empty())
                mQueue.pop();
        }
    }

    /*
     * @brief Front operation with checking empty() operation.
     *
     * @exception throw if no element. 
     */
    _T& front() 
    {
        // Ensure thread-safety
        {
            std::shared_lock sl(mMutex);
            if (!mQueue.empty())
                return mQueue.front();
            else
                throw std::out_of_range("Exception caught(out of range): dumplings::thread_safe::queue has no element");
        }
    }

    /*
     * @brief Get the first element and remove it from queue if queue is not empty, otherwise throw an exception. 
     *
     * @exception Throw an out_of_range exception if there is no element.
     */
    _T front_pop() 
    {
        // Ensure thread-safety
        _T resValue;
        {
            std::unique_lock ul(mMutex);
            if (!mQueue.empty())
            {
                resValue = std::move(mQueue.front());
                mQueue.pop(); 
                ul.unlock();
            }
            else
            {
                throw std::out_of_range("Exception caught(out of range): dumplings::thread_safe::queue has no element");
            }
        }

        return std::move(resValue);
    }

    /*
     * @brief Check if queue is empty. 
     *
     */
    [[nodiscard]] bool empty() noexcept
    {
        {
            std::shared_lock sl(mMutex);
            return mQueue.empty();
        }
    }

    /*
     * @brief Get size of queue. 
     *
     */
    std::size_t size() noexcept
    {
        {
            std::shared_lock sl(mMutex);
            return mQueue.size();
        }
    }

    /* END: Thread-safe operators on mTasks queue */

    void swap(queue& rhs) noexcept
    {
        {    
            std::unique_lock ul(mMutex);
            mQueue.swap(rhs.mQueue); 
        }
    }

private:
    std::shared_mutex mMutex;
    std::queue<_T> mQueue;

};   // Class queue
}    // Namespace thread_safe

/*
 * @brief Thread Pool with std::thread implementation.
 * 
 * @param _QUEUE_T: Thread-safe task queue which is dumplings::thread_safe::queue by default.
 *                  boost::lockfree::queue could also be used.
 */
template <typename _T>
concept UnsignedIntegral = std::is_integral_v<_T> && std::is_unsigned_v<_T>;

template <typename _QUEUE_T = thread_safe::queue<std::function<void()>>>
class thread_pool final {
public:
    /********* Initialization *********/
    // Use concpets to ensure type safety here.
    // template <UnsignedIntegral _T>
    // explicit thread_pool(_T __threadNum = std::thread::hardware_concurrency() - 1) noexcept
    template <typename _T>
    explicit thread_pool(_T __threadNum = std::thread::hardware_concurrency() - 1) noexcept requires UnsignedIntegral<_T>
        : mNumThreadDone(__threadNum),
          mNumThread(__threadNum),
	  mThreads(__threadNum)
    // explicit thread_pool(std::unsigned_integral auto threadNum = std::thread::hardware_concurrency() - 1) noexcept
    //     : mNumThreadDone(threadNum),
    //       mNumThread(threadNum),
	  // mThreads(threadNum)
    {
        // Create threads
        try {
            std::ranges::for_each(mThreads, [this](auto& thread){
                    thread = std::move(std::thread(&thread_pool::worker, this)); /// Active every thread in pool.
                    });
        } catch(...) {
            throw std::out_of_range("thread pool size");
        }
    }

    ~thread_pool()
    {
        // Join all threads if not join.
        std::ranges::for_each(mThreads, [](auto& t){
                if (t.joinable())
                {
                    t.join();
                }
                });
    }

    thread_pool(const thread_pool& rhs) = delete;
    thread_pool(thread_pool&& rhs) = delete;
    thread_pool& operator=(const thread_pool& rhs) = delete;
    thread_pool& operator=(thread_pool&& rhs) = delete;

    /********* Common API *********/
    /*
     * @brief       Request the thread pool to invoke the given function object.
     *
     * @return      Future with the result of the function(also with exception if has).
     * 
     * @exception   Throw exception if there is no thread in pool.
     */
    template <typename _Func, typename... _Args>
    auto post(_Func&& __func, _Args&&... __args) 
    {
        // Throw exception if no thread.
        if (0 == mNumThread)
        {
            throw std::runtime_error("post() failure due to no thread in pool");
        }

        // Generate a function object for func() and add it to task queue to wait for executing it.
        using resType = typename std::invoke_result<_Func, _Args...>::type;
        // std::promise<resType> promise;
        std::shared_ptr<std::promise<resType>> promise = std::make_shared<std::promise<resType>>();

        /// Ensure the funcObject is exception-safe, and the exception will be returned if there is any exception.
        std::function<void()> funcObject([promise, &__func, &__args...](){
        // std::function<void()> funcObject([&](){
                try {
                    promise->set_value(__func(std::forward<_Args>(__args)...));
                } catch(...) {
                    try {
                        promise->set_exception(std::current_exception()); /// set_exception may throw too.       
                    } catch(...) {}
                }});

        // Add this task into task queue.
        mTasks.push(std::move(funcObject));

        // Return future with the result of the task.
        return promise->get_future();
    }

    /*
     * @brief Wait for all current tasks in task queue to be completed.
     */
    void wait() noexcept
    {
        // Wait until task queue is empty and all threads completes.
        while (!mTasks.empty() || mNumThread != mNumThreadDone.load(std::memory_order_acquire))
        {}
    }

    /*
     * @brief Stop all threads
     */
    void stop() noexcept
    {
        // Notify all threads to exit.
        mStop.store(true, std::memory_order_release);

        // Reset thread pool settings.
        while (mNumThread != mNumThreadDone.load(std::memory_order_acquire))
        {}
     
        // mStop.store(false, std::memory_order_release);
        /// Clear task queue
        ///-----------------TODO-----------------///
        /// This temporarily just uses swap to clear task queue for dumplings::thread_safe::queue.
        /// The method to clear boost::asio::thread_pool still needs to be figured out.
        _QUEUE_T().swap(mTasks);
        ///--------------------------------------///

    }

private: 
    /********* Private methods *********/
    void worker() noexcept
    {
        // Keeping running until stop signal.
        while (!mStop)
        {
            /// try to get a task and execute it if successfully.
            try {
                mNumThreadDone--; //// Not yet complete this task. Ensure mTask won't be cleared by pool. 
                std::function<void()> task = mTasks.front_pop();
                task();
                mNumThreadDone++; //// Complete this task.
            } catch(...) {
                mNumThreadDone++; //// Reset it if no task.
            }                 
        }
    }
    
private:
    // Signals
    std::atomic_bool mStop = false; /// Stop signal
    std::atomic_size_t mNumThreadDone; /// Number of thread having done the current task.

    // All threads
    size_t mNumThread;
    std::vector<std::thread> mThreads;

    // Task queue
    _QUEUE_T mTasks;

};    // Class: thread_pool 

}    // Namespace Dumplings

/**
 * @file octopus_ipc_threadpool.hpp
 * @brief Simple and readable C++11 thread pool implementation
 *
 * This thread pool allows you to enqueue tasks (std::function<void()>) for asynchronous
 * execution using a fixed number of worker threads. Useful for high-frequency callbacks
 * or background tasks in systems like IPC communication.
 *
 * @author ak47
 * @date 2025-04-08
 */
#ifndef OCTOPUS_IPC_THREADPOOL_PLUS_HPP
#define OCTOPUS_IPC_THREADPOOL_PLUS_HPP

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>

// octopus_ipc_threadpool.hpp (放在类外面或类内 public 区域)
enum class TaskOverflowStrategy
{
    DropOldest, // 丢弃最老任务
    DropNewest, // 丢弃当前提交的任务
    Block       // 阻塞等待队列有空位
};

/**
 * @class ThreadPoolPlus
 * @brief A flexible, efficient thread pool with task queue limit, dynamic thread control, and task result support.
 *
 * This thread pool supports:
 * - Fixed and dynamic number of threads.
 * - Task queue with upper limit to prevent memory explosion.
 * - Tasks with or without return values using std::future.
 * - Periodic health check for potential scaling.
 */
class OctopusThreadPool
{
public:
    /**
     * @brief Construct the thread pool with initial thread count and maximum task queue size.
     * @param thread_count Number of worker threads to start initially.
     * @param max_queue_size Maximum number of tasks allowed in the queue.
     */
    OctopusThreadPool(size_t thread_count, size_t max_queue_size, TaskOverflowStrategy strategy);

    /**
     * @brief Gracefully shuts down the thread pool and joins all threads.
     */
    ~OctopusThreadPool();

    /**
     * @brief Submit a task without return value to the pool.
     * @param task A std::function<void()> representing the task.
     */
    void enqueue(const std::function<void()> &task);
    /**
     * @brief Submit a task to the thread pool for asynchronous execution with a delay.
     *
     * This function enqueues a task, but delays its execution by the specified duration
     * before it is added to the task queue. It works similarly to `enqueue`, but with an 
     * additional parameter for the delay time.
     *
     * @param task The task to be executed, wrapped in a std::function<void()>.
     * @param delay The delay time before the task is added to the queue (in milliseconds).
     */
    void enqueue_delayed(const std::function<void()> &task, unsigned int delay);

    /**
     * @brief Submit a task with return value support.
     * @tparam T Return type of the task.
     * @param task A std::function<T()> representing the task.
     * @return A std::future<T> to retrieve the result.
     */
    template <typename T>
    std::future<T> enqueue_with_result(std::function<T()> task);

    /**
     * @brief Perform a simple thread pool health check. Can be called periodically to adjust threads.
     *
     * - If the task queue is overloaded, new threads may be added.
     * - If the task queue is empty and too many threads are idle, thread reduction may be triggered (optional).
     */
    void health_check();

    /**
     * @brief Get the current number of worker threads.
     * @return Current active thread count.
     */
    size_t get_thread_count() const;

    /**
     * @brief Get the current number of pending tasks in the queue.
     * @return Task queue size.
     */
    size_t get_task_queue_size() const;

    /**
     * @brief Print the status of the thread pool for debugging.
     */
    void print_pool_status() const;

private:
    /**
     * @brief Main worker function executed by each thread.
     *
     * Each thread waits for tasks and executes them one by one.
     */
    void worker_loop();

    /**
     * @brief Add additional worker threads to the pool.
     * @param count Number of new threads to create.
     */
    void add_threads(size_t count);

    /**
     * @brief (Optional) Remove worker threads from the pool.
     * @param count Number of threads to be stopped and removed.
     */
    void remove_threads(size_t count);

    std::vector<std::thread> workers_;             ///< Vector of worker threads
    std::deque<std::function<void()>> task_queue_; ///< Task queue

    mutable std::mutex queue_mutex_;  ///< Mutex to protect task queue
    std::condition_variable task_cv_; ///< Condition variable for task notification

    std::atomic<bool> is_running_;            ///< Pool running state
    std::atomic<size_t> active_thread_count_; ///< Active worker thread count
    size_t max_queue_size_;                   ///< Maximum size of task queue

    std::atomic<bool> is_scaling_; ///< Prevent concurrent scaling during health check

    std::atomic<size_t> threads_to_terminate_{0};
    TaskOverflowStrategy overflow_strategy_;
};

template <typename T>
std::future<T> OctopusThreadPool::enqueue_with_result(std::function<T()> task)
{
    auto packaged_task = std::make_shared<std::packaged_task<T()>>(std::move(task));
    std::future<T> result = packaged_task->get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (task_queue_.size() < max_queue_size_)
        {
            task_queue_.emplace_back([packaged_task]()
                                     { (*packaged_task)(); });
        }
        else
        {
            return {}; // Return empty future if queue is full
        }
    }

    task_cv_.notify_one();
    return result;
}

#endif // OCTOPUS_IPC_THREADPOOL_PLUS_HPP

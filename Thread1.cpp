#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <future>
#include <atomic>

std::mutex mutex1, mutex2;
std::condition_variable cv;
std::condition_variable_any cvAny;

std::atomic<int> counter(0);

void threadFunction1() {
    std::lock_guard<std::mutex> lock(mutex1);

    std::cout << "Thread 1 locked mutex1 (ID: " << std::this_thread::get_id() << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    cv.notify_one();
}

void threadFunction2() {
    std::unique_lock<std::mutex> lock(mutex1);

    cv.wait_for(lock, std::chrono::milliseconds(500));

    std::cout << "Thread 2 locked mutex1 after waiting (ID: " << std::this_thread::get_id() << ")" << std::endl;
}

void threadFunction3() {
    std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mutex2, std::defer_lock);

    std::lock(lock1, lock2);

    std::cout << "Thread 3 locked mutex1 and mutex2 (ID: " << std::this_thread::get_id() << ")" << std::endl;
}

int calculateSum(int a, int b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return a + b;
}

void threadFunction4() {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::yield();
        std::cout << "Thread 4 yielding (ID: " << std::this_thread::get_id() << ")" << std::endl;
    }
}

void threadFunction5() {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        counter.fetch_add(1, std::memory_order_relaxed);
        std::cout << "Thread 5 incrementing counter: " << counter.load(std::memory_order_relaxed) << std::endl;
    }
}

int addNumbers(int a, int b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    return a + b;
}

void threadFunction6() {
    std::packaged_task<int(int, int)> task(addNumbers);
    std::future<int> futureResult = task.get_future();

    task(30, 40);

    int result = futureResult.get();
    std::cout << "Result of the packaged task: " << result << std::endl;
}

void threadFunction7(std::promise<int>& promiseObj) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    promiseObj.set_value(50);
}

int main() {
    std::thread t1(threadFunction1);
    std::thread t2(threadFunction2);

    t1.join();
    t2.detach();

    std::thread t3(threadFunction3);

    t3.join();

    std::packaged_task<int(int, int)> task(calculateSum);
    std::future<int> futureResult = task.get_future();

    std::thread t4(std::move(task), 10, 20);
    t4.join();

    int result = futureResult.get();
    std::cout << "Result of the asynchronous calculation: " << result << std::endl;

    std::thread t5(threadFunction4);
    std::thread t6(threadFunction5);

    t5.join();
    t6.join();

    std::thread t7(threadFunction6);
    t7.join();

    std::promise<int> promiseObj;
    std::future<int> future = promiseObj.get_future();
    std::thread t8(threadFunction7, std::ref(promiseObj));

    int promiseResult = future.get();
    std::cout << "Result from promise: " << promiseResult << std::endl;

    t8.join();

    return 0;
}

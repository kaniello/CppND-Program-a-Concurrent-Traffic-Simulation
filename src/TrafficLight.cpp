#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex>
    std::unique_lock<std::mutex> lockReceive(_mutex);
    //and _condition.wait() to wait for and receive new messages
    _condition.wait(lockReceive, [this]
                    { return !_queue.empty(); });
    //and pull them from the queue using move semantics.
    T msg = std::move(_queue.front());
    _queue.pop_front();
    // The received object should then be returned by the receive function.
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    std::lock_guard<std::mutex> lockSend(_mutex);
    _queue.push_back(msg);
    // as well as _condition.notify_one() to add a new message to the queue
    _condition.notify_one();
    //and afterwards send a notification.
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    bool gotGreen{false};

    while (!gotGreen)
    {
        if (this->message.receive() == TrafficLightPhase::green)
        {
            gotGreen = true;
        }
    }

    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    std::thread t(&TrafficLight::cycleThroughPhases, this);
    t.join();
    threads.emplace_back(std::move(t));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop
    while (1)
    {
        // that measures the time between two loop cycles
        auto t1 = std::chrono::high_resolution_clock::now();
        // and toggles the current phase of the traffic light between red and green
        (_currentPhase = green) ? red : green;
        // and sends an update method to the message queue using move semantics.
        message.send(std::move(_currentPhase));
        // The cycle duration should be a random value between 4 and 6 seconds.
        std::default_random_engine generator;
        std::uniform_real_distribution<double> distribution(4, 6);
        double dSleepTime = distribution(generator);
        auto sleepTime = std::chrono::duration<double>(dSleepTime);
        std::this_thread::sleep_for(sleepTime);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto timeBetweenTwoLoops = t2 - t1;
    }
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
}

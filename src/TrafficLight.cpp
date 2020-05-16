#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    std::unique_lock<std::mutex> lock(_mutex);
    // wait until _queue is non-empty
    _condition.wait(lock, [this] { return !_queue.empty(); }); 
        
    // then pop and return first deque element from _queue
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex>(_mutex);
    _queue.push_front(std::move(msg));
    _condition.notify_one();
}



/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true) {
        TrafficLightPhase phase = std::move(_trafficLightQueue.receive());
        if (phase == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread t(&TrafficLight::cycleThroughPhases);
    t.join();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    int randMin = 4000;
    int randDiff = 2000;
    int cycleLen;
    double timeDiff;
    time_t timer1;
    time_t timer2;

    // init timers
    time(&timer1); 
    time(&timer2);

    while (true) {
        // generate cycle time between 4000 and 6000 ms
        cycleLen = randMin + rand() % (randDiff + 1); 

        // keep polling timer2 until it reaches threshhold
        if (int(difftime(timer1, timer2)) < cycleLen) {
            time(&timer2); 
        }
        else 
        {
            // toggle phases
            if (_currentPhase == TrafficLightPhase::green) {
                _currentPhase = TrafficLightPhase::red;
            }
            else {
                _currentPhase == TrafficLightPhase::green;
            }
            
            // send to phase change to message queue
            _trafficLightQueue.send(std::move(_currentPhase)); 

            // reset timer1;
            time(&timer1);
            
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep between cycles
    }
}


#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> _lock(_mutex);
    _condition.wait(_lock, [this] { return !_queue.empty(); });
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&message)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> _lock(_mutex);
    _queue.push_back(std::move(message));
    _condition.notify_one();
    std::cout << "A Message was send: #" << message << std::endl;
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    //TrafficLightPhase_ = std::make_shared<MessageQueue<TrafficLightPhase>>(); // trying to add shared pointer i got some issues
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TrafficLightPhase senal = TrafficLightPhase_.receive();
        if (senal == TrafficLightPhase::green) {
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
     threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // First, the variable needed to be compared and updated
    std::chrono::time_point<std::chrono::system_clock> ultimoTiempo;
    // The cycle duration should be a random value between 4 and 6 seconds
    // Duration should be random and updated every cycle of thw while

    /*
    Before C++11 was released, the easiest way to generate random numbers in C++ was through the std::rand() function. However, C++11 now 
    offers easy-to-use and vastly superior mechanisms for generating random numbers and provides developers with the ability to sample 
    from many commonly used distributions using only machinery available in the STL. In this post, we will show the new C++11 methods 
    for generating random integers from a uniform distribution over a closed interval; sampling from other distributions will be a trivial 
    task after one understands the concepts illustrated below.
    https://diego.assencio.com/?index=6890b8c50169ef45b74db135063c227c 
    */
    std::random_device device;
    std::mt19937 generator(device());
    // using distribution(4,6) 
    std::uniform_int_distribution<int> distribution(4.0,6.0); 
    ultimoTiempo = std::chrono::system_clock::now();
    while(true) 
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        double thresholdDuration = distribution(generator);
        long duration = std::chrono::duration_cast<std::chrono::seconds>(
                                  std::chrono::system_clock::now() - ultimoTiempo)
                                .count();
        //std::cout << "DURATION:  " <<  duration  << std::endl;
        //std::cout << "DURATION THRES:  " <<  thresholdDuration  << std::endl;
        if(duration >= thresholdDuration) {
            //std::cout << "SE CUMPLIO EL PERIODO DE TIEMPO  " << TrafficLight::getCurrentPhase()  << std::endl;
            if(TrafficLight::getCurrentPhase() == red) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }
            TrafficLightPhase_.send(std::move(_currentPhase));
            ultimoTiempo = std::chrono::system_clock::now();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
    }
    

}


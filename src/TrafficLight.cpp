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
  	std::unique_lock<std::mutex> ulock(_mutex);
  	_condition.wait(ulock, [this] {return !_queue.empty(); });
  
  	T msg = std::move(_queue.back());
  	_queue.pop_back();
  
  	return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> ulock(_mutex);
  	_queue.clear();
  	_queue.emplace_back(std::move(msg));
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
  	while(true)
    {	
      	if(_messageQueue.receive() == TrafficLightPhase::green)
          	return;
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
  	std::random_device rd;
  	std::mt19937 eng(rd());
  	std::uniform_int_distribution<> distr(4000, 6000);
    auto cycleDuration = distr(eng);

    std::chrono::system_clock::time_point time_1 = std::chrono::system_clock::now();

  	while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

      	std::chrono::system_clock::time_point time_2 = std::chrono::system_clock::now();
      	auto time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(time_2 - time_1).count(); 
      
        if (time_difference > cycleDuration)
        {
          	if (_currentPhase == TrafficLightPhase::red)
            {
              	_currentPhase = TrafficLightPhase::green;
            }
          	else
            {
              	_currentPhase = TrafficLightPhase::red;
            }
            _messageQueue.send(std::move(_currentPhase));
          	time_1 = std::chrono::system_clock::now();
          	//cycleDuration = distr(eng);
        }
    }
}

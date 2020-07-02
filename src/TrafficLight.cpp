#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <future>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
	std::unique_lock<std::mutex> uLock(_mtx);
    // to wait for and receive new messages and pull them from the queue using move semantics. 
	_condition.wait(uLock, [this] { return !_queue.empty(); });
	//get from back of queue
	T msg = std::move(_queue.back());
	//remove the back
	_queue.pop_back();
	_queue.clear();
    // The received object should then be returned by the receive function. 
        return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
	std::lock_guard<std::mutex> uLock(_mtx);
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	_queue.push_back(std::move(msg));
	_condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    //set messageQueue to the shared pointer (this variable was declared as a shared pointer in TrafficLight.h
    messageQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    while (true) {
    // runs and repeatedly calls the receive function on the message queue. 
	    TrafficLightPhase msg = messageQueue->receive();
    // Once it receives TrafficLightPhase::green, the method returns.
	    if (msg == TrafficLightPhase::green)
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
    
	//threads is a vector of std::thread objects
	//use emplace_back to move the thread, not make a copy
	//call cycleThroughPhases, a private method, to have the traffic light change colors
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    //
    // initialize duration to 0 in the beginning - this will be changed in the while loop later
    double duration = 0;

    //initialize startTime outside of the loop to ensure that it can be used in multiple loop cycles - this variable will be modified at the end of each cycle
    auto startTime = std::chrono::system_clock::now();

    //infinite loop
    while (true) {
    	auto curTime = std::chrono::system_clock::now(); //get the time at the beginning of the cycle

    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 

	//calculate the variable diff which calculates the time between the start of the new traffic light phase and the current moment
    	
	std::chrono::duration<double> diff = curTime - startTime; //this returns a value in seconds

	
	//diff.count() gets the double from std::chrono::duration<double> - this value is in seconds
	//multiply by 1000 to get milliseconds
	//check if that difference is greater than or equal to the randomly calculated duration of the loop cycle that was set before

    	if (diff.count()*1000 >= duration) {

		//change the traffic light color (if the difference has exceeded the duration only)
		
		if (_currentPhase == TrafficLightPhase::green)
			_currentPhase = TrafficLightPhase::red;
		else if (_currentPhase == TrafficLightPhase::red)
			_currentPhase = TrafficLightPhase::green;
		
		//set the startTime to now - this value is later used to calculate the difference and check whether to change the traffic light color
    		
		startTime = std::chrono::system_clock::now();

		//calculate random number between 4000ms to 6000ms
    		
		std::random_device rd;
		std::mt19937 eng(rd());
		std::uniform_int_distribution<> distr(4000, 6000);

		//set the duration of the next loop cycle to this random number
		
		duration = distr(eng);

		//send the current phase to the message queue
		
		messageQueue->send(std::move(_currentPhase));

		
	}
	
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    	std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

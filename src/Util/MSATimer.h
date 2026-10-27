#pragma once

#include "ofMain.h"

#include <chrono>

namespace msa {

	class Timer {
	public:
		Timer();

		inline double getTotalSeconds();			// elapsed seconds since beginning of time
		inline double getAppSeconds();				// elapsed seconds since start of app
		inline double getSecondsSinceLastCall();	// elapsed seconds since last time this function was called

		inline void start();
		inline void stop();
		inline double getSeconds();			// elapsed seconds since you called start()

	protected:
		double appStartTime;
		double lastCallTime;
		double startTime;
		double stopTime;
		bool isRunning;
	};


	/******************** IMPLEMENTATION ************************/

    //--------------------------------------------------------------
	inline Timer::Timer() {
		appStartTime = getTotalSeconds();
		startTime = lastCallTime = appStartTime;
		isRunning = false;
	}


    //--------------------------------------------------------------
	inline double Timer::getTotalSeconds() {
		using namespace std::chrono;
		return duration<double>(steady_clock::now().time_since_epoch()).count();
	}

    //--------------------------------------------------------------
	inline double Timer::getAppSeconds() {
		return getTotalSeconds() - appStartTime;
	}

    //--------------------------------------------------------------
	inline double Timer::getSecondsSinceLastCall() {
		float nowTime = getTotalSeconds();
		float diff = nowTime - lastCallTime;
		lastCallTime = nowTime;
		return diff;
	}

	//--------------------------------------------------------------
    inline void Timer::start() {
		isRunning = true;
		startTime = getTotalSeconds();
	}

	//--------------------------------------------------------------
    inline void Timer::stop() {
		stopTime = getTotalSeconds();
		isRunning = false;
	}

    //--------------------------------------------------------------
	inline double Timer::getSeconds() {
		if(isRunning) {
			return getTotalSeconds() - startTime;
		} else {
			return stopTime - startTime;
		}
	}

}

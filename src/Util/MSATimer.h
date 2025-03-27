#pragma once

#include "ofMain.h"

#ifdef TARGET_OSX
#include <mach/mach_time.h>
#endif

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

#ifdef TARGET_OSX
		double machMultiplier;
#endif
	};

	
	/******************** IMPLEMENTATION ************************/

    //--------------------------------------------------------------
	inline Timer::Timer() {
#ifdef TARGET_OSX
		mach_timebase_info_data_t info;
		mach_timebase_info(&info); 
		machMultiplier = info.numer / 1000000000.0 / info.denom;
		appStartTime = mach_absolute_time() * machMultiplier;
#else
		appStartTime = ofGetElapsedTimef();
#endif		
		
		startTime = lastCallTime = appStartTime;
		
		isRunning = false;
	}
	
	
    //--------------------------------------------------------------
	inline double Timer::getTotalSeconds() {
#ifdef TARGET_OSX
		return mach_absolute_time() * machMultiplier;
#else
		return ofGetElapsedTimef();
#endif		
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
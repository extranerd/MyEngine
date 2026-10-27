#pragma once

#include "timer.h"

class Animation {

	Timer timer;
	int frameCount;
	
public:
	Animation() : timer(0), frameCount(0) {}
	Animation(int frameCount, float length) : frameCount(frameCount), timer(length) {}
	float getLength() { return timer.getLenght(); }
	int currentFrame() { return static_cast<int>(timer.getTime() / timer.getLenght() * frameCount); }
	void step(float deltaTime) { timer.step(deltaTime); }
};
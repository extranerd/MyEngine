#pragma once

class Timer {
	float lenght, time;
	bool timeout;
public:
	Timer(float lenght) : lenght(lenght), time(0), timeout(false) {}

	void step(float deltaTime) {
		time += deltaTime;
		if (time >= lenght) {
			time -= lenght;
			timeout = true;
		}
	}

	bool isTimeout() {
		return timeout;
	}

	float getTime() {
		return time;
	}
	float getLenght() {
		return lenght;
	}

	void reset() {
		time = 0;
		timeout = false;
	}
};
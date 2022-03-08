#pragma once

#include <chrono>

struct mvTimer
{

	std::chrono::steady_clock::time_point _start;
	std::chrono::steady_clock::time_point _last;

	mvTimer()
	{
		_last = std::chrono::steady_clock::now();
		_start = std::chrono::steady_clock::now();
	}

	float mark()
	{
		const auto old = _last;
		_last = std::chrono::steady_clock::now();
		const std::chrono::duration<float> frameTime = _last - old;
		return frameTime.count();
	}

	float peek()
	{
		return std::chrono::duration<float>(std::chrono::steady_clock::now() - _last).count();
	}

	float now()
	{
		const std::chrono::duration<float> totalTime = std::chrono::steady_clock::now() - _start;
		return totalTime.count();
	}
	
};
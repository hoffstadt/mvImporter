#pragma once

#include <chrono>
#include "mv3D_internal.h"

class mvTimer
{

public:

	mvTimer();

	f32 mark();
	f32 peek();
	f32 now();
	
private:

	std::chrono::steady_clock::time_point m_start;
	std::chrono::steady_clock::time_point m_last;
};
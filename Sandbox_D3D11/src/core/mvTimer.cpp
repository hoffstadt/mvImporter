#include "mvTimer.h"

using namespace std::chrono;

mvTimer::mvTimer()
{
	m_last = steady_clock::now();
	m_start = steady_clock::now();
}

f32
mvTimer::mark()
{
	const auto old = m_last;
	m_last = steady_clock::now();
	const duration<f32> frameTime = m_last - old;
	return frameTime.count();
}

f32
mvTimer::peek()
{
	return duration<f32>(steady_clock::now() - m_last).count();
}

f32
mvTimer::now()
{
	const duration<f32> totalTime = steady_clock::now() - m_start;
	return totalTime.count();
}

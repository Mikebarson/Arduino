#include "Timer.h"

Timer::Timer()
  : running(false), elapsedMilliseconds(0), timespanMilliseconds(0)
{
}

void Timer::SetTimespan(int seconds)
{
  timespanMilliseconds = seconds * 1000L;
}

long Timer::GetTimespan()
{
  return timespanMilliseconds / 1000L;
}

void Timer::Reset()
{
  elapsedMilliseconds = 0;
}

void Timer::Start()
{
  running = true;
}

void Timer::Pause()
{
  running = false;
}

void Timer::Update(long deltaMilliseconds)
{
  if (!running)
  {
    return;
  }
  
  elapsedMilliseconds += deltaMilliseconds;
}

long Timer::GetElapsedSeconds()
{
  return elapsedMilliseconds / 1000L;
}

bool Timer::IsExpired()
{
  return elapsedMilliseconds > timespanMilliseconds;
}


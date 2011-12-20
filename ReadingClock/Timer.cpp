#include "Timer.h"

Timer::Timer()
  : running(false), elapsedMilliseconds(0), timespanMillieconds(0)
{
}

void Timer::SetTimespan(int seconds)
{
  timespanMillieconds = seconds * 1000;
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

int Timer::GetElapsedSeconds()
{
  return elapsedMilliseconds / 1000;
}

bool Timer::IsExpired()
{
  return elapsedMilliseconds > timespanMillieconds;
}



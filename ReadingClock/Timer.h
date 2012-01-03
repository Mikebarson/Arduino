#ifndef _timer_h
#define _timer_h

#include <Arduino.h>

class Timer
{
  public:
    Timer();
    
    void SetTimespan(int seconds);
    int GetTimespan();

    void Reset();
    void Start();
    void Pause();

    void Update(uint32_t deltaMilliseconds);
    bool IsExpired();
    
    int GetElapsedSeconds();
    
  private:
    bool running;

    uint32_t elapsedMilliseconds;
    uint32_t timespanMilliseconds;
};

#endif // _timer_h


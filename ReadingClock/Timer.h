#ifndef _timer_h
#define _timer_h

class Timer
{
  public:
    Timer();
    
    void SetTimespan(int seconds);
    long GetTimespan();

    void Reset();
    void Start();
    void Pause();

    void Update(long deltaMilliseconds);    
    bool IsExpired();
    
    long GetElapsedSeconds();
    
  private:
    bool running;

    long elapsedMilliseconds;
    long timespanMilliseconds;
};

#endif // _timer_h


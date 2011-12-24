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


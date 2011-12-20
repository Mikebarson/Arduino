class Timer
{
  public:
    Timer();
    
    void SetTimespan(int seconds);

    void Reset();
    void Start();
    void Pause();

    void Update(long deltaMilliseconds);    
    bool IsExpired();
    
    int GetElapsedSeconds();
    
  private:
    bool running;

    long elapsedMilliseconds;
    int timespanMillieconds;
};


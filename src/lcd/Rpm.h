class Rpm
{
  private:
    int lastRPM;
    int rpm;
    unsigned long last_interrupt_time;
    unsigned long interrupt_time;
    
    void rising() {
     last_interrupt_time = 0;
     interrupt_time = millis();
     if (interrupt_time - last_interrupt_time > 2){
       rpm++; 
     }
     last_interrupt_time = interrupt_time;
    }

  public:
    Rpm(int RPM){
      
      pinMode(RPM, INPUT_PULLDOWN);
    }

    void checkRPM()
    {
      rising();
    }
    
    void countRPM()
    {
      lastRPM = rpm;
      rpm=0;
    }
    
    int getRPM()
    {
      return rpm*60;
    }
  
};

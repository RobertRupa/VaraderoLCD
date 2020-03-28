
class Temperature {
  private:
    float T;
    float Vo;
    int bcoefficient;
    int temperaturenominal;
    float logR2;
    float R1 = 1000;
    float R2;
    float c1, c2, c3;
  
  public:

    float externalTemp(int pin)
    {
      R2 = 1000;
      bcoefficient = 3600;
      temperaturenominal = 25;
      Vo = analogRead(pin);
      Vo = (4095.0 / Vo - 1.0);
      Vo = R1 / Vo;
      T = Vo / R2;
      T = log(T);
      T /= bcoefficient;
      T += 1.0 / (temperaturenominal + 273.15);
      T = 1.0 / T;
      T -= 273.15;
      return T;
    }
    
    float engineTemp(int pin)
    {
      R2 = 530;
      bcoefficient = 3600;
      temperaturenominal = 25;
      Vo = analogRead(pin);
      Vo = (4095.0 / Vo - 1.0);
      Vo = R1 / Vo;
      T = Vo / R2;
      T = log(T);
      T /= bcoefficient;
      T += 1.0 / (temperaturenominal + 273.15);
      T = 1.0 / T;
      T -= 273.15;
      return T;
    }
  
};

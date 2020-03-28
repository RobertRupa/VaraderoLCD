
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
//      R2 = 1000;
//      c1 = 0.001910726699965636;
//      c2 = 0.00024131947327413964;
//      c3 = -3.8610931298493383e-7;
//      Vo = analogRead(pin);
//      R2 = R1 * (4095.0 / (float)Vo - 1.0);
//      logR2 = log(R2);
//      T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
//      T = T - 273.15;
//      T = (T * 9.0) / 5.0 + 32.0;
      
      return T;
    }
  
};

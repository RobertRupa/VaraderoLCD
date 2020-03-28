class Odometr
{
  private:
      byte result;
      int i2caddr;
      int addr;
      char tmp;
      String resultOdo;
  
    // reads a byte of data from i2c memory location addr
    byte readData(unsigned int addr)
    {
      Wire.beginTransmission(i2caddr);
      // set the pointer position
      //Wire.write((int)(addr >> 8));
      Wire.write((int)(addr & 0xFF));
      Wire.endTransmission(1);
      Wire.requestFrom(i2caddr,1); // get the byte of data
      result = Wire.read();
      return result;
    }

  public:
    Odometr(int i2cadd){
      i2caddr = i2cadd;
    }
  
    String readTotalOdometr()
    {
      resultOdo = "";
      for(addr = 0x70; addr<0x78; addr++){
        tmp = readData(addr);
        resultOdo+= String(&tmp);
      }
      return resultOdo;
    }
    
    String readTripOdometr()
    {
      resultOdo = "";
      for(addr = 0x78; addr<0x7D; addr++){
        tmp = readData(addr);
        resultOdo+= String(&tmp);
      }
      return resultOdo;
    }
};

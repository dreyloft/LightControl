#include <SPI.h>
#include <Ethernet.h>
#include <TextFinder.h>
#include <SD.h>

byte mac[] = {0x00, 0xFF, 0xCE, 0x54, 0xB0, 0x6E};
byte ip[]  = {192, 168, 178, 100};
int pin;

EthernetServer server(80);

File webFile;
File colorFile;

boolean anodeBased = true;
boolean change = false;
boolean block = false;

int pin_red;
int pin_green;
int pin_blue;
int delayTime = 10;
int dumpRed;
int dumpGreen;
int dumpBlue;
int red;
int green;
int blue;

long val;
long dumpVal;

void setup()  
{
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  pinMode(10, OUTPUT);
  
  if (!SD.begin(4))
  {
    Serial.println("init SD- card error");
  }
  Serial.println("init SD- card  done");
  
  if (!SD.exists("COLOR.TXT"))
  {
    Serial.println("COLOR.TXT find error");   
  } 
  else 
  { 
    Serial.println("COLOR.TXT find done");
  }
  
  if (!SD.exists("index.htm"))
  {
    Serial.println("index.htm find error");
  }
  else
  {
    Serial.println("index.htm find done");
  }  

  readColor(); 
  fadeIn();
} 

void loop()  
{
  dumpRed = red;
  dumpGreen = green;
  dumpBlue = blue;
  dumpVal = val;
  
  webBasedControl(); 
          
  if (change == true && block == false)
  {    
    writeColor();
  }
}


void webBasedControl ()
{
  block = true;
  
  EthernetClient client = server.available();

  if(client)
  {
    TextFinder finder(client);

    if(finder.find("GET"))
    {
      while(finder.findUntil("pin", "\n\r"))
      {
        char typ = client.read();
        pin = finder.getValue();
        val = finder.getValue();        
        
        if(typ == 'D')
        {
          pinMode(pin, OUTPUT);

          val = checkValue(val, 0, 1);

          if (anodeBased == true)
          {
            if (val == 0) 
            {
              val = 1;
            }
            else
            {
              val = 0;
            }
          }
          digitalWrite(pin, val);
          change = true;
        } 
        else if(typ == 'A')
        {
          val = checkValue(val, 0, 255);
          changeVal(dumpVal, val, pin, anodeBased);
          change = true;
        }        
        else if(typ == 'M')
        { 
          pin_blue = pin % 10;
          pin_green = (pin % 100 - pin_blue) / 10;
          pin_red = (pin - pin_green - pin_blue) / 100;

          blue = val % 1000;
          green = ((val % 1000000) - blue) / 1000;
          red = (val - (green * 1000) - blue) / (1000000);

          red = checkValue(red, 0, 255);
          green = checkValue(green, 0, 255);
          blue = checkValue(blue, 0, 255);

          //showAllVar();
               
          changeVal(dumpRed, red, pin_red, true);
          changeVal(dumpGreen, green, pin_green, true);
          changeVal(dumpBlue, blue, pin_blue, true);   
          change = true;
   
          val = dumpVal;       
        }        
        else 
        {
          Serial.print("input value error");
        }
      }
      block = false;
    }

    boolean current_line_is_blank = true;

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (c == '\n' && current_line_is_blank)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          
          webFile = SD.open("index.htm");
          
          if (webFile)
          {
            while(webFile.available())
            {
              client.write(webFile.read());
            }
            webFile.close();          
          }
          break;
        }
        if (c == '\n')
        {
          current_line_is_blank = true;
        }
        else if (c != '\r')
        {
          current_line_is_blank = false;
        }
      }
    }
    delay(100);
    client.stop();
  }  
}

int checkValue (long chkval, int minVal, int maxVal) 
{
  if (chkval < minVal)
  {
    chkval = minVal;
  }
  else if (chkval > maxVal)
  {
    chkval = maxVal;
  }

  return chkval;
}

void fadeIn() 
{  
  red = anodeCheck(red, anodeBased);
  green = anodeCheck(green, anodeBased);
  blue = anodeCheck(blue, anodeBased);
  val = anodeCheck(val, anodeBased); 
    
  for (int i = 256; i > 0; i--)
  { 
    if (i > red)
    {
      analogWrite(pin_red, i);
    } 
    else 
    {
      analogWrite(pin_red, red);
    }
    
    if (i > green)
    {
      analogWrite(pin_green, i);
    } 
    else 
    {
      analogWrite(pin_green, green);
    }
    
    if (i > blue)
    {
      analogWrite(pin_blue, i);
    } 
    else 
    {
      analogWrite(pin_blue, blue);
    }
    
    if (i > val)
    {
      analogWrite(pin, i);
    } 
    else 
    {
      analogWrite(pin, val);
    }
    
    delay(delayTime);
  }  
  Serial.println("fadeIn done");
}

void changeVal(int startVal, int endVal, int pin, boolean RGBLed)
{ 
  boolean countUp = true;
  endVal = endVal - 1;

  if (startVal > endVal)
  {
    countUp = false;
  }

  if (countUp)
  {
    for(startVal; startVal < endVal; startVal++)
    {      
      int dumpVal = startVal;
      startVal = anodeCheck(startVal, RGBLed);
      if (pin == 3 || pin == 5 || pin ==6 || pin == 9)
      {
        analogWrite(pin, startVal);
      }
      else
      {
        digitalWrite(pin, endVal);
      }
      startVal = dumpVal;
      delay(delayTime);
    }
  }
  else
  {
    for(startVal; startVal > endVal; startVal--)
    {
      int dumpVal = startVal;
      startVal = anodeCheck(startVal, RGBLed);
      if (pin == 3 || pin == 5 || pin ==6 || pin == 9)
      {
        analogWrite(pin, startVal);
      }
      else
      {
        digitalWrite(pin, endVal);
      }
      startVal = dumpVal;
      delay(delayTime);
    }
  }
}

int anodeCheck(int val, boolean RGBLed)
{
  if (anodeBased == true && RGBLed == true)
  {
    val = (val - 255) * (-1);            
  }

  return val;
}

void writeColor()
{
  block = true;  
  
  if (SD.exists("COLOR.TXT")) {
    SD.remove("COLOR.TXT");
  }
    
  colorFile = SD.open("COLOR.TXT", FILE_WRITE);  
  if (colorFile) {
    colorFile.print(red);
    colorFile.println(" // red_val");
    colorFile.print(green);
    colorFile.println(" // green_val");
    colorFile.print(blue);
    colorFile.println(" // blue_val");
    colorFile.print(val);
    colorFile.println(" // white_val");
    colorFile.print(pin_red);
    colorFile.println(" // pin_red");
    colorFile.print(pin_green);
    colorFile.println(" // pin_green");
    colorFile.print(pin_blue);
    colorFile.println(" // pin_blue");
    colorFile.print(pin);
    colorFile.println(" // pin_white");
    colorFile.close();
    Serial.println("COLOR.TXT write done");
  } else {
    Serial.println("COLOR.TXT write error");
  }
  block = false;
}


void readOneLine()
{
  char linebuf[40];
  int counter=0;
  memset(linebuf,0,sizeof(linebuf));
  while (colorFile.available()) {
    linebuf[counter]=colorFile.read();
    if (linebuf[counter]=='\n') break;
    if (counter<sizeof(linebuf)-1) { counter++; }
    }
      if (strstr(linebuf,"red_val")) { red=atoi(linebuf); }
      else if (strstr(linebuf,"green_val")) { green=atoi(linebuf); }
      else if (strstr(linebuf,"blue_val")) { blue=atoi(linebuf); }
      else if (strstr(linebuf,"white_val")) { val=atoi(linebuf); }
      else if (strstr(linebuf,"pin_red")) { pin_red=atoi(linebuf); }
      else if (strstr(linebuf,"pin_green")) { pin_green=atoi(linebuf); }
      else if (strstr(linebuf,"pin_blue")) { pin_blue=atoi(linebuf); }
      else if (strstr(linebuf,"pin_white")) { pin=atoi(linebuf); }
      else
    {
    Serial.print("empty line");
    Serial.println(linebuf); 
  }
}

void readColor()
{  
  colorFile = SD.open("COLOR.TXT");
  if (colorFile) {
    while (colorFile.available()) {
      readOneLine();
    }
    colorFile.close();
    Serial.println("COLOR.TXT read done");
  } else {
  	Serial.println("COLOR.TXT read error");
        
        pin_red = 3;
        
        while(true)
        {
          for (int i = 0; i < 256; i++)
          {
            analogWrite(pin_red, i);
            delay(delayTime);
          }
          for (int i = 255; i > 0; i--)
          {
            analogWrite(pin_red, i);
            delay(delayTime);
          }
        }
  }
  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
  Serial.println(val);
  Serial.println(pin_red);
  Serial.println(pin_green);
  Serial.println(pin_blue);
  Serial.println(pin);
}


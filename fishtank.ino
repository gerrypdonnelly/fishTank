    #define primeSolenoidPin 11     //prime solenoid valve connected to pin 11
    #define openValvePin 4          //open valve connected to pin 4
    #define closeValvePin 5         //close valve connected to pin 5
    #define fillSolenoidPin 6       //fill solenoid valve connected to pin 6
    #define sumpPumpPin 7           //sump pump power connected to pin 7
    #define cycleBtnPin 8           //cycle button connected to pin 8
    #define runBtnPin 9             //run button connected to pin 9
    #define emptyBtnPin 10          //empty button connected to pin 10
    #define levelSumpPin 2          //sump sensor connected to pin 2
    #define levelTankPin 3          //tank sensor connected to pin 3
    #define debounceDelay 15        //Set debounce time millis for buttons
    #define topUpTankPin 12         //Topup button connwected to pin 12
 
    //Set up variables
    bool primeSolenoidPinPrevious =  false;
    bool openValvePinPrevious =      false;
    bool closeValvePinPrevious =     false;
    bool fillSolenoidPinPrevious =   false;
    bool sumpPumpPinPrevious =       false;

    bool primeSolenoidPinState =     false;
    bool openValvePinState =         false;
    bool closeValvePinState =        false;
    bool fillSolenoidPinState =      false;
    bool sumpPumpPinState =          false;
    bool cycleBtnState =             false;
    bool cycleBtnPrevious =          false;
    bool cycleBtnRequest =           false;
    bool runBtnState =               false;
    bool runBtnPrevious =            false;
    bool runBtnRequest =             false;
    bool emptyBtnState =             false;
    bool emptyBtnPrevious =          false;
    bool emptyBtnRequest =           false;
    bool topUpTankBtnState =         false;
    bool topUpTankBtnPrevious =      false;
    bool topUpTankBtnRequest =       false;
    bool levelSumpPinState =         false;
    bool levelSumpPinPrevious =      false;
    bool levelTankPinState =         false;
    bool levelTankPinPrevious =      false;
    bool fillRequest =               false;

    bool levelSump =                 false;
    bool levelTank =                 false;
    bool systemState =               false;

    unsigned long current_time = 0;
    unsigned long previous_time = 0;

    unsigned long primeDelay =          1000;          
    unsigned long settlingDelay =       1000;      
    unsigned long emptyDelay =          10000;       
    unsigned long sumpFillDelay =       5000;      
    unsigned long valveActuationDelay = 5000; 
    unsigned long transferDelay =       5000;     
    unsigned long issue1Flash =         100;        
    unsigned long issue2Flash =         100;
  /*
    unsigned long primeDelay = 1000;          //wait for one second
    unsigned long settlingDelay = 5000;       //wait for 5 seconds
    unsigned long emptyDelay = 3600000;       //wait for 1 hour
    unsigned long sumpFillDelay = 60000;      //wait for 1 minute
    unsigned long valveActuationDelay = 5000; //wait for 5 seconds
    unsigned long transferDelay = 300000;     //wait 5 minutes
    unsigned long issue1Flash = 100;        
    unsigned long issue2Flash = 100;
  
  */
  
    #define TROPICAL
    //#define DEBUG
    

    void primeSolenoidValve(void);
    void tankValve(void);
    void Fill(void);
    void emptySystem(void);
    void levelTankPinISR(void);
    void levelSumpPinISR(void);
    void flashLED(unsigned long);
    void flashConstantLED(void);

    /*Flash constant LED for issue 1*/
    void flashConstantLED(void)
    {
      for (int i = 0; i < 100; i++)
      {
        digitalWrite(fillSolenoidPin, HIGH);
        digitalWrite(sumpPumpPin, HIGH);
        digitalWrite(LED_BUILTIN, HIGH); // flash led                               //GPIOD->ODR |=(1U<<..);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW); // GPIOD->ODR &=~(1U<<..);
        delay(100);
      }
    }

    /*Prime the empty syphon pipe*/
    void primeSolenoidValve(void)
    {
#ifdef DEBUG
        Serial.println("Prime solenoid ON");
      #endif
      digitalWrite(primeSolenoidPin, LOW); // GPIOD->ODR|=(1U<<..);
      delay(primeDelay);
#ifdef DEBUG
       Serial.println("Prime solenoid OFF");
#endif
      digitalWrite(primeSolenoidPin, HIGH); // GPIOD->ODR&=~(1U<<..);
#ifdef DEBUG
       Serial.println("Syphon primed");
#endif
    }

    void flashLED(unsigned long flashDelay)
    {
#ifdef DEBUG
       Serial.println("Flashing LED issue LED");
#endif
      // flash the LED to indicate there could be an issue
      for (int i = 0; i < 10; i++)
      {
        digitalWrite(LED_BUILTIN, HIGH); // flash led                               //GPIOD->ODR |=(1U<<..);
        delay(flashDelay);
        digitalWrite(LED_BUILTIN, LOW); // GPIOD->ODR &=~(1U<<..);
        delay(flashDelay);
      }
    }

    /*Function to open the ball valve to empty the main tank*/
    void tankValve(void)
    {
#ifdef DEBUG
      Serial.println("Empty valve OPEN");
#endif
      digitalWrite(openValvePin, LOW); // Open valve  //GPIOD->ODR|=(1U<<..);
      delay(valveActuationDelay);
      digitalWrite(openValvePin, HIGH); // Stop opening valve  //GPIOD->ODR &=~(1U<<..);
#ifdef DEBUG
      Serial.println("Tank empty valve open & emptying sump");
#endif
      digitalWrite(sumpPumpPin, LOW);
      delay(transferDelay);
      digitalWrite(sumpPumpPin, HIGH);
      delay(emptyDelay); // wait for tank to empty
#ifdef DEBUG
      Serial.println("Empty valve CLOSE");
#endif
      digitalWrite(closeValvePin, LOW); // Close valve //GPIOD->ODR|=(1U<<..);
      delay(valveActuationDelay);
      digitalWrite(closeValvePin, HIGH); // Stop closing valve  //GPIOD->ODR &=~(1U<<..);
#ifdef DEBUG
      Serial.println("Tank empty valve closed");
#endif
    }

    /*Function to remove water from the system until the sump and main tank are empty*/
    void emptySystem(void)
    {
#ifdef DEBUG
      Serial.println("Entering the empty function");
#endif
      digitalWrite(sumpPumpPin, HIGH); // Turn off sump pump        //GPIOD->ODR &=~(1U<<..);
#ifdef DEBUG
      Serial.println("Circulating pump off");
#endif
      delay(settlingDelay);
      levelSump = digitalRead(levelSumpPin);
      levelTank = digitalRead(levelTankPin);

      if (levelSump && levelTank) // If both are not full; Prime the syphon and open the ball valve to empty the system
      {
#ifdef DEBUG
        Serial.println("Level of tank and sump both low");
#endif
        primeSolenoidValve();
        tankValve();
        emptyBtnRequest = false;
#ifdef DEBUG
        Serial.println("Empty function completed");
#endif
        return;
      }

      if (levelSump && !levelTank) // If the main tank is full but the sump is not full there could be a blockage raise an issue but continue to prime the system and open the ball valve to empty the system
      {
#ifdef DEBUG
        Serial.println("Tank is high and sump is low");
#endif
        flashLED(issue1Flash); // issue1 flash the LED to indicat ethere could be a pipe blockage
#ifdef DEBUG
        Serial.println("Flashing issue1 LED");
#endif
        emptyBtnRequest = false;
#ifdef DEBUG
        Serial.println("Potential issue found process stopped");
#endif
        flashConstantLED();
        primeSolenoidValve();
        tankValve();
        emptyBtnRequest = false;
#ifdef DEBUG
        Serial.println("Function finished tank empty");
#endif
       return;
      }

      if (!levelSump && levelTank) // If sump is full and tank is not full then prime the syphon and open the ball valve to empty the system
      {
#ifdef DEBUG
        Serial.println("Sump is High and tank is LOW");
#endif
        primeSolenoidValve();
        tankValve();
        emptyBtnRequest = false;
        Serial.println("Function finished tank empty");
        return;
      }

      if (!levelSump && !levelTank) // If both tank and sump are full raise an issue but continue to prime the system and open the ball valve to empty the system
      {
#ifdef DEBUG
        Serial.println("Both tank and sump full");
#endif
        flashLED(issue2Flash); // issue2 flash the LED to indicat ethere could be a pipe blockage
#ifdef DEBUG
        Serial.println("Flash issue 2 LED");
#endif
        primeSolenoidValve();
        tankValve();
      }
      emptyBtnRequest = false;
#ifdef DEBUG
      Serial.println("Empty function completed");
#endif
      levelSump = digitalRead(levelSumpPin);
      levelTank = digitalRead(levelTankPin);
      return;
    }

/*Function to add water to the sump and cycle it into the tank until the main tank is full*/
void Fill(void)
{
   #ifdef DEBUG 
        Serial.println("Starting to fill tank");
   #endif
  digitalWrite(sumpPumpPin, HIGH);                       //Turn off sump pump  //GPIOD->ODR &=~(1U<<..);
  delay(settlingDelay);
  levelSump = digitalRead(levelSumpPin);
  levelTank = digitalRead(levelTankPin);

  if(fillRequest)  
  {
    if(!levelSump && !levelTank) //Both sump is HIGH and tank is HIGH
    {
        digitalWrite(sumpPumpPin, LOW);  
        #ifdef DEBUG
              Serial.println("Both sumpand tank full");
        #endif 
        #ifdef DEBUG
              Serial.println("Fill function finished");
        #endif 
        fillRequest = 0;
        digitalWrite(sumpPumpPin, LOW);
        return;
    }
    if(levelSump && !levelTank)  //sump LOW and tank HIGH
    {
#ifdef DEBUG
       Serial.println("Sump LOW Tank HIGH");
#endif
      while(digitalRead(levelSumpPin)) //Fill the sump 
        {
          digitalWrite(fillSolenoidPin, LOW);
        }
      digitalWrite(fillSolenoidPin, HIGH);
      #ifdef DEBUG
            Serial.println("Sump and tank both full function finished");
      #endif
      fillRequest = 0;
      digitalWrite(sumpPumpPin, LOW);
      return;
    }     
    if(!levelSump && levelTank)     //sump full but tank not full
    {
      #ifdef DEBUG
            Serial.println("Sump HIGH Tank LOW");
      #endif
      #ifdef DEBUG
            Serial.println("Cycling sump into tank");
      #endif
      while(digitalRead(levelSumpPin))//while sump is not full
      {
        digitalWrite(fillSolenoidPin, LOW);
      }
      digitalWrite(fillSolenoidPin, HIGH);
      while(digitalRead(levelTankPin))//while tank is not full
      {
        while(digitalRead(levelSumpPin))//while sump is not full
        {
          digitalWrite(fillSolenoidPin, LOW);
        }
        digitalWrite(fillSolenoidPin, HIGH);
        digitalWrite(sumpPumpPin, LOW);
        delay(transferDelay);
        digitalWrite(sumpPumpPin, HIGH);
      }
      while(digitalRead(levelSumpPin))//while sump is not full
      {
        digitalWrite(fillSolenoidPin, LOW);
      }
      digitalWrite(fillSolenoidPin, HIGH);
      #ifdef DEBUG
            Serial.println("Sump and tank both full fill finished");
      #endif
      fillRequest = 0;
      digitalWrite(sumpPumpPin, LOW);
      return;
    }
    if (levelSump && levelTank) //both LOW fill sump and cycle into tank until both full
    {
      #ifdef DEBUG
            Serial.println("Sump Low Tank low");
      #endif
      while(digitalRead(levelTankPin))
      {
        while(digitalRead(levelSumpPin)) //While the sump is not full.  Fill the sump 
        {
          digitalWrite(fillSolenoidPin, LOW);
        }  
       digitalWrite(fillSolenoidPin,HIGH);//Sump is full stop filling
       digitalWrite(sumpPumpPin, LOW);//Cycle water into tank
       delay(transferDelay);//turn on pump for time
       digitalWrite(sumpPumpPin, HIGH);
      }
      while(digitalRead(levelSumpPin)) //While the sump is not full.  Fill the sump 
      {
        digitalWrite(fillSolenoidPin, LOW);
      }  
      digitalWrite(fillSolenoidPin,HIGH);//Sump is full stop filling
      #ifdef DEBUG
            Serial.println("Sump and tank both full fill finished");
      #endif
      fillRequest = 0;
      digitalWrite(sumpPumpPin, LOW);
      return;
    } 
  }
}

void setup()
{
  Serial.begin(9600);

#ifdef DEBUG
      Serial.println("Program Started");
#endif
  // outputs from system
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(primeSolenoidPin, OUTPUT);
  pinMode(openValvePin, OUTPUT);
  pinMode(closeValvePin, OUTPUT);
  pinMode(fillSolenoidPin, OUTPUT);
  pinMode(sumpPumpPin, OUTPUT);

  // Set all outputs to off
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(primeSolenoidPin, HIGH);
  digitalWrite(openValvePin, HIGH);
  digitalWrite(closeValvePin, HIGH);
  digitalWrite(fillSolenoidPin, HIGH);
  digitalWrite(sumpPumpPin, HIGH);

  // inputs to system
  pinMode(cycleBtnPin, INPUT_PULLUP);
  pinMode(runBtnPin, INPUT_PULLUP);
  pinMode(emptyBtnPin, INPUT_PULLUP);
  pinMode(topUpTankPin, INPUT_PULLUP);
  pinMode(levelSumpPin, INPUT_PULLUP);
  pinMode(levelTankPin, INPUT_PULLUP);
}

void loop()
{
  cycleBtnState = digitalRead(cycleBtnPin);
  runBtnState = digitalRead(runBtnPin);
  emptyBtnState = digitalRead(emptyBtnPin);
  topUpTankBtnState = digitalRead(topUpTankPin);
  levelSump = digitalRead(levelSumpPin);
  levelTank = digitalRead(levelTankPin);

  // Cycle button debounce
  if ((!cycleBtnState) && (!cycleBtnPrevious))
  {
    int unsigned long current_time = millis();
    if ((current_time - previous_time) > debounceDelay)
    {
      previous_time = current_time;
      cycleBtnPrevious = true;
#ifdef DEBUG
      Serial.println("Cycle button pressed");
#endif
    }
  }
  if ((cycleBtnState) && (cycleBtnPrevious))
  {
    cycleBtnPrevious = false;
#ifdef DEBUG
      Serial.println("Cycle button released");
#endif
    cycleBtnRequest = true;
    fillRequest = true;
  }

  // Empty button debounce
  if ((!emptyBtnState) && (!emptyBtnPrevious))
  {
    int unsigned long current_time = millis();
    if ((current_time - previous_time) > debounceDelay)
    {
      previous_time = current_time;
      emptyBtnPrevious = true;
#ifdef DEBUG
      Serial.println("Empty button pressed");
#endif
    }
  }
  if ((emptyBtnState) && (emptyBtnPrevious))
  {
    emptyBtnPrevious = false;
#ifdef DEBUG
      Serial.println("Empty button released");
#endif
    emptyBtnRequest = true;
  }

  // Run button debounce
  if ((!runBtnState) && (!runBtnPrevious))
  {
    int unsigned long current_time = millis();
    if ((current_time - previous_time) > debounceDelay)
    {
      previous_time = current_time;
      runBtnPrevious = true;
#ifdef DEBUG
      Serial.println("Run button pressed");
#endif
    }
  }
  if ((runBtnState) && (runBtnPrevious))
  {
    runBtnPrevious = false;
#ifdef DEBUG
       Serial.println("Run button released");
#endif
    runBtnRequest = true;
  }


 // topUpTank button debounce  
  if ((!topUpTankBtnState) && (!topUpTankBtnPrevious))
  {
    int unsigned long current_time = millis();
    if ((current_time - previous_time) > debounceDelay)
    {
      previous_time = current_time;
      topUpTankBtnPrevious = true;
#ifdef DEBUG
      Serial.println("Top up tank button pressed");
#endif
    }
  }
  if ((topUpTankBtnState) && (topUpTankBtnPrevious))
  {
    topUpTankBtnPrevious = false;
#ifdef DEBUG
      Serial.println("Top up tank button released");
#endif
    topUpTankBtnRequest = true;
  }

  if (!systemState) // state register starts will all 0s first run so initial state needs to be set
  {
    systemState = true;
    digitalWrite(sumpPumpPin, HIGH); // Turn off sump pump
    delay(settlingDelay);            // Allow system to settle and trigger level sensors if needed
    digitalWrite(sumpPumpPin, LOW);  // Turn on sump pump
  }



  if (systemState && emptyBtnRequest)
    emptySystem(); // If empty button request and system is ready empty the system

  if (systemState && runBtnRequest)
  {
#ifdef DEBUG
      Serial.println("Run request made \n Turning on sump pump.......");
#endif
    runBtnRequest = false;
    digitalWrite(sumpPumpPin, LOW); // If the Run flag is set and system is ready; turn on sump pump
  }


  if (systemState && cycleBtnRequest) // If a cycle request is made and the system is ready cycle the water
  {
#ifdef DEBUG
      Serial.println("Cycle request made");
#endif
    cycleBtnRequest = false;
    emptySystem();
    Fill();
  }


  if (systemState && topUpTankBtnRequest) // If a cycle request is made and the system is ready cycle the water
  {
#ifdef DEBUG
      Serial.println("Top up tank request made");
#endif
    topUpTankBtnRequest = false;
    fillRequest = true;
    Fill();
  }
  
}

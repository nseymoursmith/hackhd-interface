/*
  Serial interface for the HackHD via arduino
  N. Seymour-Smith 25/09/13
  
  This program should keep the HHD on standby at all times
  and take commands from serial to record. Set "auto = 0" to
  avoid this, and let the HHD remain off
  
  Send number command by serial (no newline):
  -2: Boot the HHD into standby-mode (takes ~12 sec)
  -1: Check status
   0: Switch off the HHD (takes ~8 sec)
  >0: Record for >0 seconds (takes ~4 sec to save)
  
  Arduino returns status of HHD:
  0: HHD off
  1: HHD on
  2: HHD received/executing command
  3: HHD recording error
  
  "ERROR: STATUS X" denotes an error due to command being
  invalid while the HHD is in state X
  */
  
const int ShutterButtonPin = 12;
const int StatusPin = 10;
const int LED = 13;

int Status = 0;
int run, boot, shutdown, check, auto_on;
//Recording time in seconds:
int t_rec;

//Serial communication
const int buff = 20;
char command[buff]; // Command string (time in sec, to record)
char inChar; // incoming character
byte index = 0; // string index
String message = "STATUS ";

// `setup' routine runs at reset:
void setup() {
  
  boot = 0;
  run = 0;
  shutdown = 0;
  check = 0;
  //debug auto off
  auto_on = 1;
  
 
  digitalWrite(ShutterButtonPin, LOW);
  pinMode(ShutterButtonPin, OUTPUT);
  pinMode(StatusPin, INPUT);
  pinMode(LED, OUTPUT); 
  
  Serial.begin(9600);
  while(!Serial);
  
  Status = digitalRead(StatusPin);
  digitalWrite(LED, Status);
}

void loop() {
  
  Status = digitalRead(StatusPin);
  digitalWrite(LED, Status);
  
  if((Status == LOW) && (auto_on == 1))
  {
    boot = 1;
  }
  else if(Serial.available()) // Get commands from serial
  {
    index = 0;
    while (Serial.available() > 0)
    {
      if(index < buff) // One less than the size of the array
      {
          inChar = Serial.read(); // Read a character
          command[index] = inChar; // Store it
          index++; // Increment where to write next
          command[index] = '\0'; // Null terminate the string
      }
      //Need a delay for Serial to catch up
      delay(10);
    }
    
    //Set command registers
    t_rec = atol(command);
    if(t_rec > 0)
    {
      switch(Status)
      {
        case HIGH:
          run = 1;
          break;
        case LOW:
          Serial.println("ERROR: STATUS 0");
          break;
      }
    }
    else if(t_rec == 0)
    {
      switch(Status)
      {
        case HIGH:
          shutdown = 1;
          break;
        case LOW:
          Serial.println("ERROR: STATUS 0");
          break;
      }
    }
    else if(t_rec == -1)
    {
      check = 1;
    }
    else if(t_rec == -2)
    {
      switch(Status)
      {
        case HIGH:
          Serial.println("ERROR: STATUS 1");
          break;
        case LOW:
          boot = 1;
          break;
      }
    }
    else
    {
      Serial.println("ERROR: UNKNOWN COMMAND");
    }
  }
  
    
  if(check == 1)
  {
    String output = message + Status; 
    Serial.println(output);
    
    check = 0;
  }

  if(boot == 1)
  {
    String output = message + "2";
    Serial.println(output);
    
    // Boot in standby mode
    push_button(8000);
    // Wait 4 sec in case we were in the wrong mode
    delay(4000);
    status_report(); 
    
    boot = 0;   
  }
  
  if(run == 1)
  {
    String output = message + "2";
    Serial.println(output);
    
    //start recording..
    push_button(400); 
    
    //Check for flashing..
    int dt;
    dt = monitor_LED(Status, 5000, 20);
    if(dt > 500)
    {
     //Wait for the recording to finish 
      //Don't forget integer rollover!
      int rem = (t_rec - 5) % 30;
      dt = monitor_LED(Status, (rem*1000), 100);
      
      int N = floor((t_rec - 5)/30);
      int i;
      for(i = 0; i < N; i++)
      {
	dt = monitor_LED(Status, (30*1000), 100);
      }
      
      //stop recording
      push_button(400);
    
      //wait for save
      delay(4000); 
    }
    else if(dt == 0)
    {
      //report a recording error
      String output = message + "3";
      Serial.println(output);
    }
    
    status_report();
    run = 0;
  }
  
  if(shutdown == 1)
  {
    String output = message + "2";
    Serial.println(output);
    
    //Shut down the HHD
    push_button(4000);
    // Wait for HHD to finish
    delay(4000);
    status_report();
    
    shutdown = 0;
  }
  
}

// Pushes the shutterbutton for t msec
void push_button(int t) {
    digitalWrite(ShutterButtonPin, HIGH);
    //debug
    //Serial.println(t);
    delay(t);
    digitalWrite(ShutterButtonPin, LOW);
}

// Reports the HHD's status to serial
void status_report(){
    Status = digitalRead(StatusPin);
    String output = message + Status;
    Serial.println(output);
    digitalWrite(LED, Status);
}

// Monitors the LED output status for 'time' at 'resolution'
// returns the time between switches if any
int monitor_LED(int Status, int time, int res) {
  int i = 0;
  int Status_last;
  int t, t_last, dt, dtmin;
  
  Status_last = Status;
  t_last = millis();
  //dtmin = 10; //debounce doesn't seem to be necessary
  dt = 0;
  while(i*res < time)
  {
    i++;
    Status = digitalRead(StatusPin);
    //debug:
    //Serial.println(Status);
    if(Status != Status_last)
    {
      t = millis();
      int temp = t - t_last;
      //Serial.println(temp);
      if((temp) > dt)
      {
        dt = temp;
      }
      t_last = t;
      digitalWrite(LED, Status);
      Status_last = Status;
    }
    delay(res);
  }
  return dt;
}

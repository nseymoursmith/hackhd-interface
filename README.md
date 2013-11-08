"""  
Arduino interface to HHD camera

Commissioned by photobot.co and released with their
kind permission

N. Seymour-Smith 26/09/13  

---

Arduino should keep the HHD on standby at all times.

Currently, there is no specific error that identifies  
memory corruption or shortage. The HHD will always  
switch itself off in this situation though, so this error  
can be identified by multiple failed boots.  

Arduino serial interface:  

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

"""

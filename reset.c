#include "reset.h"
#include "globals.h"
#include "custom_gui.h"
#include "main_menu.h"
#include "serial.h"
 

static int reset_proc(int msg, DIALOG *d, int c);
 
static char reset_status_msg[64];


static DIALOG reset_chip_dialog[] =
{
   /* (proc)            (x)  (y) (w)  (h) (fg)     (bg)          (key) (flags) (d1) (d2) (dp)              (dp2) (dp3) */
   { d_shadow_box_proc, 0,   0,  300, 64, C_BLACK, C_LIGHT_GRAY, 0,    0,      0,   0,   NULL,             NULL, NULL },
   { caption_proc,      150, 24, 138, 16, C_BLACK, C_TRANSP,     0,    0,      0,   0,   reset_status_msg, NULL, NULL },
   { reset_proc,        0,   0,  0,   0,  0,       0,            0,    0,      0,   0,   NULL,             NULL, NULL },
   { NULL,              0,   0,  0,   0,  0,       0,            0,    0,      0,   0,   NULL,             NULL, NULL }
};


void reset_chip()
{
   centre_dialog(reset_chip_dialog);
   popup_dialog(reset_chip_dialog, -1);
}


enum ResetState
{
   RESET_SEND_RESET_REQUEST, 
   RESET_GET_REPLY_TO_RESET, 
   RESET_SEND_VALIDATE_REQUEST,
   RESET_GET_REPLY_TO_VALIDATE,
   RESET_START_ECU_TIMER,
   RESET_WAIT_FOR_ECU_TIMEOUT,
   RESET_SEND_DETECT_PROTOCOL_REQUEST, 
   RESET_GET_REPLY_TO_DETECT_PROTOCOL, 
   RESET_CLOSE_DIALOG, 
   RESET_INTERFACE_NOT_FOUND, 
   RESET_HANDLE_CLONE
};


int Reset_send_reset_request(char *response)
{
   char buf[128];
      
   if (serial_timer_running) // if serial timer is running
   {
      // wait until we either get a prompt or the timer times out
      while ((read_comport(buf) != PROMPT) && !serial_time_out)
         ;
   }
   send_command("atz"); // reset the chip
   start_serial_timer(ATZ_TIMEOUT);  // start serial timer

   response[0] = 0;
   
   return RESET_GET_REPLY_TO_RESET;     
}


int Reset_get_reply_to_reset(char *response, int *device)
{
   char buf[128];
   int next_state;
   int status;
   
   status = read_comport(buf);                  // read comport
   
   if(status == DATA)                           // if new data detected in com port buffer
   {
      strcat(response, buf);                    // append contents of buf to response
      next_state = RESET_GET_REPLY_TO_RESET;    // come back for more data
   }
   else if(status == PROMPT) // if '>' detected
   {
      stop_serial_timer();
      strcat(response, buf);
      *device = process_response("atz", response);
      
      if (*device == INTERFACE_ELM323)
         next_state = RESET_START_ECU_TIMER;
      else if (*device == INTERFACE_ELM327)
         next_state = RESET_SEND_VALIDATE_REQUEST;
      else
         next_state = RESET_CLOSE_DIALOG;
   }
   else if (serial_time_out) // if the timer timed out
   {
      stop_serial_timer(); // stop the timer
      alert("Interface was not found", NULL, NULL, "OK", NULL, 0, 0);
      next_state = RESET_CLOSE_DIALOG; // close dialog
   }  
   else  // serial buffer was empty, but we still got time
      next_state = RESET_GET_REPLY_TO_RESET;    
   
   return next_state;
}


int Reset_send_validate_request(char *response)
{
   int next_state;
   
   send_command("at@1");
   start_serial_timer(AT_TIMEOUT);  // start serial timer
   response[0] = 0;
   next_state = RESET_GET_REPLY_TO_VALIDATE;   
   
   return next_state;   
}


int Reset_get_reply_to_validate(char *response)
{
   char buf[128];
   int next_state;
   int status;
   int device;
   
   status = read_comport(buf);                  // read comport
   
   if(status == DATA)                           // if new data detected in com port buffer
   {
      strcat(response, buf);                    // append contents of buf to response
      next_state = RESET_GET_REPLY_TO_VALIDATE;    // come back for more data
   }
   else if(status == PROMPT) // if '>' detected
   {
      stop_serial_timer();
      strcat(response, buf);
      device = process_response("at@1", response);
      
      if (device == GENUINE_ELMSCAN5)
         next_state = RESET_START_ECU_TIMER;
      else           
         next_state = RESET_HANDLE_CLONE;     
   }
   else if (serial_time_out) // if the timer timed out
   {
      stop_serial_timer(); // stop the timer
      alert("Interface was not found", NULL, NULL, "OK", NULL, 0, 0);
      next_state = RESET_CLOSE_DIALOG; // close dialog
   }  
   else  // serial buffer was empty, but we still got time
      next_state = RESET_GET_REPLY_TO_VALIDATE;    
   
   return next_state;
}


int Reset_start_ecu_timer()
{
   start_serial_timer(ECU_TIMEOUT);
   return RESET_WAIT_FOR_ECU_TIMEOUT;    
}


int Reset_wait_for_ecu_timeout(int *device)
{
   int next_state;
   
   if (serial_time_out) // if the timer timed out
   {
      stop_serial_timer(); // stop the timer
      if (*device == INTERFACE_ELM327)
      {
         next_state = RESET_SEND_DETECT_PROTOCOL_REQUEST;
      }
      else
         next_state = RESET_CLOSE_DIALOG;   
   }
   else     
      next_state = RESET_WAIT_FOR_ECU_TIMEOUT;   
      
   return next_state; 
}


int Reset_send_detect_protocol_request(char *response, int *device)
{
   int next_state;
   
   send_command("0100");
   start_serial_timer(OBD_REQUEST_TIMEOUT);  // start serial timer
   response[0] = 0;
   next_state = RESET_GET_REPLY_TO_DETECT_PROTOCOL;   
   
   return next_state;
}


int Reset_get_reply_to_detect_protocol(char *response)
{
   char buf[128];
   int next_state;
   int status;
      
   status = read_comport(buf);

   if(status == DATA) // if new data detected in com port buffer
   {
      strcat(response, buf); // append contents of buf to response
      next_state = RESET_GET_REPLY_TO_DETECT_PROTOCOL; // come back for more
   }
   else if (status == PROMPT)  // if we got the prompt
   {
      stop_serial_timer();
      strcat(response, buf);
      status = process_response("0100", response);

      if (status == ERR_NO_DATA || status == UNABLE_TO_CONNECT)
         alert("Protocol could not be detected.", "Please check connection to the vehicle,", "and make sure the ignition is ON", "OK", NULL, 0, 0);
      else if (status != HEX_DATA)
         alert("Communication error", NULL, NULL, "OK", NULL, 0, 0);
         
      next_state = RESET_CLOSE_DIALOG;
   }
   else if (serial_time_out) // if the timer timed out
   {
      stop_serial_timer(); // stop the timer
      alert("Interface not found", NULL, NULL, "OK", NULL, 0, 0);
      next_state = RESET_CLOSE_DIALOG;
   }   
   else
      next_state = RESET_GET_REPLY_TO_DETECT_PROTOCOL;
   
   return next_state;
}


int Reset_handle_clone()
{
   alert("Your device does not appear to be a genuine ElmScan 5. Due to their poor", 
            "quality and high support costs, ELM327 clones are no longer supported.", 
            "Please visit www.ScanTool.net to purchase a genuine scan tool.", "OK", NULL, 0, 0); 
   
   is_not_genuine_scan_tool = TRUE;
   
   return RESET_CLOSE_DIALOG;   
   
}


int reset_proc(int msg, DIALOG *d, int c)
{
   static int state = RESET_SEND_RESET_REQUEST;
   static int device = 0;
   static char response[256];
    
   // process_reset_procswitch (msg)
   //{
   switch(msg)
   {
      case MSG_START:
         state = RESET_SEND_RESET_REQUEST;
         break;
   
      case MSG_IDLE:
         switch (state)
         {
            case RESET_SEND_RESET_REQUEST:
               strcpy(reset_status_msg, "Resetting hardware interface...");               
               state = Reset_send_reset_request(response);
               return D_REDRAW;
               break;

            case RESET_GET_REPLY_TO_RESET:
               state = Reset_get_reply_to_reset(response, &device);              
               break;
               
            case RESET_SEND_VALIDATE_REQUEST:
               strcpy(reset_status_msg, "Making sure scan tool is genuine...");
               state = Reset_send_validate_request(response);
               return D_REDRAW;
               break;
               
            case RESET_GET_REPLY_TO_VALIDATE:
               state = Reset_get_reply_to_validate(response);
               break;
            
            case RESET_START_ECU_TIMER:
               strcpy(reset_status_msg, "Waiting for ECU timeout...");                
               state = Reset_start_ecu_timer();
               return D_REDRAW;
               
            case RESET_WAIT_FOR_ECU_TIMEOUT:
               state = Reset_wait_for_ecu_timeout(&device);           
               break;             
               
            case RESET_SEND_DETECT_PROTOCOL_REQUEST:
               strcpy(reset_status_msg, "Detecting OBD protocol...");               
               state = Reset_send_detect_protocol_request(response, &device);
               return D_REDRAW;
               break;
               
            case RESET_GET_REPLY_TO_DETECT_PROTOCOL:
               state = Reset_get_reply_to_detect_protocol(response);
               break;
               
            case RESET_HANDLE_CLONE:
               state = Reset_handle_clone();
               break;
            
            case RESET_CLOSE_DIALOG:
               return D_CLOSE;
         }
         
   }

   return D_O_K;
}

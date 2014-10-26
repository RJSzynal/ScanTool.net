#include "globals.h"
#include "custom_gui.h"
#include "serial.h"
#include "options.h"

#define MSG_SAVE_OPTIONS   MSG_USER
#define MSG_REFRESH        MSG_USER + 1

// Define defaults
#ifdef ALLEGRO_WINDOWS
   #define DEFAULT_DISPLAY_MODE         WINDOWED_MODE
#else
   #define DEFAULT_DISPLAY_MODE         FULL_SCREEN_MODE
#endif
#define DEFAULT_SYSTEM_OF_MEASURMENTS   IMPERIAL
#define DEFAULT_COMPORT_NUMBER          COM1
#define DEFAULT_BAUD_RATE               BAUD_RATE_9600

typedef struct
{
   int option_value;
} OPTION_ELEMENT;


static int option_element_proc(int msg, DIALOG *d, int c);
static int save_options_proc(int msg, DIALOG *d, int c);

static DIALOG options_dialog[] =
{
   /* (proc)              (x)  (y)  (w)  (h)  (fg)      (bg)           (key) (flags) (d1) (d2) (dp)                       (dp2) (dp3)                               */
   { d_shadow_box_proc,   0,   0,   231, 440, 0,         C_LIGHT_GRAY,  0,    0,      0,   0,   NULL,                      NULL, NULL                                },
   { d_shadow_box_proc,   0,   0,   231, 24,  0,         C_DARK_GRAY,   0,    0,      0,   0,   NULL,                      NULL, NULL                                },
   { caption_proc,        115, 2,   113, 19,  C_WHITE,   C_TRANSP,      0,    0,      0,   0,   "Program Options",         NULL, NULL                                },
   { d_text_proc,         16,  32,  200, 16,  C_BLACK,   C_TRANSP,      0,    0,      0,   0,   "System Of Measurements:", NULL, NULL                                },
   { option_element_proc, 48,  56,  80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      0,   0,   "Metric",                  NULL, &(OPTION_ELEMENT){METRIC}           },
   { option_element_proc, 48,  72,  88,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      0,   0,   "US",                      NULL, &(OPTION_ELEMENT){IMPERIAL}         },
   { d_text_proc,         16,  96,  152, 16,  C_BLACK,   C_TRANSP,      0,    0,      0,   0,   "COM Port:",               NULL, NULL                                },
   { option_element_proc, 48,  120, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM1",                    NULL, &(OPTION_ELEMENT){COM1}             },
   { option_element_proc, 48,  136, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM2",                    NULL, &(OPTION_ELEMENT){COM2}             },
   { option_element_proc, 48,  152, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM3",                    NULL, &(OPTION_ELEMENT){COM3}             },
   { option_element_proc, 48,  168, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM4",                    NULL, &(OPTION_ELEMENT){COM4}             },
   { option_element_proc, 48,  184, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM5",                    NULL, &(OPTION_ELEMENT){COM5}             },
   { option_element_proc, 48,  200, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM6",                    NULL, &(OPTION_ELEMENT){COM6}             },
   { option_element_proc, 48,  216, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM7",                    NULL, &(OPTION_ELEMENT){COM7}             },
   { option_element_proc, 48,  232, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      1,   0,   "COM8",                    NULL, &(OPTION_ELEMENT){COM8}             },
   { d_text_proc,         16,  256, 200, 16,  C_BLACK,   C_TRANSP,      0,    0,      0,   0,   "Baud Rate:",              NULL, NULL                                },
   { option_element_proc, 48,  280, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      2,   0,   "9600",                    NULL, &(OPTION_ELEMENT){BAUD_RATE_9600}   },
   { option_element_proc, 48,  296, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      2,   0,   "38400",                   NULL, &(OPTION_ELEMENT){BAUD_RATE_38400}  },
   { d_text_proc,         16,  320, 200, 16,  C_BLACK,   C_TRANSP,      0,    0,      0,   0,   "Display Mode:",           NULL, NULL                                },
   { option_element_proc, 48,  344, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      3,   0,   "Windowed",                NULL, &(OPTION_ELEMENT){WINDOWED_MODE}    },
   { option_element_proc, 48,  360, 80,  10,  C_BLACK,   C_LIGHT_GRAY,  0,    0,      3,   0,   "Full Screen",             NULL, &(OPTION_ELEMENT){FULL_SCREEN_MODE} },
   { save_options_proc,   16,  384, 92,  40,  C_BLACK,   C_GREEN,       's',  D_EXIT, 0,   0,   "&Save",                   NULL, NULL                                },
   { d_button_proc,       123, 384, 92,  40,  C_BLACK,   C_DARK_YELLOW, 'c',  D_EXIT, 0,   0,   "&Cancel",                 NULL, NULL                                },
   { NULL,                0,   0,   0,   0,   0,       0,               0,    0,      0,   0,   NULL,                      NULL, NULL                                }
};


int display_options()
{
   if (display_mode & WINDOWED_MODE_SET)
      display_mode |= WINDOWED_MODE;
   else
      display_mode &= ~WINDOWED_MODE;
   centre_dialog(options_dialog);
   return popup_dialog(options_dialog, -1);
}


int option_element_proc(int msg, DIALOG *d, int c)
{
   OPTION_ELEMENT *option_element = (OPTION_ELEMENT *) d->dp3;  // make element_name point to struct pointed by d->dp3;

   switch (msg)
   {
      case MSG_START:
      case MSG_REFRESH:
         switch (d->d1)
         {
            case 0:
               if (option_element->option_value == system_of_measurements) // if the element should be selected
                  d->flags |= D_SELECTED;
               else
                  d->flags &= ~D_SELECTED;
               break;
               
            case 1:
               if (option_element->option_value == comport.number)
                  d->flags |= D_SELECTED;
               else
                  d->flags &= ~D_SELECTED;
               break;
               
            case 2:
               if (option_element->option_value == comport.baud_rate)
                  d->flags |= D_SELECTED;
               else
                  d->flags &= ~D_SELECTED;
               break;
               
            case 3:
               if (option_element->option_value == (display_mode & WINDOWED_MODE))
                  d->flags |= D_SELECTED; // make it selected
               else
                  d->flags &= ~D_SELECTED;
               if (((option_element->option_value == WINDOWED_MODE) && !(display_mode & WINDOWED_MODE_SUPPORTED)) || ((option_element->option_value == FULL_SCREEN_MODE) && !(display_mode & FULLSCREEN_MODE_SUPPORTED)))
                  d->flags |= D_DISABLED;
               else
                  d->flags &= ~D_DISABLED;
               break;
         }
         if (msg == MSG_REFRESH)
            msg = MSG_DRAW;
         break;

      case MSG_SAVE_OPTIONS:
         if (d->flags & D_SELECTED)  // if the element is selected,
            switch (d->d1)
            {
               case 0:
                  system_of_measurements = option_element->option_value;
                  break;
                  
               case 1:
                  comport.number = option_element->option_value;
                  break;

               case 2:
                  comport.baud_rate = option_element->option_value;
                  break;
                  
               case 3:
                  display_mode &= ~WINDOWED_MODE;
                  display_mode |= option_element->option_value;
                  break;
            }
         break;
   
      case MSG_END:
         d->flags &= ~D_SELECTED;  // deselect
         d->flags &= ~D_DISABLED;  // and enable the element
         break;
   }

   return d_radio_proc(msg, d, c);
}


int save_options_proc(int msg, DIALOG *d, int c)
{
   int ret;
   int old_baud_rate;
   BITMAP *bmp;
   FILE *file;

   ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE)
   {
      old_baud_rate = comport.baud_rate;
      broadcast_dialog_message(MSG_SAVE_OPTIONS, 0);
      
      if (comport.baud_rate != old_baud_rate)
      {
         if (alert("WARNING!", "This operation may cause scan tool to stop responding.", "Are you sure you want to change the baud rate?", "Yes", "No", 0, 0) != 1)
         {
            comport.baud_rate = old_baud_rate;
            broadcast_dialog_message(MSG_REFRESH, 0);
            return D_O_K;
         }
      }

      close_comport(); // close current comport
      open_comport(); // try reinitializing comport (comport.status will be set)
      
      if ((!(display_mode & WINDOWED_MODE) && (display_mode & WINDOWED_MODE_SET)) || ((display_mode & WINDOWED_MODE) && !(display_mode & WINDOWED_MODE_SET)))
      {
         bmp = create_bitmap(SCREEN_W, SCREEN_H);
         if (bmp)
         {
            scare_mouse();
            blit(screen, bmp, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            unscare_mouse();

            if (display_mode & WINDOWED_MODE)
            {
               if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) == 0)
                  display_mode |= WINDOWED_MODE_SET;
               else
                  display_mode &= ~WINDOWED_MODE_SUPPORTED;
            }
            else
            {
               if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 640, 480, 0, 0) == 0)
                  display_mode &= ~WINDOWED_MODE_SET;
               else
                  display_mode &= ~FULLSCREEN_MODE_SUPPORTED;
            }

            set_pallete(datafile[MAIN_PALETTE].dat);
            gui_fg_color = C_BLACK;  // set the foreground color
            gui_bg_color = C_WHITE;  // set the background color
            gui_mg_color = C_GRAY;   // set the disabled color
            set_mouse_sprite(NULL); // make mouse use current palette
            blit(bmp, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
            show_mouse(screen);
            destroy_bitmap(bmp);
         }
         else
            alert("Error switching display modes.", "Not enough memory to save screen.", NULL, "OK", NULL, 0, 0);
      }

      file = fopen(options_file_name, "a");

      if (file == NULL)
         alert("Options could not be saved, because file", options_file_name, "could not be open for writing", "OK", NULL, 0, 0);
      else
      {
         fclose(file);
         save_program_options();
      }
   }

   return ret;
}

// DO NOT TRANSLATE BELOW THIS LINE
void load_program_options()
{
   comport.number = get_config_int("comm", "comport_number", DEFAULT_COMPORT_NUMBER);
   comport.baud_rate = get_config_int("comm", "baud_rate", DEFAULT_BAUD_RATE);
   system_of_measurements = get_config_int("general", "system_of_measurements", DEFAULT_SYSTEM_OF_MEASURMENTS);
   if (get_config_int("general", "display_mode", DEFAULT_DISPLAY_MODE))
      display_mode |= WINDOWED_MODE_SET;
   else
      display_mode &= ~WINDOWED_MODE_SET;
}


void save_program_options()
{
   set_config_int("general", "system_of_measurements", system_of_measurements);
   if (display_mode & WINDOWED_MODE_SET)
      set_config_int("general", "display_mode", WINDOWED_MODE);
   else
      set_config_int("general", "display_mode", FULL_SCREEN_MODE);
   set_config_int("comm", "baud_rate", comport.baud_rate);
   set_config_int("comm", "comport_number", comport.number);
   flush_config_file();
}

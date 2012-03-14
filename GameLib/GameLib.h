#ifndef GAMELIB_HEAD
#define GAMELIB_HEAD

#include "../DDrawLib/DDrawLib.h"
#include "../BOB/bob.h"

int Game_Init(void * parms = NULL, int num_parms = 0);
int Game_Shutdown(void * parms = NULL, int num_parms = 0);
int Game_Main(void * parms = NULL, int num_parms = 0);
int Game_Debug_Textout();


extern	unsigned int		Mouse_X;
extern	unsigned int		Mouse_Y;
extern	bool				bLBUTTONDOWN;
extern	HWND				main_window_handle; // save the window handle

#endif
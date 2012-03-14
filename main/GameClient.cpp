// WindowsModel.cpp : Defines the entry point for the application.

//say no to MFC
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define INITGUID            // include all GUIDs 

#include <stdio.h>

#include "../GameLib/GameLib.h"

#define		WINDOW_CLASS_NAME	"WindowsModel"
#define		WINDOW_TITLE		"Windows Model"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	char buffer[80];
	static int wm_paint_count=0;

	switch (message) 
	{
	case WM_CREATE:		
		break;
	case WM_CHAR:
		break;

	case WM_MOUSEMOVE:

		sprintf(buffer,"(%d,%d) ",LOWORD(lParam),HIWORD(lParam));
		SetWindowText(hWnd,buffer);
		break;
	case WM_LBUTTONDOWN:
		Mouse_X=LOWORD(lParam);
		Mouse_Y=HIWORD(lParam);
		
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:


 		break;

	case WM_SIZE:
		{
// 			int wigth = LOWORD(lParam);
// 			int height  =HIWORD(lParam);			
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		
		// TODO: Add any drawing code here...
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wcex;


	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);//,MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);//,MAKEINTRESOURCE(IDC_CURSOR1));
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(MainMenu);
	wcex.lpszClassName	= WINDOW_CLASS_NAME;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, IDI_APPLICATION);
	

	if (!RegisterClassEx(&wcex))
	{
		return 0;
	}	

	if (!(hWnd = CreateWindow(WINDOW_CLASS_NAME, 
								WINDOW_TITLE, 
								//WS_OVERLAPPEDWINDOW|WS_VISIBLE,
								WS_OVERLAPPED|WS_VISIBLE,
								//WS_POPUP | WS_VISIBLE,
								(GetSystemMetrics(SM_CXSCREEN) - SCREEN_WIDTH)/2,
								(GetSystemMetrics(SM_CYSCREEN) - SCREEN_HEIGHT)/2, 
								SCREEN_WIDTH, SCREEN_HEIGHT, 
								NULL, 
								NULL, 
								hInstance, 
								NULL))) 
	{
		return 0;
	}
	
	main_window_handle=hWnd;	

	if (Game_Init()!=0)
	{
		MessageBox(hWnd,"Game_Init error","",MB_OK);
	}
	
	while(TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}		

		Game_Main();
	}

	Game_Shutdown();	

	return msg.wParam;
}



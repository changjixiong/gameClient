#ifndef DDRAW_HEAD
#define DDRAW_HEAD

#include <ddraw.h>    // directX includes
#pragma  comment(lib,"ddraw.lib")

#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480
#define SCREEN_BPP			32
#define	BITMAP_ID			0x4D42 // universal id for a bitmap
#define	MAX_COLORS_PALETTE	256


// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))
// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))
// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

#define DDRAW_INIT_STRUCT(ddstruct) {memset(&ddstruct,0,sizeof(ddstruct));ddstruct.dwSize=sizeof(ddstruct);}

typedef struct BITMAP_FILE_TAG 
{
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;
	int				 bytes_per_line;
	PALETTEENTRY palette[256];
	UCHAR *buffer;
}BITMAP_FILE, *BITMAP_FILE_PTR;


int AdjustWindow(HWND hWnd);
int GetClientRectFromScreen(HWND hWnd, LPRECT lpRect);

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);

int Load_Bitmap_File(BITMAP_FILE_PTR pbitmap,char * filename);
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int Width, int Height, int Memflags, DWORD colorkey = 0);
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client=NULL);
int DDraw_Init(int width, int height, int bpp, int windowed=0);
int DDraw_Shutdown(void);

int Draw_Ground(LPDIRECTDRAWSURFACE7 ground);
int Draw_Refresh();

// gdi functions
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);

#endif
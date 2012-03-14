#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "DDrawLib.h"
#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream.h> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <time.h>

#include <ddraw.h>    // directX includes

HWND					main_window_handle	= NULL;

DDSURFACEDESC2			ddsd;
LPDIRECTDRAW7			lpdd				= NULL;
LPDIRECTDRAWSURFACE7	lpddsprimary		= NULL;
LPDIRECTDRAWSURFACE7	lpddsCavas			= NULL;

RECT					rectWindows;
RECT					rectMemory;
POINT					pClientToScreen;

int AdjustWindow(HWND hWnd)
{
	RECT window_rect;
	GetWindowRect(hWnd,&window_rect);
	AdjustWindowRectEx(&window_rect,
		GetWindowStyle(hWnd),
		GetMenu(hWnd)!=NULL,
		GetWindowExStyle(hWnd));
	
	MoveWindow(hWnd,
		(GetSystemMetrics(SM_CXSCREEN) - (window_rect.right - window_rect.left))/2,
		(GetSystemMetrics(SM_CYSCREEN) - (window_rect.bottom - window_rect.top))/2,
		window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top,
		TRUE);
	
	return 0;
}

int GetClientRectFromScreen(HWND hWnd, LPRECT lpRect)
{
	GetClientRect(hWnd,lpRect);
	pClientToScreen.x=0;
	pClientToScreen.y=0;
	ClientToScreen(hWnd,&pClientToScreen);
	
	lpRect->left+=pClientToScreen.x;
	lpRect->right+=pClientToScreen.x;
	lpRect->top+=pClientToScreen.y;
	lpRect->bottom+=pClientToScreen.y;
	
	return 0;
}

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
	// this function is used to flip bottom-up .BMP images
	
	UCHAR *buffer; // used to perform the image processing
	int index;     // looping index
	
	// allocate the temporary buffer
	if (!(buffer = (UCHAR *)malloc(bytes_per_line*height)))
		return(0);
	
	// copy image to work area
	memcpy(buffer,image,bytes_per_line*height);
	
	// flip vertically
	for (index=0; index < height; index++)
		memcpy(&image[((height-1) - index)*bytes_per_line],
		&buffer[index*bytes_per_line], bytes_per_line);
	
	// release the memory
	free(buffer);
	
	// return success
	return(1);
	
} // end Flip_Bitmap

int Load_Bitmap_File(BITMAP_FILE_PTR pbitmap,char * filename)
{
	int file_handle,index;
	
	UCHAR * temp_buffer = NULL;
	OFSTRUCT file_data;
	
	if ((file_handle=OpenFile(filename, &file_data, OF_READ))==-1)
	{
		return 0;
	}
	
	//load the bitmap file header
	_lread(file_handle, &pbitmap->bitmapfileheader, sizeof(BITMAPFILEHEADER));
	
	if (pbitmap->bitmapfileheader.bfType!=BITMAP_ID)
	{
		_lclose(file_handle);
		return 0;
	}
	
	_lread(file_handle, &pbitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER));
	
	if (pbitmap->bitmapinfoheader.biBitCount==8)
	{
		_lread(file_handle, &pbitmap->palette,MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));
		for (index=0;index<MAX_COLORS_PALETTE;index++)
		{
			int temp_color = pbitmap->palette[index].peRed;
			pbitmap->palette[index].peRed = pbitmap->palette[index].peBlue;
			pbitmap->palette[index].peBlue = temp_color;
			pbitmap->palette[index].peFlags=PC_NOCOLLAPSE;
		}
	}
	
	_lseek(file_handle, -(int)pbitmap->bitmapinfoheader.biSizeImage, SEEK_END);
	
	if (pbitmap->bitmapinfoheader.biBitCount==8
		||pbitmap->bitmapinfoheader.biBitCount==16
		||pbitmap->bitmapinfoheader.biBitCount==24)
	{
		if (pbitmap->buffer)
		{
			free(pbitmap->buffer);
		}
		
		if (!(pbitmap->buffer = (UCHAR *)malloc(pbitmap->bitmapinfoheader.biSizeImage)))
		{
			_lclose(file_handle);
			return 0;
		}
		
		_lread(file_handle, pbitmap->buffer, pbitmap->bitmapinfoheader.biSizeImage);
		_lclose(file_handle);
		
		pbitmap->bytes_per_line=((pbitmap->bitmapinfoheader.biWidth*(pbitmap->bitmapinfoheader.biBitCount/8)+3)/4)*4;
		
		Flip_Bitmap(pbitmap->buffer, 
            pbitmap->bytes_per_line, 
            pbitmap->bitmapinfoheader.biHeight);
		
		return 1;
		
	}
	else
	{
		return 0;
	}	
}

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
	// this function releases all memory associated with "bitmap"
	if (bitmap->buffer)
	{
		// release memory
		free(bitmap->buffer);
		
		// reset pointer
		bitmap->buffer = NULL;
		
	} // end if
	
	// return success
	return(1);
	
} // end Unload_Bitmap_File



LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int Width, int Height, int Memflags, DWORD colorkey)
{
	LPDIRECTDRAWSURFACE7 lpdds;
	
	DDRAW_INIT_STRUCT(ddsd);
	
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	
	ddsd.dwWidth=Width;
	ddsd.dwHeight=Height;
	
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN | Memflags;
	
	if (FAILED(lpdd->CreateSurface(&ddsd, &lpdds, NULL)))
	{
		return NULL;
	}
	
	//if (colorkey>=0)
	//{
	DDCOLORKEY color_key;
	color_key.dwColorSpaceHighValue=colorkey;
	color_key.dwColorSpaceLowValue=colorkey;
	lpdds->SetColorKey(DDCKEY_SRCBLT,&color_key);
	lpdds->SetColorKey(DDCKEY_DESTBLT,&color_key);
	//}		
	
	return lpdds;
}

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list)
										 
{
	// this function creates a clipper from the sent clip list and attaches
	// it to the sent surface
	
	int index;                         // looping var
	LPDIRECTDRAWCLIPPER lpddclipper;   // pointer to the newly created dd clipper
	LPRGNDATA region_data;             // pointer to the region data that contains
	// the header and clip list
	
	// first create the direct draw clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
		return(NULL);
	
	// now create the clip list from the sent data
	
	// first allocate memory for region data
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+num_rects*sizeof(RECT));
	
	// now copy the rects into region data
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);
	
	// set up fields of header
	region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
	region_data->rdh.iType           = RDH_RECTANGLES;
	region_data->rdh.nCount          = num_rects;
	region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);
	
	region_data->rdh.rcBound.left    =  64000;
	region_data->rdh.rcBound.top     =  64000;
	region_data->rdh.rcBound.right   = -64000;
	region_data->rdh.rcBound.bottom  = -64000;
	
	// find bounds of all clipping regions
	for (index=0; index<num_rects; index++)
    {
		// test if the next rectangle unioned with the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left = clip_list[index].left;
		
		if (clip_list[index].right > region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right = clip_list[index].right;
		
		if (clip_list[index].top < region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top = clip_list[index].top;
		
		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom = clip_list[index].bottom;
		
    } // end for index
	
	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list
	
	if (FAILED(lpddclipper->SetClipList(region_data, 0)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if
	
	// now attach the clipper to the surface
	if (FAILED(lpdds->SetClipper(lpddclipper)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if
	
	// all is well, so release memory and send back the pointer to the new clipper
	free(region_data);
	return(lpddclipper);
	
} // end DDraw_Attach_Clipper

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure
	
	// clear out the structure and set the size field 
	DDRAW_INIT_STRUCT(ddbltfx);
	
	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color; 
	
	// ready to blt to surface
	lpdds->Blt(client,     // ptr to dest rectangle
		NULL,       // ptr to source surface, NA            
		NULL,       // ptr to source rectangle, NA
		DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait                   
		&ddbltfx);  // ptr to DDBLTFX structure
	
	// return success
	return(1);
} // end DDraw_Fill_Surface

int DDraw_Init(int width, int height, int bpp, int windowed)
{
	if (FAILED(DirectDrawCreateEx(NULL, (void**)&lpdd, IID_IDirectDraw7, NULL)))
	{
		return -1;
	}
	
	if (FAILED(lpdd->SetCooperativeLevel(main_window_handle,DDSCL_NORMAL)))
	{
		return -2;
	}
	
	DDRAW_INIT_STRUCT(ddsd);
	ddsd.dwFlags=DDSD_CAPS;
	ddsd.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	
	if (FAILED(lpdd->CreateSurface(&ddsd,&lpddsprimary,NULL)))
	{
		return -5;
	}
	
	//---------------------------
	//创建剪裁器    
	IDirectDrawClipper		*lpClipper			= NULL;    
    
    if (lpdd->CreateClipper(0, &lpClipper, NULL) != DD_OK) 
    {
        return -6;
    }
	
    //将剪裁器与窗口关联
    if (lpClipper->SetHWnd(0, main_window_handle) != DD_OK) 
    {
		return -7;
    }
    
    //将剪裁器与主页面关联
    if (lpddsprimary->SetClipper(lpClipper) != DD_OK) 
    {
        return -8;
    }

	if (lpClipper)
	{
		lpClipper->Release();
		lpClipper=NULL;
	}

	//---------------------------
	DDCOLORKEY color_key;
	color_key.dwColorSpaceHighValue=0;
	color_key.dwColorSpaceLowValue=0;
	lpddsprimary->SetColorKey(DDCKEY_SRCBLT,&color_key);	
	lpddsCavas=DDraw_Create_Surface(SCREEN_WIDTH,SCREEN_HEIGHT,DDSCAPS_VIDEOMEMORY);


	
	rectMemory.left=0;
	rectMemory.top=0;
	rectMemory.right=SCREEN_WIDTH;
	rectMemory.bottom=SCREEN_HEIGHT;

	DDraw_Attach_Clipper(lpddsCavas,1,&rectMemory);

	return 0;
}

int DDraw_Shutdown(void)
{


	if (lpddsCavas)
	{
		lpddsCavas->Release();
		lpddsCavas=NULL;
	}

	if (lpddsprimary)
	{
		lpddsprimary->Release();
		lpddsprimary=NULL;
	}

	if (lpdd)
	{
		lpdd->Release();
		lpdd=NULL;
	}

	

	return 0;
}

int Draw_Ground(LPDIRECTDRAWSURFACE7 ground)
{
	//if (FAILED(lpddsCavas->Blt(&rectMemory,bit_obj_Ground.lpddsurface,&rectMemory,DDBLT_WAIT|DDBLT_KEYSRC,NULL)))
	if (FAILED(lpddsCavas->Blt(&rectMemory,ground,&rectMemory,DDBLT_WAIT|DDBLT_KEYSRC,NULL)))
	{
		return -1;
	}
	
	return 0;
}

int Draw_Refresh()
{	
	GetClientRectFromScreen(main_window_handle,&rectWindows);
	if (FAILED(lpddsprimary->Blt(&rectWindows,lpddsCavas,&rectMemory,DDBLT_WAIT|DDBLT_KEYSRC,NULL)))
	{
		return -3;
	}
	
	return 0;
}



int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds)
{
	// this function draws the sent text on the sent surface 
	// using color index as the color in the palette
	
	HDC xdc; // the working dc
	
	// get the dc from surface
	if (FAILED(lpdds->GetDC(&xdc)))
		return(0);
	
	// set the colors for the text up
	SetTextColor(xdc,color);
	
	// set background mode to transparent so black isn't copied
	SetBkMode(xdc, TRANSPARENT);
	
	// draw the text a
	TextOut(xdc,x,y,text,strlen(text));
	
	// release the dc
	lpdds->ReleaseDC(xdc);
	
	// return success
	return 0;
} // end Draw_Text_GDI
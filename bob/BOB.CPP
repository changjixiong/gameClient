#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include "bob.h"


extern	DDSURFACEDESC2				ddsd;
extern	long						Mouse_X;
extern	long						Mouse_Y;
extern	bool						bLBUTTONDOWN;

////////////////////////////////////////////////////////////////////////////////

enum DIR
{
		SOUTH,
		SOUTHWEST,
		WEST,
		NORTHWEST,
		NORTH,
		NORTHEAST,
		EAST,
		SOUTHEAST,
};

DWORD					start_clock_count = 0;     // used for timing

int AdjustPos(POINT & pos)
{
	if (pos.x % WALK_LEN_X <= WALK_LEN_X>>1)
	{
		pos.x = (pos.x/WALK_LEN_X)*WALK_LEN_X;
	}
	else
	{
		pos.x = (pos.x/WALK_LEN_X + 1)*WALK_LEN_X;
	}
	
	if (pos.y % WALK_LEN_Y <= WALK_LEN_X>>1)
	{
		pos.y = (pos.y/WALK_LEN_Y)*WALK_LEN_Y;
	}
	else
	{
		pos.y = (pos.y/WALK_LEN_Y + 1)*WALK_LEN_Y;
	}
	
	return 0;
}

DWORD Get_Clock(void)
{
	// this function returns the current tick count
	
	// return time
	return(GetTickCount());
	
} // end Get_Clock

DWORD Start_Clock(void)
{
	// this function starts the clock, that is, saves the current
	// count, use in conjunction with Wait_Clock()
	
	return(start_clock_count = Get_Clock());
	
} // end Start_Clock

DWORD Wait_Clock(DWORD count)
{
	// this function is used to wait for a specific number of clicks
	// since the call to Start_Clock
	
	while((Get_Clock() - start_clock_count) < count);
	return(Get_Clock());
	
} // end Wait_Clock



int Load_BIT_OBJ(BIT_OBJ_PTR bit_obj, char * pBitMapFile,DWORD colorkey)
{	
	BITMAP_FILE bitmap;
	memset(&bitmap,0,sizeof(bitmap));
	
	Load_Bitmap_File(&bitmap,pBitMapFile);
	bit_obj->lpddsurface=DDraw_Create_Surface(SCREEN_WIDTH,SCREEN_HEIGHT,DDSCAPS_VIDEOMEMORY,colorkey);
	DDRAW_INIT_STRUCT(ddsd);
	
	if (FAILED(bit_obj->lpddsurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL)))
	{
		Unload_Bitmap_File(&bitmap);
		return -1;
	}
	
	DWORD * buffer=(DWORD *)ddsd.lpSurface;
	
	memset(buffer,0,SCREEN_HEIGHT*ddsd.lPitch);	
	
	for (int y=0;y<bitmap.bitmapinfoheader.biHeight;y++)
	{		
		for (int x=0;x<bitmap.bitmapinfoheader.biWidth;x++)
		{
			UCHAR r=bitmap.buffer[x*3+y*bitmap.bytes_per_line+2];
			UCHAR g=bitmap.buffer[x*3+y*bitmap.bytes_per_line+1];
			UCHAR b=bitmap.buffer[x*3+y*bitmap.bytes_per_line+0];		
			
			buffer[x]=_RGB32BIT(0,r,g,b);
		}
		buffer+=ddsd.lPitch>>2;
	}
	
	if (FAILED(bit_obj->lpddsurface->Unlock(NULL)))
	{
		Unload_Bitmap_File(&bitmap);
		return -2;
	}
	
	Unload_Bitmap_File(&bitmap);
	return 0;
}

int Unload_BIT_OBJ(BIT_OBJ_PTR bit_obj)
{
	if (bit_obj->lpddsurface)
	{
		bit_obj->lpddsurface->Release();
		bit_obj->lpddsurface=NULL;
	}
	
	return 0;
}

int Create_BOB(BOB_PTR bob,           // the bob to create
               int x, int y,          // initial posiiton
               int num_frames,        // number of frames
               int attr,              // attrs
               int mem_flags,         // memory flags in DD format
               DWORD color_key_value, // default color key
               int bpp)                // bits per pixel
			   
{
	//if create a BOB_ATTR_MULTI_ANIM bob, num_frames is useless , set it to -1 for default
	int index;           // looping var
	
	// set state and attributes of BOB
	bob->state          = BOB_STATE_ALIVE;
	bob->attr           = attr;
	bob->anim_state     = 0;
	bob->counter_1      = 0;     
	bob->counter_2      = 0;
	bob->max_count_1    = 0;
	bob->max_count_2    = 0;
	
	bob->curr_frame     = 0;
	bob->num_frames     = num_frames;
	bob->bpp            = bpp;
	bob->curr_animation = 0;
	bob->anim_counter   = 0;
	bob->anim_index     = 0;
	bob->anim_count_max = 0; 
	bob->x              = x;
	bob->y              = y;
	bob->xv             = 0;
	bob->yv             = 0;
	bob->dir			= SOUTH;
	bob->nAction		= 0;
	
	//bob->width  = width;
	//bob->height = height;
	
	// set all images to null
	for (index=0; index<MAX_BOB_FRAMES; index++)
		bob->images[index] = NULL;
	
	// set all animations to null
	for (index=0; index<MAX_BOB_ANIMATIONS; index++)
		bob->animations[index] = NULL;
	

	return 0;
}

int Load_Frame_BOB(BOB_PTR bob, char * pzFilepath, int imageIndex, int FromNo, int nCount)
{
	BITMAP_FILE bitmap;
	char sztemp[120];
	
	for (int i=FromNo;i<FromNo+nCount;i++,imageIndex++)
	{
		sprintf(sztemp,"%s%05d.bmp",pzFilepath,i);
		memset(&bitmap,0,sizeof(bitmap));
		Load_Bitmap_File(&bitmap,sztemp);	
		
		bob->images[imageIndex]=DDraw_Create_Surface(bitmap.bitmapinfoheader.biWidth,
														bitmap.bitmapinfoheader.biHeight,
														DDSCAPS_SYSTEMMEMORY,
														RGB(255,255,255));
		
		bob->imagesSize[imageIndex].x=bitmap.bitmapinfoheader.biWidth;
		bob->imagesSize[imageIndex].y=bitmap.bitmapinfoheader.biHeight;
		DDRAW_INIT_STRUCT(ddsd);

		int errNo;	 

		
		if (FAILED(errNo=(bob->images[imageIndex])->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT, NULL)))
		{
			Unload_Bitmap_File(&bitmap);
			return -1;		  		
		}
		
 		DWORD * buffer=(DWORD *)ddsd.lpSurface;		
 		
		for (int y=0;y<bitmap.bitmapinfoheader.biHeight;y++)
		{		
			for (int x=0;x<bitmap.bitmapinfoheader.biWidth;x++)
			{
				UCHAR r=bitmap.buffer[x*3+y*bitmap.bytes_per_line+2];
				UCHAR g=bitmap.buffer[x*3+y*bitmap.bytes_per_line+1];
				UCHAR b=bitmap.buffer[x*3+y*bitmap.bytes_per_line+0];		
				
				buffer[x]=_RGB32BIT(0,r,g,b);
			}
			buffer+=ddsd.lPitch>>2;
		}
		
		if (FAILED((bob->images[imageIndex])->Unlock(NULL)))
		{
			Unload_Bitmap_File(&bitmap);
			return -2;
		}
		
		Unload_Bitmap_File(&bitmap);	
	}
	
	bob->attr |= BOB_ATTR_LOADED;
	
	return 0;
}

int Load_Animation_BOB(BOB_PTR bob, int anim_index, int num_frames, int *sequence)
{
	// this function load an animation sequence for a bob
	// the sequence consists of frame indices, the function
	// will append a -1 to the end of the list so the display
	// software knows when to restart the animation sequence
	
	// is this bob valid
	if (!bob)
		return(0);
	
	// allocate memory for bob animation
	if (!(bob->animations[anim_index] = (int *)malloc((num_frames+1)*sizeof(int))))
		return(0);
	
	// load data into 
	for (int index=0; index<num_frames; index++)
		bob->animations[anim_index][index] = sequence[index];
	
	// set the end of the list to a -1
	bob->animations[anim_index][index] = -1;
	
	// return success
	return(1);
	
} // end Load_Animation_BOB

int Destroy_BOB(BOB_PTR bob)
{
	// destroy the BOB, tests if this is a real bob or a clone
	// if real then release all the memory, otherwise, just resets
	// the pointers to null
	
	int index; // looping var
	
	// is this bob valid
	if (!bob)
		return(0);
	
	// test if this is a clone
	if (bob->attr && BOB_ATTR_CLONE)
    {
		// null link all surfaces
		for (index=0; index<MAX_BOB_FRAMES; index++)
			if (bob->images[index])
				bob->images[index]=NULL;
			
			// release memory for animation sequences 
			for (index=0; index<MAX_BOB_ANIMATIONS; index++)
				if (bob->animations[index])
					bob->animations[index]=NULL;
				
    } // end if
	else
    {
		// destroy each bitmap surface
		for (index=0; index<MAX_BOB_FRAMES; index++)
			if (bob->images[index])
				(bob->images[index])->Release();
			
			// release memory for animation sequences 
			for (index=0; index<MAX_BOB_ANIMATIONS; index++)
				if (bob->animations[index])
					free(bob->animations[index]);
				
    } // end else not clone
	
	// return success
	return 0;
	
} // end Destroy_BOB

int Animate_BOB(BOB_PTR bob)
{
	// this function animates a bob, basically it takes a look at
	// the attributes of the bob and determines if the bob is 
	// a single frame, multiframe, or multi animation, updates
	// the counters and frames appropriately
	
	// is this a valid bob
	if (!bob)
		return(0);
	
	// test the level of animation
	if (bob->attr & BOB_ATTR_SINGLE_FRAME)
    {
		// current frame always = 0
		bob->curr_frame = 0;
		return(1);
    } // end if
	else if (bob->attr & BOB_ATTR_MULTI_FRAME)
	{
		// update the counter and test if its time to increment frame
		if (bob->anim_counter++ >= bob->anim_count_max)
		{
			// reset counter
			bob->anim_counter = 0;
			
			// move to next frame
			if (++bob->curr_frame >= bob->num_frames)
				bob->curr_frame = 0;
			
		} // end if
		
	} // end elseif
	else if (bob->attr & BOB_ATTR_MULTI_ANIM)
	{
		// this is the most complex of the animations it must look up the
		// next frame in the animation sequence
		
		// first test if its time to animate
		if (bob->anim_counter++ >= bob->anim_count_max)
		{
			// reset counter
			bob->anim_counter = 0;
			
			// increment the animation frame index
			//bob->anim_index++;
			
			// extract the next frame from animation list 
			//bob->curr_frame = bob->animations[bob->curr_animation][bob->anim_index];
			bob->curr_frame = bob->animations[bob->curr_animation][bob->anim_index++];
			
			// is this and end sequence flag -1
			if (bob->curr_frame == -1)
			{
				// test if this is a single shot animation
				if (bob->attr & BOB_ATTR_ANIM_ONE_SHOT)
				{
					// set animation state message to done
					bob->anim_state = BOB_STATE_ANIM_DONE;
					
					// reset frame back one
					bob->anim_index--;
					
					// extract animation frame
					bob->curr_frame = bob->animations[bob->curr_animation][bob->anim_index];    
					
				} // end if
				else
				{
					// reset animation index
					bob->anim_index = 0;
					
					// extract first animation frame
					bob->curr_frame = bob->animations[bob->curr_animation][bob->anim_index];
				} // end else
				
			}  // end if

			
			
		} // end if
		
	} // end elseif
	
	
	// return success
	return(1);
			
} // end Amimate_BOB

int Drive_BOB(BOB_PTR bob,int nAction, int DestX, int DestY)
{
	bob->NextDest.x=DestX;
	bob->NextDest.y=DestY;
	bob->nAction=nAction;
	AdjustPos(bob->NextDest);

	return 0;
}

int Move_BOB(BOB_PTR bob)
{
	if (bob->anim_counter < bob->anim_count_max)
	{		
		return 0;
	}

	switch (bob->nAction)
	{
	case Action_STOP:

		if (bob->NextDest.x >= 0 && bob->NextDest.y >= 0)
		{
			bob->Dest.x=bob->NextDest.x;
 			bob->Dest.y=bob->NextDest.y;
		}		

		if ( bob->Dest.x != bob->x || bob->Dest.y != bob->y )
		{
// 			bob->Dest.x=Mouse_X;
// 			bob->Dest.y=Mouse_Y;
// 			AdjustPos(bob->Dest);
// 
// 			Mouse_X=-1;
// 			Mouse_Y=-1;
			
			bob->nAction=Action_WALK;

			Set_BOB_Dir(bob);

			Set_Animation_BOB(bob,ANIMATION_OFFSET_WLAK + bob->dir);
 		}
		break;
	case Action_WALK:
// 		if (Mouse_X>=0 && Mouse_Y>=0)
// 		{
// 			bob->NextDest.x=Mouse_X;
// 			bob->NextDest.y=Mouse_Y;
// 			AdjustPos(bob->NextDest);
// 
// 			Mouse_X=-1;
// 			Mouse_Y=-1;
// 		}

		if (bob->animations[bob->curr_animation][bob->anim_index+1] == -1)
		{
			if (bob->NextDest.x != -1 
				&& bob->NextDest.y != -1)
			{
				bob->Dest.x = bob->NextDest.x;
				bob->Dest.y = bob->NextDest.y;
				
// 				bob->NextDest.x=-1;
// 				bob->NextDest.y=-1;
			}		
		}

		if (bob->x == bob->Dest.x && bob->y == bob->Dest.y)
		{
			bob->nAction=Action_STOP;
			Set_Animation_BOB(bob,ANIMATION_OFFSET_STOP + bob->dir);
		}
		else
		{
			enum DIR dir=bob->dir;
			Set_BOB_Dir(bob);
			if (bob->dir != dir)
			{
				Set_Animation_BOB(bob,ANIMATION_OFFSET_WLAK + bob->dir);
			}	
		}
		
		break;
	default:
		break;
	}
	
	if (Action_WALK==bob->nAction)
	{	
		if (bob->dir == EAST || bob->dir == SOUTHEAST || bob->dir == NORTHEAST)
		{
			bob->x +=bob->xv;
			if (bob->dir == SOUTHEAST)
			{
				bob->y += bob->yv;
			}
			else if (bob->dir == NORTHEAST)
			{
				bob->y -= bob->yv;
			}
		}
		else if (bob->dir == WEST || bob->dir == SOUTHWEST || bob->dir == NORTHWEST)
		{
			bob->x -= bob->xv;
			if (bob->dir == SOUTHWEST)
			{
				bob->y += bob->yv;
			}
			else if (bob->dir == NORTHWEST)
			{
				bob->y -= bob->yv;
			}
		}
		else if (bob->dir == SOUTH)
		{
			bob->y += bob->yv;
		}
		else if (bob->dir == NORTH)
		{
			bob->y -= bob->yv;
		}
	}

	return 0;
}

int Draw_BOB(BOB_PTR bob,               // bob to draw
             LPDIRECTDRAWSURFACE7 dest) // surface to draw the bob on
{
	// draw a bob at the x,y defined in the BOB
	// on the destination surface defined in dest
	
	RECT dest_rect,   // the destination rectangle
		source_rect; // the source rectangle                             
	
	// is this a valid bob
	if (!bob)
		return(0);
	
	// is bob visible
	if (!(bob->attr & BOB_ATTR_VISIBLE))
		return(0);
	
	// fill in the destination rect
	dest_rect.left   = bob->x;
	dest_rect.top    = bob->y;
	dest_rect.right  = bob->x+bob->imagesSize[bob->curr_frame].x;
	dest_rect.bottom = bob->y+bob->imagesSize[bob->curr_frame].y;
	
	// fill in the source rect
	source_rect.left    = 0;
	source_rect.top     = 0;
	source_rect.right   = bob->imagesSize[bob->curr_frame].x;
	source_rect.bottom  = bob->imagesSize[bob->curr_frame].y;
	
	// blt to destination surface
	if (FAILED(dest->Blt(&dest_rect, bob->images[bob->curr_frame],
		&source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
		NULL)))
		return(-1);
	
	// return success
	
	return(0);
	
	
} // end Draw_BOB


void Set_BOB_Dir(BOB_PTR bob)
{

	if ( bob->x > bob->Dest.x )
	{
		if (bob->y > bob->Dest.y)
		{
			bob->dir = NORTHWEST;
		}
		else if (bob->y == bob->Dest.y)
		{
			bob->dir = WEST;
		}
		else if (bob->y < bob->Dest.y)
		{
			bob->dir = SOUTHWEST;
		}
		
	}
	else if (bob->x == bob->Dest.x)
	{
		if (bob->y > bob->Dest.y)
		{
			bob->dir = NORTH;
		}
		else if (bob->y < bob->Dest.y)
		{
			bob->dir = SOUTH;
		}
	}
	else if (bob->x < bob->Dest.x)
	{
		if (bob->y > bob->Dest.y)
		{
			bob->dir = NORTHEAST;
		}
		else if (bob->y == bob->Dest.y)
		{
			bob->dir = EAST;
		}
		else if (bob->y < bob->Dest.y)
		{
			bob->dir = SOUTHEAST;
		}
	}
	
}



int Set_Animation_BOB(BOB_PTR bob, int anim_index)
{
	// this function sets the animation to play
	
	// is this a valid bob
	if (!bob)
		return(0);
	
	// set the animation index
	bob->curr_animation = anim_index;
	
	// reset animation 
	bob->anim_index = 0;
	
	// return success
	return(1);
	
} // end Set_Animation_BOB

int Set_Anim_Speed_BOB(BOB_PTR bob,int speed)
{
	// this function simply sets the animation speed of a bob
    
	// is this a valid bob
	if (!bob)
		return(0);
	
	// set speed
	bob->anim_count_max = speed;
	
	// return success
	return(1);
	
} // end Set_Anim_Speed

int Set_Vel_BOB(BOB_PTR bob,int xv, int yv)
{
	// this function sets the velocity of a bob
	
	// is this a valid bob
	if (!bob)
		return(0);
	
	// set velocity
	bob->xv = xv;
	bob->yv = yv;
	
	// return success
	return(1);
} // end Set_Vel_BOB

int Set_Pos_BOB(BOB_PTR bob, int x, int y)
{
	// this functions sets the postion of a bob
	
	// is this a valid bob
	if (!bob)
		return(0);
	
	// set positin
	bob->x = x;
	bob->y = y;

	bob->Dest.x = -1;
	bob->Dest.y = -1;
	bob->NextDest.x = -1;
	bob->NextDest.y = -1;
	// return success
	return(1);
} // end Set_Pos_BOB

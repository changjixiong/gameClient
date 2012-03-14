#ifndef BOB_HEAD
#define BOB_HEAD
#include "../DDrawLib/DDrawLib.h"

#define BOB_STATE_DEAD         0    // this is a dead bob
#define BOB_STATE_ALIVE        1    // this is a live bob
#define BOB_STATE_DYING        2    // this bob is dying
#define BOB_STATE_ANIM_DONE    1    // done animation state
#define MAX_BOB_FRAMES         1024   // maximum number of bob frames
#define MAX_BOB_ANIMATIONS     16   // maximum number of animation sequeces

#define BOB_ATTR_SINGLE_FRAME   1   // bob has single frame
#define BOB_ATTR_MULTI_FRAME    2   // bob has multiple frames
#define BOB_ATTR_MULTI_ANIM     4   // bob has multiple animations
#define BOB_ATTR_ANIM_ONE_SHOT  8   // bob will perform the animation once
#define BOB_ATTR_VISIBLE        16  // bob is visible
#define BOB_ATTR_BOUNCE         32  // bob bounces off edges
#define BOB_ATTR_WRAPAROUND     64  // bob wraps around edges
#define BOB_ATTR_LOADED         128 // the bob has been loaded
#define BOB_ATTR_CLONE          256 // the bob is a clone

#define Action_STOP				0
#define Action_WALK				1
#define Action_RUN				2
#define	WALK_LEN_X				40
#define WALK_LEN_Y				20
#define	ANIMATION_OFFSET_STOP	0
#define	ANIMATION_OFFSET_WLAK	8

#define KEYDOWN(vk_code) (GetAsyncKeyState(vk_code) & 0x8000 ? 1 : 0 )
#define KEYUP(vk_code) (GetAsyncKeyState(vk_code) & 0x8000 ? 0 : 1 )

typedef struct  BIT_OBJ_TYP 
{
	LPDIRECTDRAWSURFACE7 lpddsurface;
	int x,y;
}BIT_OBJ, * BIT_OBJ_PTR;

typedef struct BOB_TYP
{
	int state;          // the state of the object (general)
	int anim_state;     // an animation state variable, up to you
	int attr;           // attributes pertaining to the object (general)
	int x,y;            // position bitmap will be displayed at
	int xv,yv;          // velocity of object
	int width, height;  // the width and height of the bob
	int width_fill;     // internal, used to force 8*x wide surfaces
	int bpp;            // bits per pixel
	int counter_1;      // general counters
	int counter_2;
	int max_count_1;    // general threshold values;
	int max_count_2;
	// 	int varsI[16];      // stack of 16 integers
	// 	float varsF[16];    // stack of 16 floats
	int curr_frame;     // current animation frame
	int num_frames;     // total number of animation frames
	int curr_animation; // index of current animation
	int anim_counter;   // used to time animation transitions
	int anim_move_counter;
	int anim_index;     // animation element index
	int anim_count_max; // number of cycles before animation
	int *animations[MAX_BOB_ANIMATIONS]; // animation sequences
	
	POINT Dest;
	POINT NextDest;
	//int Dest_x,Dest_y;
	enum DIR dir;
	int nAction;
	
	LPDIRECTDRAWSURFACE7 images[MAX_BOB_FRAMES]; // the bitmap images DD surfaces
	POINT	imagesSize[MAX_BOB_FRAMES];
	
} BOB, *BOB_PTR;

DWORD Get_Clock(void);
DWORD Start_Clock(void);
DWORD Wait_Clock(DWORD count);


int Load_BIT_OBJ(BIT_OBJ_PTR bit_obj, char * pBitMapFile,DWORD colorkey = 0);
int Unload_BIT_OBJ(BIT_OBJ_PTR bit_obj);

int Create_BOB(BOB_PTR bob,int x, int y,
			   int num_frames,int attr,
               int mem_flags=0, DWORD color_key_value=0, int bpp=8);  

int Load_Frame_BOB(BOB_PTR bob, char * pzFilepath, int imageIndex, int FromNo, int nCount);
int Load_Animation_BOB(BOB_PTR bob, int anim_index, int num_frames, int *sequence);
int Destroy_BOB(BOB_PTR bob);
int Animate_BOB(BOB_PTR bob);
int Move_BOB(BOB_PTR bob);
int Draw_BOB(BOB_PTR bob, LPDIRECTDRAWSURFACE7 dest);


void Set_BOB_Dir(BOB_PTR bob);
int Set_Animation_BOB(BOB_PTR bob, int anim_index);
int Set_Anim_Speed_BOB(BOB_PTR bob,int speed);
int Set_Vel_BOB(BOB_PTR bob,int xv, int yv);
int Set_Pos_BOB(BOB_PTR bob, int x, int y);


#endif
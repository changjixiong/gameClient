#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "GameLib.h"
#include <stdio.h>
#include <string>

#include "../SocketLib/SocketLib.h"
#include "../ThreadLib/ThreadLib.h"

using namespace std;

long	Mouse_X;
long	Mouse_Y;

////////////////////////////////////////////////////////////////////////////////

extern LPDIRECTDRAW7				lpdd;
extern LPDIRECTDRAWSURFACE7			lpddsprimary;
extern LPDIRECTDRAWSURFACE7			lpddsCavas;

int man_anims[8][5] = { {0,1,2,3,4},
						{5,6,7,8,9},
						{10,11,12,13,14},
						{15,16,17,18,19},
						{20,21,22,23,24},
						{25,26,27,28,29},
						{30,31,32,33,34},
						{35,36,37,38,39},};

int man_walk[8][10] = {{40,41,42,43,44,45,46,47,48,49},
						{50,51,52,53,54,55,56,57,58,59},
						{60,61,62,63,64,65,66,67,68,69},
						{70,71,72,73,74,75,76,77,78,79},
						{80,81,82,83,84,85,86,87,88,89},
						{90,91,92,93,94,95,96,97,98,99},
						{100,101,102,103,104,105,106,107,108,109},
						{110,111,112,113,114,115,116,117,118,119},};



BOB						man;     // the player skelaton
BOB						animal;
BIT_OBJ					bit_obj_Ground;

SocketLib::DataSocket datasock;

const int		GAME_STATE_START		= 1;
const int		GAME_STATE_CONNECTED	= 2;
const int		GAME_STATE_LOGINING		= 3;	
const int		GAME_STATE_LOGINED		= 4;

int				Game_State		= 0;
string			ServerDataPool	= "";
string			Error_MSG		= "";
string			DateForSend		= "";

const string& GetOneMsg(string & strDest, string &strSrc)
{
	strDest = "";

	string::size_type posEnd;

	posEnd = strSrc.find("]");

	if (posEnd != string::npos)
	{
		strDest = strSrc.substr(0, posEnd+1);
		strSrc = strSrc.substr(posEnd+1);
	}

	return strDest;
}

int GetData(SocketLib::DataSocket &dsock, string & strDest)
{
	char szServerCMD[128];
	int nDataLen = 0;
	memset(szServerCMD,0,sizeof(szServerCMD));

	try
	{
		nDataLen = datasock.Receive(szServerCMD,128);
		if ( nDataLen > 0 )
		{		
			strDest+= szServerCMD;				
		}
	}
	catch (SocketLib::Exception & e)
	{
		Error_MSG = e.PrintError();		
	}
	catch (...)
	{
		
	}

	return nDataLen;
}

int SendData(SocketLib::DataSocket &dsock, string & strSrc)
{
	int nDataLen=0;

	if (strSrc.length()==0)
	{
		return 0;
	}

	try
	{
		nDataLen = dsock.Send(strSrc.c_str(), strSrc.length());

		if ( nDataLen > 0 )
		{
			strSrc = strSrc.substr(nDataLen);
		}

	}
	catch (SocketLib::Exception & e)
	{
		Error_MSG = e.PrintError();		
	}
	catch (...)
	{
		
	}
	
	return nDataLen;
}

int Login(const string & strName)
{
	string	ServerMsg = "";	

	datasock.Connect(SocketLib::GetIPAddress("127.0.0.1"),5099);
	datasock.SetBlocking(false);
	
	ThreadLib::YieldThread(1000);
	GetData(datasock, ServerDataPool);
	GetOneMsg(ServerMsg, ServerDataPool);
	
	if (ServerMsg!="[user name]")
	{
		return -1;		
	}	

	DateForSend = strName+"\r\n";		
	SendData(datasock, DateForSend);

	ThreadLib::YieldThread(1000);
	GetData(datasock, ServerDataPool);
	GetOneMsg(ServerMsg, ServerDataPool);
	
	if (ServerMsg!="["+strName+" entered]")
	{
		return -1;		
	}

	return 0;

}

int Game_Init(void * parms, int num_parms)
{

// 	if (Login("chang")!=0)
// 	{
// 		MessageBox(main_window_handle,"Login failed","",MB_OK);
// 		return -1;
// 	}


	Mouse_X=-1;
	Mouse_Y=-1;

	DDraw_Init(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP);
	
	Load_BIT_OBJ(&bit_obj_Ground,"./pic/ground/ground.bmp",RGB(255,255,255));
	
	if (Create_BOB(&man,0,0,-1,
		BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_ANIM,DDSCAPS_SYSTEMMEMORY,RGB(255,255,255),SCREEN_BPP)!=0)
		return(0);

	if (Create_BOB(&animal,0,0,-1,
		BOB_ATTR_VISIBLE | BOB_ATTR_MULTI_ANIM,DDSCAPS_SYSTEMMEMORY,RGB(255,255,255),SCREEN_BPP)!=0)
		return(0);
	
	for (int i=0;i<8;i++)
	{
		Load_Frame_BOB(&man, "./pic/man/c", i*5, i*5, 5);
		Load_Animation_BOB(&man,i,5,man_anims[i]);

		Load_Frame_BOB(&animal, "./pic/animal/c", i*5, i*5, 5);
		Load_Animation_BOB(&animal,i,5,man_anims[i]);
	}
	
	for (i=4;i<12;i++)
	{
		Load_Frame_BOB(&man, "./pic/man/c", i*10, i*10, 10);
		Load_Animation_BOB(&man,8 + i-4 ,10,man_walk[i-4]);
	}
	
	Set_Animation_BOB(&man, 0);
	Set_Anim_Speed_BOB(&man, 4);
	//Set_Anim_Speed_BOB(&man, 0);
	Set_Vel_BOB(&man, 4,2);
	Set_Pos_BOB(&man, 300, 100);

	Set_Animation_BOB(&animal, 0);
	Set_Anim_Speed_BOB(&animal, 4);
	Set_Vel_BOB(&animal, 4,2);
	Set_Pos_BOB(&animal, 400, 200);

	man.Dest.x=man.x;
	man.Dest.y=man.y;
	return 0;
}

int Game_Shutdown(void * parms, int num_parms)
{
	
	Unload_BIT_OBJ(&bit_obj_Ground);
	Destroy_BOB(&man);
	
	DDraw_Shutdown();	
	
	return 0;
}

int Game_Main(void * parms, int num_parms)
{

	if (KEYDOWN(VK_ESCAPE))
	{
		PostMessage(main_window_handle,WM_CLOSE,0,0);
	}
	
	Start_Clock();	

	char szMouse_x[32];
	char szMouse_y[32];
	
	string	ServerMsg = "";

	if (Mouse_X>=0 && Mouse_Y>=0)
	{
		memset(szMouse_x,0,sizeof(szMouse_x));
		memset(szMouse_y,0,sizeof(szMouse_y));

		ltoa(Mouse_X, szMouse_x, 10);
		ltoa(Mouse_Y, szMouse_y, 10);
		
		DateForSend = szMouse_x+string("|")+szMouse_y+string("\r\n");

		Mouse_X = Mouse_Y = -1;
		ServerMsg = string("[")+szMouse_x+string("|")+szMouse_y+string("]");
	}			
// 
//  	SendData(datasock, DateForSend);
//  	GetData(datasock, ServerDataPool);
//  	GetOneMsg(ServerMsg, ServerDataPool);

	

	if (ServerMsg.length()>0)
	{
		string posx=ServerMsg.substr(1,ServerMsg.find('|')-1);
		string posy=ServerMsg.substr(ServerMsg.find('|')+1,ServerMsg.find(']') - ServerMsg.find('|'));
		Drive_BOB(&man,Action_WALK, atoi(posx.c_str()),atoi(posy.c_str()));
	}

	
	DDraw_Fill_Surface(lpddsCavas,0);
	
	Draw_Ground(bit_obj_Ground.lpddsurface);

	Move_BOB(&man);
	//Move_BOB(&animal);
	
	Animate_BOB(&man);	
	//Animate_BOB(&animal);

	Draw_BOB(&man, lpddsCavas);
	//Draw_BOB(&animal, lpddsCavas);
	
	Game_Debug_Textout();
	
	Draw_Refresh();	
	
	Wait_Clock(16);
	
	return 0;
}

int Game_Debug_Textout()
{
	char szTextOut[128];	

	Draw_Text_GDI("USE ARROW KEYS TO MOVE, <ESC> TO EXIT.",0,0,RGB(0,0xE6,0xBF),lpddsCavas);
	
	sprintf(szTextOut,"next frame NO %d",man.anim_index);	
	Draw_Text_GDI(szTextOut,0,20,RGB(0,0,0xff),lpddsCavas);
	
	sprintf(szTextOut,
		"[pos x:%d, y:%d] [Dest x:%d, y:%d] [nextDest x:%d, y:%d]",
		man.x,man.y,man.Dest.x, man.Dest.y,man.NextDest.x, man.NextDest.y);
	Draw_Text_GDI(szTextOut,0,40,RGB(0,0,0xff),lpddsCavas);

	//Draw_Text_GDI((char *)Server_CMD.c_str(),0,60,RGB(0,0,0xff),lpddsCavas);
	Draw_Text_GDI((char *)Error_MSG.c_str(),0,80,RGB(0,0,0xff),lpddsCavas);

	return 0;
}

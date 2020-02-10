#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <stdio.h>

#include "ddutil.h"

#include <dsound.h>
#include "dsutil.h"


#define _GetKeyState( vkey ) HIBYTE(GetAsyncKeyState( vkey ))
#define _GetKeyPush( vkey )  LOBYTE(GetAsyncKeyState( vkey ))

HWND MainHwnd;

LPDIRECTDRAW         DirectOBJ;
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWSURFACE  Player;
LPDIRECTDRAWSURFACE  BackGround;
LPDIRECTDRAWSURFACE  Cacao;
LPDIRECTDRAWSURFACE  Chocolate;
LPDIRECTDRAWSURFACE  Blueberry;

LPDIRECTDRAWSURFACE  success;	//성공화면
LPDIRECTDRAWSURFACE  fail;	//블루베리 먹었을 시 실패 화면
LPDIRECTDRAWSURFACE  Violet;	//블루베리 먹었을 시 실패 화면

LPDIRECTDRAWCLIPPER	ClipScreen;

int gFullScreen=0, Click=0;
int gWidth=1200, gHeight=500;
int MouseX=100, MouseY=gHeight/2;

int direction = 2;	//위아래양옆 방향
int eat_cacao = 0;	//카카오 몇 개 먹었는지
int eat_berry = 0;	//블루베리 몇 개 먹었는지
int destroy_cacao[20] = { 0, };	//0이면 카카오 보이고 아니면 카카오 없어짐
int destroy_berry[10] = { 0, };	//0이면 블루베리 보이고 아니면 블루베리 없어짐

////////////////////

LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

HSNDOBJ Sound[10];


BOOL _InitDirectSound( void )
{
    if ( DirectSoundCreate(NULL,&SoundOBJ,NULL) == DS_OK )
    {
        if (SoundOBJ->SetCooperativeLevel(MainHwnd,DSSCL_PRIORITY)!=DS_OK) return FALSE;

        memset(&DSB_desc,0,sizeof(DSBUFFERDESC));
        DSB_desc.dwSize = sizeof(DSBUFFERDESC);
        DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;

        if (SoundOBJ->CreateSoundBuffer(&DSB_desc,&SoundDSB,NULL)!=DS_OK) return FALSE;
        SoundDSB -> SetVolume(DSBVOLUME_MAX); // DSBVOLUME_MIN
        SoundDSB -> SetPan(DSBPAN_RIGHT);
        return TRUE;
    }
    return FALSE;
}

void _Play( int num )
{
    SndObjPlay( Sound[num], NULL );
}

////////////////////////


BOOL Fail( HWND hwnd )
{
    ShowWindow( hwnd, SW_HIDE );
    MessageBox( hwnd, "DIRECT X 초기화에 실패했습니다.", "게임 디자인", MB_OK );
    DestroyWindow( hwnd );
    return FALSE;
}

void _ReleaseAll( void )
{
    if ( DirectOBJ != NULL )
    {
        if ( RealScreen != NULL )
        {
            RealScreen->Release();
            RealScreen = NULL;
        }
        if ( Player != NULL )
        {
            Player->Release();
            Player = NULL;
        }
        if ( BackGround != NULL )
        {
            BackGround->Release();
            BackGround = NULL;
        }
        DirectOBJ->Release();
        DirectOBJ = NULL;
    }
}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int Step=5;


    switch ( message )
    {
/*
        case    WM_MOUSEMOVE    :   MouseX = LOWORD(lParam);
                                    MouseY = HIWORD(lParam);
                                    break;
*/
		case	WM_LBUTTONDOWN	: 	Click=1;
									//_Play( 3 );
									break;
        case	WM_KEYDOWN:            
            switch (wParam)
            {
                case VK_ESCAPE:
                case VK_F12: 
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0;            

				case VK_LEFT:
					direction = 1;
					Click = 1;
					MouseX -= Step;
					return 0;

				case VK_RIGHT:
					direction = 2;
					Click = 1;
					MouseX += Step;
					return 0;

				case VK_UP:
					direction = 3;
					Click = 1;
					MouseY -= Step;
					return 0;

				case VK_DOWN:
					direction = 4;
					Click = 1;
					MouseY += Step;
					return 0;

					/*case VK_SPACE:
					//Click=1;
					//_Play( 3 );
					break;*/
			}
            break;

        case    WM_DESTROY      :  _ReleaseAll();
                                    PostQuitMessage( 0 );
                                    break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}

BOOL _GameMode( HINSTANCE hInstance, int nCmdShow, int x, int y, int bpp )
{
    HRESULT result;
    WNDCLASS wc;
    DDSURFACEDESC ddsd;
    DDSCAPS ddscaps;
    LPDIRECTDRAW pdd;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TEST";
    RegisterClass( &wc );


	if(gFullScreen){
		if((MainHwnd=CreateWindowEx (0, "TEST", NULL, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
				GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL ))==NULL)
			ExitProcess(1);
	}
	else{
		if((MainHwnd=CreateWindow("TEST", "Slave", WS_OVERLAPPEDWINDOW, 0, 0, x, 
									y, NULL, NULL, hInstance, NULL))==NULL)
			ExitProcess(1);
		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER);
	}

    SetFocus( MainHwnd );
    ShowWindow( MainHwnd, nCmdShow );
    UpdateWindow( MainHwnd );
//    ShowCursor( FALSE );

    result = DirectDrawCreate( NULL, &pdd, NULL );
    if ( result != DD_OK ) return Fail( MainHwnd );

    result = pdd->QueryInterface(IID_IDirectDraw, (LPVOID *) &DirectOBJ);
    if ( result != DD_OK ) return Fail( MainHwnd );


	// 윈도우 핸들의 협력 단계를 설정한다.
	if(gFullScreen){
	    result = DirectOBJ->SetCooperativeLevel( MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = DirectOBJ->SetDisplayMode( x, y, bpp);
		if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

	    result = DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL );
	   if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddscaps, 0, sizeof(ddscaps) );
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		result = RealScreen -> GetAttachedSurface( &ddscaps, &BackScreen );
		if ( result != DD_OK ) return Fail( MainHwnd );
	}
	else{
	    result = DirectOBJ->SetCooperativeLevel( MainHwnd, DDSCL_NORMAL );
		if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddsd, 0, sizeof(ddsd) );
	    ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;

		result = DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL );
	    if(result != DD_OK) return Fail(MainHwnd);

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x; 
		ddsd.dwHeight = y;
		result = DirectOBJ->CreateSurface( &ddsd, &BackScreen, NULL );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = DirectOBJ->CreateClipper( 0, &ClipScreen, NULL);
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = ClipScreen->SetHWnd( 0, MainHwnd );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = RealScreen->SetClipper( ClipScreen );
		if ( result != DD_OK ) return Fail( MainHwnd );

		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER | SWP_NOACTIVATE); 
	}


    return TRUE;
}


extern void CommInit(int argc, char **argv);
extern void CommSend(char *sending);
extern void CommRecv(char *recvData);



void _GameProcDraw(char *recvData)
{
    RECT BackRect = { 0, 0, 1200, 500 };
	RECT DispRect = { 0, 0, gWidth, gHeight };
	RECT SpriteRect, dstRect, WinRect;
	int type;
	static int Rcount=0;

	if(Rcount==0){
		BackScreen -> BltFast(0, 0, BackGround, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY); 
		//BackScreen -> BltFast(640, 0, BackGround, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY); 
	}

	//받을 때마다 dsRect, Sprite 하나씩 새로 받아서 줘서 하나로 해도 돼?-혜성피셜
	sscanf(recvData, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &type, &dstRect.left,&dstRect.top,&dstRect.right,&dstRect.bottom,&SpriteRect.left, &SpriteRect.top, &SpriteRect.right, &SpriteRect.bottom);
	//1==player, 2=berry, 3=cacao, 4=success, 5=choco, 6=fail, 7=violet
	if(type==1)
		BackScreen -> BltFast( dstRect.left, dstRect.top, Player, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY );
	else if(type==2)
		BackScreen->Blt(&dstRect, Blueberry, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	else if(type==3)
		BackScreen->Blt(&dstRect, Cacao, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	else if(type==4)
		BackScreen->BltFast(dstRect.left, dstRect.top, success, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		//BackScreen->Blt(&dstRect, success, &SpriteRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);	//호설
	else if(type==5)
		BackScreen->BltFast(dstRect.left, dstRect.top, Chocolate, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	else if(type==6)
		BackScreen->BltFast(dstRect.left, dstRect.top, fail, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	else if(type==7)
		BackScreen->BltFast(dstRect.left, dstRect.top, Violet, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

	Rcount++;
	if(Rcount==33){	// player 1 + cacao 20 + blueberry 10 + success/fail 1 + chocolate/violet 1
		GetWindowRect(MainHwnd, &WinRect);
		RealScreen->Blt(&WinRect, BackScreen, &DispRect, DDBLT_WAIT, NULL ); 
		Rcount=0;
	}
}



int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG msg;

    if ( !_GameMode(hInstance, nCmdShow, gWidth, gHeight, 32) ) return FALSE;

	Player = DDLoadBitmap(DirectOBJ, "_Boy.BMP", 0, 0);	//주인공 찰리
	BackGround = DDLoadBitmap(DirectOBJ, "_Background_Chocolate-Factory.BMP", 0, 0);	//초콜릿 공장 배경
	Cacao = DDLoadBitmap(DirectOBJ, "_Cacao.BMP", 0, 0);	//카카오
	Chocolate = DDLoadBitmap(DirectOBJ, "_Chocolate.BMP", 0, 0);	//성공시 초콜릿
	Blueberry = DDLoadBitmap(DirectOBJ, "_Blueberry.BMP", 0, 0);	//장애물	, _Blueberry.BMP
	success = DDLoadBitmap(DirectOBJ, "_Success.BMP", 0, 0);	//SUCCESS! 글씨
	fail = DDLoadBitmap(DirectOBJ, "_Fail2.BMP", 0, 0);	//FAIL.. 글씨
	Violet = DDLoadBitmap(DirectOBJ, "_Violet.BMP", 0, 0);	//실패시 블루베리가 된 바이올렛, _Violet_Bluberry

	DDSetColorKey(Player, RGB(0, 0, 0));
	DDSetColorKey(Cacao, RGB(0, 0, 0));
	DDSetColorKey(Chocolate, RGB(0, 0, 0));
	DDSetColorKey(Blueberry, RGB(0, 0, 0));
	DDSetColorKey(success, RGB(0, 0, 0));
	DDSetColorKey(fail, RGB(0, 0, 0));
	DDSetColorKey(Violet, RGB(0, 0, 0));



	CommInit(NULL, NULL);

///////////////////

    if ( _InitDirectSound() )
    {
		Sound[0] = SndObjCreate(SoundOBJ, "_Willy-Wonka.WAV", 1);	//배경음악
		Sound[1] = SndObjCreate(SoundOBJ, "_Get_Cacao.WAV", 2);	//돌에 부딪히면->카카오 먹으면
		Sound[2] = SndObjCreate(SoundOBJ, "_Berry_beep.WAV", 2);	//블루베리 먹으면
		//Sound[3] = SndObjCreate(SoundOBJ, "_Success.WAV", 2);	//게임 성공

//        SndObjPlay( Sound[0], DSBPLAY_LOOPING );
    }
//////////////////

    while ( !_GetKeyState(VK_ESCAPE) )
    {

        if ( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( !GetMessage(&msg, NULL, 0, 0) ) return msg.wParam;

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
//        else _GameProc();
    }
    DestroyWindow( MainHwnd );

    return TRUE;
}

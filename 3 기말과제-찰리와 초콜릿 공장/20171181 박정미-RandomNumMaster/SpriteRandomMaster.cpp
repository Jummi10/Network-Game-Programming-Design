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
LPDIRECTDRAWSURFACE  RealScreen;	//Primary Surface
LPDIRECTDRAWSURFACE  BackScreen;	//Back Surface
LPDIRECTDRAWSURFACE  Player;
LPDIRECTDRAWSURFACE  BackGround;
LPDIRECTDRAWSURFACE  Cacao;
LPDIRECTDRAWSURFACE  Chocolate;
LPDIRECTDRAWSURFACE  Blueberry;

LPDIRECTDRAWSURFACE  success;	//����ȭ��
LPDIRECTDRAWSURFACE  fail;	//��纣�� �Ծ��� �� ���� ȭ��
LPDIRECTDRAWSURFACE  Violet;	//��纣�� �Ծ��� �� ���� ȭ��

LPDIRECTDRAWCLIPPER	ClipScreen;

int gFullScreen=0, Click=0;
int gWidth=1200, gHeight=500;	//background ũ��
int MouseX=100, MouseY=gHeight/2;

int direction = 2;	//���Ʒ��翷 ����, ù ���� ������(2)
int eat_cacao = 0;	//īī�� �� �� �Ծ�����
int eat_berry = 0;	//��纣�� �� �� �Ծ�����
int destroy_cacao[20] = { 0, };	//0�̸� īī�� ���̰� �ƴϸ� īī�� ������
int destroy_berry[10] = { 0, };	//0�̸� ��纣�� ���̰� �ƴϸ� ��纣�� ������

////////////////////

LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

HSNDOBJ Sound[3];	//���� 3�� ���


BOOL _InitDirectSound( void )
{
    if ( DirectSoundCreate(NULL,&SoundOBJ,NULL) == DS_OK )
    {
        if (SoundOBJ->SetCooperativeLevel(MainHwnd,DSSCL_PRIORITY)!=DS_OK) return FALSE;

        memset(&DSB_desc,0,sizeof(DSBUFFERDESC));
        DSB_desc.dwSize = sizeof(DSBUFFERDESC);
        DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;

        if (SoundOBJ->CreateSoundBuffer(&DSB_desc,&SoundDSB,NULL)!=DS_OK) return FALSE;
        SoundDSB -> SetVolume(DSBVOLUME_MAX); // DSBVOLUME_MIN+
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
    MessageBox( hwnd, "DIRECT X �ʱ�ȭ�� �����߽��ϴ�.", "���� ������", MB_OK );
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
		if((MainHwnd=CreateWindow("TEST", "Master", WS_OVERLAPPEDWINDOW, 0, 0, x, 
									y, NULL, NULL, hInstance, NULL))==NULL)
			ExitProcess(1);
		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER);
	}

    SetFocus( MainHwnd );
    ShowWindow( MainHwnd, nCmdShow );
    UpdateWindow( MainHwnd );
//    ShowCursor( FALSE );	// ���콺 �� ���̰�

    result = DirectDrawCreate( NULL, &pdd, NULL );
    if ( result != DD_OK ) return Fail( MainHwnd );

    result = pdd->QueryInterface(IID_IDirectDraw, (LPVOID *) &DirectOBJ);	// create�س��� obj ����
    if ( result != DD_OK ) return Fail( MainHwnd );


	// ������ �ڵ��� ���� �ܰ踦 �����Ѵ�.
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


void CALLBACK _GameProc(HWND hWnd, UINT message, UINT wParam, DWORD lParam)
{
	//_c�� īī��, _b�� ��纣��
    RECT BackRect = { 0, 0, 1200, 500 };
	RECT DispRect = { 0, 0, gWidth, gHeight };	// realscreen�� ũ��
	RECT dstRect_c, dstRect_b, WinRect;	//dstRect: ���� ������ ��ġ ������
	RECT SpriteRectL, SpriteRectR, SpriteRectU, SpriteRectD;	//player�� sprite
	RECT SpriteRect_Cacao[20];	//īī��, �������� ������
	RECT SpriteRect_Berry[10];	//��纣��
	RECT SpriteRect_Choco, SpriteRect_Success, SpriteRect_Fail, SpriteRect_Violet;
	char sendData[200];


    BackScreen -> BltFast(0, 0, BackGround, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY); 

    static int Frame = 0;

	//ĳ���� ũ�� (����*����)66*66
	// Press Down: direction == 4
	SpriteRectD.left = Frame * 66;
	SpriteRectD.top = 268;
	SpriteRectD.right = SpriteRectD.left + 66;
	SpriteRectD.bottom = 334;

	// Press Left: direction == 1
	SpriteRectL.left = Frame * 66;
	SpriteRectL.top = 335;
	SpriteRectL.right = SpriteRectD.left + 66;
	SpriteRectL.bottom = 401;

	// Press Right: direction == 2
	SpriteRectR.left = Frame * 66;
	SpriteRectR.top = 402;
	SpriteRectR.right = SpriteRectD.left + 66;
	SpriteRectR.bottom = 468;

	// Press Up: direction == 3
	SpriteRectU.left = Frame * 66;
	SpriteRectU.top = 469;
	SpriteRectU.right = SpriteRectD.left + 66;
	SpriteRectU.bottom = 535;



	if(Click){
		if ( ++Frame >= 2 ){	// frame�� 0, 1, 2�� ��
			Frame = 0;
			Click = 0;
		}
	}

	if (MouseX <= 66) MouseX = 66;	//ĳ���Ͱ� ȭ�� ������ �� ������
	if (MouseX>gWidth) MouseX = gWidth;
	if (MouseY <= 66) MouseY = 66;
	if (MouseY>gHeight) MouseY = gHeight;

	if (direction == 1) {	//left
		BackScreen->BltFast(MouseX - 33, MouseY - 33, Player, &SpriteRectL, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		// &type, &dstRect.left,&dstRect.top,&dstRect.right,&dstRect.bottom,&SpriteRect.left, &SpriteRect.top, &SpriteRect.right, &SpriteRect.bottom
		sprintf(sendData, "1,%d,%d,%d,%d,%d,%d,%d,%d", MouseX - 33, MouseY - 33, 0, 0, SpriteRectL.left, SpriteRectL.top, SpriteRectL.right, SpriteRectL.bottom);
		CommSend(sendData);
	}

	if (direction == 2) {	//right
		BackScreen->BltFast(MouseX - 33, MouseY - 33, Player, &SpriteRectR, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		sprintf(sendData, "1,%d,%d,%d,%d,%d,%d,%d,%d", MouseX - 33, MouseY - 33, 0, 0, SpriteRectR.left, SpriteRectR.top, SpriteRectR.right, SpriteRectR.bottom);
		CommSend(sendData);
	}

	if (direction == 3) {	//up
		BackScreen->BltFast(MouseX - 33, MouseY - 33, Player, &SpriteRectU, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		sprintf(sendData, "1,%d,%d,%d,%d,%d,%d,%d,%d", MouseX - 33, MouseY - 33, 0, 0, SpriteRectU.left, SpriteRectU.top, SpriteRectU.right, SpriteRectU.bottom);
		CommSend(sendData);
	}

	if (direction == 4) {	//down
		BackScreen->BltFast(MouseX - 33, MouseY - 33, Player, &SpriteRectD, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		sprintf(sendData, "1,%d,%d,%d,%d,%d,%d,%d,%d", MouseX - 33, MouseY - 33, 0, 0, SpriteRectD.left, SpriteRectD.top, SpriteRectD.right, SpriteRectD.bottom);
		CommSend(sendData);
	}


	//////////////////////////////
	// Enter splite animation here
	// use srand(int seed); rand();
	// stone            (380, 375)            (405, 395)

	static int SrcX=0, SrcY=0, Collision_C=0, Collision_B=0;	// SrcX-- �����ν� �������� �����۵� �����̰�, SRcY�� �� ���ӿ��� ���� �ʴ´�.
	int posx_c, posy_c, size_c, coll_cacao = 0;
	int posx_b, posy_b, size_b, coll_berry = 0;
	srand(10);

	for(int k=0; k<10; k++){	//10���� ��纣��
		posx_b =(SrcX+rand())%(gWidth-30)+15; // (seed=)10<=rand()<=RAND_MAX ?
		posy_b =(SrcY+rand())%(gHeight-30)+15;
		size_b =(rand()%10) -10;	//(10(seed)+0���� 10+9����) - 10 == 0~9

		if (destroy_berry[k] == 0) {	//0�̸� ��纣���� ���δ�.
			SpriteRect_Berry[k].left = 0;
			SpriteRect_Berry[k].top = 0;
			SpriteRect_Berry[k].right = 35;
			SpriteRect_Berry[k].bottom = 30;
		}

		if(abs(MouseX-(posx_b +17))<35 && abs(MouseY-(posy_b +15))<30 ){	// collision, ������ �ε�����
			SpriteRect_Berry[k].left = 0;
			SpriteRect_Berry[k].top = 0;
			SpriteRect_Berry[k].right = 0;
			SpriteRect_Berry[k].bottom = 0;
			coll_berry = 1;
			destroy_berry[k] = 1;	//���� �����
		}
		
		dstRect_b.left = posx_b;
		dstRect_b.top = posy_b;
		dstRect_b.right = dstRect_b.left+35+size_b;
		dstRect_b.bottom = dstRect_b.top+30+size_b;

		BackScreen->Blt(&dstRect_b, Blueberry, &SpriteRect_Berry[k], DDBLT_WAIT | DDBLT_KEYSRC, NULL);	// Blt: ũ�� ���� ����? <-> BltFast: �ִ� �״��

		sprintf(sendData, "2,%d,%d,%d,%d,%d,%d,%d,%d", dstRect_b.left, dstRect_b.top, dstRect_b.right, dstRect_b.bottom, SpriteRect_Berry[k].left, SpriteRect_Berry[k].top, SpriteRect_Berry[k].right, SpriteRect_Berry[k].bottom);
		CommSend(sendData);
	}
	
	
	for(int k=0; k<20; k++){	//20���� īī��
		posx_c =(SrcX+rand())%(gWidth-30)+15;
		posy_c =(SrcY+rand())%(gHeight-30)+15;
		size_c =(rand()%10) -10;

		if (destroy_cacao[k] == 0) {	//0�̸� īī���� ���δ�.
			//īī�� ũ�� 36*29
			SpriteRect_Cacao[k].left = 0;
			SpriteRect_Cacao[k].top = 0;
			SpriteRect_Cacao[k].right = 36;
			SpriteRect_Cacao[k].bottom = 29;
		}

		if(abs(MouseX-(posx_c +18))<36 && abs(MouseY-(posy_c +15))<30 ){	// collision, īī���� �ε�����
			SpriteRect_Cacao[k].left = 0;
			SpriteRect_Cacao[k].top = 0;
			SpriteRect_Cacao[k].right = 0;
			SpriteRect_Cacao[k].bottom = 0;
			coll_cacao=1;
			destroy_cacao[k] = 1;	//īī�� �����
		}

		dstRect_c.left = posx_c;
		dstRect_c.top = posy_c;
		dstRect_c.right = dstRect_c.left+36+size_c;
		dstRect_c.bottom = dstRect_c.top+29+size_c;

		BackScreen->Blt(&dstRect_c, Cacao, &SpriteRect_Cacao[k], DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		sprintf(sendData, "3,%d,%d,%d,%d,%d,%d,%d,%d", dstRect_c.left, dstRect_c.top, dstRect_c.right, dstRect_c.bottom,SpriteRect_Cacao[k].left, SpriteRect_Cacao[k].top, SpriteRect_Cacao[k].right, SpriteRect_Cacao[k].bottom);
		CommSend(sendData);
	}

	//chocolate
	SpriteRect_Choco.left = 0;
	SpriteRect_Choco.top = 0;
	SpriteRect_Choco.right = 400;
	SpriteRect_Choco.bottom = 300;

	//success
	SpriteRect_Success.left = 0;
	SpriteRect_Success.top = 0;
	SpriteRect_Success.right = 400;
	SpriteRect_Success.bottom = 150;

	//fail
	SpriteRect_Fail.left = 0;
	SpriteRect_Fail.top = 0;
	SpriteRect_Fail.right = 500;
	SpriteRect_Fail.bottom = 150;
	
	//violet
	SpriteRect_Violet.left = 60;
	SpriteRect_Violet.top = 0;
	SpriteRect_Violet.right = 500;
	SpriteRect_Violet.bottom = 300;


	if(coll_cacao){
		if(!Collision_C){
			Collision_C=1;
			eat_cacao++;
			_Play(1);
		}
	}
	else
		Collision_C=0;


	if (coll_berry) {
		if (!Collision_B) {
			Collision_B = 1;
			eat_berry++;
			_Play(2);
		}
	}
	else
		Collision_B = 0;

	//īī�� 10�� ������
	if (eat_cacao >= 10) {
		//����ȭ�� ��ȯ, success, Chocolate
		BackScreen->BltFast(400, 0, success, &SpriteRect_Success, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);	//SUCCESS!
		//ȣ��: sprintf(sendData, "4,%d,%d,%d,%d,%d,%d,%d,%d",400, 0, 800, 150, SpriteRect_Success.left, SpriteRect_Success.top, SpriteRect_Success.right, SpriteRect_Success.bottom);
		sprintf(sendData, "4,%d,%d,%d,%d,%d,%d,%d,%d",400, 0, 0, 0, SpriteRect_Success.left, SpriteRect_Success.top, SpriteRect_Success.right, SpriteRect_Success.bottom);
		CommSend(sendData);

		BackScreen->BltFast(400, 150, Chocolate, &SpriteRect_Choco, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);	//���ݸ� �ϼ�!
		sprintf(sendData, "5,%d,%d,%d,%d,%d,%d,%d,%d", 400, 150, 0, 0, SpriteRect_Choco.left, SpriteRect_Choco.top, SpriteRect_Choco.right, SpriteRect_Choco.bottom);
		CommSend(sendData);

		//_Play(3);
	}

	//��纣�� ������
	else if (eat_berry >= 1) {
		//����ȭ��, fail, violet
		BackScreen->BltFast(350, 0, fail, &SpriteRect_Fail, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);	//Fail..
		sprintf(sendData, "6,%d,%d,%d,%d,%d,%d,%d,%d", 350, 0, 0, 0, SpriteRect_Fail.left, SpriteRect_Fail.top, SpriteRect_Fail.right, SpriteRect_Fail.bottom);
		CommSend(sendData);

		BackScreen->BltFast(380, 150, Violet, &SpriteRect_Violet, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);	//��纣�� �� ��
		sprintf(sendData, "7,%d,%d,%d,%d,%d,%d,%d,%d", 380, 150, 0, 0, SpriteRect_Violet.left, SpriteRect_Violet.top, SpriteRect_Violet.right, SpriteRect_Violet.bottom);
		CommSend(sendData);

	}

	SrcX--;	// īī��, ��纣�� �������� �����̰�

	if(gFullScreen)	//Ǯ��ũ���̸�
		RealScreen->Flip(NULL, DDFLIP_WAIT);
	else{	//Ǯ��ũ���� �ƴϸ�->�츮�� ���, ������ â/��׶��� ũ�� ��������
		GetWindowRect(MainHwnd, &WinRect);	// ������ â�� ��ġ ��������
		RealScreen->Blt(&WinRect, BackScreen, &DispRect, DDBLT_WAIT, NULL ); //real�� DispRect�� ũ��(������ â ũ��, �� ó���� ����)��ŭ backscreen�� ����(blt)
	}
}




int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG msg;

    if ( !_GameMode(hInstance, nCmdShow, gWidth, gHeight, 32) ) return FALSE;

    Player = DDLoadBitmap( DirectOBJ, "_Boy.BMP", 0, 0 );	//���ΰ� ����
    BackGround  = DDLoadBitmap( DirectOBJ, "_Background_Chocolate-Factory.BMP", 0, 0 );	//���ݸ� ���� ���
    Cacao  = DDLoadBitmap( DirectOBJ, "_Cacao.BMP", 0, 0 );	//īī��
    Chocolate  = DDLoadBitmap( DirectOBJ, "_Chocolate.BMP", 0, 0 );	//������ ���ݸ�
    Blueberry  = DDLoadBitmap( DirectOBJ, "_Blueberry.BMP", 0, 0 );	//��ֹ�	, _Blueberry.BMP
    success  = DDLoadBitmap( DirectOBJ, "_Success.BMP", 0, 0 );	//SUCCESS! �۾�
    fail  = DDLoadBitmap( DirectOBJ, "_Fail2.BMP", 0, 0 );	//FAIL.. �۾�
    Violet  = DDLoadBitmap( DirectOBJ, "_Violet.BMP", 0, 0 );	//���н� ��纣���� �� ���̿÷�, _Violet_Bluberry

    DDSetColorKey( Player, RGB(0,0,0) );	//(0,0,0) �������� destroy
    DDSetColorKey( Cacao, RGB(0,0,0) );
    DDSetColorKey( Chocolate, RGB(0,0,0) );
    DDSetColorKey( Blueberry, RGB(0,0,0) );
    DDSetColorKey( success, RGB(0,0,0) );
    DDSetColorKey( fail, RGB(0,0,0) );
    DDSetColorKey( Violet, RGB(0,0,0) );

	SetTimer(MainHwnd, 1, 30, _GameProc);	// MainHWnd(windowâ)�� 1�ʿ� 30���� _GameProc�� �θ���.

	CommInit(NULL, NULL);

///////////////////

    if ( _InitDirectSound() )
    {
        Sound[0] = SndObjCreate(SoundOBJ,"_Willy-Wonka.WAV",1);	//�������
        Sound[1] = SndObjCreate(SoundOBJ,"_Get_Cacao.WAV",2);	//���� �ε�����->īī�� ������
        Sound[2] = SndObjCreate(SoundOBJ,"_Berry_beep.WAV",2);	//��纣�� ������
		//Sound[3] = SndObjCreate(SoundOBJ, "_Success.WAV", 2);	//���� ����
		

        SndObjPlay( Sound[0], DSBPLAY_LOOPING );	//������� ���
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

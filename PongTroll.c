////////////////////////////////////////////////////////////////////////
// PongTroll.c
// Siul - DLabs - 2016
// Basado en el código del PongC de Mochilote,
// creador de la página www.cpcmania.com.
// Agradecimientos a Lopenovi y Lugerh de
// DLabs, por sus ideas para el juego.
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Sprites.h"


////////////////////////////////////////////////////////////////////////
//------------------------ Generic variables -------------------------//
////////////////////////////////////////////////////////////////////////
#define SOUND_BUFF 0x4FF6 //9 bytes
#define ENT_BUFF 0x4FE6 //16 bytes


////////////////////////////////////////////////////////////////////////
//------------------------ Generic functions -------------------------//
////////////////////////////////////////////////////////////////////////
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define A10	       (rand() % 10 + 1)


////////////////////////////////////////////////////////////////////////
//sound
////////////////////////////////////////////////////////////////////////
unsigned char bQueue = 0;
unsigned char sound(unsigned char nChannelStatus, int nTonePeriod, int nDuration, unsigned char nVolume, char nVolumeEnvelope, char nToneEnvelope, unsigned char nNoisePeriod)
{
	//This function uses 9 bytes of memory for sound buffer. Firmware requires it in the central 32K of RAM (0x4000 to 0xC000)
	/*
		The bytes required to define the sound are as follows
		byte 0 - channel status byte
		byte 1 - volume envelope to use
		byte 2 - tone envelope to use
		bytes 3&4 - tone period
		byte 5 - noise period
		byte 6 - start volume
		bytes 7&8 - duration of the sound, or envelope repeat count	
	*/
	
	__asm
		LD HL, #SOUND_BUFF

		LD A, 4 (IX) ;nChannelStatus
		LD (HL), A
		INC HL

		LD A, 10 (IX) ;nVolumeEnvelope
		LD (HL), A
		INC HL

		LD A, 11 (IX) ;nToneEnvelope
		LD (HL), A
		INC HL

		LD A, 5 (IX) ;nTonePeriod
		LD (HL), A
		INC HL
		LD A, 6 (IX) ;nTonePeriod
		LD (HL), A
		INC HL

		LD A, 12 (IX) ;nNoisePeriod
		LD (HL), A
		INC HL

		LD A, 9 (IX) ;nVolume
		LD (HL), A
		INC HL

		LD A, 7 (IX) ;nDuration
		LD (HL), A
		INC HL
		LD A, 8 (IX) ;nDuration
		LD (HL), A
		INC HL

		LD HL, #SOUND_BUFF
		CALL #0xBCAA ;SOUND QUEUE
	
		LD HL, #_bQueue
		LD (HL), #0
		JP NC, _EndSound
		LD (HL), #1
		_EndSound:
	__endasm;
	
	return bQueue;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ent
////////////////////////////////////////////////////////////////////////
void ent(unsigned char nEnvelopeNumber, unsigned char nNumberOfSteps, char nTonePeriodOfStep, unsigned char nTimePerStep)
{
	//This function uses 16 bytes of memory for ent buffer. Firmware requires it in the central 32K of RAM (0x4000 to 0xC000)
	
	__asm
		LD HL, #ENT_BUFF

		LD A, #0x1
		LD (HL), A
		INC HL

		LD A, 5 (IX) ;nNumberOfSteps
		LD (HL), A
		INC HL

		LD A, 6 (IX) ;nTonePeriodOfStep
		LD (HL), A
		INC HL

		LD A, 7 (IX) ;nTimePerStep
		LD (HL), A
		INC HL

		LD A, 4 (IX) ;nEnvelopeNumber
		LD HL, #ENT_BUFF
		CALL #0xBCBF ;SOUND TONE ENVELOPE
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//GetTime
////////////////////////////////////////////////////////////////////////
unsigned char char3,char4;

unsigned int GetTime()
{
	unsigned int nTime = 0;

	__asm
		CALL #0xBD0D ;KL TIME PLEASE
		PUSH HL
		POP DE
		LD HL, #_char3
		LD (HL), D
		LD HL, #_char4
		LD (HL), E
	__endasm;

	nTime = (char3 << 8) + char4;

	return nTime;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//SetColor
////////////////////////////////////////////////////////////////////////
void SetColor(unsigned char nColorIndex, unsigned char nPaletteIndex)
{
	__asm
		ld a, 4 (ix)
		ld b, 5 (ix)
		ld c, b
		call #0xBC32 ;SCR SET INK
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//SetMode
////////////////////////////////////////////////////////////////////////
void SetMode(unsigned char nMode)
{
	__asm
		ld a, 4 (ix)
		call #0xBC0E ;SCR_SET_MODE
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//SetBorder
////////////////////////////////////////////////////////////////////////
void SetBorder(unsigned char nColor)
{
	__asm
		ld b, 4 (ix)
		ld c, b
		call #0xBC38 ;SCR_SET_BORDER
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//SetCursor
////////////////////////////////////////////////////////////////////////
void SetCursor(unsigned char nColum, unsigned char nLine)
{
	__asm
		ld h, 4 (ix)
		ld l, 5 (ix)
		call #0xBB75 ;TXT SET CURSOR
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//PutSprite
////////////////////////////////////////////////////////////////////////
void PutSprite(unsigned char *pAddress, unsigned char nWidth, unsigned char nHeight, unsigned char *pSprite)
{
	__asm
		LD L, 4(IX) 
		LD H, 5(IX) 
		LD C, 6(IX) 
		LD B, 7(IX)            
		LD E, 8(IX) 
		LD D, 9(IX) 

		_loop_alto:
			PUSH BC
			LD B,C
			PUSH HL
		_loop_ancho:
			LD A,(DE)
			LD (HL),A
			INC DE
			INC HL
			DJNZ _loop_ancho
			POP HL
			LD A,H
			ADD #0x08
			LD H,A
			SUB #0xC0
			JP NC, _sig_linea
			LD BC, #0xC050
			ADD HL,BC
		_sig_linea:
			POP BC
			DJNZ _loop_alto
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//WaitVsync
////////////////////////////////////////////////////////////////////////
void WaitVsync()
{
	__asm
		ld b,#0xf5          ;; PPI port B input
		_wait_vsync:
		in a,(c)            ;; [4] read PPI port B input
    		                ;; (bit 0 = "1" if vsync is active,
    		                ;;  or bit 0 = "0" if vsync is in-active)
		rra                 ;; [1] put bit 0 into carry flag
		jp nc,_wait_vsync   ;; [3] if carry not set, loop, otherwise continue
	__endasm;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//LineHMode2Byte
////////////////////////////////////////////////////////////////////////
void LineHMode2Byte(unsigned int nX, unsigned int nY, unsigned char nColor, unsigned char nBytes)
{
	memset((unsigned char *)(0xC000 + ((nY / 8) * 80) + ((nY % 8) * 2048) + (nX / 8)), nColor, nBytes);
}
////////////////////////////////////////////////////////////////////////


//Enumeration to identify each physical key
typedef enum _eKey
{
	Key_CursorUp = 0,
	Key_CursorRight,
	Key_CursorDown,
	Key_F9,
	Key_F6,
	Key_F3,
	Key_Enter,
	Key_FDot,
	Key_CursorLeft, //8
	Key_Copy,
	Key_F7,
	Key_F8,
	Key_F5,
	Key_F1,
	Key_F2,
	Key_F0,
	Key_Clr, //16
	Key_BraceOpen,
	Key_Return,
	Key_BraceClose,
	Key_F4,
	Key_Shift,
	Key_BackSlash,
	Key_Control,
	Key_Caret, //24
	Key_Hyphen,
	Key_At,
	Key_P,
	Key_SemiColon,
	Key_Colon,
	Key_Slash,
	Key_Dot,
	Key_0, //32
	Key_9,
	Key_O,
	Key_I,
	Key_L,
	Key_K,
	Key_M,
	Key_Comma,
	Key_8, //40
	Key_7,
	Key_U,
	Key_Y,
	Key_H,
	Key_J,
	Key_N,
	Key_Space,
	Key_6_Joy2Up, //48
	Key_5_Joy2Down,
	Key_R_Joy2Left,
	Key_T_Joy2Right,
	Key_G_Joy2Fire,
	Key_F,
	Key_B,
	Key_V,
	Key_4, //56
	Key_3,
	Key_E,
	Key_W,
	Key_S,
	Key_D,
	Key_C,
	Key_X,
	Key_1, //64
	Key_2,
	Key_Esc,
	Key_Q,
	Key_Tab,
	Key_A,
	Key_CapsLock,
	Key_Z,
	Key_Joy1Up, //72
	Key_Joy1Down,
	Key_Joy1Left,
	Key_Joy1Right,
	Key_Joy1Fire1,
	Key_Joy1Fire2,
	Key_Joy1Fire3,
	Key_Del,
	Key_Max //80
}_ekey;


////////////////////////////////////////////////////////////////////////
//IsKeyPressedFW
////////////////////////////////////////////////////////////////////////
char nKeyPressed;
unsigned char IsKeyPressedFW(unsigned char eKey)
{
	__asm
		LD HL, #_nKeyPressed
		LD (HL), #0
		LD A, 4 (IX)
		CALL #0xBB1E ;KM TEST KEY
		JP Z, _end_IsKeyPressed
		LD HL, #_nKeyPressed
		LD (HL), #1
		_end_IsKeyPressed:
	__endasm;
	
	return nKeyPressed;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//------------------------ Specific variables ------------------------//
////////////////////////////////////////////////////////////////////////

#define NUM_PLAYERS 2

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 200

#define SCORE_Y 20
#define SCORE_X_1 ((SCREEN_WIDTH / 2) - 50 - NUMBER_WIDTH / 2)
#define SCORE_X_2 ((SCREEN_WIDTH / 2) + 50 - NUMBER_WIDTH / 2)

#define SCORE_Y_ROUNDS 10
#define SCORE_X_1_ROUNDS ((SCREEN_WIDTH / 2) - 280 - NUMBER_WIDTH / 2)
#define SCORE_X_2_ROUNDS ((SCREEN_WIDTH / 2) + 280 - NUMBER_WIDTH / 2)

#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 30
#define PLAYER_WIDTH_BYTES 2

#define BIG_PLAYER_HEIGHT 60

#define MAX_SCORE 5

//Variables Generales del Juego

char playerPowerPressed[2][2];
char haHechoContactoPlayer[2];
char numRondas;
char dificultad;
char eleccion;

//Variables de los Poderes de Defensa

struct _tPoderesDefensa
{
	char contRayoDestructor; //Poder 1
	char presionadoTuneles; //Poder 4
	char contRayoTractor; //Poder 7
	char nYDirAntiguo; //Poder 7
	char contRalentizacionBola; //Poder 8
	char activarPoder;
}_tPoderesDefensa;

struct _tPoderesDefensa tPoderesDefensa[NUM_PLAYERS];

//Variables de los Poderes de Ataque

struct _tPoderesAtaque
{
	char invertidoRalentizadoParpadeante; //Poderes 2, 5 y 9
	char bolaAgitada; //Poderes 3 y 7
	char rebotesRestantes; //Poder 8
	char pelotaFalsaMovimiento; //Poder 10
	char activarPoder;
}_tPoderesAtaque;

struct _tPoderesAtaque tPoderesAtaque[NUM_PLAYERS];

//Variables de Jugador

struct _tPlayer
{
	unsigned int nY;
	unsigned int nX;
	_ekey ekeyUp;
	_ekey ekeyDown;
	_ekey ekeyLeft;
	_ekey ekeyRight;
	char nScore;
	char nRounds;
	char powers[NUM_PLAYERS];
	char nLastYMove;
}_tPlayer;

struct _tPlayer aPlayer[NUM_PLAYERS];

//Variables de Bola Verdadera y Bola Falsa (Bola Divergente)

struct _tBall
{
	int nY;
	int nX;
	char nYDir;
	char nXDir;
}_tBall;

struct _tBall tTrueBall, tFalseBall;

//Variables de Marcadores de Puntuación por Ronda y Rondas Ganadas

unsigned char *pScoreScreen[NUM_PLAYERS], *pNumRoundsScreen[NUM_PLAYERS];
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//------------------------ Specific functions ------------------------//
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Sonido
////////////////////////////////////////////////////////////////////////
void Sonido(char numero)
{
	int aSounds[12] = { 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253 };
	int n = 0;

	switch(numero) {
		case 1:
			for(n=0; n < 12; n++)
			{
				while(!sound(1, aSounds[n], 5, 15, 0, 0, 0));
			}
			break;
		case 2: 
			ent(1, 20, -11, 1);
			sound(1, 200, 20, 15, 0, 1, 0);	
			break;
		case 3:
			ent(1, 20, -11, 1);
			sound(1, 428, 10, 15, 0, 1, 0);
			break;
		case 4:
			ent(1, 25, 120, 6);
			sound(1, 428, 25, 15, 1, 1, 14);
			break;
		case 5:
			sound(1, 200, 10, 15, 0, 0, 0);
			break;
		case 6:
			sound(1, 284, 1000, 1, 1, 0, 0);
			break;
		case 7:
			ent(1, 10, 2, 2);
			sound(1,284,20,15,0,1,0);
			break;
		case 8:
			ent(1, 10, -2, 2);
			sound(1, 284, 20, 15, 0, 1, 0);	
	}

}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//DrawPlayer
////////////////////////////////////////////////////////////////////////
void DrawPlayer(unsigned char nPlayer)
{
	unsigned char nY = 0;
	for(nY = 0; nY < PLAYER_HEIGHT; nY++)
		LineHMode2Byte(aPlayer[nPlayer].nX, aPlayer[nPlayer].nY + nY, 0x0F, PLAYER_WIDTH_BYTES);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//DrawOrDeleteBigPlayer
////////////////////////////////////////////////////////////////////////
void DrawOrDeleteBigPlayer(unsigned char nPlayer, unsigned char nColor)
{
	unsigned char nY = 0;
	unsigned char topePintar = 0;

	topePintar = MIN(PLAYER_HEIGHT, SCREEN_HEIGHT - (aPlayer[nPlayer].nY + PLAYER_HEIGHT));

	for(nY = 0; nY < topePintar; nY++)
		LineHMode2Byte(aPlayer[nPlayer].nX, aPlayer[nPlayer].nY + PLAYER_HEIGHT + nY, nColor, PLAYER_WIDTH_BYTES);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//DrawPlayerInc
////////////////////////////////////////////////////////////////////////
void DraworDeletePlayerInc(unsigned char nPlayer, unsigned char player_y_inc, unsigned char nYToDrawOrDelete, unsigned char nColor)
{
	unsigned char nY = 0;
	for(nY = 0; nY < player_y_inc; nY++)
		LineHMode2Byte(aPlayer[nPlayer].nX, nYToDrawOrDelete + nY, nColor, PLAYER_WIDTH_BYTES);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//MovePlayer
////////////////////////////////////////////////////////////////////////
void MovePlayer(unsigned char nPlayer, unsigned char player_y_inc)
{
	unsigned char nY = 0;
	unsigned char nYToDelete = 0;
	unsigned char nYToDraw = 0;
	int nYNew = 0;
	unsigned char bKeyUp = IsKeyPressedFW(aPlayer[nPlayer].ekeyUp);
	unsigned char bKeyDown = IsKeyPressedFW(aPlayer[nPlayer].ekeyDown);

	aPlayer[nPlayer].nLastYMove = 0;

	if (tPoderesAtaque[nPlayer].invertidoRalentizadoParpadeante == 1)
	{
		//Invertir controles
		bKeyUp = IsKeyPressedFW(aPlayer[nPlayer].ekeyDown);
		bKeyDown = IsKeyPressedFW(aPlayer[nPlayer].ekeyUp);
	}

	if(bKeyUp == bKeyDown)
		return;

	nYNew = bKeyUp ? aPlayer[nPlayer].nY - player_y_inc : aPlayer[nPlayer].nY + player_y_inc;
	
	if(bKeyDown)
	{
		if ((tPoderesDefensa[nPlayer].activarPoder == 6) && (nYNew + BIG_PLAYER_HEIGHT > SCREEN_HEIGHT))
			//Hacer barra grande
			nYNew = SCREEN_HEIGHT - BIG_PLAYER_HEIGHT;

		else if(nYNew + PLAYER_HEIGHT > SCREEN_HEIGHT)
			nYNew = SCREEN_HEIGHT - PLAYER_HEIGHT;

		aPlayer[nPlayer].nY = nYNew;		
		nYToDelete = aPlayer[nPlayer].nY - player_y_inc;

		if (tPoderesDefensa[nPlayer].activarPoder == 6)
			//Hacer barra grande
			nYToDraw = aPlayer[nPlayer].nY + BIG_PLAYER_HEIGHT - player_y_inc;
		else
			nYToDraw = aPlayer[nPlayer].nY + PLAYER_HEIGHT - player_y_inc;

		aPlayer[nPlayer].nLastYMove = 1;
	}
	else
	{
		if(nYNew <= 0)
			nYNew = 0;
		
		aPlayer[nPlayer].nY = nYNew;

		if (tPoderesDefensa[nPlayer].activarPoder == 6)
			//Hacer barra grande
			nYToDelete = aPlayer[nPlayer].nY + BIG_PLAYER_HEIGHT;
		else
			nYToDelete = aPlayer[nPlayer].nY + PLAYER_HEIGHT;

		nYToDraw = aPlayer[nPlayer].nY;
		aPlayer[nPlayer].nLastYMove = -1;
	}
	
	switch (tPoderesAtaque[nPlayer].invertidoRalentizadoParpadeante) {

		case 1:
			//Controles del oponente invertidos
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDelete, 0x00);
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDraw, 0xFF);
			break;

		case 2:
			//Bola eléctrica
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDelete, 0x00);
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDraw, 0xF0);
			break;

		case 3:
			//Jugador parpadeante
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDelete, 0x00);
			if ((rand() % 2) == 0)
				DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDraw, 0xF0);
			break;

		default:
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDelete, 0x00);
			DraworDeletePlayerInc(nPlayer, player_y_inc, nYToDraw, 0x0F);
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//DrawOrDeleteBall
////////////////////////////////////////////////////////////////////////
void DrawOrDeleteBall(struct _tBall *tBall, char nColor)
{
	PutSprite((unsigned char *) 0xC000 + ((tBall->nY / 8) * 80) + ((tBall->nY % 8) * 2048) + (tBall->nX / 8), BALL_WIDTH_BYTES, BALL_HEIGHT, aBallSprite[nColor]);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//PowersToZero
////////////////////////////////////////////////////////////////////////
void PowersToZero()
{
	playerPowerPressed[0][0] = 0;
	playerPowerPressed[0][1] = 0;
	playerPowerPressed[1][0] = 0;
	playerPowerPressed[1][1] = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//BallOut
////////////////////////////////////////////////////////////////////////
char BallOut(struct _tBall *tBall, char nPlayer)
{
	char nReturn = -1;

	if (tPoderesAtaque[1-nPlayer].activarPoder == 4)
	{
		//Pelota de despiste
		nReturn = 4;
	}
	else
	{
		if (tBall->nX == 0)
			nReturn = 1;
		else
			nReturn = 0;		
	}

	memset(tPoderesDefensa, 0, sizeof(tPoderesDefensa));
	memset(tPoderesAtaque, 0, sizeof(tPoderesAtaque));

	haHechoContactoPlayer[0] = 0;
	haHechoContactoPlayer[1] = 0;

	return nReturn;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Rebound
////////////////////////////////////////////////////////////////////////
void Rebound(struct _tBall *tBall, char nPlayer, int nYGhost, int nYRest)
{
	Sonido(5);

	if (tPoderesAtaque[nPlayer].activarPoder == 0)
	{
		tPoderesAtaque[nPlayer].activarPoder = playerPowerPressed[nPlayer][1];
		playerPowerPressed[nPlayer][1] = 0;
	}
	tPoderesAtaque[1-nPlayer].activarPoder = 0;

	if (tPoderesAtaque[nPlayer].activarPoder == 6)
		//Bola fantasma
		tBall->nY = nYGhost;	

	else if (tPoderesAtaque[nPlayer].activarPoder == 3)
	{
		//Bola con efecto
		tPoderesAtaque[nPlayer].activarPoder = 0;
		tBall->nYDir *= -2;
		tBall->nY = nYRest;
		tPoderesAtaque[nPlayer].bolaAgitada = 3;
	}
	else if (tPoderesAtaque[nPlayer].activarPoder == 7)
	{
		//Acelerar bola
		tPoderesAtaque[nPlayer].activarPoder = 0;
		tBall->nXDir *= 2;
		tBall->nYDir *= -2;
		tPoderesAtaque[0].bolaAgitada = 7;
	}
	else
	{
		tBall->nYDir *= -1;
		tBall->nY = nYRest;
	}

	haHechoContactoPlayer[0] = 0;
	haHechoContactoPlayer[1] = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Invertir
////////////////////////////////////////////////////////////////////////
void Invertir (unsigned char nPlayer)
{
	if (tPoderesAtaque[nPlayer].activarPoder == 2)
		//Jugador invertido
		tPoderesAtaque[1-nPlayer].invertidoRalentizadoParpadeante = 1;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Ralentizar
////////////////////////////////////////////////////////////////////////
void Ralentizar (unsigned char nPlayer)
{
	if (tPoderesAtaque[nPlayer].activarPoder == 5)
	{
		//Bola eléctrica
		tPoderesAtaque[1-nPlayer].invertidoRalentizadoParpadeante = 2;		
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//BallPlayerContact
////////////////////////////////////////////////////////////////////////
char BallPlayerContact(struct _tBall *tBall, char nPlayer, char soundNum)
{
	char nReturn = -1;

	Sonido(soundNum);

	//Estirar barra - Borrado
	if (tPoderesDefensa[nPlayer].activarPoder == 6)
		DrawOrDeleteBigPlayer(nPlayer,0x00);

	//Controles invertidos
	Invertir(1-nPlayer);

	//Bola eléctrica
	Ralentizar(1-nPlayer);

	//Bola Reptadora
	if (tPoderesDefensa[nPlayer].nYDirAntiguo != 0)
	{
		tBall->nYDir = tPoderesDefensa[nPlayer].nYDirAntiguo;
                     tBall->nXDir *= 3;
		tPoderesDefensa[nPlayer].nYDirAntiguo = 0;
	}

           // //Bola Reptante
           // if (tPoderesDefensa[1-nPlayer].nYDirAntiguo != 0)
           // {
           //           tBall->nXDir /= 3;
           //           tPoderesDefensa[1-nPlayer].nYDirAntiguo = 0;
           // }

	if (tPoderesAtaque[1-nPlayer].activarPoder == 4)
		//Pelota de despiste
		nReturn = -1;

	else if (tPoderesAtaque[1-nPlayer].activarPoder == 9)
	{
		//Jugador 1 ó 2 parpadeante
		if ((rand() % 5) < 2)
			nReturn = nPlayer;
		else 
			nReturn = -1;
	}
	else
		nReturn = nPlayer;

	//Bola Divergente
	if (tPoderesAtaque[1-nPlayer].pelotaFalsaMovimiento == 1)
		tPoderesAtaque[1-nPlayer].pelotaFalsaMovimiento = -1;
	else
		tPoderesAtaque[1-nPlayer].pelotaFalsaMovimiento = 0;

	//Ralentizar bola
	tPoderesDefensa[nPlayer].contRalentizacionBola = 0;

	tPoderesDefensa[nPlayer].activarPoder = 0;
	tPoderesAtaque[1-nPlayer].activarPoder = 0;

	return nReturn;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Block
////////////////////////////////////////////////////////////////////////
void Barrera(struct _tBall *tBall, char nPlayer)
{
	if (tPoderesAtaque[nPlayer].rebotesRestantes > 0)
	{
                     //Bloque en contra del oponente
                     if ((nPlayer == 0) && (tBall->nX > SCREEN_WIDTH/2))
		{
			tBall->nXDir *= -1;
			tPoderesAtaque[nPlayer].rebotesRestantes--;
		}
                     else if ((nPlayer == 1) && (tBall->nX < SCREEN_WIDTH/2))
                    {
                                tBall->nXDir *= -1;
                                tPoderesAtaque[nPlayer].rebotesRestantes--;
                    }
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//AgujerosGusano
////////////////////////////////////////////////////////////////////////
void AgujerosGusano (struct _tBall *tBall, unsigned char nPlayer)
{
	tBall->nXDir *= -1;
	tBall->nX = SCREEN_WIDTH / 2;
	tPoderesDefensa[nPlayer].presionadoTuneles = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Invulnerabilidad
////////////////////////////////////////////////////////////////////////
char Invulnerabilidad (unsigned char nPlayer)
{
	tPoderesDefensa[nPlayer].activarPoder = 0;
	return nPlayer;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//CampoRepulsivo
////////////////////////////////////////////////////////////////////////
void CampoRepulsivo (struct _tBall *tBall, unsigned char nPlayer)
{
	char nAuxDir = 0;
	if (nPlayer == 0)
		nAuxDir = abs(tBall->nYDir);
	else
		nAuxDir = -abs(tBall->nYDir);
	tBall->nYDir = tBall->nXDir;
	tBall->nXDir = nAuxDir;
	tPoderesDefensa[nPlayer].activarPoder = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//CampoAtractivo
////////////////////////////////////////////////////////////////////////
void CampoAtractivo (struct _tBall *tBall, unsigned char nPlayer)
{
	if (nPlayer == 0)
		tBall->nX = aPlayer[0].nX + PLAYER_WIDTH;
	else
		tBall->nX = aPlayer[1].nX - BALL_WIDTH;
	tBall->nY = aPlayer[nPlayer].nY + PLAYER_WIDTH/2;
	tPoderesDefensa[nPlayer].activarPoder = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//AtraerBola
////////////////////////////////////////////////////////////////////////
void AtraerBola (struct _tBall *tBall, unsigned char nPlayer)
{
	tPoderesDefensa[nPlayer].activarPoder = 0;
	tPoderesDefensa[nPlayer].nYDirAntiguo = tBall->nYDir;
	tBall->nYDir = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//BloquearMovimiento
////////////////////////////////////////////////////////////////////////
void BloquearMovimiento (unsigned char nPlayer)
{
	aPlayer[nPlayer].ekeyUp = Key_BraceOpen;
	aPlayer[nPlayer].ekeyDown = Key_BraceClose;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//DestruirBola
////////////////////////////////////////////////////////////////////////
void DestruirBola (struct _tBall *tBall)
{
	memset(tPoderesDefensa, 0, sizeof(tPoderesDefensa));
	memset(tPoderesAtaque, 0, sizeof(tPoderesAtaque));

	haHechoContactoPlayer[0] = 0;
	haHechoContactoPlayer[1] = 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//PintarRayo
////////////////////////////////////////////////////////////////////////
void PintarRayo (unsigned char nPlayer, char nRayo)
{
	Sonido(2*nRayo + 2);

	BloquearMovimiento(nPlayer);

	PutSprite((unsigned char *)0xC000 + (((aPlayer[nPlayer].nY + 11) / 8) * 80) + (((aPlayer[nPlayer].nY + 11) % 8) * 2048) + 3 + nPlayer, RAY_WIDTH_BYTES, RAY_HEIGHT, raySprite[2]);
	PutSprite((unsigned char *)0xC000 + (((aPlayer[nPlayer].nY + 11) / 8) * 80) + (((aPlayer[nPlayer].nY + 11) % 8) * 2048) + 3 + nPlayer, RAY_WIDTH_BYTES, RAY_HEIGHT, raySprite[nRayo]);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Desbloquear
////////////////////////////////////////////////////////////////////////
void Desbloquear (unsigned char nPlayer)
{
	//Rayo Destructor o Rayo Tractor
	if (nPlayer == 0)
	{
		aPlayer[0].ekeyUp = Key_W;
		aPlayer[0].ekeyDown = Key_S;
	}
	else
	{
		aPlayer[1].ekeyUp = Key_CursorUp;
		aPlayer[1].ekeyDown = Key_CursorDown;
	}

	PutSprite((unsigned char *)0xC000 + (((aPlayer[nPlayer].nY + 11) / 8) * 80) + (((aPlayer[nPlayer].nY + 11) % 8) * 2048) + 3 + nPlayer, RAY_WIDTH_BYTES, RAY_HEIGHT, raySprite[2]);
	
	tPoderesDefensa[nPlayer].contRayoDestructor = 0;	
	tPoderesDefensa[nPlayer].contRayoTractor = 0;			
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//MoveBall
////////////////////////////////////////////////////////////////////////
char MoveBall(struct _tBall *tBall)
{
	char nReturn = -1;
	char deCanto = 0;
	char bCollision = -1;
	
	if ((tPoderesDefensa[0].activarPoder == 5) || (tPoderesDefensa[1].activarPoder == 5) ||
	    (tPoderesAtaque[0].activarPoder == 4) ||(tPoderesAtaque[1].activarPoder == 4))
		//Hacer bola grande o Bola de Despiste -> Delete ball
		PutSprite((unsigned char *)0xC000 + ((tBall->nY / 8) * 80) + ((tBall->nY % 8) * 2048) + (tBall->nX / 8), BIG_BALL_WIDTH_BYTES, BIG_BALL_HEIGHT, bigBallSprite[2]);
	else
		DrawOrDeleteBall(tBall,3);
		
	tBall->nY = tBall->nY + tBall->nYDir;

	if ((tBall->nYDir > 0 && tBall->nY >= (SCREEN_HEIGHT - BALL_HEIGHT)) || //Hacer bola grande
		(tBall->nYDir > 0 && ((tPoderesDefensa[0].activarPoder == 5) || (tPoderesDefensa[1].activarPoder == 5)) && 
			tBall->nY >= (SCREEN_HEIGHT - BIG_BALL_HEIGHT))) 
	{
		if (tBall->nXDir > 0)
			Rebound(tBall, 0, 0, SCREEN_HEIGHT - BALL_HEIGHT);
		else
			Rebound(tBall, 1, 0, SCREEN_HEIGHT - BALL_HEIGHT);
	}
	
	if (tBall->nYDir < 0 && tBall->nY <= 0)
	{
		if (tBall->nXDir > 0)
			Rebound(tBall, 0, SCREEN_HEIGHT - BALL_HEIGHT, 0);
		else
			Rebound(tBall, 1, SCREEN_HEIGHT - BALL_HEIGHT, 0);			
	}
	
	tBall->nX = tBall->nX + tBall->nXDir;

	if(tBall->nXDir < 0) //ball moving to left
	{
		if (tPoderesDefensa[0].activarPoder == 0)
		{
			tPoderesDefensa[0].activarPoder = playerPowerPressed[0][0];
			playerPowerPressed[0][0] = 0;
		}
		tPoderesDefensa[1].activarPoder = 0;

		switch (tPoderesDefensa[0].activarPoder)
		{
			case 1:
				//Rayo Destructor
				PintarRayo(0,1);
				if (tBall->nY >= aPlayer[0].nY + 11 && tBall->nY < aPlayer[0].nY + PLAYER_HEIGHT - 11)
                                          {
					DestruirBola(tBall);
                                                    nReturn = 0;
                                          }
				break;
			case 2:
				if (tBall->nX == aPlayer[0].nX)
					//Invulnerabilidad
					bCollision = Invulnerabilidad(0);	
				break;
			case 3:
				if (tBall->nX < SCREEN_WIDTH / 2)
					//Devolver bola
					tBall->nXDir *= -1;	
				break;
			case 4:
				if (tBall->nX < SCREEN_WIDTH / 2)
					//Agujeros de gusano
					tPoderesDefensa[0].presionadoTuneles = 1;
				break;
			case 7:				
				//Bola reptadora
				if (tPoderesDefensa[0].contRayoTractor > 0)
				{
					PintarRayo(0,0);
					if (tBall->nY >= aPlayer[0].nY + 11 && tBall->nY < aPlayer[0].nY + PLAYER_HEIGHT - 11)
						AtraerBola(tBall,0);
				}
				break;
			case 9:
				if ((tBall->nX < aPlayer[0].nX + PLAYER_WIDTH + 3*PLAYER_HEIGHT) 
					&& ((tBall->nY > aPlayer[0].nY - PLAYER_HEIGHT && tBall->nY < aPlayer[0].nY + 2*PLAYER_HEIGHT)
					|| (tBall->nY + BALL_HEIGHT > aPlayer[0].nY - PLAYER_HEIGHT && tBall->nY + BALL_HEIGHT < aPlayer[0].nY + 2*PLAYER_HEIGHT)))
					//Campo repulsivo
					CampoRepulsivo(tBall, 0);
				break;
			case 10:
				if ((tBall->nX < aPlayer[0].nX + PLAYER_WIDTH + 3*PLAYER_HEIGHT) 
					&& ((tBall->nY > aPlayer[0].nY - PLAYER_HEIGHT && tBall->nY < aPlayer[0].nY + 2*PLAYER_HEIGHT)
					|| (tBall->nY + BALL_HEIGHT > aPlayer[0].nY - PLAYER_HEIGHT && tBall->nY + BALL_HEIGHT < aPlayer[0].nY + 2*PLAYER_HEIGHT)))
					//Campo atractivo
					CampoAtractivo(tBall, 0);
		}

		if ((tPoderesDefensa[0].presionadoTuneles == 1) && (tBall->nX < 0))
			//Agujeros de gusano
			AgujerosGusano(tBall, 0);

		if ((tPoderesAtaque[1].activarPoder == 8) && (tBall->nX < SCREEN_WIDTH / 2))
			//Barrera contra el oponente
			tPoderesAtaque[0].rebotesRestantes = 3;		

		Barrera(tBall, 1);

		if(tBall->nX == aPlayer[0].nX + PLAYER_WIDTH)
		{
			if((tBall->nY > aPlayer[0].nY && tBall->nY < aPlayer[0].nY + PLAYER_HEIGHT) ||
				 (tBall->nY + BALL_HEIGHT > aPlayer[0].nY && tBall->nY + BALL_HEIGHT < aPlayer[0].nY + PLAYER_HEIGHT) || //Hacer bola grande
				 ((tPoderesDefensa[0].activarPoder == 5) && (tBall->nY + BIG_BALL_HEIGHT > aPlayer[0].nY && tBall->nY + BIG_BALL_HEIGHT < aPlayer[0].nY + PLAYER_HEIGHT)) || //Hacer barra grande
				 ((tPoderesDefensa[0].activarPoder == 6) && (tBall->nY + BALL_HEIGHT > aPlayer[0].nY && tBall->nY + BALL_HEIGHT < aPlayer[0].nY + BIG_PLAYER_HEIGHT)) || 
				 ((tPoderesDefensa[0].activarPoder == 6) && (tBall->nY > aPlayer[0].nY && tBall->nY < aPlayer[0].nY + BIG_PLAYER_HEIGHT)))
			{		
				bCollision = BallPlayerContact(tBall,0,8);

				haHechoContactoPlayer[0] = 1;
				haHechoContactoPlayer[1] = 0;

				PowersToZero();
			}
		}
		else if ((tBall->nX < aPlayer[0].nX + PLAYER_WIDTH) && (tBall->nX > 0))
		{
			if (haHechoContactoPlayer[0] == 0)
			{
				if (((tBall->nY <= aPlayer[0].nY + PLAYER_HEIGHT) && (tBall->nY + BALL_HEIGHT >= aPlayer[0].nY)) || //Hacer bola grande
					((tPoderesDefensa[0].activarPoder == 5) && (tBall->nY <= aPlayer[0].nY + PLAYER_HEIGHT) && (tBall->nY + BIG_BALL_HEIGHT >= aPlayer[0].nY)) || //Hacer barra grande
					((tPoderesDefensa[0].activarPoder == 6) && (tBall->nY <= aPlayer[0].nY + BIG_PLAYER_HEIGHT) && (tBall->nY + BALL_HEIGHT >= aPlayer[0].nY))) 
				{
					deCanto = 1;

					bCollision = BallPlayerContact(tBall,0,8);

					PowersToZero();
				}
				haHechoContactoPlayer[1] = 0;
			}		
		}
		else if (tBall->nX < 0)
		{
			tBall->nX = 0;

			nReturn = BallOut(tBall,0);

			PowersToZero();
		}
		if (((tPoderesDefensa[0].contRayoDestructor > 0) || (tPoderesDefensa[0].contRayoTractor > 0)) && 
			(tPoderesDefensa[0].activarPoder == 0))
			Desbloquear(0);
	}
	else //ball moving to right
	{
		tPoderesDefensa[0].activarPoder = 0;
		if (tPoderesDefensa[1].activarPoder == 0)
		{
			tPoderesDefensa[1].activarPoder = playerPowerPressed[1][0];
			playerPowerPressed[1][0] = 0;
		}

		switch (tPoderesDefensa[1].activarPoder)
		{
			case 1:
				//Rayo Destructor
				PintarRayo(1,1);				
				if (tBall->nY >= aPlayer[1].nY + 11 && tBall->nY < aPlayer[1].nY + PLAYER_HEIGHT - 11)
                                          {
					DestruirBola(tBall);	
                                                    nReturn = 1;		
                                          }
				break;				
			case 2:
				if (tBall->nX == aPlayer[1].nX + PLAYER_WIDTH - BALL_WIDTH)
					//Invulnerabilidad
					bCollision = Invulnerabilidad(1);			
				break;
			case 3:
				if (tBall->nX > SCREEN_WIDTH / 2)
					//Devolver bola
					tBall->nXDir *= -1;	
				break;
			case 4:
				if (tBall->nX > SCREEN_WIDTH / 2)
					//Agujeros de gusano
					tPoderesDefensa[1].presionadoTuneles = 1;
				break;
			case 7:
				//Bola reptadora
				if (tPoderesDefensa[1].contRayoTractor > 0)
				{
					PintarRayo(1,0);
					if (tBall->nY >= aPlayer[1].nY + 11 && tBall->nY < aPlayer[1].nY + PLAYER_HEIGHT - 11)		
						AtraerBola(tBall,1);
				}
				break;
			case 9:
				if ((tBall->nX + BALL_WIDTH > aPlayer[1].nX - 3*PLAYER_HEIGHT) 
					&& ((tBall->nY > aPlayer[1].nY - PLAYER_HEIGHT && tBall->nY < aPlayer[1].nY + 2*PLAYER_HEIGHT)
					|| (tBall->nY + BALL_HEIGHT > aPlayer[1].nY - PLAYER_HEIGHT && tBall->nY + BALL_HEIGHT < aPlayer[1].nY + 2*PLAYER_HEIGHT)))	 
					//Campo repulsivo
					CampoRepulsivo(tBall, 1);			
				break;
			case 10:				
				if ((tBall->nX + BALL_WIDTH > aPlayer[1].nX - 3*PLAYER_HEIGHT) 
					&& ((tBall->nY > aPlayer[1].nY - PLAYER_HEIGHT && tBall->nY < aPlayer[1].nY + 2*PLAYER_HEIGHT)
					|| (tBall->nY + BALL_HEIGHT > aPlayer[1].nY - PLAYER_HEIGHT && tBall->nY + BALL_HEIGHT < aPlayer[1].nY + 2*PLAYER_HEIGHT)))
					//Campo atractivo
					CampoAtractivo(tBall, 1);
		}

		if ((tPoderesDefensa[1].presionadoTuneles == 1) && (tBall->nX + BALL_WIDTH > SCREEN_WIDTH))
			AgujerosGusano(tBall, 1);


		if ((tPoderesAtaque[0].activarPoder == 8) && (tBall->nX + BALL_WIDTH > SCREEN_WIDTH / 2))
			//Barrera contra el oponente
			tPoderesAtaque[1].rebotesRestantes = 3;

		Barrera(tBall, 0);

		if ((tBall->nX + BALL_WIDTH == aPlayer[1].nX) || //Hacer bola grande
		 	((tPoderesDefensa[1].activarPoder == 5) && (tBall->nX + BIG_BALL_WIDTH == aPlayer[1].nX)))
		{
			if((tBall->nY > aPlayer[1].nY && tBall->nY < aPlayer[1].nY + PLAYER_HEIGHT) ||
				 (tBall->nY + BALL_HEIGHT > aPlayer[1].nY && tBall->nY + BALL_HEIGHT < aPlayer[1].nY + PLAYER_HEIGHT) || //Hacer bola grande
				 ((tPoderesDefensa[1].activarPoder == 5) && (tBall->nY + BIG_BALL_HEIGHT > aPlayer[1].nY && tBall->nY + BIG_BALL_HEIGHT < aPlayer[1].nY + PLAYER_HEIGHT)) || //Hacer barra grande
				 ((tPoderesDefensa[1].activarPoder == 6) && (tBall->nY > aPlayer[1].nY && tBall->nY < aPlayer[1].nY + BIG_PLAYER_HEIGHT)) ||
				 ((tPoderesDefensa[1].activarPoder == 6) && (tBall->nY + BALL_HEIGHT > aPlayer[1].nY && tBall->nY + BALL_HEIGHT < aPlayer[1].nY + BIG_PLAYER_HEIGHT)))
			{				
				bCollision = BallPlayerContact(tBall,1,7);

				haHechoContactoPlayer[0] = 0;
				haHechoContactoPlayer[1] = 1;

				PowersToZero();
			}
		}
		else if (((tBall->nX + BALL_WIDTH > aPlayer[1].nX) && (tBall->nX + BALL_WIDTH < SCREEN_WIDTH)) || //Hacer bola grande
			((tPoderesDefensa[1].activarPoder == 5) && (tBall->nX + BIG_BALL_WIDTH > aPlayer[1].nX) && (tBall->nX + BIG_BALL_WIDTH < SCREEN_WIDTH)))
		{
			if (haHechoContactoPlayer[1] == 0)
			{
				if (((tBall->nY <= aPlayer[1].nY + PLAYER_HEIGHT) && (tBall->nY + BALL_HEIGHT >= aPlayer[1].nY)) || //Hacer bola grande
					((tPoderesDefensa[1].activarPoder == 5) && (tBall->nY <= aPlayer[1].nY + PLAYER_HEIGHT) && (tBall->nY + BIG_BALL_HEIGHT >= aPlayer[1].nY)) || //Hacer barra grande
					((tPoderesDefensa[1].activarPoder == 6) && (tBall->nY <= aPlayer[1].nY + BIG_PLAYER_HEIGHT) && (tBall->nY + BALL_HEIGHT >= aPlayer[1].nY))) 
				{
					deCanto = 1;

					bCollision = BallPlayerContact(tBall,1,7);

					PowersToZero();
				}
				haHechoContactoPlayer[0] = 0;					
			}
		}
		else if ((tBall->nX + BALL_WIDTH > SCREEN_WIDTH) || //Hacer bola grande
			((tPoderesAtaque[1].activarPoder == 5) && (tBall->nX + BIG_BALL_WIDTH > SCREEN_WIDTH)))
		{
			if (tPoderesAtaque[1].activarPoder == 5)
				//Hacer bola grande
				tBall->nX = SCREEN_WIDTH - BIG_BALL_WIDTH;
			else
				tBall->nX = SCREEN_WIDTH - BALL_WIDTH;

			nReturn = BallOut(tBall,1);

			PowersToZero();
		}

		if (((tPoderesDefensa[1].contRayoDestructor > 0) || (tPoderesDefensa[1].contRayoTractor > 0)) && 
			(tPoderesDefensa[1].activarPoder == 0))
			Desbloquear(1);
	}

	if(bCollision != -1)
	{
		if ((tPoderesAtaque[0].bolaAgitada == 3) || (tPoderesAtaque[1].bolaAgitada == 3))
		{
			//Bola con Efecto
			tBall->nXDir *= -1;
			tBall->nYDir /= 2;
			tPoderesAtaque[0].bolaAgitada = 0;
			tPoderesAtaque[1].bolaAgitada = 0;
		}
		else if ((tPoderesAtaque[0].bolaAgitada == 7) || (tPoderesAtaque[1].bolaAgitada == 7))
		{
			//Acelerar Bola
			tBall->nXDir /= -2;
			tBall->nYDir /= 2;
			tPoderesAtaque[0].bolaAgitada = 0;
			tPoderesAtaque[1].bolaAgitada = 0;
		}
		else
		{
			tBall->nXDir *= -1;

			if (deCanto == 1)
			{
				tBall->nYDir *= -1;
				deCanto = 0;
			}
		}

		if(aPlayer[bCollision].nLastYMove != 0)
		{
			tBall->nYDir = aPlayer[bCollision].nLastYMove > 0 ? tBall->nYDir - 1 : tBall->nYDir + 1;

			if(tBall->nYDir > 0)
			{
				tBall->nYDir = MIN(tBall->nYDir , 8);
				tBall->nYDir = MAX(tBall->nYDir , 4);
			}
			else
			{
				tBall->nYDir = MIN(tBall->nYDir , -4);
				tBall->nYDir = MAX(tBall->nYDir , -8);
			}
		}

		if (tPoderesAtaque[0].invertidoRalentizadoParpadeante == 3)
			tPoderesAtaque[0].invertidoRalentizadoParpadeante = 0;

		if (tPoderesAtaque[1].invertidoRalentizadoParpadeante == 3)
			tPoderesAtaque[1].invertidoRalentizadoParpadeante = 0;
	}

	if ((tPoderesAtaque[0].activarPoder == 2) || (tPoderesAtaque[1].activarPoder == 2))
	{
		//Controles invertidos
		if ((rand() % 2) == 0)
			DrawOrDeleteBall(tBall,2);
	}

	else if ((tPoderesAtaque[0].bolaAgitada == 3) || (tPoderesAtaque[1].bolaAgitada == 3)) 
		//Bola con efecto
		DrawOrDeleteBall(tBall,1);

	else if ((tPoderesAtaque[0].activarPoder == 5) || (tPoderesAtaque[1].activarPoder == 5))
	{
		//Bola eléctrica
		if ((rand() % 2) == 0)
			DrawOrDeleteBall(tBall,1);
	}

	else if ((tPoderesAtaque[0].bolaAgitada == 7) || (tPoderesAtaque[1].bolaAgitada == 7)) 
		//Acelerar bola
		DrawOrDeleteBall(tBall,2);

	else if ((tPoderesAtaque[0].activarPoder == 6) || (tPoderesAtaque[1].activarPoder == 6))
	{
		//Bola fantasma
		if ((rand() % 2) == 0)
			DrawOrDeleteBall(tBall,0);
	}

	else if ((tPoderesAtaque[0].activarPoder == 1) || (tPoderesAtaque[1].activarPoder == 1)) 
	{
		//Bola intermitente
		if ((rand() % 10) == 0)
			DrawOrDeleteBall(tBall,0);
	}

	else if ((tPoderesAtaque[0].activarPoder == 4) || (tPoderesAtaque[1].activarPoder == 4))
		//Pelota de despiste
		PutSprite((unsigned char *)0xC000 + ((tBall->nY / 8) * 80) + ((tBall->nY % 8) * 2048) + (tBall->nX / 8), BIG_BALL_WIDTH_BYTES, BIG_BALL_HEIGHT, bigBallSprite[1]);
	
	else if ((tPoderesDefensa[0].activarPoder == 5) || (tPoderesDefensa[1].activarPoder == 5))
		//Hacer bola grande
		PutSprite((unsigned char *)0xC000 + ((tBall->nY / 8) * 80) + ((tBall->nY % 8) * 2048) + (tBall->nX / 8), BIG_BALL_WIDTH_BYTES, BIG_BALL_HEIGHT, bigBallSprite[0]);
	
          else if ((tPoderesDefensa[0].nYDirAntiguo != 0) || (tPoderesDefensa[1].nYDirAntiguo != 0))
                    //Rayo tractor
                    DrawOrDeleteBall(tBall,1);

	else
	          DrawOrDeleteBall(tBall,0);
	
	return nReturn;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
void ShowMenuRules2(char *volver);
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenuRules
////////////////////////////////////////////////////////////////////////
void ShowMenuRules(char *volver)
{
	SetMode(1);

	SetCursor(9, 2);
	printf("PONG TROLL - REGLAS (1):");

	SetCursor(1, 4);
	printf(" 1. Las barras de cada jugador solo");

	SetCursor(1, 6);
	printf("    cobran movimiento cuando la bola va");

	SetCursor(1, 8);
	printf("    en sentido a la propia porteria.");

	SetCursor(1, 11);
	printf(" 2. La velocidad de la bola y de las");

	SetCursor(1, 13);
	printf("    barras se incrementara con la ronda.");

	SetCursor(1, 16);
	printf(" 3. Cada poder puede ser ejecutado una");

	SetCursor(1, 18);
	printf("    sola vez por ronda.");

	SetCursor(8, 22);
	printf("Pulsa 'Delete' para Volver");

	SetCursor(8, 24);
	printf("Pulsa 'Return' para Continuar");

	while((!IsKeyPressedFW(Key_Return)) && (!IsKeyPressedFW(Key_Del))) {}

	if (IsKeyPressedFW(Key_Del))
		*volver = 1;
	else if (IsKeyPressedFW(Key_Return))
	{
		Sonido(2);
		ShowMenuRules2(volver);
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenuRules2
////////////////////////////////////////////////////////////////////////
void ShowMenuRules2(char *volver)
{
	SetMode(1);

	SetCursor(9, 2);
	printf("PONG TROLL - REGLAS (2):");

	SetCursor(1, 4);
	printf(" 4. El poder de defensa debe ejecutarse");

	SetCursor(1, 6);
	printf("    en cualquier momento en que la bola");

	SetCursor(1, 8);
	printf("    vaya hacia nuestra porteria.");	

	SetCursor(1, 11);
	printf(" 5. Para hacer efectivo el poder de");

	SetCursor(1, 13);
	printf("    ataque, es necesario 'pulsarlo'");

	SetCursor(1, 15);
	printf("    antes de que la bola rebote con las");

	SetCursor(1, 17);
	printf("    paredes superior o inferior, en");

	SetCursor(1, 19);
	printf("    sentido a la porteria contraria.");

	SetCursor(8, 22);
	printf("Pulsa 'Delete' para Volver");

	SetCursor(8, 24);
	printf("Pulsa 'Return' para Continuar");

	while((!IsKeyPressedFW(Key_Del)) && (!IsKeyPressedFW(Key_Return))) {}

	if (IsKeyPressedFW(Key_Return))
		*volver = 1;
	else if (IsKeyPressedFW(Key_Del))
	{
		Sonido(2);
		ShowMenuRules(volver);	
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenuMode
////////////////////////////////////////////////////////////////////////
void ShowMenuMode(char *volver)
{
	SetMode(1);

	SetCursor(15, 2);
	printf("PONG TROLL");

	SetCursor(1, 6);
	printf("  Elige el Modo de Juego (Elegido: '%d')", eleccion);

	SetCursor(6, 9);
	printf("1: Facil, Al Mejor de 3 Rondas");

	SetCursor(6, 12);
	printf("2: Facil, Al Mejor de 5 Rondas");

	SetCursor(6, 15);
	printf("3: Dificil, Al Mejor de 3 Rondas");	
	
	SetCursor(6, 18);
	printf("4: Dificil, Al Mejor de 5 Rondas");

	SetCursor(6, 22);
	printf("Pulsa 'Delete' para Volver");

	while((!IsKeyPressedFW(Key_Del)) && (!IsKeyPressedFW(Key_1)) && (!IsKeyPressedFW(Key_2)) 
		&& (!IsKeyPressedFW(Key_3)) && (!IsKeyPressedFW(Key_4))) {}

	*volver = 1;
	if (IsKeyPressedFW(Key_1))
	{
		numRondas = 3;
		dificultad = 0;
                     eleccion = 1;
	}
	else if (IsKeyPressedFW(Key_2))
	{
		numRondas = 5;
		dificultad = 0;
                     eleccion = 2;
	}
	else if (IsKeyPressedFW(Key_3))
	{
		numRondas = 3;
		dificultad = 1;
                     eleccion = 3;
	}
	else if (IsKeyPressedFW(Key_4))
	{
		numRondas = 5;
		dificultad = 1;
                     eleccion = 4;
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenu1
////////////////////////////////////////////////////////////////////////
void ShowMenu1(char *volver)
{
	SetMode(1);

	SetCursor(15, 2);
	printf("PONG TROLL");

	SetCursor(1, 5);
	printf("   JUGADOR 1       VS       JUGADOR 2");

	SetCursor(1, 7);
	printf("    'W'-'S'  < MOVIMIENTO >  '%c'-'%c'", 240, 241);

	SetCursor(1, 9);
	printf("    'A'-'D'    < PODERES >   '%c'-'%c'", 242, 243);
 
 	SetCursor(1, 12);
	printf("   'Space' => Pausar el Juego");

	SetCursor(1, 14);
	printf("   'Esc'   => Salir del Juego");

	SetCursor(1, 17);
	printf("   Pulsa 'R' para Ver las Reglas");	

	SetCursor(1, 19);
	printf("   Pulsa 'C' para Configurar el Juego");

	SetCursor(1, 21);
	printf("   Pulsa 'Return' para Continuar");

	SetCursor(10, 24);
	printf("Siul - DLabs - 2016");

	while((!IsKeyPressedFW(Key_Return)) && (!IsKeyPressedFW(Key_R_Joy2Left)) && (!IsKeyPressedFW(Key_C))) {}

	if (IsKeyPressedFW(Key_R_Joy2Left))
	{
		Sonido(2);
		ShowMenuRules(volver);
	}
	else if (IsKeyPressedFW(Key_C))
	{
		Sonido(2);
		ShowMenuMode(volver);
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//WritePower
////////////////////////////////////////////////////////////////////////
void WritePowerDefense(char power)
{	
	if (power == 1)
		printf(" Rayo Destructor ");
	else if (power == 2)
		printf("Inmortalidad Temp");
	else if (power == 3)
		printf("  Devolver Bola  ");
	else if (power == 4)
		printf("   Tuneles E-T   ");
	else if (power == 5)
		printf("  Agrandar Bola  ");
	else if (power == 6)
		printf("  Estirar Barra  ");
	else if (power == 7)
		printf("  Rayo Tractor   ");
	else if (power == 8)
		printf(" Ralentizar Bola ");
	else if (power == 9)
		printf(" Campo Repulsivo ");
	else if (power == 10)
		printf(" Campo Atractivo ");
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//WritePower
////////////////////////////////////////////////////////////////////////
void WritePowerAttack(char power)
{	
	if (power == 1)
		printf("Bola Intermitente");
	else if (power == 2)
		printf("Contr Invertidos ");
	else if (power == 3)
		printf(" Bola con Efecto ");
	else if (power == 4)
		printf("Bola Traicionera ");
	else if (power == 5)
		printf(" Bola Electrica  ");
	else if (power == 6)
		printf("  Bola Fantasma  ");
	else if (power == 7)
		printf("  Acelerar Bola  ");
	else if (power == 8)
		printf("     Barrera     ");
	else if (power == 9)
		printf("Barra Parpadeante");
	else if (power == 10)
		printf(" Bola Divergente ");
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenu2
////////////////////////////////////////////////////////////////////////
void ShowMenu2(char *volver)
{
	SetMode(1);

	SetCursor(15, 2);
	printf("PONG TROLL");

	SetCursor(4, 4);
	printf("Numero de Rondas:                %d", numRondas);

	SetCursor(4, 6);
	printf("Maxima Puntuacion por Ronda:     %d", MAX_SCORE);

	SetCursor(4, 8);
	if (dificultad == 0)
		printf("Dificultad:                  FACIL");
	else
		printf("Dificultad:                DIFICIL");

	SetCursor(1, 11);
	printf("                PODERES");

	SetCursor(1, 13);
	printf("     JUGADOR 1     |     JUGADOR 2");
	
	SetCursor(1, 15);
	printf("        'A'   < DEFENSA >   '%c'", 243);

	SetCursor(2, 17);
	WritePowerDefense(aPlayer[0].powers[0]);

	SetCursor(19, 17);
	printf(" | ");	

	SetCursor(22, 17);
	WritePowerDefense(aPlayer[1].powers[0]);

	SetCursor(1, 19);
	printf("        'D'   < ATAQUE >    '%c'", 242);

	SetCursor(2, 21);
	WritePowerAttack(aPlayer[0].powers[1]);

	SetCursor(19, 21);
	printf(" | ");

	SetCursor(22, 21);	
	WritePowerAttack(aPlayer[1].powers[1]);

	SetCursor(1, 24);
	printf("'Delete' => Volver | 'Return' => Empezar");

	while((!IsKeyPressedFW(Key_Return)) && (!IsKeyPressedFW(Key_Del))) {}

	if (IsKeyPressedFW(Key_Del))
		*volver = 1;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenuWinner
////////////////////////////////////////////////////////////////////////
void ShowMenuWinner(unsigned char nPlayer)
{
	SetMode(1);

	SetCursor(15, 2);
	printf("PONG TROLL");

	SetCursor(9, 6);
	if (nPlayer == 0)
		printf("El Jugador 1 (Izquierdo)");
	else
		printf(" El Jugador 2 (Derecho)");

	SetCursor(9, 9);
	printf("   ha ganado el Juego");

	SetCursor(9, 12);
	if (nPlayer == 0)
		printf("          %c", 248);
	else
		printf("            %c", 248);

	SetCursor(9, 13);
	if (nPlayer == 0)
		printf("          %c%c%c", 143, 215, 251);
	else
		printf("          %c%c%c", 250, 214, 143);

          SetCursor(1, 20);
          printf("   Agradecimientos a Lopenovi y Lugerh");

	SetCursor(9, 24);
	printf("Pulsa 'Return' para Salir");

	while(!IsKeyPressedFW(Key_Return)) {}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ShowMenuPartialWinner
////////////////////////////////////////////////////////////////////////
void ShowMenuPartialWinner(unsigned char nPlayer, unsigned char round)
{
	SetMode(1);

	SetCursor(15, 2);
		printf("PONG TROLL");

	SetCursor(9, 10);
	if (nPlayer == 0)
		printf("El Jugador 1 (Izquierdo)");
	else
		printf(" El Jugador 2 (Derecho)");

	SetCursor(9, 13);
	printf("  ha ganado la Ronda %d", round);

	SetCursor(9, 16);
	if (nPlayer == 0)
		printf("     %c           %c", 224, 225);
	else
		printf("     %c           %c", 225, 224);

	SetCursor(7, 24);
	printf("Pulsa 'Return' para Continuar");

	while(!IsKeyPressedFW(Key_Return)) {}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//InitGame
////////////////////////////////////////////////////////////////////////
void InitGame(char mode, char ronda)
{
	if (mode == 0)
	{
		aPlayer[0].nScore = 0;
		aPlayer[1].nScore = 0;
	}

	aPlayer[0].nY = 100 - PLAYER_HEIGHT / 2;
	aPlayer[1].nY = 100 - PLAYER_HEIGHT / 2;
	aPlayer[0].nX = 8;
	aPlayer[1].nX = (SCREEN_WIDTH - 8) - PLAYER_WIDTH;

	aPlayer[0].ekeyUp = Key_W;
	aPlayer[1].ekeyUp = Key_CursorUp;
	aPlayer[0].ekeyDown = Key_S;
	aPlayer[1].ekeyDown = Key_CursorDown;
	aPlayer[0].ekeyLeft = Key_A;
	aPlayer[1].ekeyLeft = Key_CursorLeft;
	aPlayer[0].ekeyRight = Key_D;
	aPlayer[1].ekeyRight = Key_CursorRight;

	memset(&tTrueBall, 0, sizeof(tTrueBall));
	memset(&tFalseBall, 0, sizeof(tFalseBall));

	if ((rand() % 2) == 0)
	{
		tTrueBall.nX = aPlayer[0].nX + PLAYER_WIDTH + 96;
		tTrueBall.nY = SCREEN_HEIGHT / 2;

		if (numRondas == 3)
		{
			tTrueBall.nYDir = -4 * ((rand() % ronda) + 1);
			tTrueBall.nXDir = 8 * ((rand() % ronda) / 2 + 1 + dificultad);	
		}
		else if (numRondas == 5)
		{
			tTrueBall.nYDir = -4 * ((rand() % ronda) / 2 + 1);
			tTrueBall.nXDir = 8 * ((rand() % ronda) / 4 + 1 + dificultad);	
		}	
	}
	else
	{
		tTrueBall.nX = aPlayer[1].nX - BALL_WIDTH - 96;
		tTrueBall.nY = SCREEN_HEIGHT / 2;
		
		if (numRondas == 3)
		{
			tTrueBall.nYDir = 4 * ((rand() % ronda) + 1);
			tTrueBall.nXDir = -8 * ((rand() % ronda) / 2 + 1 + dificultad);	
		}
		else if (numRondas == 5)
		{
			tTrueBall.nYDir = 4 * ((rand() % ronda) / 2 + 1);
			tTrueBall.nXDir = -8 * ((rand() % ronda) / 4 + 1 + dificultad);	
		}	
	}

	SetMode(1);

  	DrawPlayer(0);
  	DrawPlayer(1);

  	pScoreScreen[0] = (unsigned char *)0xC000 + ((SCORE_Y / 8) * 80) + ((SCORE_Y % 8) * 2048) + (SCORE_X_1 / 8);
	pScoreScreen[1] = (unsigned char *)0xC000 + ((SCORE_Y / 8) * 80) + ((SCORE_Y % 8) * 2048) + (SCORE_X_2 / 8);

	pNumRoundsScreen[0] = (unsigned char *)0xC000 + ((SCORE_Y_ROUNDS / 8) * 80) + ((SCORE_Y_ROUNDS % 8) * 2048) + (SCORE_X_1_ROUNDS / 8);
	pNumRoundsScreen[1] = (unsigned char *)0xC000 + ((SCORE_Y_ROUNDS / 8) * 80) + ((SCORE_Y_ROUNDS % 8) * 2048) + (SCORE_X_2_ROUNDS / 8);
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//BolaDivergente
////////////////////////////////////////////////////////////////////////
void BolaDivergente(struct _tBall *tBall, unsigned char nPlayer)
{
	if (tPoderesAtaque[nPlayer].activarPoder == 10)
	{
		if (tPoderesAtaque[nPlayer].pelotaFalsaMovimiento == 0)
		{
			tFalseBall.nX = tBall->nX;
			tFalseBall.nY = tBall->nY;
			if ((rand() % 2) == 0)
			{
				tFalseBall.nYDir = tBall->nYDir * 2;
			}
			else
			{
				tFalseBall.nYDir = tBall->nYDir;
				tBall->nYDir *= 2;
			}
			tFalseBall.nXDir = tBall->nXDir;
		}

		tPoderesAtaque[nPlayer].pelotaFalsaMovimiento = 1;
		MoveBall(&tFalseBall);
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//PressPower
////////////////////////////////////////////////////////////////////////
char PressPower(char firstUsePlayer, unsigned char nPlayer, unsigned char nPower, unsigned char ekey)
{
	char teclaPulsada = IsKeyPressedFW(ekey);

	if (firstUsePlayer == 1)
	{
		if (teclaPulsada)
		{
			Sonido(3);
			playerPowerPressed[nPlayer][nPower] = aPlayer[nPlayer].powers[nPower];
                               return 0;
		}
		else
                               return 1;
	}
	else
		return 0;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Parpadeante
////////////////////////////////////////////////////////////////////////
void Parpadeante (unsigned char nPlayer)
{
	if (tPoderesAtaque[nPlayer].activarPoder == 9)
		//Jugador parpadeante
		tPoderesAtaque[1-nPlayer].invertidoRalentizadoParpadeante = 3;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//RayoDestructor
////////////////////////////////////////////////////////////////////////
void RayoDestructor (unsigned char nPlayer)
{
	if (tPoderesDefensa[nPlayer].activarPoder == 1)
	{
		tPoderesDefensa[nPlayer].contRayoDestructor++;
		if (tPoderesDefensa[nPlayer].contRayoDestructor >= 40)				
			tPoderesDefensa[nPlayer].activarPoder = 0;
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//RayoTractor
////////////////////////////////////////////////////////////////////////
void RayoTractor (unsigned char nPlayer)
{
	if (tPoderesDefensa[nPlayer].activarPoder == 7)
	{
		tPoderesDefensa[nPlayer].contRayoTractor++;
		if (tPoderesDefensa[nPlayer].contRayoTractor >= 40)				
			tPoderesDefensa[nPlayer].activarPoder = 0;
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//RalentizarBola
////////////////////////////////////////////////////////////////////////
char RalentizarBola (struct _tBall *tBall, unsigned char nPlayer)
{
	//Ralentizar bola
    tPoderesDefensa[nPlayer].contRalentizacionBola++;

	if (tPoderesDefensa[nPlayer].contRalentizacionBola % 3 == 0)				
		return MoveBall(tBall);
	else 
		return -1;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//EstirarBarra
////////////////////////////////////////////////////////////////////////
void EstirarBarra (unsigned char nPlayer)
{
	if (playerPowerPressed[nPlayer][0] == 1)
	{
		//Estirar Barra
		DrawOrDeleteBigPlayer(nPlayer,0x0F);
		playerPowerPressed[nPlayer][0] = 0;
	}
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//ActivarPoderesYMover
////////////////////////////////////////////////////////////////////////
char ActivarPoderesYMover (unsigned char nPlayer, char player_y_inc)
{
	char nWinner = -1;

	if (tPoderesDefensa[nPlayer].activarPoder == 8)
		//Ralentizar bola
		nWinner = RalentizarBola(&tTrueBall, nPlayer);
	else
		nWinner = MoveBall(&tTrueBall);	

	RayoDestructor(nPlayer);

	EstirarBarra(nPlayer);

	RayoTractor(nPlayer);

	Parpadeante(1-nPlayer);	

	BolaDivergente(&tTrueBall, 1-nPlayer);
	
	if (tPoderesAtaque[1-nPlayer].pelotaFalsaMovimiento == -1)
		DrawOrDeleteBall(&tFalseBall,3);

	if (tPoderesAtaque[nPlayer].invertidoRalentizadoParpadeante == 2)
		MovePlayer(nPlayer,player_y_inc/2);
	else
		MovePlayer(nPlayer,player_y_inc);

	return nWinner;
}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//Game
////////////////////////////////////////////////////////////////////////
void Game()
{
	char ronda = 1;
	char firstUsePlayer[2][2] = {{1,1},{1,1}};
	char pausar = 0;
	char player_y_inc = 0;
	char nWinner = -1;

	playerPowerPressed[0][0] = 0;
	playerPowerPressed[0][1] = 0;
	playerPowerPressed[1][0] = 0;
	playerPowerPressed[1][1] = 0;

	haHechoContactoPlayer[0] = 0;
	haHechoContactoPlayer[1] = 0;

	memset(tPoderesDefensa, 0, sizeof(tPoderesDefensa));
	memset(tPoderesAtaque, 0, sizeof(tPoderesAtaque));

	player_y_inc = 2 + 2*dificultad + 3*ronda;

	InitGame(0,ronda);	
	Sonido(1);

	while(!IsKeyPressedFW(Key_Esc))
	{
		WaitVsync();

		pausar = IsKeyPressedFW(Key_Space);

		while (pausar == 1)
		{
			// Huevo de Pascua
			if (IsKeyPressedFW(Key_Return))
			{
				Sonido(3);
				aPlayer[0].nRounds = rand() % (numRondas/2 + 1);
				aPlayer[1].nRounds = rand() % (numRondas/2 + 1);
				ronda = aPlayer[0].nRounds + aPlayer[1].nRounds + 1;				
			}
			else if (IsKeyPressedFW(Key_Del))
				pausar = 0;
		}

		firstUsePlayer[0][0] = PressPower(firstUsePlayer[0][0],0,0,aPlayer[0].ekeyLeft);
		firstUsePlayer[0][1] = PressPower(firstUsePlayer[0][1],0,1,aPlayer[0].ekeyRight);
		firstUsePlayer[1][0] = PressPower(firstUsePlayer[1][0],1,0,aPlayer[1].ekeyRight);
		firstUsePlayer[1][1] = PressPower(firstUsePlayer[1][1],1,1,aPlayer[1].ekeyLeft);
		
		if (tTrueBall.nXDir < 0)
			nWinner = ActivarPoderesYMover(0,player_y_inc);
		else
			nWinner = ActivarPoderesYMover(1,player_y_inc);
		
		PutSprite(pScoreScreen[0], NUMBER_WIDTH_BYTES, NUMBER_HEIGHT, aNumberSprite[aPlayer[0].nScore]);
		PutSprite(pScoreScreen[1], NUMBER_WIDTH_BYTES, NUMBER_HEIGHT, aNumberSprite[aPlayer[1].nScore]);

		PutSprite(pNumRoundsScreen[0], NUMBER_R_WIDTH_BYTES, NUMBER_R_HEIGHT, rNumberSprite[aPlayer[0].nRounds]);
		PutSprite(pNumRoundsScreen[1], NUMBER_R_WIDTH_BYTES, NUMBER_R_HEIGHT, rNumberSprite[aPlayer[1].nRounds]);

		if(nWinner != -1)
		{					
			if (nWinner != 4)
				//No es pelota de despiste
				aPlayer[nWinner].nScore++;	

			if(aPlayer[nWinner].nScore == MAX_SCORE)
			{
				if (nWinner == 0)
					aPlayer[0].nRounds++;
				else
					aPlayer[1].nRounds++;

				ShowMenuPartialWinner(nWinner,ronda);
				Sonido(2);

				ronda++;

				if (numRondas == 3)
					player_y_inc += 2;
				else if (numRondas == 5)
					player_y_inc += 1;

				if ((aPlayer[0].nRounds <= numRondas/2) && (aPlayer[1].nRounds <= numRondas/2))
				{
					firstUsePlayer[0][0] = 1;
					firstUsePlayer[0][1] = 1;
					firstUsePlayer[1][0] = 1;
					firstUsePlayer[1][1] = 1;

					InitGame(0,ronda);
					Sonido(1);
				}
			}
			else
			{
				InitGame(1,ronda);
				Sonido(1);
			}	

			if (aPlayer[0].nRounds > (numRondas/2)) 
			{
				ShowMenuWinner(0);
				Sonido(2);
				return;
			}
			else if (aPlayer[1].nRounds > (numRondas/2))
			{
				ShowMenuWinner(1);
				Sonido(2);
				return;
			}

			nWinner = -1;
		}

	}
	if (IsKeyPressedFW(Key_Esc))
		Sonido(4);

}
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//main
////////////////////////////////////////////////////////////////////////
void main()
{
	char volver = 0;
	char fijarPoderes = 0;

	numRondas = 3;
	dificultad = 0;
          eleccion = 1;

	while(1)
	{	
		SetMode(1);
		SetBorder(15);

		memset(aPlayer, 0, sizeof(aPlayer));

		volver = 0;
		fijarPoderes = 0;
		
		do
		{	
			if ((volver == 0) || (volver == 1)) 
			{
				volver = 0;
				ShowMenu1(&volver);
				Sonido(2);				
			}

			if (volver == 0)
			{
				volver = 0;		

				srand(GetTime());

				if (fijarPoderes == 0)
				{
					aPlayer[0].powers[0] = A10;
					aPlayer[0].powers[1] = A10;
					aPlayer[1].powers[0] = A10;
					aPlayer[1].powers[1] = A10;

					fijarPoderes = 1;
				}

				ShowMenu2(&volver);
				Sonido(2);
			}	
		} 
		while (volver != 0);

		Game();
	}
}
////////////////////////////////////////////////////////////////////////
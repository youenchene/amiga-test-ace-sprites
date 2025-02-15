#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/managers/joy.h>
#include <ace/utils/palette.h>
#include <ace/managers/advancedsprite.h> 
#include <ace/utils/font.h>
#include <ace/utils/custom.h>
#include <ace/managers/rand.h>


// Let's make code more readable by giving names to numbers
// It is a good practice to name constant stuff using uppercase
#define BALL_WIDTH 8
#define BALL_COLOR 1
#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 32
#define PADDLE_LEFT_COLOR 2
#define PADDLE_RIGHT_COLOR 3
#define SCORE_COLOR 1
#define WALL_HEIGHT 1
#define WALL_COLOR 1
#define PLAYFIELD_HEIGHT (256-32)
#define PADDLE_MAX_POS_Y (PLAYFIELD_HEIGHT - PADDLE_HEIGHT - 1)
#define PADDLE_SPEED 4
#define PADDLE_BG_BUFFER_WIDTH CEIL_TO_FACTOR(PADDLE_WIDTH, 16)
#define BALL_BG_BUFFER_WIDTH CEIL_TO_FACTOR(BALL_WIDTH, 16)
#define PADDLE_LEFT_BITMAP_OFFSET_Y 0
#define PADDLE_RIGHT_BITMAP_OFFSET_Y PADDLE_HEIGHT
#define BALL_BITMAP_OFFSET_Y (PADDLE_RIGHT_BITMAP_OFFSET_Y + PADDLE_HEIGHT)


static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
static tAdvancedSprite *s_pASprite4;
static tAdvancedSprite *s_pASprite6;
static tAdvancedSprite *s_pASprite0;
static tBitMap *s_pStripe;
static tBitMap *s_pStripe32;
static tBitMap *s_pStripe32;
static tBitMap *s_pStripe416;
static tBitMap *s_pStripe432;

tRandManager *g_sRand;

static tFont *s_pFont;
static tTextBitMap *s_pTextBitMap;

static int frame=0;

static int spritecontrol=0;

void gameGsCreate(void) {


  g_sRand=randCreate(456,876);

  s_pView = viewCreate(0, 
  TAG_VIEW_GLOBAL_PALETTE, 1,
  
  TAG_END);

  // Viewport for score bar - on top of screen
  s_pVpScore = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 5,
    TAG_VPORT_HEIGHT, 32,
  TAG_END);
  s_pScoreBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpScore,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
  TAG_END);

  // Now let's do the same for main playfield
  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 5,
  TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpMain,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
  TAG_END);

  s_pFont = fontCreateFromPath("data/fonts/silkscreen.fnt");
	s_pTextBitMap = fontCreateTextBitMap(320, s_pFont->uwHeight);

  paletteLoadFromPath("data/flappypal6.plt", s_pVpScore->pPalette, 32);



  s_pVpScore->pPalette[20]=  s_pVpScore->pPalette[0];
  s_pVpScore->pPalette[21]=  s_pVpScore->pPalette[1];
  s_pVpScore->pPalette[22]=  s_pVpScore->pPalette[2];
  s_pVpScore->pPalette[23]=  s_pVpScore->pPalette[3];
  s_pVpScore->pPalette[24]=  s_pVpScore->pPalette[4];
  s_pVpScore->pPalette[25]=  s_pVpScore->pPalette[5];
  s_pVpScore->pPalette[26]=  s_pVpScore->pPalette[6];
  s_pVpScore->pPalette[27]=  s_pVpScore->pPalette[7];
  s_pVpScore->pPalette[28]=  s_pVpScore->pPalette[8];
  s_pVpScore->pPalette[29]=  s_pVpScore->pPalette[9];
  s_pVpScore->pPalette[30]=  s_pVpScore->pPalette[10];
  s_pVpScore->pPalette[31]=  s_pVpScore->pPalette[11];
  s_pVpScore->pPalette[16]=  s_pVpScore->pPalette[12];
  s_pVpScore->pPalette[17]=  s_pVpScore->pPalette[13];
  s_pVpScore->pPalette[18]=  s_pVpScore->pPalette[14];
  s_pVpScore->pPalette[19]=  s_pVpScore->pPalette[15];


  // Draw line separating score VPort and main VPort, leave one line blank after it
  blitLine(
    s_pScoreBuffer->pBack,
    0, s_pVpScore->uwHeight - 2,
    s_pVpScore->uwWidth - 1, s_pVpScore->uwHeight - 2,
    SCORE_COLOR, 0xFFFF, 0 // Try patterns 0xAAAA, 0xEEEE, etc.
  );
 

  for(UWORD i=0; i < 16;i++)  {
     blitRect(s_pMainBuffer->pBack,16*i, 120, 16, 16,i);
  }
    
  for(UWORD i=0; i < 16;i++)  {
     blitRect(s_pMainBuffer->pBack,16*i, 136, 16, 16,16+i);
  }



  advancedSpriteManagerCreate(s_pView, 0);
  systemSetDmaBit(DMAB_SPRITE, 1);

  // 4 col 16px sprite
  s_pStripe = bitmapCreate(16, 32*10, 2, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 2BPP
  for(int i=0; i<10; i++) {
    char msg[50];
    sprintf(msg, "%d",i);
	  fontDrawStr(s_pFont,  s_pStripe, 0, i*32+0, msg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);
    for(int j=0; j<4; j++) {
      blitRect(s_pStripe,8*(j%2), i*32+8+8*((j-1>0)&1), randUwMinMax(g_sRand,4,8), randUwMinMax(g_sRand,4,8), j);
    }
  }
  s_pASprite4 = advancedSpriteAdd(4, s_pStripe, 32);
  advancedSpriteSetPos(s_pASprite4,80,100);

  // 4 col 32px sprite
  s_pStripe32 = bitmapCreate(32, 32*10, 2, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 2BPP
  for(int i=0; i<10; i++) {
    char msg[50];
    sprintf(msg, "%d",i);
	  fontDrawStr(s_pFont,  s_pStripe32, 0, i*32+0, msg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);
    for(int j=0; j<4; j++) {
      blitRect(s_pStripe32,16*(j%2), i*32+8+8*((j-1>0)&1), randUwMinMax(g_sRand,8,16), randUwMinMax(g_sRand,4,8), j);
    }
  }
  s_pASprite6 = advancedSpriteAdd(6, s_pStripe32, 32);
  advancedSpriteSetPos(s_pASprite6,180,100);

  // 16 col 16px sprite
  /*
  s_pStripe416 = bitmapCreate(16, 32*10, 4, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 4BPP
  for(int i=0; i<10; i++) {
    char msg[50];
    sprintf(msg, "%d",i);
	  fontDrawStr(s_pFont,  s_pStripe416, 0, i*32+0, msg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);
    for(int j=0; j<4; j++) {
      blitRect(s_pStripe416,0, i*32+8+4*j, randUwMinMax(g_sRand,2,4), randUwMinMax(g_sRand,2,4), j*2);
      blitRect(s_pStripe416,4, i*32+8+4*j, randUwMinMax(g_sRand,2,4), randUwMinMax(g_sRand,2,4), j*2+1);
      blitRect(s_pStripe416,8, i*32+8+4*j, randUwMinMax(g_sRand,2,4), randUwMinMax(g_sRand,2,4), 8+j*2);
       blitRect(s_pStripe416,12, i*32+8+4*j, randUwMinMax(g_sRand,2,4), randUwMinMax(g_sRand,2,4), 8+j*2+1);
    }
  }
  s_pASprite0 = advancedSpriteAdd(0, s_pStripe416, 32);
 */

  s_pStripe432 = bitmapCreate(32, 32*10, 4, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 4BPP
  for(int i=0; i<10; i++) {
    char msg[50];
    sprintf(msg, "%d",i);
	  fontDrawStr(s_pFont,  s_pStripe432, 0, i*32+0, msg, 1, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);
    for(int j=0; j<4; j++) {
      blitRect(s_pStripe432,0, i*32+8+4*j, randUwMinMax(g_sRand,4,8), randUwMinMax(g_sRand,2,4), j*2);
      blitRect(s_pStripe432,8, i*32+8+4*j, randUwMinMax(g_sRand,4,8), randUwMinMax(g_sRand,2,4), j*2+1);
      blitRect(s_pStripe432,16, i*32+8+4*j, randUwMinMax(g_sRand,4,8), randUwMinMax(g_sRand,2,4), 8+j*2);
      blitRect(s_pStripe432,24, i*32+8+4*j, randUwMinMax(g_sRand,4,8), randUwMinMax(g_sRand,2,4), 8+j*2+1);
    }
  }
  s_pASprite0 = advancedSpriteAdd(0, s_pStripe432, 32);


  advancedSpriteSetPos(s_pASprite0,280,100);



                             





	char szMsg[50];
	sprintf(szMsg, "BlitCopy");
	fontDrawStr(s_pFont,  s_pMainBuffer->pFront, 8, 8, szMsg, 4, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);


  blitCopy(
    s_pStripe416, 0, 0,
    s_pMainBuffer->pFront,
    16,16,
    16, 32,
    MINTERM_COOKIE
  );

	sprintf(szMsg, "Sprite");
	fontDrawStr(s_pFont,  s_pMainBuffer->pBack, 90, 90, szMsg, 4, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);
  bitmapDestroy(s_pStripe);


    systemUnuse();

      // Load the view
  viewLoad(s_pView);

  // Reset blcon2 to put sprite in front of http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0159.html
  g_pCustom->bplcon2=0b00100000;
}

void gameGsLoop(void) {

  if(keyCheck(KEY_ESCAPE)) {
    gameExit();
    return; 
  }

  tAdvancedSprite *sprite;
  
  switch (spritecontrol)
  {
  case 0:
    sprite=s_pASprite4;
    break;

  case 1:
    sprite=s_pASprite6;
    break;
  
  case 2:
    sprite=s_pASprite0;
    break;

  default:
    sprite=s_pASprite4;
    break;
  }

  if(keyCheck(KEY_SPACE)) {
    spritecontrol++;
    if (spritecontrol>2) {
      spritecontrol=0;
    }
  }

  if(joyCheck(JOY1_UP)) {
		advancedSpriteSetPosY(sprite,sprite->wY-2);
	}
	if(joyCheck(JOY1_DOWN)) {
		advancedSpriteSetPosY(sprite,sprite->wY+2);
	}
	if(joyCheck(JOY1_LEFT)) {
		advancedSpriteSetPosX(sprite,sprite->wX-2);
	}
	if(joyCheck(JOY1_RIGHT)) {
		advancedSpriteSetPosX(sprite,sprite->wX+2);
	}

  if(joyCheck(JOY1_FIRE)) {
    frame++;
    if (frame>=10) {
      frame=0;
    }
    advancedSpriteSetFrame(sprite,frame);
	}
  
  
  advancedSpriteProcess(s_pASprite0);
  advancedSpriteProcessChannel(0,s_pASprite0); 

  advancedSpriteProcess(s_pASprite4);
  advancedSpriteProcessChannel(4,s_pASprite4); 

  advancedSpriteProcess(s_pASprite6);
  advancedSpriteProcessChannel(6,s_pASprite6); 
  

  copProcessBlocks();

  vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
	fontDestroyTextBitMap(s_pTextBitMap);
	fontDestroy(s_pFont);
  randDestroy(g_sRand);
  advancedSpriteRemove(s_pASprite0);
  advancedSpriteRemove(s_pASprite4);
  advancedSpriteRemove(s_pASprite6);

  systemSetDmaBit(DMAB_SPRITE, 0); // Disable sprite DMA
  advancedSpriteManagerDestroy();

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
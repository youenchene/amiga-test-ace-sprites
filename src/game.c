#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/blit.h> // Blitting fns
#include <ace/managers/joy.h>
#include <ace/utils/palette.h>
#include <ace/managers/sprite.h> 
#include <ace/utils/font.h>


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
static tSprite *s_pSprite0;
static tSprite *s_pSprite1;
static tBitMap *s_pSprite0Data;
static tBitMap *s_pSprite1Data;

static tFont *s_pFont;
static tTextBitMap *s_pTextBitMap;

void gameGsCreate(void) {
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

  // Draw line separating score VPort and main VPort, leave one line blank after it
  blitLine(
    s_pScoreBuffer->pBack,
    0, s_pVpScore->uwHeight - 2,
    s_pVpScore->uwWidth - 1, s_pVpScore->uwHeight - 2,
    SCORE_COLOR, 0xFFFF, 0 // Try patterns 0xAAAA, 0xEEEE, etc.
  );

  spriteManagerCreate(s_pView, 0);
   systemSetDmaBit(DMAB_SPRITE, 1); 

  for(UWORD i=0; i < 16;i++)  {
     blitRect(s_pMainBuffer->pBack,16*i, 120, 16, 16,i);
  }
    
  for(UWORD i=0; i < 16;i++)  {
     blitRect(s_pMainBuffer->pBack,16*i, 136, 16, 16,16+i);
  }

  s_pSprite0Data = bitmapCreate(16, 34, 2, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 2BPP
  blitRect(s_pSprite0Data,0, 0, 16, 4, 0);
  blitRect(s_pSprite0Data,0, 4, 16, 4, 1);
  blitRect(s_pSprite0Data,0, 8, 16, 4, 2);
  blitRect(s_pSprite0Data,0, 12, 16, 4, 3);

 
  // You need to use 2 bitmaps, cause the sprite writes information to the sprite data.
  s_pSprite1Data = bitmapCreate(16, 34, 2, BMF_CLEAR|BMF_INTERLEAVED); // 16x32 2BPP
  blitRect(s_pSprite1Data,0, 0, 16, 4, 0);
  blitRect(s_pSprite1Data,0, 4, 16, 4, 1);
  blitRect(s_pSprite1Data,0, 8, 16, 4, 2);
  blitRect(s_pSprite1Data,0, 12, 16, 4, 3);
  
  s_pSprite0 = spriteAdd(0, s_pSprite0Data); // Add sprite to channel 
  s_pSprite1 = spriteAdd(3, s_pSprite1Data); // Add sprite to channel 
  
  s_pSprite0->wX=100;
  s_pSprite0->wY=100;
  s_pSprite1->wX=200;
  s_pSprite1->wY=100;
  spriteSetEnabled(s_pSprite0,1);
  spriteSetEnabled(s_pSprite1,1);
                             
  systemUnuse();

  // Load the view
  viewLoad(s_pView);


	char szMsg[50];
	sprintf(szMsg, "BlitCopy");
	fontDrawStr(s_pFont,  s_pMainBuffer->pFront, 8, 8, szMsg, 4, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);


  blitCopy(
    s_pSprite0Data, 0, 0,
    s_pMainBuffer->pFront,
    16,16,
    16, 32,
    MINTERM_COOKIE
  );

	sprintf(szMsg, "Sprite");
	fontDrawStr(s_pFont,  s_pMainBuffer->pFront, 90, 90, szMsg, 4, FONT_LEFT | FONT_TOP | FONT_COOKIE, s_pTextBitMap);



}

void gameGsLoop(void) {
  if(keyCheck(KEY_ESCAPE)) {
    gameExit();
    return; 
  }

  if(joyCheck(JOY1_UP)) {
		s_pSprite0->wY-=2;
	}
	if(joyCheck(JOY1_DOWN)) {
		s_pSprite0->wY+=2;
	}
	if(joyCheck(JOY1_LEFT)) {
		s_pSprite0->wX-=2;
	}
	if(joyCheck(JOY1_RIGHT)) {
		s_pSprite0->wX+=2;
	}
  
  spriteRequestMetadataUpdate(s_pSprite0);
  
  
  spriteProcess(s_pSprite0);
  
  
  spriteProcessChannel(0); // Should only be on create
  spriteRequestMetadataUpdate(s_pSprite1);
  spriteProcess(s_pSprite1);
  spriteProcessChannel(3); // Should only be on create



  copProcessBlocks();

  vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
	fontDestroyTextBitMap(s_pTextBitMap);
	fontDestroy(s_pFont);
  bitmapDestroy(s_pSprite0Data);

  spriteRemove(s_pSprite0);

  systemSetDmaBit(DMAB_SPRITE, 0); // Disable sprite DMA
  spriteManagerDestroy();

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
#include "game.h"
#include "util.h"
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

static tBitMap *s_Block;
static tBitMap *s_Block_InvX;

tRandManager *g_sRand;

static tFont *s_pFont;
static tTextBitMap *s_pTextBitMap;

static int frame=0;

static int spritecontrol=0;


static tBitMap *s_pEnemies;



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

  paletteLoadFromPath("data/W1-palette.gpl", s_pVpScore->pPalette, 32);

  memcpy(s_pVpScore->pPalette, s_pPalette, sizeof(s_pVpScore->pPalette));
	memcpy(s_pVpMain->pPalette, s_pPalette, sizeof(s_pVpMain->pPalette));

  // Draw line separating score VPort and main VPort, leave one line blank after it
  blitLine(
    s_pScoreBuffer->pBack,
    0, s_pVpScore->uwHeight - 2,
    s_pVpScore->uwWidth - 1, s_pVpScore->uwHeight - 2,
    SCORE_COLOR, 0xFFFF, 0 // Try patterns 0xAAAA, 0xEEEE, etc.
  );

  advancedSpriteManagerCreate(s_pView, 0);
  systemSetDmaBit(DMAB_SPRITE, 1);
  
  s_pEnemies= bitmapCreateFromPath("data/enemies-sprites.bm", 0);

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
  
  
  //advancedSpriteProcess(s_pASprite0);
  //advancedSpriteProcessChannel(0,s_pASprite0); 

  copProcessBlocks();

  vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
	fontDestroyTextBitMap(s_pTextBitMap);
	fontDestroy(s_pFont);
  randDestroy(g_sRand);
  bitmapDestroy(s_pEnemies);
}
  systemSetDmaBit(DMAB_SPRITE, 0); // Disable sprite DMA
  advancedSpriteManagerDestroy();

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
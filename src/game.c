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
#include <ace/managers/multiplexedsprite.h> 
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
#define SCORE_COLOR 4
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
#define NUMBER_OF_MULTIPLEXED_SPRITES 3 

static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;
//static tAdvancedSprite *s_pASprite4;
//static tAdvancedSprite *s_pASprite6;
//static tAdvancedSprite *s_pASprite0;

static UWORD s_pPalette[32];
//static tBitMap *s_pStripe;
//static tBitMap *s_pStripe32;
//static tBitMap *s_pStripe32;
//static tBitMap *s_pStripe416;
//static tBitMap *s_pStripe432;

//static tBitMap *s_Block;
//static tBitMap *s_Block_InvX;

//tRandManager *g_sRand;

//static int frame=0;

//static int spritecontrol=0;


//static tBitMap *s_pEnemies;

tBitMap **s_pEnemiesFrames;


//Sprites
static tAdvancedSprite *s_pEnemies;
static tMultiplexedSprite *s_pEnemies4;

typedef struct tEnemy {
	WORD x;
	WORD y;
  UBYTE frame;
  UBYTE speed;
} tEnemy;

tEnemy **s_pEnemiesList;


void gameGsCreate(void) {


  //g_sRand=randCreate(456,876);

  s_pView = viewCreate(0, 
  TAG_VIEW_GLOBAL_PALETTE, 1,
  TAG_END);

	s_pVpScore = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 5,
		TAG_VPORT_HEIGHT, 16,
    	TAG_END);

	s_pScoreBuffer = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_VPORT, s_pVpScore,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, 320,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 16,
	TAG_END);

	// Now let's do the same for main playfield
	s_pVpMain = vPortCreate(0,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_BPP, 5,
	TAG_END);

  s_pMainBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpMain,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
  TAG_END);

  paletteLoadFromPath("data/W1-palette.plt", s_pPalette, 32);
	memcpy(s_pVpScore->pPalette, s_pPalette, sizeof(s_pVpScore->pPalette));
	memcpy(s_pVpMain->pPalette, s_pPalette, sizeof(s_pVpMain->pPalette));

  // Draw line separating score VPort and main VPort, leave one line blank after it
  blitLine(
    s_pScoreBuffer->pBack,
    0, s_pVpScore->uwHeight - 2,
    s_pVpScore->uwWidth - 1, s_pVpScore->uwHeight - 2,
    SCORE_COLOR, 0xFFFF, 0 // Try patterns 0xAAAA, 0xEEEE, etc.
  );

  //sky
  blitRect(s_pMainBuffer->pBack,0, 0, 320, 240, 4);
  //cloud
  blitRect(s_pMainBuffer->pBack,200, 80, 30, 20, 31);
  blitRect(s_pMainBuffer->pBack,220, 86, 30, 20, 31);
  blitRect(s_pMainBuffer->pBack,240, 82, 20, 15, 31);

  //cloud
  blitRect(s_pMainBuffer->pBack,90, 140, 20, 20, 30);
  blitRect(s_pMainBuffer->pBack,100, 146, 30, 20, 30);
  blitRect(s_pMainBuffer->pBack,120, 142, 20, 18, 30);
  
  spriteManagerCreate(s_pView, 0, 0);
  systemSetDmaBit(DMAB_SPRITE, 1);
  
  /*tBitMap *s_pSpriteEnemies=bitmapCreateFromPath("data/enemies-sprites.bm", 0);

  s_pEnemiesList[0].x=310;
  s_pEnemiesList[0].y=40;
  s_pEnemiesList[0].frame=5;
  s_pEnemiesList[0].speed=3;

  s_pEnemies = advancedSpriteAdd(0, 16, s_pSpriteEnemies, NULL,0,1); // Add Main sprite to channel 0
	bitmapDestroy(s_pSpriteEnemies);

	advancedSpriteSetPos(s_pEnemies, s_pEnemiesList[0].x,s_pEnemiesList[0].y);
  advancedSpriteSetFrame(s_pEnemies,s_pEnemiesList[0].frame);
  */

  s_pEnemiesList=(tEnemy **) memAllocFastClear(sizeof(tEnemy*) * NUMBER_OF_MULTIPLEXED_SPRITES);


  logWrite("#### Prepare Enemies...!");

  for(UBYTE i=0;i<NUMBER_OF_MULTIPLEXED_SPRITES;i++) {
    s_pEnemiesList[i] = memAllocFastClear(sizeof(tEnemy));
    s_pEnemiesList[i]->x=0;
    s_pEnemiesList[i]->y=i*20;
    s_pEnemiesList[i]->frame=0;
    s_pEnemiesList[i]->speed=1 + i%2;
  }

  logWrite("#### Prepare Enemies Frames...!");

  tBitMap *s_pSpriteEnemies4=bitmapCreateFromPath("data/enemies-sprites-4.bm", 0);

  s_pEnemiesFrames=(tBitMap **)memAllocFastClear(sizeof(tBitMap*) * 3);

  for(UBYTE i=0;i<3;i++) {
    s_pEnemiesFrames[i]=bitmapCreate(
        16, 10,
        2, BMF_CLEAR | BMF_INTERLEAVED
    );
    blitCopy(
      s_pSpriteEnemies4, 0, i*10,
      s_pEnemiesFrames[i],
      0, 0,
      16, 10,
      MINTERM_COOKIE
    );
  }
  bitmapDestroy(s_pSpriteEnemies4);

  s_pEnemies4=spriteMultiplexedAdd(0,10,NUMBER_OF_MULTIPLEXED_SPRITES);


  for(UBYTE i=0;i<NUMBER_OF_MULTIPLEXED_SPRITES;i++) {
    logWrite("#### Start feeding element %d", i);
      spriteMultiplexedSetElement(s_pEnemies4, i, 10, 1, 0);
      logWrite("#### Set Element !");
      spriteMultiplexedSetBitmap(s_pEnemies4, i, s_pEnemiesFrames[s_pEnemiesList[i]->frame]);
      logWrite("#### Set Bitmap !");
      spriteMultiplexedSpriteSetPos(s_pEnemies4, i, s_pEnemiesList[i]->x,s_pEnemiesList[i]->y);
      logWrite("#### Set Pos !");
  }

  logWrite("#### Enemies initialized !");
  
  systemUnuse();
  logWrite("#### systemUnuse");

      // Load the view
  viewLoad(s_pView);

   logWrite("#### View loaded !");

  // Reset blcon2 to put sprite in front of http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0159.html
  g_pCustom->bplcon2=0b00100000;
  logWrite("#### Create Done !");
}

void gameGsLoop(void) {
  logWrite("###> Loop");
  if(keyCheck(KEY_ESCAPE)) {
    gameExit();
    return; 
  }

  /*
  s_pEnemiesList[0].x-=s_pEnemiesList[0].speed;
  if (s_pEnemiesList[0].x < -32) {
    s_pEnemiesList[0].x=320;
  }
  s_pEnemiesList[0].y=40;
  s_pEnemiesList[0].frame++;
  if (s_pEnemiesList[0].frame > 9) {
    s_pEnemiesList[0].frame=5;
  } 

  advancedSpriteSetPos(s_pEnemies,s_pEnemiesList[0].x,s_pEnemiesList[0].y);
  advancedSpriteSetFrame(s_pEnemies, s_pEnemiesList[0].frame);
  
  advancedSpriteProcess(s_pEnemies);
  advancedSpriteProcessChannel(s_pEnemies); 
  */

  //READY TO BE TESTED

  for(UBYTE i=0;i<NUMBER_OF_MULTIPLEXED_SPRITES;i++) {
    s_pEnemiesList[i]->x+=s_pEnemiesList[i]->speed;
    if (s_pEnemiesList[i]->x > 320) {
      s_pEnemiesList[i]->x=-16;
    }
    s_pEnemiesList[i]->frame++;
    if (s_pEnemiesList[i]->frame > 2) {
      s_pEnemiesList[i]->frame=0;
    }
    spriteMultiplexedSpriteSetPos(s_pEnemies4, i, s_pEnemiesList[i]->x,s_pEnemiesList[i]->y);
    spriteMultiplexedSetBitmap(s_pEnemies4, i, s_pEnemiesFrames[s_pEnemiesList[i]->frame]);
  }


  spriteMultiplexedProcess(s_pEnemies4);
  spriteMultiplexedProcessChannel(0);

  //viewProcessManagers(s_pView);

  copProcessBlocks();

  vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  systemUse();
  advancedSpriteRemove(s_pEnemies);
  systemSetDmaBit(DMAB_SPRITE, 0); // Disable sprite DMA
  spriteManagerDestroy();
  viewDestroy(s_pView);
}
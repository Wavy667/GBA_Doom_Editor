#ifndef __DEFS_H__
#define __DEFS_H__

#define IMG_WPNX 128
#define IMG_WPNY 128

#define NUM_MAP_LUMPS 11

#define OFFS_SOUDS 0x030EE2
#define OFFS_SNDTAB 0x92ADC
#define OFFS_WAD 0x0942BC
#define OFFS_LEAFTAB 0x06C76D0
#define OFFS_NAMETAB 0x05D6858
#define OFFS_FRAME 0x06BEE9C
#define OFFS_MOBJ 0x06C1DDC
#define OFFS_SPRFRM 0x06C3A0C
#define OFFS_SPRDEF 0x06C73F8
// real sounds start 0x092A5C
#define OFFS_SBAR 0x04FD070

#define SNDSIZE 400103
#define WADSIZE 4623792
#define MAX_SNDTAB 52
#define MAX_NAMETAB 91
#define MAX_FRAMES 337
#define MAX_MOBJ 81

// 0x006A6E1C - anim, switch

#define MODE_NONE 0
#define MODE_FLATS 1
#define MODE_TEXTURES 2
#define MODE_PATCHES 3
#define MODE_SPRITES 4
#define MODE_OTHER 5
#define MODE_MAPS 6
#define MODE_SOUNDS 7
#define MODE_HACKS 8
#define MODE_SPRNAMES 11

#define TYPE_NONE 0
#define TYPE_SPRITE 1
#define TYPE_TEXTURE 2
#define TYPE_FLAT 3
#define TYPE_PATCH 4
#define TYPE_MAP 5
#define TYPE_TEXHACK 6

typedef unsigned char byte;

typedef struct wadhead_s {
	int id;
	int numlumps;
	int offset;
} wadhead_t;

typedef struct waditem_s {
	int offset;
	int size;
	byte name[8];
} waditem_t;

typedef struct item_s {
	struct item_s *next;
	struct item_s *prev;
	int type;
	int size;
	char name[8];
	byte data[];
} item_t;

typedef struct {
	int numtextures;
	int table[];
} texhead_t;

typedef struct {
	char name[8];
	int masked;
	short width;
	short height;
	int ignored;
	short numpatches; // 1
	short x;
	short y;
	short num;
	short unk[2];
} texitem_t;

typedef struct {
	short width;
	short height;
	short id; // 3
	short unk1;
	int unk2[2];
} patchhead_t;

typedef struct {
	short width;
	short height;
	unsigned short xoffs;
	unsigned short yoffs;
} sprhead_t;

typedef struct {
	byte start;
	byte num;
	unsigned short offs;
} sprdata_t;

typedef struct {
	unsigned int offs;
	unsigned int size; // real = size << 4;
	int unk2;
	int unk3;
} sound_t;

typedef struct {
	char name[8];
} sprname_t;

typedef struct
{
	int rotate;
	int lump[8];
	byte flip[8];
} spriteframe_t;

typedef struct
{
	int numframes;
	int offset;
} spritedef_t;

typedef struct {
	unsigned short signature; // 0x4D42
	unsigned int filesize;
	unsigned short reserved[2]; // 0
	unsigned int imgstart;
	unsigned int headsize; // 40
	unsigned int width;
	unsigned int height;
	unsigned short planes; // 1
	unsigned short bpp; // 1, 4, 8, 24, 32
	unsigned int compres; // 0
	unsigned int imgsize;
	unsigned int etc[4];
} __attribute__((packed)) bmphead_t;

typedef struct {
	int riff; // 0x46464952
	int size; // soundsize + 36
	int wave; // 0x45564157
	int fmt; // 0x20746d66
	int size2; // 16
	short format; // 1
	short channels; // 1
	int rate; // 8000
	int brate; // 8000
	short align; // 1
	short bits; // 8
	int data; // 0x61746164
	int soundsize;
} __attribute__((packed)) wavhead_t;

typedef struct
{
	int	sprite;
	int	frame;
	int	tics;
	int	action;
	int	nextstate;
	int	misc1, misc2;
} state_t;

typedef struct {
	int	doomednum;
	int	spawnstate;
	int	spawnhealth;
	int	seestate;
	int	seesound;
	int	reactiontime;
	int	attacksound;
	int	painstate;
	int	painchance;
	int	painsound;
	int	meleestate;
	int	missilestate;
	int	deathstate;
	int	xdeathstate;
	int	deathsound;
	int	speed;
	int	radius;
	int	height;
	int	mass;
	int	damage;
	int	activesound;
	int	flags;
} mobjinfo_t;

#endif

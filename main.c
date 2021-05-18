/* kgsws's GBA doom editor
	This is my unfinished project, it had to be just a small app
	but it has grown larger than i expected so code is a mess.
	Feel free to finish it, or create better based on my code.
*/

#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "defs.h"

// mode variables
int mode = MODE_NONE;
int lineover = 0;
HDC hdc = NULL;
byte *img = NULL;
int winx, winy, imgw, imgh;
item_t *item;
wadhead_t wadhead;
bmphead_t bmphead;
int palette[256];
int o1, o2;
int mobjedit;
HINSTANCE hSavedInstance;
// file variables
int totalsize;
char filename[_MAX_PATH] = {0};
char savename[_MAX_PATH] = {0};
FILE *f;
byte *buff = NULL;
// dynamic wad variables
item_t *topitem = NULL;
item_t *curitem = NULL;
item_t *texture = NULL;
sound_t *sndtab;
byte *sounds[MAX_SNDTAB];
int *leaftab;
spritedef_t *spritedef;
spriteframe_t *spritefrm;
sprname_t *nametab;
state_t *states;
mobjinfo_t *mobjinfo;
// save/load select
char *filter_tab[] = {
	"GBA Doom 1 (*.gba)\0*.gba\0\0",
	"Bitmap image (*.bmp)\0*.bmp\0\0",
	"Sound file (*.wav)\0*.wav\0\0",
	"Lump file (*.lmp)\0*.lmp\0\0",
	"WAD file (*.wad)\0*.wad\0\0"
};
char *end_tab[] = {
	"gba",
	"bmp",
	"wav",
	"lmp",
	"wad"
};
char *export_err[] = {
	NULL,
	"New WAD is too big, delete something.",
	"Sound block is too big, make it smaller."
};
int mode_files[] = {
	0, // none
	1, // flats
	1, // textures
	1, // patches
	1, // sprites
	3, // lumps
	4, // maps
	2, // sounds
};
int type_table[] = {
	-1,
	TYPE_FLAT,
	TYPE_TEXTURE,
	TYPE_PATCH,
	TYPE_SPRITE
};
wavhead_t outwav = {
	0x46464952,
	0,
	0x45564157,
	0x20746d66,
	16,
	1,
	1,
	8000,
	8000,
	1,
	8,
	0x61746164,
	0
};
int sound_num[] = {
	60,
	5,
        57,
	48,
	49,
	34,
	35,
	53,
	38,
	51,
	37,
	43,
	58,
	24,
	19,
	18,
	13,
	15,
	27,
	64,
	29, // ? 62
	1,
	4,
	44,
	23,
	45,
	46,
	25,
	56,
	31,
	32,
	16,
	17,
	61,
	10,
	12,
	8,
	9,
	7,
	6,
	3,
        42,
	36,
	2,
        41,
	62, // ? 29
	26,
	20,
	21,
	22,
	30,
	28
};
char *map_names[] = {
	"MAP01",
	"MAP02",
	"MAP03",
	"MAP04",
	"MAP05",
	"MAP06",
	"MAP07",
	"MAP08",
	"MAP09",
	"MAP10",
	"MAP11",
	"MAP12",
	"MAP13",
	"MAP14",
	"MAP15",
	"MAP16",
	"MAP17",
	"MAP18",
	"MAP19",
	"MAP20",
	"MAP21",
	"MAP22",
	"MAP23",
	"MAP24",
	"E1M4",
	"E1M1",
	"E3M7",
	"E1M2",
	"E3M5",
	"E3M4",
	"E1M3",
	"E1M5"
};
char *mobjnames[] = {
	"DoomPlayer",
	"ZombieMan",
	"ShotgunGuy",
	"DoomImp",
	"Demon",
	"Spectre",
	"Cacodemon",
	"BaronOfHell",
	"LostSoul",
	"ExplosiveBarrel",
	"DoomImpBall",
	"CacodemonBall",
	"BaronBall",
	"Rocket",
	"PlasmaBall",
	"BFGBall",
	"BulletPuff",
	"BloodSplat",
	"TeleportFog",
	"ItemFog",
	"TeleportDestination",
	"BFGExtra",
	"GreenArmor",
	"BlueArmor",
	"HealthBonus",
	"ArmorBonus",
	"BlueCard",
	"RedCard",
	"YellowCard",
	"YellowSkull",
	"RedSkull",
	"BlueSkull",
	"Stimpack",
	"Medikit",
	"SoulSphere",
	"Invulnerability",
	"Berserk",
	"Invisibility",
	"RadSuit",
	"AllMap",
	"Infrared",
	"Clip",
	"ClipBox",
	"RocketAmmo",
	"RocketBox",
	"Cell",
	"CellPack",
	"Shell",
	"ShellBox",
	"Backpack",
	"BFG9000",
	"Chaingun",
	"Chainsaw",
	"RocketLauncher",
	"PlasmaRifle",
	"Shotgun",
	"MT_MISC30",
	"MT_MISC31",
	"MT_MISC32",
	"MT_MISC33",
	"MT_MISC34",
	"MT_MISC35",
	"MT_MISC36",
	"MT_MISC37",
	"MT_MISC38",
	"MT_MISC39",
	"MT_MISC40",
	"MT_MISC41",
	"MT_MISC42",
	"MT_MISC43",
	"MT_MISC44",
	"MT_MISC45",
	"MT_MISC46",
	"MT_MISC47",
	"MT_MISC48",
	"MT_MISC49",
	"MT_MISC50",
	"MT_MISC51",
	"MT_MISC52",
	"MT_MISC53",
	"MT_MISC54"
};

// compare lump names, accepts wildcards in t1
int compare(char *t1, char *t2)
{
	int i;
	char c;
	for(i = 0; i < 8; i++) {
		c = *t2;
		if(*t1 == '*' && c != 0) c = '*';
		if(c >= 'a' && c <= 'z') c -= 0x20;
		if(*t1 != c) return 0;
		if(*t1 == 0) break;
		t1++;
		t2++;
	}
	return 1;
}

// create lump name from file name
char *file2name()
{
	int i;
	char *name;
	char temp[9];

	for(i = strlen(savename); i >= 0; i--) {
		if(savename[i] == '.') savename[i] = 0;
		if(savename[i] == '\\') break;
	}
	name = savename + i + 1;
	name[8] = 0;
	memset(temp, 0, sizeof(temp));
	strcpy(temp, name);
	strupr(temp);
	strcpy(savename, temp);
	return savename;
}

// put pixel into working place, buffer or window
void PutPixel(int x, int y, byte col)
{
	if(hdc) SetPixel(hdc, winx + x, winy + y, palette[col]);
	if(img) img[x + y * imgw] = col;
}

// set position in window, and hdc
void SetLocation(HWND hwnd, int width, int height)
{
	HWND center;
	RECT rect;
	int cx, cy;

	if(hwnd) {
		center = GetDlgItem(hwnd, IDC_CENTER);
		if(center) {
			GetWindowRect(center, &rect);
			cx = (rect.right - rect.left) / 2;
			cy = (rect.bottom - rect.top) / 2;
			hdc = GetDC(center);
		}
		winx = cx - width / 2;
		winy = cy - height / 2;
	} else {
		hdc = NULL;
		imgw = width;
		imgh = height;
	}
}

int FileDialog(HWND hwndOwner, int save)
{
	OPENFILENAME ofn;
	char temp[16];
	int i;
	int type = mode_files[mode];

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	if(mode == MODE_NONE) {
		filename[0] = 0;
		ofn.lpstrFile = filename;
	} else {
		savename[0] = 0;
		ofn.lpstrFile = savename;
	}
	ofn.hwndOwner = hwndOwner;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = filter_tab[type];
	if(save) {
		if(mode >= MODE_FLATS && mode <= MODE_MAPS) {
			memcpy(savename, item->name, 8);
			for(i = 0; i < 8; i++) if(item->name[i] == 0) break;
			savename[i] = 0;
		}
		ofn.lpstrDefExt = end_tab[type];
		ofn.Flags = OFN_PATHMUSTEXIST;
		if(!GetSaveFileName(&ofn)) return 0;
	} else {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if(!GetOpenFileName(&ofn)) return 0;
	}
	return 1;
}

int PlaySoundData(byte *data, int size)
{
	HWAVEOUT hWaveOut;
	WAVEFORMATEX WaveFormat;
	WAVEHDR WaveHeader;
	HANDLE Event;

	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 1;
	WaveFormat.wBitsPerSample = 8;
	WaveFormat.nSamplesPerSec = 8000;
	WaveFormat.nBlockAlign = 1;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
	WaveFormat.cbSize = 0;

	Event = CreateEvent(0, FALSE, FALSE, 0);

	if(waveOutOpen(&hWaveOut,0,&WaveFormat,(DWORD)Event,0,CALLBACK_EVENT) != MMSYSERR_NOERROR)
		return 0;

	WaveHeader.lpData = data;
	WaveHeader.dwBufferLength = size;
	WaveHeader.dwFlags = 0;
	WaveHeader.dwLoops = 0;

	if(waveOutPrepareHeader(hWaveOut,&WaveHeader,sizeof(WaveHeader)) != MMSYSERR_NOERROR)
		return 0;

	ResetEvent(Event);
	waveOutWrite(hWaveOut,&WaveHeader,sizeof(WaveHeader));
	WaitForSingleObject(Event,INFINITE);
	waveOutUnprepareHeader(hWaveOut,&WaveHeader,sizeof(WaveHeader));
	waveOutClose(hWaveOut);
	CloseHandle(Event);

	return 1;
}

int SoundOffset(unsigned int num)
{
	return num & 0x00FFFFFF;
}

unsigned int FlipE(unsigned int num)
{
	return (num >> 24) | ((num & 0xFF0000) >> 8) | ((num & 0xFF00) << 8) | (num << 24);
}

unsigned short FlipS(unsigned short num)
{
	return (num >> 8) | (num << 8);
}

item_t *AddItem(char *name, byte *data, int size, int type)
{
	item_t *temp;

	if(topitem) {
		temp = malloc(sizeof(item_t) + size);
		if(!temp) return NULL;
		temp->next = NULL;
		temp->prev = curitem;
		temp->size = size;
		temp->type = type;
		curitem->next = temp;
		memcpy(temp->name, name, 8);
		if(data) memcpy(temp->data, data, size);
		curitem = temp;
		return temp;
	} else {
		topitem = curitem = malloc(sizeof(item_t) + size);
		if(!topitem) return NULL;
		topitem->next = NULL;
		topitem->prev = NULL;
		topitem->size = size;
		topitem->type = type;
		memcpy(topitem->name, name, 8);
		if(data) memcpy(topitem->data, data, size);
		return topitem;
	}
}

item_t *ReplaceItem(item_t *item, byte *data, int size)
{
	int type = item->type;
	item_t *next = item->next;
	item_t *prev = item->prev;
	char name[8];

	if(item->size == size) {
		if(data) memcpy(item->data, data, size);
		return item;
	}
	memcpy(name, item->name, 8);
	free(item);
	item = malloc(size + sizeof(item_t));
	if(!item) return NULL;
	item->type = type;
	item->size = size;
	item->next = next;
	item->prev = prev;
	if(next) next->prev = item;
	else curitem = item;
	if(prev) prev->next = item;
	else topitem = item;
	memcpy(item->name, name, 8);
	if(data) memcpy(item->data, data, size);
	return item;
}

void DeleteItem(item_t *del)
{
	if(del->prev)
		del->prev->next = del->next;
	else
		topitem = del->next;
	if(del->next)
		del->next->prev = del->prev;
	else
		curitem = del->prev;
	free(del);
}

void ClearItems()
{
	curitem = topitem;
	while(curitem) {
		topitem = curitem;
		curitem = topitem->next;
		free(topitem);
	}
	curitem = topitem = NULL;
}

item_t *ItemByName(char *name)
{
	item_t *temp = topitem;

	while(temp) {
		if(compare(name, temp->name)) return temp;
		temp = temp->next;
	}
	return temp;
}

void ReadPalette(byte *data)
{
	int i;

	for(i = 0; i < 256; i++) {
		palette[i] = *data;
		data++;
		palette[i] |= *data << 8;
		data++;
		palette[i] |= *data << 16;
		data++;
	}
	o1 = palette[0];
	o2 = palette[247];
}

int LoadWad(byte *wadbuff)
{
	wadhead_t *head = (wadhead_t*)wadbuff;
	waditem_t *items;
	item_t *temp;
	int i, type, num;

	if(head->id != 0x44415749) return 1;
	items = (waditem_t*)(wadbuff + FlipE(head->offset));
	num = FlipE(head->numlumps);
	type = TYPE_SPRITE;
	for(i = 0; i < num; i++) {
		if(type == TYPE_NONE || type == TYPE_TEXHACK) type = TYPE_PATCH;
		if(type == TYPE_SPRITE && !strcmp(items->name, "T_START")) type = TYPE_TEXTURE;
		if(type == TYPE_TEXTURE && !strcmp(items->name, "F_START")) type = TYPE_FLAT;
		if(type == TYPE_FLAT && !strcmp(items->name, "F_END")) type = TYPE_PATCH;
		if(type == TYPE_PATCH) {
			if(!strcmp(items->name, "CRYPAL")) type = TYPE_NONE;
			if(!memcmp(items->name, "TEXTURE1", 8)) type = TYPE_TEXHACK;
			if(!strcmp(items->name, "PLAYPAL")) {
				ReadPalette(wadbuff + FlipE(items->offset));
				type = TYPE_NONE;
			}
		}
		if((type == TYPE_MAP || type == TYPE_PATCH) && !memcmp(items->name, "COLORMAP", 8)) type = TYPE_NONE;
		if((type == TYPE_PATCH || type == TYPE_NONE || type == TYPE_TEXHACK) && !strcmp(items->name, "MAP01")) type = TYPE_MAP;
		if(items->size || type == TYPE_MAP) {
			temp = AddItem(items->name, wadbuff + FlipE(items->offset), FlipE(items->size), type);
			if(!temp) return 0;
			if(type == TYPE_TEXHACK)
				texture = temp;
		}
		items++;
	}

	return 0;
}

void LoadSounds(byte *sndptr)
{
	byte *out;
	char *in;
	int i, j, size;

	sndtab = (sound_t*)sndptr;
	for(i = 0; i < MAX_SNDTAB; i++) {
		size = sndtab[i].size << 4;
		in = buff + SoundOffset(sndtab[i].offs);
		sounds[i] = out = malloc(size);
		if(out)
			for(j = 0; j < size; j++, in++, out++)
				*out = 128 + *in;
	}
}

int CheckMap(char *name)
{
	if(compare("MAP**", name)) return 1;
	if(compare("E*M*", name)) return 1;
	return 0;
}

void SelectMode(HWND hwnd, int newmode, int type)
{
	item_t *temp = topitem;
	char name[9];

	switch(newmode) {
		case MODE_SPRITES:
			palette[0] = o1;
			palette[247] = 0xFFFF00;
		break;
		case MODE_PATCHES:
			palette[0] = 0xFFFF00;
			palette[247] = o2;
		break;
		default:
			palette[0] = o1;
			palette[247] = o2;
		break;
	}
	mode = newmode;
	SendDlgItemMessage(hwnd, IDC_LST1, LB_RESETCONTENT, 0, 0);
	name[8] = 0;
	while(temp) {
		if(temp->type == type) {
			if(	(mode != MODE_SPRITES || temp->name[0] != '.') &&
				(mode != MODE_MAPS || CheckMap(temp->name))
			) {
				memcpy(name, temp->name, 8);
				SendDlgItemMessage(hwnd, IDC_LST1, LB_ADDSTRING, 0, (LPARAM)name);
			}
		}
		temp = temp->next;
	}
}

void SelectSounds(HWND hwnd)
{
	int i;
	char name[16];

	mode = MODE_SOUNDS;
	SendDlgItemMessage(hwnd, IDC_LST1, LB_RESETCONTENT, 0, 0);
	for(i = 0; i < MAX_SNDTAB; i++) {
		sprintf(name, "sound %i", sound_num[i]);
		SendDlgItemMessage(hwnd, IDC_LST1, LB_ADDSTRING, 0, (LPARAM)name);
	}
}

void SelectSpriteNames(HWND hwnd)
{
	int i;
	char name[16];

	mode = MODE_SPRNAMES;
	SendDlgItemMessage(hwnd, IDC_LST1, LB_RESETCONTENT, 0, 0);
	for(i = MAX_NAMETAB - 1; i >= 0; i--)
		SendDlgItemMessage(hwnd, IDC_LST1, LB_ADDSTRING, 0, (LPARAM)nametab[i].name);
}

texitem_t *TextureByName(char *name)
{
	int i;
	texhead_t *head = (texhead_t*)texture->data;
	texitem_t *item;

	for(i = 0; i < head->numtextures; i++) {
		item = (texitem_t*)(texture->data + head->table[i]);
		if(compare(name, item->name)) return item;
	}
	return NULL;
}

void DrawFlat(HWND hwnd, byte *data)
{
	int x, y;

	SetLocation(hwnd, 64, 64);
	for(y = 0; y < 64; y++) for(x = 0; x < 64; x++) {
		PutPixel(x, y, *data);
		data++;
	}
}

void DrawTexture(HWND hwnd, char *name, byte *data)
{
	int x, y;
	char res[16];
	texitem_t *tex;

	tex = TextureByName(name);
	if(!tex) {
		if(hwnd) SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)"error");
		return;
	}
	if(hwnd) {
		sprintf(res, "%i x %i", tex->width, tex->height);
		SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)res);
	}
	SetLocation(hwnd, tex->width, tex->height);
	for(x = 0; x < tex->width; x++) for(y = 0; y < tex->height; y++) {
		PutPixel(x, y, *data);
		data++;
	}
}

void DrawPatch(HWND hwnd, byte *data)
{
	int x, y;
	int width, height;
	char res[16];
	patchhead_t *head = (patchhead_t*)data;

	data += sizeof(patchhead_t);
	if(head->id != 0x0300) {
		if(hwnd) SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)"error");
		return;
	}
	width = FlipS(head->width);
	height = FlipS(head->height);
        if(hwnd) {
		sprintf(res, "%i x %i", width, height);
		SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)res);
	}
	SetLocation(hwnd, width, height);
	for(y = 0; y < height; y++) for(x = 0; x < width; x++) {
		PutPixel(x, y, *data);
		data++;
	}
}

void DrawSprite(HWND hwnd, item_t *item)
{
	sprhead_t *head = (sprhead_t*)item->data;
	short *offs = (short*)(item->data + sizeof(sprhead_t));
	sprdata_t *info;
	byte *data;
	int width, height;
	int x, y, o;
	int ox, oy, cx, cy;
	char res[16];
	HWND center;
	RECT rect;

	if(!item->next || item->next->type != TYPE_SPRITE || item->next->name[0] != '.') {
		if(hwnd) SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)"error");
		return;
	}
	width = FlipS(head->width);
	height = FlipS(head->height);
        if(hwnd) {
		ox = (short)FlipS(head->xoffs);
		oy = (short)FlipS(head->yoffs);
		sprintf(res, "%i x %i", width, height);
		SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)res);
		sprintf(res, "%i %i", ox, oy);
		SendDlgItemMessage(hwnd, IDC_EDT1, WM_SETTEXT, 0, (LPARAM)res);
		center = GetDlgItem(hwnd, IDC_CENTER);
		if(center) {
			GetWindowRect(center, &rect);
			hdc = GetDC(center);
		}
		if(ox < 0) {
			cx = cy = 0;
		} else {
			cx = (rect.right - rect.left) / 2;
			cy = (rect.bottom - rect.top) / 2;
		}
		if(!lineover) {
			if(ox < 0) {
				for(x = 0; x < 128; x++)
					SetPixel(hdc, cx + x, cy, 0x80);
				for(y = 0; y < 128; y++)
					SetPixel(hdc, cx, cy + y, 0x80);
			} else {
				for(x = 0; x < 128; x++)
					SetPixel(hdc, cx - 64 + x, cy, 0x80);
				for(y = 0; y < 128; y++)
					SetPixel(hdc, cx, cy - 64 + y, 0x80);
			}
		}
	} else {
		ox = 0;
		oy = 0;
	}
	SetLocation(hwnd, width, height);
	for(x = 0; x < width; x++) {
		info = (sprdata_t*)(item->data + FlipS(*offs));
		while(info->offs != 0xFFFF) {
			y = info->start;
			o = y + info->num;
			data = item->next->data + FlipS(info->offs);
			while(y != o) {
				if(!hwnd) PutPixel(x, y, *data);
				else SetPixel(hdc, cx + x - ox, cy + y - oy, palette[*data]);
				data++;
				y++;
			}
			info++;
		}
		offs++;
	}
	if(hwnd && lineover) {
		center = GetDlgItem(hwnd, IDC_CENTER);
		if(center) {
			GetWindowRect(center, &rect);
			hdc = GetDC(center);
		}
		if(ox < 0) {
			for(x = 0; x < 128; x++)
				SetPixel(hdc, cx + x, cy, 0x80);
			for(y = 0; y < 128; y++)
				SetPixel(hdc, cx, cy + y, 0x80);
		} else {
			for(x = 0; x < 128; x++)
				SetPixel(hdc, cx - 64 + x, cy, 0x80);
			for(y = 0; y < 128; y++)
				SetPixel(hdc, cx, cy - 64 + y, 0x80);
		}
	}
}

int CountMapData(item_t *start, int num)
{
	int i, size;

	size = 0;
	for(i = 0; i < num; i++) {
		size += start->size;
		if(start->size & 3) size += 4 - (start->size & 3);
		start = start->next;
		if(!start) break;
	}
	return size;
}

void WriteMapData(FILE *f, item_t *start)
{
	int i, temp;

	temp = 0;
	for(i = 0; i < NUM_MAP_LUMPS; i++) {
		if(start->size) fwrite(start->data, start->size, 1, f);
		if(start->size & 3) fwrite(&temp, 4 - (start->size & 3), 1, f);
		start = start->next;
	}
}

void WriteMapDir(FILE *f, item_t *start)
{
	int i, size;

	size = 12;
	for(i = 0; i < NUM_MAP_LUMPS; i++) {
		fwrite(&size, 4, 1, f);
		fwrite(&start->size, 4, 1, f);
		fwrite(start->name, 8, 1, f);
		size += start->size;
		if(start->size & 3) size += 4 - (start->size & 3);
		start = start->next;
	}
}

int CountExportData(int *size)
{
	int items;
	item_t *temp = topitem;

	*size = 0;
	items = 0;
	while(temp) {
		*size += temp->size;
		if(temp->size & 3) *size += 4 - (temp->size & 3);
		temp = temp->next;
		items++;
	}
	return items;
}

waditem_t *ExportType(waditem_t *list, int *offset, int type)
{
	int size, i;
	item_t *start = topitem;
	byte *data = buff + OFFS_WAD + *offset;

	while(start) {
		if(start->type == type) {
			if(type == TYPE_MAP) {
				if(compare("LEAF**", start->name)) {
					i = (start->name[4] - 0x30) * 10 + start->name[5] - 0x30;
					leaftab[i] = OFFS_WAD + *offset + 0x08000000;
				}
			}
			size = start->size;
			if(size & 3) size += 4 - (size & 3);
			memcpy(data, start->data, start->size);
			data += size;
			list->offset = FlipE(*offset);
			list->size = FlipE(start->size);
			memcpy(list->name, start->name, 8);
			*offset += size;
			list++;
		}
		start = start->next;
	}
	return list;
}

int LumpNum(char *name, waditem_t *items, int max)
{
	int i;
	for(i = 0; i < max; i++) {
		if(compare(name, items->name)) return i;
		items++;
	}
	return -1;
}

int ExportGame(byte *wadbase)
{
	int i, j, k, size, stoffs;
	wadhead_t *head = (wadhead_t*)wadbase;
	waditem_t *items = (waditem_t*)(wadbase + WADSIZE);
	int offset = 12;
	char *out;
	byte *in;
	char name[8];

	head->numlumps = CountExportData(&head->offset) + 4; // T_START T_END F_START F_END
        items -= head->numlumps;

	if(head->offset + head->numlumps * sizeof(waditem_t) + sizeof(wadhead_t) > WADSIZE)
		return 1; // wad too big

	// Export WAD

	items = ExportType(items, &offset, TYPE_SPRITE);
	items->offset = 0;
	items->size = 0;
	memcpy(items->name, "T_START", 8);
	items++;
	items = ExportType(items, &offset, TYPE_TEXTURE);
	items->offset = 0;
	items->size = 0;
	memcpy(items->name, "T_END\0\0", 8);
	items++;
	items->offset = 0;
	items->size = 0;
	memcpy(items->name, "F_START", 8);
	items++;
	items = ExportType(items, &offset, TYPE_FLAT);
	items->offset = 0;
	items->size = 0;
	memcpy(items->name, "F_END\0\0", 8);
	items++;
	items = ExportType(items, &offset, TYPE_PATCH);
	items = ExportType(items, &offset, TYPE_TEXHACK);
	items = ExportType(items, &offset, TYPE_NONE);
	items = ExportType(items, &offset, TYPE_MAP);

	// Export sounds

	offset = 0;
	in = (char*)(buff + OFFS_SOUDS);
	for(i = 0; i < MAX_SNDTAB; i++) {
		sndtab[i+MAX_SNDTAB].offs = sndtab[i].offs = offset + OFFS_SOUDS + 0x08000000;
		sndtab[i+MAX_SNDTAB].size = sndtab[i].size;
		size = sndtab[i].size << 4;
		out = sounds[i];
		if(offset + size > SNDSIZE) return 2; // sounds too big
		for(j = 0; j < size; j++, in++, out++, offset++) *in = *out - 128;
	}

	// recreate sprite tables

	items -= head->numlumps;
	offset = 0;
	for(i = 0; i < MAX_NAMETAB; i++) {
                stoffs = offset;
		if(offset >= MAX_FRAMES) return 3; // too many sprites
		strcpy(name, nametab[MAX_NAMETAB - i - 1].name);
		j = 0;
		while(1) {
			name[4] = j + 'A';
			name[5] = '0';
			name[6] = 0;
			k = LumpNum(name, items, head->numlumps);
			if(k != -1) {
				spritefrm[offset].rotate = 0;
				for(size = 0; size < 8; size++)
					spritefrm[offset].lump[size] = k;
				memset(spritefrm[offset].flip, 0, 8);
				j++;
				offset++;
				continue;
			}
			name[4] = j + 'A';
			name[5] = '1';
			name[6] = 0;
			k = LumpNum(name, items, head->numlumps);
			if(k == -1) break;
			spritefrm[offset].rotate = 1;
			spritefrm[offset].lump[0] = k;
			spritefrm[offset].flip[0] = 0;
			for(size = 1; size < 8; size++) {
				name[4] = j + 'A';
				name[5] = size + '1';
				name[6] = 0;
				spritefrm[offset].flip[size] = 0;
				k = LumpNum(name, items, head->numlumps);
				if(k == -1) {
					name[4] = j + 'A';
					name[5] = size + '1';
					name[6] = '*';
					name[7] = '*';
					k = LumpNum(name, items, head->numlumps);
					if(k == -1) {
						name[4] = '*';
						name[5] = '*';
						name[6] = j + 'A';
						name[7] = size + '1';
						k = LumpNum(name, items, head->numlumps);
						if(k == -1) k = 0;
						else spritefrm[offset].flip[size] = 1;
					}
				}
				spritefrm[offset].lump[size] = k;
			}
			j++;
			offset++;
		}
		spritedef[i].numframes = j;
		spritedef[i].offset = OFFS_SPRFRM + stoffs * sizeof(spriteframe_t) + 0x08000000;
	}

	// Finalize WAD head

        head->offset = WADSIZE - sizeof(waditem_t) * head->numlumps;
	head->numlumps = FlipE(head->numlumps);
	head->offset = FlipE(head->offset);

	return 0;
}

int ReadBMP(FILE *f, bmphead_t *head, int width, int height)
{
	int i, e, d, t;
	byte *in;

	if(head->signature != 0x4D42 || head->headsize != 40 || head->planes != 1 || head->bpp != 8 || head->compres != 0)
		return 1;

	if(width && head->width != width)
		return 2;
	if(height && head->height != height)
		return 2;

	if(head->width > 256 || head->height > 256)
		return 3;

	imgw = head->width;
	imgh = head->height;
	if(imgw & 3) e = 4 - (imgw & 3);
	else e = 0;
	img = malloc(imgw * imgh);
	if(!img) return 4;

	fseek(f, head->imgstart, SEEK_SET);
	for(i = 0; i < imgh; i++) {
		in = img + (imgh - i - 1) * imgw;
		fread(in, imgw, 1, f);
		if(e) fread(&t, e, 1, f);
	}
	return 0;
}

int CreateFlat(item_t *item)
{
	if(item) {
		memcpy(item->data, img, item->size);
		return 0;
	}
	AddItem(file2name(), img, 64*64, TYPE_FLAT);
}

int ReplaceTexture(item_t *item)
{
	int x, y;
	texitem_t *tex;
	char name[8];
	byte *in, *out;

	if(item->size != imgw * imgh) {
		tex = TextureByName(item->name);
		if(!tex) return 1;
		tex->width = imgw;
		tex->height = imgh;
		item = ReplaceItem(item, NULL, imgw * imgh);
		if(!item) return 1;
	}
	out = img;
	for(y = 0; y < imgh; y++) {
		in = item->data + y;
		for(x = 0; x < imgw; x++) {
			*in = *out;
			out++;
			in += imgh;
		}
	}
	return 0;
}

int CreateTexture(item_t *item)
{
	int i, old;
	byte *temp;
	texhead_t *head = (texhead_t*)texture->data;
	texitem_t *newtex, *oldtex;
	char *name;

	if(item) return ReplaceTexture(item);

	name = file2name();
	item = AddItem(name, img, imgw * imgh, TYPE_TEXTURE);
	if(!item) return 1;
	old = head->numtextures;
	temp = malloc(texture->size);
	if(!temp) return 1;
	memcpy(temp, texture->data, texture->size);
	texture = ReplaceItem(texture, NULL, texture->size + sizeof(texitem_t) + 4);
	head = (texhead_t*)texture->data;
	head->numtextures = old + 1;
	newtex = (texitem_t*)(texture->data + head->numtextures * 4 + 4);
	oldtex = (texitem_t*)(temp + old * 4 + 4);
	for(i = 0; i <= old; i++) {
		head->table[i] = 4 + 4 * head->numtextures + i * sizeof(texitem_t);
		if(i != old) {
			memcpy(newtex, oldtex, sizeof(texitem_t));
			newtex++;
			oldtex++;
		}
	}
	memset(newtex, 0, sizeof(texitem_t));
	newtex->width = imgw;
	newtex->height = imgh;
	newtex->numpatches = 1;
	newtex->num = old;
	memcpy(newtex->name, name, 8);
	free(temp);
	return ReplaceTexture(item);
}

int CreatePatch(item_t *item)
{
	int i;
	patchhead_t *head;

	if(item) {
		head = (patchhead_t*)item->data;
		if(head->width != imgw || head->height != imgh)
			item = ReplaceItem(item, NULL, imgw * imgh + sizeof(patchhead_t));
	} else {
		item = AddItem(file2name(), NULL, imgw * imgh + sizeof(patchhead_t), TYPE_PATCH);
	}

	head = (patchhead_t*)item->data;
	memset(head, 0, sizeof(patchhead_t));
	head->width = FlipS(imgw);
	head->height = FlipS(imgh);
	head->id = 0x0300;
	memcpy(item->data + sizeof(patchhead_t), img, imgw * imgh);
	return 0;
}

int CreateSprite(item_t *item)
{
	int x, y;
	int pixel;
	byte *b1, *b2;
	byte *out, *in;
	sprhead_t *head;
	sprdata_t *info;
	short *offs;
	char *name;

//	b1 = malloc(sizeof(sprhead_t) + 2 * imgw + imgh * sizeof(sprdata_t));
	b1 = malloc(64*1024);
	if(!b1) return 1;
	b2 = malloc(imgw * imgh);
	if(!b2) {
		free(b1);
		return 1;
	}
	head = (sprhead_t *)b1;
	offs = (short*)(b1 + sizeof(sprhead_t));
	info = (sprdata_t*)(b1 + sizeof(sprhead_t) + 2 * imgw);
	head->width = FlipS(imgw);
	head->height = FlipS(imgh);
	head->xoffs = head->yoffs = 0;
	out = b2;
	for(x = 0; x < imgw; x++) {
		in = img + x;
		*offs = FlipS((byte*)info - b1);
		offs++;
		pixel = 0;
		for(y = 0; y < imgh; y++) {
			if(!pixel && *in != 247) {
				info->offs = FlipS(out - b2);
				info->start = y;
				info->num = 1;
				pixel = 1;
			}
			if(pixel) {
				if(*in == 247) {
					info++;
					pixel = 0;
				} else {
					info->num++;
					*out = *in;
					out++;
				}
			}
			in += imgw;
		}
		if(pixel) info++;
		info->start = 0xFF;
		info->num = 0xFF;
		info->offs = 0xFFFF;
		info++;
	}

	if(item) {
		ReplaceItem(item->next, b2, out - b2);
		ReplaceItem(item, b1, (byte*)info - b1);
	} else {
		AddItem(file2name(), b1, (byte*)info - b1, TYPE_SPRITE);
		AddItem(".\0\0\0\0\0\0", b2, out - b2, TYPE_SPRITE);
	}

	free(b1);
	free(b2);
}

int ReplaceSound(int num)
{
	int size, extra;
	FILE *f;
	wavhead_t head;

	f = fopen(savename, "rb");
	if(!f) return 1;

	fread(&head, sizeof(wavhead_t), 1, f);
	if(	head.riff != 0x46464952 ||
		head.wave != 0x45564157 ||
		head.fmt != 0x20746d66 ||
		head.data != 0x61746164 ||
		head.format != 1 ||
		head.bits != 8 ||
		head.channels != 1
	) {
		fclose(f);
		return 2;
	}

	size = head.soundsize;
	if(size & 7) extra = 8 - (size & 7);
	else extra = 0;

	free(sounds[num]);
	sounds[num] = malloc(size + extra);
	memset(sounds[num], 128, size + extra);
	fread(sounds[num], size, 1, f);
	size += extra;
	sndtab[num].size = size >> 4;
	fclose(f);
	return 0;
}

int ReplaceMap(byte *data, int num)
{
	wadhead_t *head = (wadhead_t*)data;
	waditem_t *items;
	item_t *item;
	char name[8];
	int i;

	if(head->numlumps != 12) return 1;
	items = (waditem_t*)(data + head->offset);
	if(!compare("THINGS", items[1].name)) return 1;
	item = ItemByName(map_names[num]);
	if(!item) return 2; // should not happen
	item = item->next;
	for(i = 1; i < 11; i++) {
		item = ReplaceItem(item, data + items[i].offset, items[i].size);
		item = item->next;
	}
	sprintf(name, "LEAF%02i", num);
	item = ItemByName(name);
	if(item)
                item = ReplaceItem(item, data + items[i].offset, items[i].size);
	else
		item = AddItem(name, data + items[11].offset, items[i].size, TYPE_MAP);
}

void DeleteTexture(item_t *item)
{
	int i, num, j;
	texhead_t *head = (texhead_t*)texture->data;
	texitem_t *oldtex, *newtex;
	byte *temp;
	int *offs;

	num = head->numtextures - 1;
	temp = malloc(4 + 4 * num + sizeof(texitem_t) * num);
	if(!temp) return;
	oldtex = (texitem_t*)(texture->data + 4 + 4 * head->numtextures);
	newtex = (texitem_t*)(temp + 4 + 4 * num);
	offs = (int*)(temp + 4);
	j = 0;
	for(i = 0; i < head->numtextures; i++) {
		if(!compare(item->name, oldtex->name)) {
			*offs = 4 + 4 * num + j * sizeof(texitem_t);
			memcpy(newtex, oldtex, sizeof(texitem_t));
			newtex++;
			offs++;
			j++;
		}
		oldtex++;
	}
	head = (texhead_t*)temp;
	head->numtextures = num;
	texture = ReplaceItem(texture, temp, texture->size - sizeof(texitem_t) - 4);
	free(temp);
}

INT_PTR CALLBACK FlagCB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int ctrl, i;
	static int flags;
	switch(uMsg) {
		case WM_COMMAND:
			ctrl = LOWORD(wParam);
			if(ctrl == IDCANCEL) EndDialog(hwnd, 0);
			if(ctrl == IDOK) {
				mobjinfo[mobjedit].flags = flags;
				EndDialog(hwnd, 0);
			}
			if(ctrl >= IDC_CHK1 && ctrl <= IDC_CHK26)
				flags ^= (1 << (ctrl - IDC_CHK1));
		break;
		case WM_INITDIALOG:
			flags = mobjinfo[mobjedit].flags;
			for(i = 0; i < 26; i++)
				if(flags & (1 << i))
					SendDlgItemMessage(hwnd, IDC_CHK1 + i, BM_SETCHECK, BST_CHECKED, 0);
		break;
	}
	return FALSE;
}

int ReadValue(HWND hwnd, int id, int type)
{
	int i = 0;
	char temp[16];

	SendDlgItemMessage(hwnd, id, WM_GETTEXT, 16, (LPARAM)temp);
	if(type) sscanf(temp, "0x%X", &i);
	else sscanf(temp, "%i", &i);
	return i;
}

void SpriteByState(HWND hwnd, state_t *state)
{
	char temp[8];
	HWND center;

	center = GetDlgItem(hwnd, IDC_CENTER);
	if(center) {
		InvalidateRect(center, NULL, TRUE);
		UpdateWindow(center);
	}

	strcpy(temp, nametab[MAX_NAMETAB - state->sprite - 1].name);
	temp[4] = 'A' + state->frame & 0xFF;
	temp[5] = '*';
	temp[6] = 0;
	item = ItemByName(temp);
	if(item) DrawSprite(hwnd, item);
}

INT_PTR CALLBACK ActorCB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i, of, ctrl;
	char temp[32];
	static int prepare = 1;
	HWND center;
	switch(uMsg) {
		case WM_COMMAND:
			ctrl = LOWORD(wParam);
			if(ctrl >= IDC_EDT17 && ctrl <= IDC_EDT23) {
				if(prepare) break;
				if(mobjedit == -1) break;
				SendDlgItemMessage(hwnd, ctrl, WM_GETTEXT, 8, (LPARAM)temp);
				i = -1;
				sscanf(temp, "%i", &i);
				if(i == -1) {
					SendDlgItemMessage(hwnd, ctrl, WM_SETTEXT, 0, (LPARAM)"0");
					i = 0;
				}
				if(i == 0) break;
				SpriteByState(hwnd, &states[i]);
			}
			switch(ctrl) {
				case IDCANCEL:
					EndDialog(hwnd, 0);
				break;
				case IDC_BTN3:
					if(mobjedit == -1) break;
					of = mobjinfo[mobjedit].flags;
					DialogBox(hSavedInstance, MAKEINTRESOURCE(IDD_FLAG), hwnd, (DLGPROC)FlagCB);
					if((of & 0x10000) != (mobjinfo[mobjedit].flags & 0x10000)) {
						if(of & 0x10000) mobjinfo[mobjedit].speed >>= 16;
						else mobjinfo[mobjedit].speed <<= 16;
					}
				break;
				case IDC_CMB1:
					i = SendDlgItemMessage(hwnd, IDC_CMB1, CB_GETCURSEL, 0, 0);
					if(i == CB_ERR) {
						mobjedit = -1;
						break;
					}
					prepare = 1;
					center = GetDlgItem(hwnd, IDC_CENTER);
					if(center) {
						InvalidateRect(center, NULL, TRUE);
						UpdateWindow(center);
					}
					sprintf(temp, "Actor editor - %i", i);
					SetWindowText(hwnd, temp);
					mobjedit = i;
					// Actor info
					sprintf(temp, "%i", mobjinfo[i].doomednum);
					SendDlgItemMessage(hwnd, IDC_EDT3, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].spawnhealth);
					SendDlgItemMessage(hwnd, IDC_EDT4, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].reactiontime);
					SendDlgItemMessage(hwnd, IDC_EDT5, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].painchance);
					SendDlgItemMessage(hwnd, IDC_EDT6, WM_SETTEXT, 0, (LPARAM)temp);
					if(mobjinfo[i].flags & 0x10000)
						sprintf(temp, "%i", mobjinfo[i].speed >> 16);
					else
						sprintf(temp, "%i", mobjinfo[i].speed);
					SendDlgItemMessage(hwnd, IDC_EDT7, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].radius >> 16);
					SendDlgItemMessage(hwnd, IDC_EDT8, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].height >> 16);
					SendDlgItemMessage(hwnd, IDC_EDT9, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].mass);
					SendDlgItemMessage(hwnd, IDC_EDT10, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].damage);
					SendDlgItemMessage(hwnd, IDC_EDT11, WM_SETTEXT, 0, (LPARAM)temp);
					// Actor sounds
					sprintf(temp, "%i", mobjinfo[i].seesound);
					SendDlgItemMessage(hwnd, IDC_EDT12, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].attacksound);
					SendDlgItemMessage(hwnd, IDC_EDT13, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].painsound);
					SendDlgItemMessage(hwnd, IDC_EDT14, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].deathsound);
					SendDlgItemMessage(hwnd, IDC_EDT15, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].activesound);
					SendDlgItemMessage(hwnd, IDC_EDT16, WM_SETTEXT, 0, (LPARAM)temp);
					// Actor states
					sprintf(temp, "%i", mobjinfo[i].spawnstate);
					SendDlgItemMessage(hwnd, IDC_EDT17, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].seestate);
					SendDlgItemMessage(hwnd, IDC_EDT18, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].painstate);
					SendDlgItemMessage(hwnd, IDC_EDT19, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].meleestate);
					SendDlgItemMessage(hwnd, IDC_EDT20, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].missilestate);
					SendDlgItemMessage(hwnd, IDC_EDT21, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].deathstate);
					SendDlgItemMessage(hwnd, IDC_EDT22, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", mobjinfo[i].xdeathstate);
					SendDlgItemMessage(hwnd, IDC_EDT23, WM_SETTEXT, 0, (LPARAM)temp);
					prepare = 0;
					if(mobjinfo[i].spawnstate)
						SpriteByState(hwnd, &states[mobjinfo[i].spawnstate]);
				break;
				case IDOK:
					i = SendDlgItemMessage(hwnd, IDC_CMB1, CB_GETCURSEL, 0, 0);
					if(i == CB_ERR) break;
					// Actor info
					mobjinfo[i].doomednum = ReadValue(hwnd, IDC_EDT3, 0);
					mobjinfo[i].spawnhealth = ReadValue(hwnd, IDC_EDT4, 0);
					mobjinfo[i].reactiontime = ReadValue(hwnd, IDC_EDT5, 0);
					mobjinfo[i].painchance = ReadValue(hwnd, IDC_EDT6, 0);
					mobjinfo[i].speed = ReadValue(hwnd, IDC_EDT7, 0);
					if(mobjinfo[i].flags & 0x10000) mobjinfo[i].speed <<= 16;
					mobjinfo[i].radius = ReadValue(hwnd, IDC_EDT8, 0) << 16;
					mobjinfo[i].height = ReadValue(hwnd, IDC_EDT9, 0) << 16;
					mobjinfo[i].mass = ReadValue(hwnd, IDC_EDT10, 0);
					mobjinfo[i].damage = ReadValue(hwnd, IDC_EDT11, 0);
					// Actor sounds
					mobjinfo[i].seesound = ReadValue(hwnd, IDC_EDT12, 0);
					mobjinfo[i].attacksound = ReadValue(hwnd, IDC_EDT13, 0);
					mobjinfo[i].painsound = ReadValue(hwnd, IDC_EDT14, 0);
					mobjinfo[i].deathsound = ReadValue(hwnd, IDC_EDT15, 0);
					mobjinfo[i].activesound = ReadValue(hwnd, IDC_EDT16, 0);
					// Actor states
					mobjinfo[i].spawnstate = ReadValue(hwnd, IDC_EDT17, 0);
					mobjinfo[i].seestate = ReadValue(hwnd, IDC_EDT18, 0);
					mobjinfo[i].painstate = ReadValue(hwnd, IDC_EDT19, 0);
					mobjinfo[i].meleestate = ReadValue(hwnd, IDC_EDT20, 0);
					mobjinfo[i].missilestate = ReadValue(hwnd, IDC_EDT21, 0);
					mobjinfo[i].deathstate = ReadValue(hwnd, IDC_EDT22, 0);
					mobjinfo[i].xdeathstate = ReadValue(hwnd, IDC_EDT23, 0);
				break;
			}
		break;
		case WM_INITDIALOG:
			mobjedit = -1;
			for(i = 0; i < MAX_MOBJ; i++)
				SendDlgItemMessage(hwnd, IDC_CMB1, CB_ADDSTRING, 0, (LPARAM)mobjnames[i]);
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK StateCB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	char temp[8];
	switch(uMsg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
					EndDialog(hwnd, 0);
				break;
				case IDC_BTN1:
					SendDlgItemMessage(hwnd, IDC_EDT2, WM_GETTEXT, 8, (LPARAM)temp);
					i = -1;
					sscanf(temp, "%i", &i);
					if(i == -1) {
						i = 0;
						SendDlgItemMessage(hwnd, IDC_EDT2, WM_SETTEXT, 0, (LPARAM)"0");
					}
				DrawInfo:
					SendDlgItemMessage(hwnd, IDC_CMB1, CB_SETCURSEL, states[i].sprite, 0);
					sprintf(temp, "%i", states[i].frame & 0xFF);
					SendDlgItemMessage(hwnd, IDC_EDT3, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", states[i].nextstate);
					SendDlgItemMessage(hwnd, IDC_EDT4, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", states[i].tics);
					SendDlgItemMessage(hwnd, IDC_EDT5, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", states[i].misc1);
					SendDlgItemMessage(hwnd, IDC_EDT6, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "%i", states[i].misc2);
					SendDlgItemMessage(hwnd, IDC_EDT7, WM_SETTEXT, 0, (LPARAM)temp);
					sprintf(temp, "0x%08X", states[i].action);
					SendDlgItemMessage(hwnd, IDC_EDT8, WM_SETTEXT, 0, (LPARAM)temp);
					if(i == 0) break;
					SpriteByState(hwnd, &states[i]);
				break;
				case IDC_BTN2:
					SendDlgItemMessage(hwnd, IDC_EDT2, WM_GETTEXT, 8, (LPARAM)temp);
					i = -1;
					sscanf(temp, "%i", &i);
					if(i == -1) {
						i = 0;
						SendDlgItemMessage(hwnd, IDC_EDT2, WM_SETTEXT, 0, (LPARAM)"0");
					}
					i = states[i].nextstate;
					sprintf(temp, "%i", i);
					SendDlgItemMessage(hwnd, IDC_EDT2, WM_SETTEXT, 0, (LPARAM)temp);
					goto DrawInfo;
				break;
				case IDOK:
					SendDlgItemMessage(hwnd, IDC_EDT2, WM_GETTEXT, 8, (LPARAM)temp);
					i = -1;
					sscanf(temp, "%i", &i);
					if(i == -1) {
						i = 0;
						SendDlgItemMessage(hwnd, IDC_EDT2, WM_SETTEXT, 0, (LPARAM)"0");
					}
					states[i].sprite = SendDlgItemMessage(hwnd, IDC_CMB1, CB_GETCURSEL, 0, 0);
					states[i].frame = ReadValue(hwnd, IDC_EDT3, 0);
					states[i].nextstate = ReadValue(hwnd, IDC_EDT4, 0);
					states[i].tics = ReadValue(hwnd, IDC_EDT5, 0);
					states[i].misc1 = ReadValue(hwnd, IDC_EDT6, 0);
					states[i].misc2 = ReadValue(hwnd, IDC_EDT7, 0);
					states[i].action = ReadValue(hwnd, IDC_EDT8, 1);
					SpriteByState(hwnd, &states[i]);
				break;
			}
		break;
		case WM_INITDIALOG:
			for(i = MAX_NAMETAB - 1; i >= 0; i--)
				SendDlgItemMessage(hwnd, IDC_CMB1, CB_ADDSTRING, 0, (LPARAM)nametab[i].name);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK MainCB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i, j;
	char name[16];
	HMENU hMenu;
	sprhead_t *sprhead;
	int size;
	byte *data;
	item_t *tempitem;
	switch(uMsg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
					EndDialog(hwnd, 0);
				break;
				case IDC_LST1:
					if(mode == MODE_NONE) break;
					if(mode > MODE_OTHER && mode < MODE_SPRNAMES) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					SendDlgItemMessage(hwnd, IDC_LST1, LB_GETTEXT, i, (LPARAM)&name);
					if(mode == MODE_SPRNAMES) {
						SendDlgItemMessage(hwnd, IDC_EDT1, WM_SETTEXT, 0, (LPARAM)name);
						break;
					}
					item = ItemByName(name);
					if(!item) break;
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					switch(mode) {
						case MODE_FLATS:
							DrawFlat(hwnd, item->data);
						break;
						case MODE_TEXTURES:
							DrawTexture(hwnd, item->name, item->data);
						break;
						case MODE_PATCHES:
							DrawPatch(hwnd, item->data);
						break;
						case MODE_SPRITES:
							DrawSprite(hwnd, item);
						break;
						case MODE_OTHER:
							sprintf(name, "%ib", item->size);
							SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)name);
						break;
					}
				break;
				case IDC_BTN1:
					if(mode != MODE_SPRITES && mode != MODE_SPRNAMES) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					if(mode == MODE_SPRNAMES) {
						SendDlgItemMessage(hwnd, IDC_EDT1, WM_GETTEXT, 5, (LPARAM)name);
						if(strlen(name) != 4) break;
						strupr(name);
						memcpy(&nametab[MAX_NAMETAB - 1 - i], name, 4);
						SelectSpriteNames(hwnd);
						SendDlgItemMessage(hwnd, IDC_LST1, LB_SETCURSEL, i, 0);
						SendDlgItemMessage(hwnd, IDC_EDT1, WM_SETTEXT, 0, (LPARAM)name);
						break;
					}
					SendDlgItemMessage(hwnd, IDC_LST1, LB_GETTEXT, i, (LPARAM)&name);
					item = ItemByName(name);
					if(!item) break;
					SendDlgItemMessage(hwnd, IDC_EDT1, WM_GETTEXT, 16, (LPARAM)name);
					i = j = 32767;
					sscanf(name, "%i %i", &i, &j);
					if(i == 32767 || j == 32767) break;
					sprhead = (sprhead_t*)item->data;
					sprhead->xoffs = FlipS(i);
					sprhead->yoffs = FlipS(j);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					DrawSprite(hwnd, item);
				break;
				case IDC_BTN2:
					if(mode != MODE_SOUNDS) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					if(!PlaySoundData(sounds[i], sndtab[i].size << 4))
                                                MessageBox(hwnd, "Playback error.", "GBA Doom editor", 0);
				break;
				case IDC_CHK1:
					lineover ^= 1;
				break;
				case IDM_A2:
					if(!buff) break;
					i = ExportGame(buff + OFFS_WAD);
					if(i) {
						MessageBox(hwnd, export_err[i], "GBA Doom editor", 0);
						break;
					}
					f = fopen(filename, "wb");
					if(!f) {
						MessageBox(hwnd, "Failed to open file to save.", "GBA Doom editor", 0);
						break;
					}
					fwrite(buff, totalsize, 1, f);
					fclose(f);
				break;
				case IDM_A1:
					mode = MODE_NONE;
					SendDlgItemMessage(hwnd, IDC_LST1, LB_RESETCONTENT, 0, 0);
					if(!FileDialog(hwnd, 0)) break;
					ClearItems();
					if(buff) free(buff);
					buff = NULL;
					f = fopen(filename, "rb");
					if(!f) {
						MessageBox(hwnd, "Failed to open file.", "GBA Doom editor", 0);
						break;
					}
					fseek(f, 0, SEEK_END);
					totalsize = ftell(f);
					fseek(f, 0, SEEK_SET);
					buff = malloc(totalsize);
					if(!buff) {
						MessageBox(hwnd, "Failed to allocate enough memory.", "GBA Doom editor", 0);
						break;
					}
					fread(buff, totalsize, 1, f);
					fclose(f);
					if(LoadWad(buff + OFFS_WAD)) {
						MessageBox(hwnd, "Failed to load IWAD.", "GBA Doom editor", 0);
						break;
					}
					LoadSounds(buff + OFFS_SNDTAB);
					leaftab = (int*)(buff + OFFS_LEAFTAB);
					spritedef = (spritedef_t*)(buff + OFFS_SPRDEF);
					spritefrm = (spriteframe_t*)(buff + OFFS_SPRFRM);
					nametab = (sprname_t*)(buff + OFFS_NAMETAB);
					mobjinfo = (mobjinfo_t*)(buff + OFFS_MOBJ);
					states = (state_t*)(buff + OFFS_FRAME);
					mode = -1; // invalidate
					// fall trough
				case IDM_B1:
					if(mode == MODE_NONE) break;
					if(mode == MODE_FLATS) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C4, MF_ENABLED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)"64 x 64");
					SelectMode(hwnd, MODE_FLATS, TYPE_FLAT);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B2:
					if(mode == MODE_NONE) break;
					if(mode == MODE_TEXTURES) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C4, MF_ENABLED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectMode(hwnd, MODE_TEXTURES, TYPE_TEXTURE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B3:
					if(mode == MODE_NONE) break;
					if(mode == MODE_PATCHES) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C4, MF_ENABLED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectMode(hwnd, MODE_PATCHES, TYPE_PATCH);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B4:
					if(mode == MODE_NONE) break;
					if(mode == MODE_SPRITES) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C4, MF_ENABLED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectMode(hwnd, MODE_SPRITES, TYPE_SPRITE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B5:
					if(mode == MODE_NONE) break;
					if(mode == MODE_OTHER) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C4, MF_GRAYED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectMode(hwnd, MODE_OTHER, TYPE_NONE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B6:
					if(mode == MODE_NONE) break;
					if(mode == MODE_MAPS) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_CHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C4, MF_GRAYED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectMode(hwnd, MODE_MAPS, TYPE_MAP);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_B7:
					if(mode == MODE_NONE) break;
					if(mode == MODE_SOUNDS) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_CHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C2, MF_ENABLED);
					EnableMenuItem(hMenu, IDM_C3, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C4, MF_GRAYED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectSounds(hwnd);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_SHOW);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_D4:
					if(mode == MODE_NONE) break;
					if(mode == MODE_SPRNAMES) break;
					hMenu = GetMenu(hwnd);
					CheckMenuItem(hMenu, IDM_B1, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B2, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B3, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B4, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B5, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B6, MF_UNCHECKED);
					CheckMenuItem(hMenu, IDM_B7, MF_UNCHECKED);
					EnableMenuItem(hMenu, IDM_C1, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C2, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C3, MF_GRAYED);
					EnableMenuItem(hMenu, IDM_C4, MF_GRAYED);
					SendDlgItemMessage(hwnd, IDC_STC1, WM_SETTEXT, 0, (LPARAM)" ");
					SelectSpriteNames(hwnd);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN1), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_EDT1), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_CHK1), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN2), SW_HIDE);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_C1:
					if(mode == MODE_NONE) break;
					if(mode > MODE_SOUNDS) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					if(mode == MODE_SOUNDS) {
						if(!FileDialog(hwnd, 1)) break;
						f = fopen(savename, "wb");
						if(!f) {
							MessageBox(hwnd, "Failed to open file to save.", "GBA Doom editor", 0);
							break;
						}
						outwav.soundsize = sndtab[i].size << 4;
						outwav.size = 36 + outwav.soundsize;
						fwrite(&outwav, sizeof(wavhead_t), 1, f);
						fwrite(sounds[i], outwav.soundsize, 1, f);
						fclose(f);
						break;
					}
					SendDlgItemMessage(hwnd, IDC_LST1, LB_GETTEXT, i, (LPARAM)&name);
					item = ItemByName(name);
					if(!item) break;
					if(!FileDialog(hwnd, 1)) break;
					if(mode == MODE_OTHER) {
						f = fopen(savename, "wb");
						if(!f) {
							MessageBox(hwnd, "Failed to open file to save.", "GBA Doom editor", 0);
							break;
						}
						fwrite(item->data, item->size, 1, f);
						fclose(f);
						break;
					}
					if(mode == MODE_MAPS) {
						f = fopen(savename, "wb");
						if(!f) {
							MessageBox(hwnd, "Failed to open file to save.", "GBA Doom editor", 0);
							break;
						}
						wadhead.id = 0x44415750;
						wadhead.numlumps = NUM_MAP_LUMPS;
						wadhead.offset = 12 + CountMapData(item, NUM_MAP_LUMPS);
						fwrite(&wadhead, sizeof(wadhead_t), 1, f);
						WriteMapData(f, item);
						WriteMapDir(f, item);
						fclose(f);
						break;
					}
					img = malloc(128 * 1024);
					if(!img) {
						MessageBox(hwnd, "Failed to allocate enough memory.", "GBA Doom editor", 0);
						break;
					}
					f = fopen(savename, "wb");
					if(!f) {
						free(img);
						img = NULL;
						MessageBox(hwnd, "Failed to open file to save.", "GBA Doom editor", 0);
						break;
					}
					if(mode == MODE_SPRITES) memset(img, 247, 128 * 1024);
					switch(mode) {
						case MODE_FLATS:
							DrawFlat(NULL, item->data);
						break;
						case MODE_TEXTURES:
							DrawTexture(NULL, item->name, item->data);
						break;
						case MODE_PATCHES:
							DrawPatch(NULL, item->data);
						break;
						case MODE_SPRITES:
							DrawSprite(NULL, item);
						break;
					}
					memset(&bmphead, 0, sizeof(bmphead_t));
					bmphead.signature = 0x4D42;
					bmphead.filesize = imgw * imgh + sizeof(bmphead_t) + 1024;
					bmphead.imgstart = sizeof(bmphead_t) + 1024;
					bmphead.headsize = 40;
					bmphead.width = imgw;
					bmphead.height = imgh;
					bmphead.planes = 1;
					bmphead.bpp = 8;
					bmphead.imgsize = imgw * imgh;
					fwrite(&bmphead, sizeof(bmphead_t), 1, f);
					for(i = 0; i < 256; i++) {
						fputc(palette[i] >> 16, f);
						fputc((palette[i] >> 8) & 0xFF, f);
						fputc(palette[i] & 0xFF, f);
						fputc(0, f);
					}
					if(imgw & 3) j = 4 - (imgw & 3);
					else j = 0;
					for(i = 0; i < imgh; i++) {
						fwrite(img + (imgh - i - 1) * imgw, imgw, 1, f);
						if(j) fwrite(img, j, 1, f);
					}
					fclose(f);
					free(img);
					img = NULL;
				break;
				case IDM_C2:
					if(mode == MODE_NONE) break;
					if(mode == MODE_OTHER) break;
					if(mode > MODE_SOUNDS) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					if(mode == MODE_MAPS) {
						if(!FileDialog(hwnd, 0)) break;
						f = fopen(savename, "rb");
						if(!f) {
							MessageBox(hwnd, "Failed to open file.", "GBA Doom editor", 0);
							break;
						}
						fseek(f, 0, SEEK_END);
						size = ftell(f);
						fseek(f, 0, SEEK_SET);
						data = malloc(size);
						if(!data) {
							MessageBox(hwnd, "Failed to allocate enough memory.", "GBA Doom editor", 0);
							break;
						}
						fread(data, size, 1, f);
						fclose(f);
						if(ReplaceMap(data, i)){
							MessageBox(hwnd, "Failed to load map.\nIs it really map WAD?\nDoes it contain LEAFS?", "GBA Doom editor", 0);
							break;
						}
						free(data);
						break;
					}
					if(mode == MODE_SOUNDS) {
						if(!FileDialog(hwnd, 0)) break;
						if(ReplaceSound(i))
							MessageBox(hwnd, "Sound data error.", "GBA Doom editor", 0);
						break;
					}
					SendDlgItemMessage(hwnd, IDC_LST1, LB_GETTEXT, i, (LPARAM)&name);
					tempitem = ItemByName(name);
					if(!tempitem) break;
					// fall trough
				case IDM_C3:
					if(LOWORD(wParam) == IDM_C3) {
						if(mode == MODE_NONE) break;
						if(mode > MODE_SPRITES) break;
						tempitem = NULL;
					}
					if(!FileDialog(hwnd, 0)) break;
					f = fopen(savename, "rb");
					if(!f) {
						MessageBox(hwnd, "Failed to open file.", "GBA Doom editor", 0);
						break;
					}
					fread(&bmphead, sizeof(bmphead_t), 1, f);
					i = j = 0;
					if(mode == MODE_FLATS) i = j = 64;
					i = ReadBMP(f, &bmphead, i, j);
					fclose(f);
					if(i) {
						MessageBox(hwnd, "Something is wrong with this bitmap.", "GBA Doom editor", 0);
						break;
					}
					switch(mode) {
						case MODE_FLATS:
							CreateFlat(tempitem);
						break;
						case MODE_TEXTURES:
							CreateTexture(tempitem);
						break;
						case MODE_PATCHES:
							CreatePatch(tempitem);
						break;
						case MODE_SPRITES:
							CreateSprite(tempitem);
						break;
					}
					free(img);
					img = NULL;
					if(!tempitem) SelectMode(hwnd, mode, type_table[mode]);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_C4:
					if(mode == MODE_NONE) break;
					if(mode > MODE_SPRITES) break;
					i = SendDlgItemMessage(hwnd, IDC_LST1, LB_GETCURSEL, 0, 0);
					if(i == LB_ERR) break;
					SendDlgItemMessage(hwnd, IDC_LST1, LB_GETTEXT, i, (LPARAM)&name);
					item = ItemByName(name);
					if(!item) break;
					if(item->type == TYPE_TEXTURE) DeleteTexture(item);
					if(item->type == TYPE_SPRITE) DeleteItem(item->next);
					DeleteItem(item);
					SelectMode(hwnd, mode, type_table[mode]);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
				break;
				case IDM_D1:
					if(mode == MODE_NONE) break;
					i = mode;
					mode = MODE_HACKS;
					DialogBox(hSavedInstance, MAKEINTRESOURCE(IDD_MOBJ), hwnd, (DLGPROC)ActorCB);
					mode = i;
				break;
				case IDM_D3:
					if(mode == MODE_NONE) break;
					i = mode;
					mode = MODE_HACKS;
					DialogBox(hSavedInstance, MAKEINTRESOURCE(IDD_STATE), hwnd, (DLGPROC)StateCB);
					mode = i;
				break;
			}
		break;
	}
	return FALSE;
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
	hSavedInstance = hThisInstance;
	DialogBox(hThisInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainCB);
	ClearItems();
	if(buff) free(buff);
	return 0;
}

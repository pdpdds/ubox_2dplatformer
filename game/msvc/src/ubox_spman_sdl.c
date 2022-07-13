#include <spman.h>
#if !defined(HXWIN32)
#include <SDL2/SDL.h>

#include "cvector.h"

extern SDL_Renderer* g_renderer;
extern SDL_Surface* load_png(const char* filename);

#else
#include <SDL.h>
#include <SDL_image.h>

SDL_Surface* screen;
#endif

#include <game.h>

#define RAPID_DEVELOPMENT

SDL_Color sprite_pallete[16] =
{
	{255, 0, 255, 0},
	{0, 0, 0, 0},
	{102, 204, 102, 0 },
	{136, 238, 136, 0},
	{68, 68, 221, 0},
	{119, 119, 255, 0},
	{187, 85, 85, 0},
	{119, 221, 221, 0},
	{221, 102, 102, 0},
	{255, 119, 119, 0},
	{204, 204, 85, 0},
	{238, 238, 136, 0},
	{85, 170, 85, 0},
	{187, 85, 187, 0},
	{204, 204, 204, 0},
	{238, 238, 238, 0},
};

#define CVECTOR_LOGARITHMIC_GROWTH

const char max_pattern = 64;

typedef struct tag_pattern_info
{
	uint8_t* data;
	uint8_t pattern_length;
	uint8_t type;
	uint8_t flip;
}pattern_info;

typedef struct tag_image_info
{
	uint8_t color_count;
	SDL_Texture* texture;

}image_info;



#define MAX_IMAGE_LENGTH 10

image_info g_image_name[10] = { 0, };

cvector_vector_type(pattern_info) g_pattern_info = NULL;



void AddSprite(uint8_t index, const char* filename, uint8_t color_count)
{
	if(g_image_name[index].texture)
		SDL_DestroyTexture(g_image_name[index].texture);

	SDL_Surface* temp = load_png(filename);
	SDL_SetColorKey(temp, SDL_TRUE, SDL_MapRGB(temp->format, 28, 28, 28));
	g_image_name[index].texture = SDL_CreateTextureFromSurface(g_renderer, temp);
	g_image_name[index].color_count = color_count;
	SDL_FreeSurface(temp);
}

void spman_init()
{
#if defined(HXWIN32)
	return;
#endif

	cvector_free(g_pattern_info);
	g_pattern_info = NULL;
	
	int index = 0;
	AddSprite(index++, "player.png", 2);
	AddSprite(index++, "enemy.png", 1);
}

uint8_t spman_alloc_pat(uint8_t type, uint8_t* data, uint8_t len, uint8_t flip)
{
#if defined(HXWIN32)
	return 0;
#endif

	int index = 0;
	if (g_pattern_info) {
		int* it;

		for (it = cvector_begin((int*)g_pattern_info); it != cvector_end((int*)g_pattern_info); ++it)
		{
			pattern_info* info = (pattern_info*)it;

			if (info->data == data)
				return index * max_pattern;
			++index;
		}
	}

	pattern_info element = { data, len, type, flip };
	cvector_push_back(g_pattern_info, element);

	return index * max_pattern;
}

uint8_t RenderSprite(int type, int x, int y, int patternIndex, uint8_t flip)
{
	SDL_Rect srcRect;
	SDL_Rect dstRect;


	if (patternIndex == 1 || patternIndex == 3 || patternIndex == 5)
		patternIndex -= 1;

	patternIndex /= 4;

	if (g_image_name[type].color_count != 1)
	{
		if (patternIndex == 1)
			patternIndex = 0;
		if (patternIndex == 2 || patternIndex == 3)
			patternIndex = 1;
		if (patternIndex == 4 || patternIndex == 5)
			patternIndex = 2;
	}

	uint8_t src_x = patternIndex * 16;
	uint8_t src_y = 0;


	srcRect.x = src_x;
	srcRect.y = src_y;
	srcRect.w = 16;
	srcRect.h = 16;


	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = 16;
	dstRect.h = 16;

	SDL_RenderCopyEx(g_renderer, g_image_name[type].texture, &srcRect, &dstRect, 0, 0, flip);

	return 1;
}

void spman_alloc_fixed_sprite(struct sprite_attr* sp)
{
#if defined(HXWIN32)
	return;
#endif

	spman_alloc_sprite(sp);
}

// next pattern = prev pattern + 4
void spman_alloc_sprite(struct sprite_attr* sp)
{
#if defined(HXWIN32)
	return;
#endif

	uint8_t flip = 0;
	uint8_t type_index = sp->pattern / max_pattern;
	uint8_t pattern_index = sp->pattern % max_pattern;

	pattern_info info = g_pattern_info[type_index];
	uint8_t* data = info.data;

	if (pattern_index >= info.pattern_length * 4)
	{
		pattern_index -= info.pattern_length * 4;
		flip = 1;
	}

#if defined(RAPID_DEVELOPMENT)	
	RenderSprite(type_index, sp->x, sp->y, pattern_index, flip);
#else
	uint8_t color = 0;
	for (int i = 0; i < 4 * 8; i++)
	{
		color = data[pattern_index * 8 + i];

		for (int j = 0; j < 8; j++)
		{
			uint8_t mask = 0;

			mask = 1 << (7 - j);

			if (color & mask)
			{
				SDL_SetRenderDrawColor(g_renderer, sprite_pallete[sp->attr].r, sprite_pallete[sp->attr].g, sprite_pallete[sp->attr].b, 255);

				if (flip)
					SDL_RenderDrawPoint(g_renderer, sp->x + 16 - ((i / 16) * 8 + j), sp->y + (i % 16));
				else
					SDL_RenderDrawPoint(g_renderer, sp->x + ((i / 16) * 8 + j), sp->y + (i % 16));
			}
		}
	}
#endif
}

void spman_update()
{
#if defined(HXWIN32)

	SDL_Flip(screen);
#else
	SDL_RenderPresent(g_renderer);
#endif	
}

void spman_sprite_flush()
{

}

void spman_hide_all_sprites()
{

}
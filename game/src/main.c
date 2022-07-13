#include <stdio.h>
#include <stdint.h>

#include <ubox.h>
#include <mplayer.h>
#include "helpers.h"
#include "game.h"
#include <map_summary.h>
#include <tiles.h>

#define LOCAL
#include "main.h"


void draw_menu()
{
    uint8_t i = 0;

    ubox_disable_screen();

    ubox_fill_screen(WHITESPACE_TILE);

    // game logo; starts in tile 24 and it is 8 x 3 tiles
    for (i = 0; i < 9; ++i)
    {
        ubox_put_tile(i + 11, 6, 96 + i);
        ubox_put_tile(i + 11, 7, 128 + i);
        ubox_put_tile(i + 11, 8, 160 + i);
    }

    put_text(11, 11, (uint8_t*)"PRESS FIRE");

    put_text(6, 2, (uint8_t*)"UBOX MSX LIB DEMO 3!");
    put_text(10, 18, (uint8_t*)"JUHANG PARK");
    
    // index 95 , our Copyright sign
    ubox_put_tile(8, 21, 95);
    put_text(9, 21, (uint8_t*)"2021 USEBOX.NET");

    ubox_enable_screen();

    
    while (1)
    {
        ctl = ubox_select_ctl();

        if (ctl != UBOX_MSX_CTL_NONE)
        {
#if defined(__ANDROID__) || defined(WIN32) || defined(__linux) 
            if (ctl == UBOX_MSX_CTL_EXIT)
            {
                g_gamestate = STATE_EXIT;
                break;
            }
#endif
            mplayer_play_effect_p(EFX_START, EFX_CHAN_NO, 0);
            ubox_wait_for(16);
            g_gamestate = STATE_GAME_RESET;
            lives = MAX_LIVES;
            g_stage = 1;
            break;
        }

        ubox_wait();
    }    
}

void draw_end_game()
{
    ubox_disable_screen();

    ubox_fill_screen(WHITESPACE_TILE);

    put_text(3, 9, (uint8_t*)"CONGRATUATION!!");
    put_text(3, 12, (uint8_t*)"(PRESS ESC)");

    ubox_enable_screen();

    while (1)
    {
        if (ubox_read_keys(7) == UBOX_MSX_KEY_ESC)
            break;

        ubox_wait();
    }

    g_gamestate = STATE_TITLE;
}

void draw_game_over()
{
    ubox_disable_screen();

    ubox_fill_screen(WHITESPACE_TILE);
    put_text(11, 10, (uint8_t*)"GAME  OVER");

    ubox_enable_screen();

    mplayer_init(SONG, SONG_GAME_OVER);

    ubox_wait_for(128);

    g_gamestate = STATE_TITLE;
}

void draw_stage_clear()
{
    ubox_disable_screen();

    put_text(11, 10, (uint8_t*)"STAGE CLEAR");

    ubox_enable_screen();

    ubox_wait_for(96);

    g_gamestate = STATE_GAME_RESET;
    g_stage++;
}


void draw_stage_reset()
{
    if(g_stage > MAX_LEVEL || g_map[g_stage - 1] == 0)
    {
        g_gamestate = STATE_NO_MAP;
        return;
    }

    ubox_disable_screen();
    ubox_fill_screen(WHITESPACE_TILE);
    char buffer[10];
    sprintf(buffer, "STAGE %d", g_stage);
    put_text(12, 10, (uint8_t*)buffer);

    ubox_enable_screen();

    ubox_wait_for(96);

    g_gamestate = STATE_IN_GAME;
    
}

#if defined(HXWIN32)
#include <windows.h>
#include <SDL.h>
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)

#elif defined(WIN32) || defined(__linux)
#include <SDL2/SDL.h>
int main(int argc, char** argv)
#elif defined(SKYOS32)
#include <SDL.h>
int main(int argc, char** argv)
#elif defined(__ANDROID__)
#include <SDL.h>
int SDL_main(int argc, char** argv)
#else
void main()
#endif
{
    ubox_init_isr(2);

    ubox_set_mode(2);
    ubox_set_colors(1, 1, 1);

    ubox_disable_screen();

    ubox_set_tiles((uint8_t*)tiles);
    ubox_set_tiles_colors((uint8_t*)tiles_colors);

    ubox_fill_screen(WHITESPACE_TILE);

    ubox_enable_screen();
    ubox_wvdp(1, 0xe2);

    mplayer_init(SONG, SONG_SILENCE);
    mplayer_init_effects(EFFECTS);
    ubox_set_user_isr(mplayer_play);

    g_gamestate = STATE_TITLE;
    g_stage = 1;

    while (STATE_EXIT != g_gamestate)
    {
        switch (g_gamestate)
        {
        case STATE_TITLE:
            draw_menu();
            break;
        case STATE_IN_GAME:
            run_game(g_stage);

            if(g_gamestate == STATE_IN_GAME)
                g_gamestate = STATE_TITLE;            
            break;
        case STATE_GAME_RESET:
            draw_stage_reset();
            break;
        case STATE_GAME_CLEAR:
            draw_stage_clear();
            break;
        case STATE_GAME_OVER:
            draw_game_over();
            break;
        case STATE_NO_MAP:
            draw_end_game();
            break;    
        }

         ubox_wait();
    }
#if defined(__ANDROID__) || defined(WIN32)
    return 0;
#endif
   
}

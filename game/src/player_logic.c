#include "player_logic.h"
#include "game.h"
#include "game_util.h"
#include <spman.h>
#include <mplayer.h>
#include "main.h"
#include "foothold_logic.h"

uint8_t update_player_foothold();
uint8_t update_player_jump();
uint8_t update_player_ladder();

uint8_t check_player_floor()
{
    return check_floor(self->x + 4, self->y + 16) || check_floor(self->x + 12, self->y + 16);
}

uint8_t check_player_trap()
{
    return check_trap(self->x + 8, self->y + 16) || check_trap(self->x + 12, self->y + 16);
}


uint8_t check_player_ladder(int offset_x, int offset_y)
{
    return check_ladder(self->x, self->y + offset_y) && check_ladder(self->x + offset_x, self->y + offset_y);
}

struct entity* check_player_foothold()
{
    struct entity *object = check_foothold(self->x + 4, self->y + 16);

    if (object == 0)
    {
        object = check_foothold(self->x + 12, self->y + 16);
    }

    return object;
}

uint8_t update_player_move()
{
    uint8_t moved = 0;
    if (control & UBOX_MSX_CTL_RIGHT)
    {
        self->dir = DIR_RIGHT;
        moved = 1;

        if (self->x == 256 - 16)
        {
            g_cur_room_id += 1;
            self->x = 2;
            self->roomId = g_cur_room_id;

            move_next_room(g_cur_room_id);
        }
        else if (!is_map_blocked(self->x + 15, self->y + 15))
            self->x += 2;
    }

    if (control & UBOX_MSX_CTL_LEFT)
    {
        self->dir = DIR_LEFT;
        moved = 1;

        if (self->x == 2)
        {
            g_cur_room_id -= 1;
            self->roomId = g_cur_room_id;
            self->x = (uint8_t)(256 - 16);

            move_next_room(g_cur_room_id);
        }
        else if (!is_map_blocked(self->x, self->y + 15))
            self->x -= 2;
    }

    is_map_jewel(self->x + 8, self->y + 15);

    if (control & UBOX_MSX_CTL_FIRE1)
    {

        if (!self->flags)
        {
            self->flags = 1;

            struct entity *object = find_collide_object(self->x, self->y, ET_WARP);

            if (object)
            {
                mplayer_play_effect_p(EFX_ELEVATOR, EFX_CHAN_NO, 0);
                struct entity *next = find_object(object->extra);

                if (next)
                {
                    if (next->roomId != self->roomId)
                    {
                        g_cur_room_id = next->roomId;
                        self->roomId = g_cur_room_id;

                        move_next_room(next->roomId);
                    }

                    self->x = next->x;
                    self->y = next->y;
                }
            }
            else
            {
                moved = 1;
            }
        }
    }
    else
    {
        if (self->flags)
            self->flags = 0;
    }

    return moved;
}

void update_player_fall()
{
    self->y += 4;

    if(check_player_floor())
    {
        g_player_info.state = PS_NORMAL;

        self->y -= self->y % 8;
    }
}


void update_player_state(uint8_t moved)
{
    if (g_player_info.state == PS_FALL)
    {
        if (check_player_floor())
        {
            g_player_info.state = PS_NORMAL;
        }
        else if (check_player_foothold())
        {

            g_player_info.state = PS_FOOTHOLD;
        }
    }

    else if (g_player_info.state == PS_NORMAL && moved)
    {
        uint8_t loc_x = self->x;
        uint8_t loc_y = self->y + 16;

         if(!check_player_floor())
        {
            if(check_player_foothold())
            {
                g_player_info.state = PS_FOOTHOLD;
            }
            else
            {
                g_player_info.state = PS_FALL;
            }
        }
        else if (control & UBOX_MSX_CTL_FIRE1)
        {
            g_player_info.state = PS_JUMP;
        }
        
    }
    else if (g_player_info.state == PS_NORMAL)
    {
        
       if (control & UBOX_MSX_CTL_DOWN)
       {
            if (check_player_ladder(12, 16))
                g_player_info.state = PS_LADDER;
       }
       else if (control & UBOX_MSX_CTL_UP)
       {
            if (check_player_ladder(12, 0))
                g_player_info.state = PS_LADDER;
       }
      
    }
    else if (g_player_info.state == PS_FOOTHOLD && moved)
    {
        struct entity *object = check_player_foothold();
        
        if (!object)
        {
            if (check_player_floor())
            {
                g_player_info.state = PS_NORMAL;
            }
            else
                g_player_info.state = PS_FALL;
        }
    }
}

void update_player()
{
    uint8_t moved = 0;

    if (invuln)
        invuln--;

    if (!gameover_delay)
    {
        if (g_player_info.state == PS_NORMAL)
        {
            moved = update_player_move();
        }
        else if (g_player_info.state == PS_FALL)
        {
            update_player_fall();
        }
        else if (g_player_info.state == PS_FOOTHOLD)
        {
            moved = update_player_foothold();
        }
        else if (g_player_info.state == PS_LADDER)
        {
            moved = update_player_ladder();
        }
        else if (g_player_info.state == PS_JUMP)
        {
            moved = update_player_jump();
        }

        // 플레이어가 움직였다면 애니메이션을 갱신한다.
        if (moved)
        {
            //걷기 애니메이션을 갱신
            if (self->delay++ == FRAME_WAIT)
            {
                self->delay = 0;
                if (++self->frame == WALK_CYCLE)
                    self->frame = 0;
            }
        }
        else
        {
            //움직이지 않았다면 단순히 서있는 스프라이트를 그린다.
            if (self->frame)
            {
                self->frame = 0;
                self->delay = 0;
            }
        }

        struct entity *exitobject = find_collide_object(self->x, self->y, ET_EXIT);

        if (exitobject && jewels == 0)
            g_gamestate = STATE_GAME_CLEAR;
    }

    update_player_state(moved);

	if (check_player_trap())
	{
        if (!gameover_delay)
        {
            lives--;
            invuln = INVUL_TIME;

            if (!lives)
            {
                mplayer_init(SONG, SONG_SILENCE);
                mplayer_play_effect_p(EFX_DEAD, EFX_CHAN_NO, 0);
                gameover_delay = GAMEOVER_DELAY;
                g_gamestate = STATE_GAME_OVER;
            }
            else
            {
                mplayer_play_effect_p(EFX_HIT, EFX_CHAN_NO, 0);
                gameover_delay = GAMEOVER_DELAY;
                g_gamestate = STATE_GAME_RESET;
            }
        }
		

	}

    if (invuln & 1)
        return;

    sp.x = self->x;
    sp.y = self->y - 1;

    sp.pattern = self->pat + (walk_frames[self->frame] + self->dir * 3) * 8;
    sp.attr = 12;
    spman_alloc_fixed_sprite(&sp);

    sp.pattern += 4;
    sp.attr = 15;
    spman_alloc_fixed_sprite(&sp);
}

uint8_t update_player_foothold()
{
    uint8_t moved = update_player_move();

    struct entity *object = check_player_foothold();

    if (object)
    {
        if (object->delay == 0)
        {

            if ((!object->dir) && !is_map_blocked(self->x + 12, self->y + 15))
            {
                self->x += 1;
            }
            else if (object->dir && !is_map_blocked(self->x + 4, self->y + 15))
                self->x -= 1;
        }
    }

    return moved;
}

static uint8_t jump_dir = DIR_UP;
#define DEFAULT_PLAYER_JUMP_VELOCITY 7
uint8_t accel_y = 1;
uint8_t vel_y = DEFAULT_PLAYER_JUMP_VELOCITY;

void update_player_jump_up()
{
    if (vel_y > 0)
    {
        vel_y = vel_y - 1;
        self->y -= vel_y;

    }
    else
        jump_dir = DIR_DOWN;


}

void update_player_jump_down()
{
    if (vel_y < DEFAULT_PLAYER_JUMP_VELOCITY)
    {
        vel_y = vel_y + 1;

    }

    self->y += vel_y;


    if (check_player_floor())
    {
        self->y -= self->y % 8;

        g_player_info.state = PS_NORMAL;
        vel_y = DEFAULT_PLAYER_JUMP_VELOCITY;
        jump_dir = DIR_UP;
    }
}

uint8_t update_player_jump()
{
     uint8_t x_pos = self->x;
   
    if ((!self->dir) && (control & UBOX_MSX_CTL_RIGHT) && !is_map_blocked(self->x + 12, self->y + 15))
    {
        self->x += 2;

        if (is_map_blocked(self->x + 12, self->y + 15))
            self->x = x_pos;

    }
    else if (self->dir && (control & UBOX_MSX_CTL_LEFT) && !is_map_blocked(self->x + 4, self->y + 15))
    {
        self->x -= 2;

        if (is_map_blocked(self->x + 4, self->y + 15))
            self->x = x_pos;
      
    }

    if (jump_dir == DIR_UP)
    {
        update_player_jump_up();
    }
    else
    {
        update_player_jump_down();
    }


   

  

    return 1;
}

uint8_t update_player_ladder()
{
    uint8_t moved = 0;
    if (control & UBOX_MSX_CTL_DOWN)
    {
        //self->dir = DIR_RIGHT;
        moved = 1;
        self->y += 2;

        if (check_player_floor())
        {
            g_player_info.state = PS_NORMAL;
        }
       
           
    }

    if (control & UBOX_MSX_CTL_UP)
    {
        //self->dir = DIR_LEFT;
        moved = 1;

        self->y -= 2;

        if (!check_player_ladder(12, 16))
        {
            g_player_info.state = PS_NORMAL;
        }
    }

    return moved;
}

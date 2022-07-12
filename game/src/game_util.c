#include "game_util.h"
#include "game.h"
#include <ubox.h>

struct entity* check_foothold(uint8_t x, uint8_t y)
{
    uint16_t i;
    struct entity *object;

    for (i = 0, object = entities; i < g_maxEntities; i++, object++)
    {
        if (object->type == ET_FOOTHOLD && g_cur_room_id == object->roomId)
        {
            if ((x >= object->x && (x <= (object->x + 16))) && y == object->y)
            {
                return object;
            }
        }
    }

    return 0;
}

uint8_t check_floor(uint8_t x, uint8_t y)
{
    uint8_t tile_index = cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W];

    if (tile_index == LADDER1_TILE || tile_index == LADDER2_TILE)
        return 1;

    if (tile_index == LADDER1_PART_TILE || tile_index == LADDER2_PART_TILE)
        return 0;

    return tile_index >= 192;
}

uint8_t check_trap(uint8_t x, uint8_t y)
{
    uint8_t tile = cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W];
    return tile == TRAP1_TILE || tile == TRAP2_TILE;
}

uint8_t check_ladder(uint8_t x, uint8_t y)
{
    uint8_t tile = cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W];
    return tile == LADDER1_TILE || tile == LADDER2_TILE || tile == LADDER1_PART_TILE || tile == LADDER2_PART_TILE;
}

struct entity *find_object(uint8_t id)
{
    uint16_t i;
    struct entity *object;
    for (i = 0, object = entities; i < g_maxEntities; i++, object++)
    {
        if (id == object->identifier)
            return object;
    }

    return 0;
}

struct entity *find_collide_object(uint8_t x, uint8_t y, int type)
{
    uint16_t i;
    struct entity *object;

    for (i = 0, object = entities; i < g_maxEntities; i++, object++)
    {
        if (object->type == type && g_cur_room_id == object->roomId)
        {
            if (x > object->x - 4 && (x + 4) < (object->x + 16) && y == object->y)
            {
                return object;
            }
        }
    }

    return 0;
}

// x and y in pixels
uint8_t is_map_blocked(uint8_t x, uint8_t y)
{
    uint8_t tile_index = cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W];

    if (tile_index == LADDER1_PART_TILE || tile_index == LADDER2_PART_TILE)
        return 0;

    //return cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W] < LAST_SOLID_TILE + 1;
    return cur_map_data[self->roomId][(x >> 3) + (y >> 3) * MAP_W] >= 192;
}

uint8_t is_map_jewel(uint8_t x, uint8_t y)
{
    uint16_t i;
    struct entity *object;
    for (i = 0, object = entities; i < g_maxEntities; i++, object++)
    {

        if (object->type == ET_KEY && object->roomId == g_cur_room_id)
        {
            if (((x >> 3) == (object->x >> 3)) && ((y >> 3) == (object->y >> 3)))
            {
                ubox_put_tile(object->x >> 3, object->y >> 3, BLANK_TILE);
                object->type = ET_UNUSED;
                jewels--;
                return 1;
            }
        }
    }

    return 0;
}
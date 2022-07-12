#ifndef _MAP_SUMMARY_H
#define _MAP_SUMMARY_H
#define MAX_LEVEL 1
#ifdef LOCAL
#include "map.h"
unsigned char** g_map[MAX_LEVEL + 1] =  {
   map1, 
   0,
};
unsigned char g_map_room_count[MAX_LEVEL + 1] =  {
   MAP1_RW_COUNT, 
   0,
};
#else
extern unsigned char** g_map[MAX_LEVEL + 1];
extern unsigned char g_map_room_count[MAX_LEVEL + 1];
#endif // LOCAL
#endif // _MAP_SUMMARY_H

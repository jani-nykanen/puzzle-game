/// Stage (header)
/// (c) 2018 Jani Nykänen

#ifndef __STAGE__
#define __STAGE__

#include "../engine/assets.h"

/// Initialize stage
/// < ass Asset pack
void stage_init(ASSET_PACK* ass);

/// Update stage
/// < tm Time mul.
void stage_update(float tm);

/// Draw stage
void stage_draw();

/// Get collision map
/// > Collision map
int* stage_get_collision_map();

#endif // __STAGE__
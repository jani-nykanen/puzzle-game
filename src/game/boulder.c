/// Boulder (source)
/// (c) 2018 Jani Nykänen

#include "boulder.h"

#include "../engine/graphics.h"
#include "../engine/sample.h"

#include "../vpad.h"

#include "stage.h"
#include "player.h"
#include "objects.h"

#include "stdio.h"
#include "math.h"

// Boulder bitmap
static BITMAP* bmpBoulder;

// Sound effects
static SAMPLE* sThwomp;
static SAMPLE* sTransf;
static SAMPLE* sPush;


// Get gravity
static void b_get_gravity(BOULDER* b)
{
    b->falling = false;
    b->gravity = 0.0f;

    int oldy = b->y;
    while(!stage_is_solid(b->x,b->y+1))
    {
        ++b->y;
    }

    if(b->y != oldy)
    {
        b->preventMovement = true;
        b->falling = true;
        stage_set_collision_tile(b->x,oldy,0);
        stage_set_collision_tile(b->x,b->y,1);
    }
}


// Update boulder location to collision map
static void b_update_location(BOULDER* b)
{
    if(!stage_is_lava(b->x,b->y))
        stage_set_collision_tile(b->x,b->y,1);

    else if(b->spr.frame == 5)
    {
        b->exist = false;
        stage_set_collision_tile(b->x,b->y,1);
        stage_set_tile(b->x,b->y,5);
        return;
    }
}


// Fall
static void b_fall(BOULDER* b, float tm)
{
    const float GRAV_MAX = 4.0f;
    const float GRAV_SPEED = 0.2f;

    float target = b->y*16.0f;

    // If close to lava, start changing to soil
    if(stage_is_lava(b->x,b->y) && fabs(target-b->vpos.y) <= 16.0f)
    {
        b->changing = true;
        play_sample(sTransf,0.50f);
    }

    if(b->vpos.y < target)
    {
        b->gravity += GRAV_SPEED * tm;
        if(b->gravity > GRAV_MAX)
        {
            b->gravity = GRAV_MAX;
        }
        b->vpos.y += b->gravity * tm;

        if(b->vpos.y >= target)
        {
            b->vpos.y = target;
            b->falling = false;

            if(!b->changing)
                play_sample(sThwomp,0.60f);
        }
    }
}


// Move boulder
static void b_move(BOULDER* b,float tm)
{
    float target = b->x * 16.0f;

    b->vpos.x += 0.75f * b->dir * tm;

    if((b->dir == 1 && b->vpos.x > target) || (b->dir == -1 && b->vpos.x < target))
    {
        b->moving = false;
        b_get_gravity(b);
        b->vpos.x = target;
    }
}


// Boulder-player collision
static void boulder_player_collision(void* o, void* p)
{
    const float DELTA = 0.1f;

    PLAYER* pl = (PLAYER*)p;
    BOULDER* b = (BOULDER*)o;

    if(!b->exist || !obj_can_move()) return;
    if(b->moving)
    {
        return;
    }

    VEC2 stick = vpad_get_stick();

    // Push
    if(pl->canMove && !pl->bouncing && !pl->moving && !b->falling 
       && pl->y == b->y && abs(pl->x-b->x) == 1 
       && stage_is_solid(pl->x,pl->y+1)
       && fabs(stick.x) > DELTA)
    {
        b->dir = stick.x > 0.0f ? 1 : -1;
        int pdir = pl->x > b->x ? -1 : 1;
        if(b->dir != pdir) return;

        if(!stage_is_solid(b->x+b->dir,b->y))
        {
            stage_set_collision_tile(b->x,b->y,0);
            b->oldx = b->x;
            b->x += b->dir;
            pl->pushing = true;
            b->moving = true;

            play_sample(sPush,0.60f);
        }
    }

    // Fall if not being pushed and the player is not moving
    if(!b->falling && !b->moving && !stage_is_solid(b->x,b->y+1))
    {
        b_get_gravity(b);
    }
}


// Update boulder
static void boulder_update(void* o, float tm)
{
    BOULDER* b = (BOULDER*)o;
    
    b->preventMovement = false;

    if(!b->exist) return;
    if(b->moving)
    {
        b_move(b,tm);
    }
    if(b->falling)
    {
        b->preventMovement = true;
        b_fall(b,tm);
    }

    if(b->changing)
    {
        b->preventMovement = true;
        if(b->spr.frame < 5)
            spr_animate(&b->spr,0,0,5,6,tm);
    }
    else
    {
        spr_animate(&b->spr,0,0,0,0,tm);
    }

    b_update_location(b);
}


// Draw boulder
static void boulder_draw(void* o)
{
    BOULDER* b = (BOULDER*)o;
    if(b->exist == false) return;

    spr_draw(&b->spr,bmpBoulder,(int)round(b->vpos.x),(int)round(b->vpos.y) +1,0);
}


// Reset boulder
static void boulder_reset(void* o)
{
    BOULDER* b = (BOULDER*)o;
    b->exist = true;
    b->falling = false;
    b->moving = false;
    b->changing = false;
    b->spr.frame = 0;
    b->spr.count = 0;

    stage_set_collision_tile(b->x,b->y,1);
}


// Initialize
void boulder_init(ASSET_PACK* ass)
{
    // Get asset
    bmpBoulder = (BITMAP*)get_asset(ass,"boulder");
    sThwomp = (SAMPLE*)get_asset(ass,"thwomp");
    sTransf = (SAMPLE*)get_asset(ass,"transf");
    sPush = (SAMPLE*)get_asset(ass,"push");
}


// Create a new boulder
BOULDER boulder_create(int x, int y)
{
    BOULDER b;

    b.x = x;
    b.y = y;
    b.vpos = vec2(x*16.0f,y*16.0f);
    b.spr = create_sprite(16,16);
    b.onDraw = boulder_draw;
    b.onUpdate = boulder_update;
    b.onPlayerCollision = boulder_player_collision;
    b.onReset = boulder_reset;
    b.exist = true;
    b.moving = false;
    b.falling = false;
    b.changing = false;
    b.preventMovement = false;
    b.oldx = x;
    b.gravity = 0.0f;

    stage_set_collision_tile(b.x,b.y,1);

    return b;
}

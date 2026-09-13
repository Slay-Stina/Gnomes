#include "collision.h"

/*
 Kollar ifall punkten är innanför eller på kanten av rektangeln
 Kollision är sann ifall alla är sanna
                        * <- y < bounds.y
                    ▐▀▀▀▀▀▀▌
x < bounds.x -> *   ▐      ▌  * <- x > bounds.x + bounds.w
                    ▐▄▄▄▄▄▄▌
                        * <- y > bounds.y + bounds.h
 */

bool CheckCollisionInsideBounds(SDL_FRect bounds, float x, float y) {
    return x >= bounds.x && x <= bounds.x + bounds.w && y >= bounds.y && y <= bounds.y + bounds.h;
}

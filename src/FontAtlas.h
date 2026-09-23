#pragma once
#include <SDL3/SDL.h>

struct Glyph {
    SDL_FRect atlasPosition;
};

struct FontAtlas {
    static constexpr int GLYPH_COUNT = 128;

    SDL_Texture* atlasTexture = nullptr;

    void LoadFont( SDL_Renderer* renderer, const char* fontPath, float fontSize );

    Glyph GetGlyph( int index ) const;

private:
    Glyph glyphs[GLYPH_COUNT] = {};
};

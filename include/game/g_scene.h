#ifndef G_SCENE_H
#define G_SCENE_H

#define G_SCENE_INIT { .init=NULL, .process_input=NULL, .update=NULL, .draw=NULL, .end=NULL }

#include <stdbool.h>

#include <sdl/SDL.h>

typedef struct g_scene {
    void (*init)(void);

    void (*process_input)(const SDL_Event*);//const SDL_Event* e
    void (*update)(float);//const float delta_time
    void (*draw)(SDL_Renderer*);

    void (*end)(void);
} g_scene;

void g_scene_change(g_scene new_scene);
bool g_scene_run(SDL_Renderer* renderer);
void g_scene_request_quit(void);

#endif//G_SCENE_H
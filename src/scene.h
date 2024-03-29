#pragma once
#include "headers.h"
#include "jukebox.h"
#include "enemy.h"
#include "player.h"
#include "text.h"
#include "framebuffer.h"
#include "controller.h"
#include "hud.h"
#include "bullets.h"
#include "buttons.h"

class LevelScene {
private:
    vector<Bullet *> stars_l1;
    vector<Bullet *> stars_l2;
    vector<Enemy * > enemies = {};
    vector<int> erased_enemy_i = {};
    Player * player = nullptr;
    double time_left = 0.0;
    double countdown = 0.0;
    int countdown_n = 4;
    double shoot_timer = 0.0;
    double shot_interval = 0.0;
    string direction = "left";
    string enemy_state = "PHASE_LEFT";
    string last_state;
    int enemies_dead = 0;
    AnimatedSprite * countdown_sprite;
    SDL_Renderer * renderer;
    Framebuffer * framebuffer;
    TextCache * text_renderer;
    Hud * hud;
    SDL_RendererFlip * flip;
    bool filling_stars = true;
    bool options = false;
    bool winner = false;
public:
    bool starting;
    bool running;
    bool finished;
    bool paused;
    LevelScene(SDL_Renderer * renderer, Framebuffer * Framebuffer, SpriteCache * , TextCache *,SDL_RendererFlip * flip);
    
    void AddEnemy(Enemy * enemy);
    void AddPlayer(Player * player);
    void CreateHUD(Player * player);
    void Reset(Jukebox * jukebox);
    void Process(Clock * clock, KeyboardManager * keyboard, MouseManager * mouse, ControllerManager * controllers, Jukebox * jukebox, string *state, int width, int height);
    void ManageEnemies(Clock * clock, ControllerManager * controllers, Jukebox * jukebox, int width, int height);
    void RenderScene();

    ~LevelScene();
};

class MenuScene {
    private:
        Player * player;
        SDL_RendererFlip * flip;
        SpriteCache * cache;
        Framebuffer * framebuffer;
        AnimatedSprite * title;
        map<string, Button *> buttons;
        map<string, Button *> level_options;
        vector<Bullet *> stars;
        SDL_Renderer * renderer;
        TextCache * text_cache;
        double seconds_passed = 0.0;
        double animate_interval = 0.0;
        double song_ending_time = 0.0;
        bool select_options = false;

    public:
        bool starting;
        bool running;
        bool finished;
        MenuScene(SpriteCache *, Framebuffer * framebuffer, TextCache *, SDL_RendererFlip *, Player *);
        ~MenuScene();
        bool Process(Clock * clock, MouseManager * mouse, Jukebox * jukebox, string * state, string * scene_path);
        void RenderScene();
};
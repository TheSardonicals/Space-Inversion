#include "scene.h"

LevelScene::LevelScene(SDL_Renderer * r,Framebuffer * framebuffer, SpriteCache * sprite_cache, TextCache * text_cache, SDL_RendererFlip * flip){
    this->framebuffer = framebuffer;
    this->hud = new Hud(sprite_cache, framebuffer, player,text_cache);
    starting = true;
    running = false;
    finished = false;
    paused = false;
    renderer = r;
    shot_interval = 1;
    this->flip = flip;
    input_time = .1;
}

void LevelScene::AddEnemy(Enemy * enemy){
    enemies.push_back(enemy);
}

void LevelScene::AddPlayer(Player * p){
    player = p;
    hud->player = p;
}

void LevelScene::Process(Clock * clock, KeyboardManager * keyboard, ControllerManager * controllers, Jukebox * jukebox,  int width, int height){
    input_timer += clock->delta_time_s;

    if (starting){
        // This is here in case we need to set individual player state based on stuff.

        // below we are creating random stars to populate the level, using different layering.
        for (int i=0; i < 9; i++){
            int random_x = rand() % (width-5) + 10;
            int random_y = rand() % (height-5) + 10;
            stars_l1.push_back(new Bullet(renderer, random_x, random_y, 5, 5, {255, 255, 255, 255}));
        }

        for (int i=0; i < 5; i++){
            int random_x = rand() % (width-5) + 10;
            int random_y = rand() % (height-5) + 10;
            stars_l2.push_back(new Bullet(renderer, random_x, random_y, 5, 5, {255, 255, 255, 255}));
        }

        jukebox->PlayMusic("stage_music");
        running = true;
        starting = false;
    }

    if (running){
        if ((keyboard->KeyIsPressed(SDL_SCANCODE_P)&& (input_timer >= input_time)) || controllers->GetControllerButtonWasPressed(0, "START")){
            if (paused){
                paused = false;
                jukebox->ResumeMusic();
                jukebox->ResumeSoundEffects();
            }
            else {
                paused = true;
                jukebox->PauseMusic();
                jukebox->PauseSoundEffects();
            }
        }
        
        if (!paused){
            // Managing movement for player
            if (keyboard->KeyIsPressed(SDL_SCANCODE_A) || controllers->GetControllerButtonPressed(0, "LEFT")){
                player->Move("left");
            }
            else if (keyboard->KeyIsPressed(SDL_SCANCODE_D) || controllers->GetControllerButtonPressed(0, "RIGHT")) {
                player->Move("right");
            }
            else{
                player->Move("none");
            }

            if (keyboard->KeyIsPressed(SDL_SCANCODE_SPACE) || controllers->GetControllerButtonPressed(0, "X")) {
                if (player->Attack()){
                    jukebox->PlaySoundEffect("blast");
                    controllers->SetControllerRumble(0, 20, 0, .3);
                } 
            }

            if (keyboard->KeyWasPressed(SDL_SCANCODE_R)) {
                Reset(jukebox);
            }

            // make sure the player can't move outta bounds.
            if (player->d_rect.x < -1){
                player->Move("none");
                player->x_pos += 1;
            }
            else if (player->d_rect.x > width - (player->d_rect.w - 1)){
                player->Move("none");
                player->x_pos -= 1;
            }

            // Process player and enemies.
            player->Process(clock);
            ManageEnemies(clock, controllers, jukebox, width, height);

            // Move the stars.
            for (auto star: stars_l1){
                star->y_pos += (3 * 100) * clock->delta_time_s;
                if (star->y_pos >= height){
                    star->y_pos = star->y_pos - height;
                }
            }
            for (auto star: stars_l2){
                star->y_pos += (3 * 100) * clock->delta_time_s;
                if (star->y_pos >= height){
                    star->y_pos = star->y_pos - height;
                }
            }
        }

        if (input_timer > input_time){
            input_timer = 0.0;
        }
    }
}


// This method handles all of the specific interactions between enemies (of different types), and the player, as well
// as that's important for interactions.
void LevelScene::ManageEnemies(Clock * clock, ControllerManager * controllers, Jukebox * jukebox, int width, int height){
    if (enemy_state == "PHASE_DOWN"){
        countdown += clock->delta_time_s;
    }

    shoot_timer += clock->delta_time_s;

    int random_index = 0;
    if (enemies.size()){
        random_index = rand() % enemies.size();
    }

    enemies_dead = 0;
    for (int i = 0; i < enemies.size();i++){
        /*
        "DYING" state is captured quicker than dead boolean or dead state
        (as enemy plays dying animation before the dead boolean is set),
        allowing the score to be set immediately upon the enemy dying
        */ 
        if (enemies[i]->state == "DYING" || enemies[i]->state == "DEAD"){
            enemies_dead++;
        }
    }

    hud->SetScore(enemies_dead * 200);

    for (int i = 0; i < enemies.size(); i++){

        //"player is dying" is used to check if the player is dying, so that events respond accordingly.
        bool player_is_dying = (player->state == "DYING") || (player->state == "RESPAWNING");

        // Process the enemy as soon as it comes up.
        if (!player_is_dying) {
            // select a random ship to shoot at the player.
            if (!enemies[i]->dead){
                if (shoot_timer >= shot_interval){
                    if (i == random_index){
                        if (enemies[i]->Attack()){
                            jukebox->PlaySoundEffect("blast");
                        }       
                    }
                }
            }

            if (!enemies[i]->dead){
            double speed_multiplier = double(enemies_dead)/double((enemies.size()-1))*12;
            enemies[i]->speed = int(speed_multiplier) + enemies[i]->default_speed;
            }

            if (!enemies[i]->dead){
                if (enemy_state == "PHASE_LEFT"){
                    enemies[i]->Move("left");
                    if (enemies[i]->d_rect.x <= 0){
                        last_state = enemy_state;
                        enemy_state = "PHASE_DOWN";
                    } 
                }
                else if (enemy_state == "PHASE_RIGHT"){
                    enemies[i]->Move("right");
                    if (enemies[i]->d_rect.x >= width - enemies[i]->d_rect.w){
                        last_state = enemy_state;
                        enemy_state = "PHASE_DOWN";
                    }
                }
                if (enemy_state == "PHASE_DOWN"){
                    enemies[i]->Move("down");
                    if (countdown >= .2){
                        if (last_state == "PHASE_LEFT"){
                            enemy_state = "PHASE_RIGHT";
                            countdown = 0.0;
                        }
                        else{
                            enemy_state = "PHASE_LEFT";
                            countdown = 0.0;
                        }
                    }
                }     
            }
            //check if the player collided with any of the enemies
            if (player->TouchingEnemy(&enemies[i]->d_rect)){
                if (enemies[i]->state != "DYING"){
                    player->Hurt();
                    controllers->SetControllerRumble(0, 0, 60, .3);
                    jukebox->PlaySoundEffect("dying_p");
                }
                enemies[i]->state = "DYING";
            }

            // check if the player collided with any of the enemy bullets.
            int bullet_index = 0;
            if (enemies[i]->TouchingBullet(&player->d_rect)){
                for (auto bullet: enemies[i]->bullets){
                    if (bullet->IsTouchingRect(&player->d_rect)){
                        /*
                            TODO: only use this logic for basic pawn bullets. 
                            differentiate when "projectile" class is created and used
                            instead.
                        */
                        if (!bullet->hit){
                            player->Hurt();
                            controllers->SetControllerRumble(0, 0, 60, .3);
                            jukebox->PlaySoundEffect("dying_p");

                            if (*flip == SDL_FLIP_NONE){
                                *flip = SDL_FLIP_VERTICAL;
                            } else {
                                *flip = SDL_FLIP_NONE;
                            }
                        }
                        bullet->hit = true;
                        enemies[i]->erased.push_back(bullet_index);
                    }
                    bullet_index++;
                }
            }
        }
        else {
            enemies[i]->Move("none");
        }

        // check if the enemy collided with any of the players bullets.
        int index = 0;
        if (player->TouchingBullet(&enemies[i]->d_rect)){
            for (auto bullet: player->bullets){
                if (enemies[i]->state != "DYING"){
                    if (bullet->IsTouchingRect(&enemies[i]->d_rect)){
                        if (!bullet->hit) {
                            bullet->hit = true;
                            player->erased.push_back(index); 
                        }
                    }
                    jukebox->PlaySoundEffect("dying");
                } 
                index++;  
            }
            enemies[i]->state = "DYING";   
        }

        //reset enemies to the top if they go off screen
        if (enemies[i]-> y_pos >= 600){
            enemies[i]->SetPos(enemies[i]->x_pos,0);
        }
        enemies[i]->Process(clock, height);
    }

    if (shoot_timer >= shot_interval){shoot_timer = 0.0;}

    // finally, destroy the enemy ship if it is dead.
    // for ( auto enemy = enemies.begin(); enemy != enemies.end(); ) {
    //     if( (*enemy)->dead ) {
    //         delete * enemy;  
    //         enemy = enemies.erase(enemy);
    //     }
    //     else {
    //         ++enemy;
    //     }
    // }

}

void LevelScene::Reset(Jukebox * jukebox){
    *flip = SDL_FLIP_NONE;

    for (auto enemy: enemies){
        enemy->Reset();        
    }

    player->Reset();
    jukebox->StopMusic();
    jukebox->StopSoundEffects();
    jukebox->PlayMusic("stage_music");
}

void LevelScene::RenderScene(){

    //Rendering
    framebuffer->SetActiveBuffer("GAME");
    SDL_SetRenderDrawColor(renderer, 9, 21, 61, 255);
    SDL_RenderClear(renderer);

    for (auto star: stars_l1){
        star->Render();
    }
    for (auto enemy: enemies){
        enemy->Render(); 
    }

    player->Render();

    for (auto star: stars_l2){
        star->Render();
    }

    framebuffer->UnsetBuffers();

    hud->Render();
}

LevelScene::~LevelScene(){
    for (int i=0; i < stars_l1.size(); i++){
        delete stars_l1[i];
    }
    for (int i=0; i < stars_l2.size(); i++){
        delete stars_l2[i];
    }
    for (int i=0; i < enemies.size(); i++){
        delete enemies[i];
    }

	delete hud;
}


MenuScene::MenuScene(SpriteCache * cache, Framebuffer * framebuffer){
    this->framebuffer = framebuffer;
    this->cache = cache;
    for (int i=0; i < 40; i++){
            int random_x = rand() %  (1280 - 5)+ 10;
            int random_y = rand() % (720 - 5) + 10;
            stars.push_back(new Bullet(cache->renderer, random_x, random_y, 5, 5, {255, 255, 255, 255}));
        }
    starting = true;
    running = false;
    finished = false;
    renderer = cache->renderer;

    title = new AnimatedSprite(cache, {0, 0, 64, 64}, {640, 270, 700, 380}, "resources/title.bmp", 64, 7, .16);
    buttons["start"] = new SpriteButton(cache, "resources/start_button.bmp", 640, 600, 150, 100, {0, 0, 64, 64}, 7, 64, .06);

    song_ending_time = 82.9;
    animate_interval = 1.2;
}

MenuScene::~MenuScene(){
    for (int i=0; i < stars.size(); i++){
        delete stars[i];
    }

    for (auto const &button : buttons){
            delete button.second;
    }
    buttons.clear();

    delete title;
}

void MenuScene::Process(Clock * clock, MouseManager * mouse, Jukebox * jukebox, string * state, LevelScene * scene){
    if (starting){
        jukebox->PlayMusic("title");
        running = true;
        finished = false;
    }
    if (running){
        seconds_passed += clock->delta_time_s;
        if (buttons["start"]->MouseClicking(mouse)){
            *state = "GAME";
            jukebox->StopMusic();
            title->Reset();
            seconds_passed = 0;
            return;
        }

        if (seconds_passed >= animate_interval){
            seconds_passed = animate_interval;
            title->Animate(clock);
        }
        
        for (auto const &button : buttons){
            button.second->Process(clock);
        }

        // Move the stars.
        for (auto star: stars){
            star->y_pos += (3 * 100) * clock->delta_time_s;
            if (star->y_pos >= 720){
                star->y_pos = star->y_pos - 720;
            }
        }
    }
}

void MenuScene::RenderScene(){
    //Rendering
    framebuffer->SetActiveBuffer("MENU");
    SDL_SetRenderDrawColor(renderer, 9, 21, 61, 255);
    SDL_RenderClear(renderer);

    for (auto star: stars){
        star->Render();
    }
    
    for (auto const &button : buttons){
            button.second->Render();
    }

    title->Render();

    framebuffer->UnsetBuffers();
}





#include "mouse.h"

MouseManager::MouseManager(){
    x_pos = 0;
    y_pos = 0;
    clicking = false;
    clicked = false;
    has_clicked = false;
    mouse_rect = {0, 0, 20, 20};
}

void MouseManager::Process(){
    // clicking check to see if the mouse is currently in the process of clicking something.
    // SDL_GetMouseState also sets the x position and y position of the mouse rect.
    clicking = SDL_GetMouseState(&x_pos, &y_pos) & SDL_BUTTON(1);

    // has clicked checks to see if the mouse has clicked, but is not currently clicking something.
    has_clicked = clicked && !clicking;

    // clicked is set to the last state of the mouse, reset every frame
    clicked = clicking;

    // make sure the middle of the rectangle represents the mouse's point.
    mouse_rect.x = (x_pos - int(mouse_rect.w / 2));
    mouse_rect.y = (y_pos - int(mouse_rect.h / 2));
}

bool MouseManager::IsClicking(SDL_Rect * rect){
    return SDL_HasIntersection(&mouse_rect, rect) && clicking;
}

bool MouseManager::HasClicked(SDL_Rect * rect){
    return SDL_HasIntersection(&mouse_rect, rect) && has_clicked;
}

void MouseManager::Render(SDL_Renderer * renderer){
    if (clicking) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &mouse_rect);
    }

    else if (has_clicked){
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &mouse_rect);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &mouse_rect);
    }
}
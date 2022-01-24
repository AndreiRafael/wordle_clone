#if (defined(__MINGW32__) || defined(__GNUC__)) && defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif//_MSC_VER

#include <sdl/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

const int wordle_padding = 10;
const int wordle_spacing = 5;

const int wordle_letter_count = 5;
const int wordle_try_count = 6;

const int wordle_letter_original_size = 32;
const int wordle_letter_target_size = 64;

typedef enum wordle_match_t {
    wordle_match_unknown = 0,
    wordle_match_nowhere,
    wordle_match_wrong_spot,
    wordle_match_correct
} wordle_match_t;

typedef struct game_data_t {
    SDL_Renderer* renderer;
    SDL_Texture* letters_texture;
    int window_w;
    int window_h;

    bool quit;
    bool end_game;
    bool won_game;

    int try_index;
    int letter_index;

    int word_count;
    char* correct_word;
    char* all_words;

    char* full_letter_board;
    wordle_match_t* full_result_board;

    float flash_timer;
    Uint32 prev_ticks;
} game_data_t;

bool wordle_try_match(char* try_word, char* correct_word, wordle_match_t* results, int len) {
    char* correct_cpy = malloc((size_t)len * sizeof(char));
    for(int i = 0; i < len; i++) {//reset all results and copy correct word
        results[i] = wordle_match_unknown;
        correct_cpy[i] = correct_word[i];
    }

    //check for letters in te correct spot first
    for(int i = 0; i < len; i++) {
        if(try_word[i] == correct_cpy[i]) {
            results[i] = wordle_match_correct;
            correct_cpy[i] = '\0';
        }
    }

    //check for letters in the wrong spot
    for(int i = 0; i < len; i++) {
        if(results[i] == wordle_match_correct) {
            continue;//skip if correct
        }
        results[i] = wordle_match_nowhere;//default to nowhere

        const char letter = try_word[i];
        for(int j = 0; j < len; j++) {
            if(correct_cpy[j] != '\0' && correct_cpy[j] == letter) {
                results[i] = wordle_match_wrong_spot;
                correct_cpy[j] = '\0';
                break;
            }
        }
    }

    free(correct_cpy);

    for(int i = 0; i < len; i++) {
        if(results[i] != wordle_match_correct) {
            return false;
        }
    }
    return true;
}

void wordle_render_bg(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    for(int t = 0; t < wordle_try_count; t++) {
        for(int l = 0; l < wordle_letter_count; l++) {
            SDL_Rect r = {
                .x = wordle_padding + l * (wordle_letter_target_size + wordle_spacing),
                .y = wordle_padding + t * (wordle_letter_target_size + wordle_spacing),
                .w = wordle_letter_target_size,
                .h = wordle_letter_target_size
            };

            SDL_RenderFillRect(renderer, &r);
        }
    }
}

void wordle_draw_word(SDL_Renderer* renderer, SDL_Texture* texture, const char* word, int len, int x, int y, int size) {
    int start_x = x - (size * len) / 2;
    int start_y = y - size / 2;

    SDL_SetRenderDrawColor(renderer, 255, 150, 150, 255);
    SDL_Rect bg_rect = {
        .x = start_x,
        .y = start_y,
        .w = size * len,
        .h = size
    };
    SDL_RenderFillRect(renderer, &bg_rect);
    for(int i = 0; i < len; i++) {
        char c = word[i];
        if(c < 97 || c > 122) {
            continue;
        }

        int tex_index = (int)c - 97;
        SDL_Rect src_rect = {
            .x = tex_index * wordle_letter_original_size,
            .y = 0,
            .w = wordle_letter_original_size,
            .h = wordle_letter_original_size
        };

        SDL_Rect dest_rect = {
            .x = start_x + size * i,
            .y = start_y,
            .w = size,
            .h = size
        };

        SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
    }
}

void wordle_render_board(SDL_Renderer* renderer, SDL_Texture* letters_texture, char* board_letters, wordle_match_t* board_results, int try_index, int letter_index) {
    for(int i = 0; i < try_index; i++) {//loop through previous tries
        for(int j = 0; j < wordle_letter_count; j++) {
            int board_index = i * wordle_letter_count + j;
            wordle_match_t entry_result = board_results[board_index];
            char entry_letter = board_letters[board_index];
            if(entry_result == wordle_match_unknown) {
                continue;
            }

            char letter = entry_letter;
            if(letter < 97 || letter > 122) {
                continue;//out of render range
            }

            switch (entry_result)
            {
            case wordle_match_correct:
                SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
                break; 
            case wordle_match_wrong_spot:
                SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
                break;
            case wordle_match_nowhere:
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                break;            
            default:
                break;
            }
            SDL_Rect dest_rect = {
                .x = wordle_padding + j * (wordle_letter_target_size + wordle_spacing),
                .y = wordle_padding + i * (wordle_letter_target_size + wordle_spacing),
                .w = wordle_letter_target_size,
                .h = wordle_letter_target_size
            };
            SDL_RenderFillRect(renderer, &dest_rect);

            int tex_index = (int)letter - 97;
            SDL_Rect src_rect = {
                .x = wordle_letter_original_size * tex_index,
                .y = 0,
                .w = wordle_letter_original_size,
                .h = wordle_letter_original_size
            };
            SDL_RenderCopy(renderer, letters_texture, &src_rect, &dest_rect);
        }
    }

    //draw current typed word
    for(int i = 0; i < letter_index; i++) {
        int board_index = try_index * wordle_letter_count + i;
        char entry_letter = board_letters[board_index];

        char letter = entry_letter;
        if(letter < 97 || letter > 122) {
            continue;//out of render range
        }

        SDL_Rect dest_rect = {
            .x = wordle_padding + i * (wordle_letter_target_size + wordle_spacing),
            .y = wordle_padding + try_index * (wordle_letter_target_size + wordle_spacing),
            .w = wordle_letter_target_size,
            .h = wordle_letter_target_size
        };

        int tex_index = (int)letter - 97;
        SDL_Rect src_rect = {
            .x = wordle_letter_original_size * tex_index,
            .y = 0,
            .w = wordle_letter_original_size,
            .h = wordle_letter_original_size
        };
        SDL_RenderCopy(renderer, letters_texture, &src_rect, &dest_rect);
    }
}

char* wordle_read_words(const char* file_path, int* word_count) {
    FILE* f = fopen(file_path, "r");
    if(!f) {
        return NULL;
    }

    int line_count = 1;
    {
        int counted_valid = 0;
        char c;
        while((c = (char)fgetc(f)) != EOF) {
            if(c == '\n') {
                line_count++;
                counted_valid = 0;
            }
            else {
                counted_valid++;
            }
        }

        //last line bad formatting check
        if(counted_valid < wordle_letter_count) {
            line_count--;
        }
    }
    rewind(f);

    *word_count = line_count;
    char* out_arr = malloc(sizeof(char) * (size_t)(line_count * wordle_letter_count));
    {
        int word_index = 0;
        int letter_index = 0;
        char c;
        while((c = (char)fgetc(f)) != EOF && word_index < line_count) {
            char* word_ptr = out_arr + wordle_letter_count * word_index;
            if(c == '\n') {
                word_index++;
                letter_index = 0;
            }
            else if(letter_index < wordle_letter_count) {
                *(word_ptr + letter_index++) = c;
            }
        }
    }
    fclose(f);

    return out_arr;
}

bool wordle_validate_word(const char* word_arr, int word_count, const char* word) {
    for(int i = 0; i < word_count; i++) {
        const char* word_ptr = word_arr + i * wordle_letter_count;
        if(memcmp(word, word_ptr, (size_t)wordle_letter_count) == 0) {
            return true;
        }
    }
    return false;
}

void wordle_select_word(char* word_arr, int word_count, char* buffer) {
    srand((unsigned int)time(NULL));

    int selected_word = rand() % word_count;
    char* selected_ptr = word_arr + (selected_word * wordle_letter_count);
    for(int i = 0; i < wordle_letter_count; i++) {
        buffer[i] = selected_ptr[i];
    }
}

void game_tick(game_data_t* game_data) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            game_data->quit = true;
        }
        else if(e.type == SDL_KEYDOWN) {
            SDL_Keysym keysym = e.key.keysym;
            if(keysym.sym == SDLK_BACKSPACE) {
                if(!game_data->end_game && game_data->letter_index > 0) {
                    game_data->letter_index--;
                }
            }
            else if(keysym.sym == SDLK_KP_ENTER || keysym.sym == SDLK_RETURN || keysym.sym == SDLK_RETURN2) {
                if(game_data->end_game) {//restart the game with a new word
                    wordle_select_word(game_data->all_words, game_data->word_count, game_data->correct_word);
                    game_data->end_game = game_data->won_game = false;
                    game_data->letter_index = game_data->try_index = 0;
                }
                else if(game_data->letter_index == wordle_letter_count) {
                    int board_offset = game_data->try_index * wordle_letter_count;
                    bool correct = wordle_try_match(
                        game_data->full_letter_board + board_offset, game_data->correct_word,//words
                        game_data->full_result_board + board_offset,//results
                        wordle_letter_count
                    );
                    if(wordle_validate_word(game_data->all_words, game_data->word_count, game_data->full_letter_board + board_offset)) {
                        game_data->try_index++;
                        if(correct) {
                            game_data->end_game = game_data->won_game = true;
                        }
                        else {
                            if(game_data->try_index >= wordle_try_count) {
                                game_data->end_game = true;//lost
                            }
                        }
                        game_data->letter_index = 0;
                    }
                    else {
                        game_data->flash_timer = 1.f;
                    }
                }
            }
            else if(keysym.scancode >= 4 && keysym.scancode <= 29) {
                if(!game_data->end_game && game_data->letter_index < wordle_letter_count) {//submit letter
                    int board_index = game_data->try_index * wordle_letter_count + game_data->letter_index;
                    game_data->full_letter_board[board_index] = (char)((int)keysym.scancode + 93);
                    game_data->full_result_board[board_index] = wordle_match_unknown;
                    game_data->letter_index++;
                }
            }
        }
    }

    //TIMER UPDATE
    Uint32 new_ticks = SDL_GetTicks();
    Uint32 delta_ticks = new_ticks - game_data->prev_ticks;
    game_data->prev_ticks = new_ticks;
    float delta_time = (float)delta_ticks / 1000.f;

    game_data->flash_timer = SDL_max(game_data->flash_timer - delta_time * 3.f, 0.f);

    //RENDER
    if(game_data->end_game) {
        if(game_data->won_game) {
            SDL_SetRenderDrawColor(game_data->renderer, 150, 255, 150, 255);
        }
        else {
            SDL_SetRenderDrawColor(game_data->renderer, 255, 150, 150, 255);
        }
    }
    else {
        Uint8 val = 255u - (Uint8)(255.f * game_data->flash_timer);
        SDL_SetRenderDrawColor(game_data->renderer, 255, val, val, 255);
    }
    SDL_RenderClear(game_data->renderer);

    wordle_render_bg(game_data->renderer);
    wordle_render_board(
        game_data->renderer, game_data->letters_texture,
        game_data->full_letter_board, game_data->full_result_board,
        game_data->try_index, game_data->letter_index
    );
    if(game_data->end_game && !game_data->won_game) {//draw correct word at the center of the screen
        wordle_draw_word(
            game_data->renderer, game_data->letters_texture,
            game_data->correct_word, wordle_letter_count,
            game_data->window_w / 2, game_data->window_h / 2,
            wordle_letter_original_size
        );
    }

    SDL_RenderPresent(game_data->renderer);
}

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
void emscripten_tick(void* arg) {
    game_tick((game_data_t*)arg);
}
#endif//__EMSCRIPTEN__

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    #ifdef SDL_MAIN_HANDLED
    SDL_SetMainReady();
    #endif

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Could not initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    game_data_t game_data;

    game_data.window_w = wordle_letter_count * wordle_letter_target_size + wordle_padding * 2 + (wordle_letter_count - 1) * wordle_spacing;
    game_data.window_h = wordle_padding * 2 + wordle_try_count * wordle_letter_target_size + (wordle_try_count - 1) * wordle_spacing;
    SDL_Window* window = SDL_CreateWindow(
        "Wordle",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        game_data.window_w,//w
        game_data.window_h,//h
        SDL_WINDOW_SHOWN
    );

    game_data.renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    {
        SDL_Surface* letters_surf = SDL_LoadBMP("./res/letters.bmp");
        if(letters_surf == NULL) {
            printf("Could not load bmp: %s\n", SDL_GetError());
            return -1;
        }

        game_data.letters_texture = SDL_CreateTextureFromSurface(game_data.renderer, letters_surf);
        SDL_FreeSurface(letters_surf);
        if(game_data.letters_texture == NULL) {
            printf("Could not create texture: %s\n", SDL_GetError());
            return -1;
        }
    }

    game_data.full_letter_board = malloc((size_t)(wordle_try_count * wordle_letter_count) * sizeof(char));
    game_data.full_result_board = malloc((size_t)(wordle_try_count * wordle_letter_count) * sizeof(wordle_match_t));
    for(int i = 0; i < wordle_try_count * wordle_letter_count; i++) {
        game_data.full_result_board[i] = wordle_match_unknown;
    }

    game_data.correct_word = malloc(sizeof(char) * (size_t)wordle_letter_count);
    game_data.all_words = wordle_read_words("./res/sgb-words.txt", &game_data.word_count);
    wordle_select_word(game_data.all_words, game_data.word_count, game_data.correct_word);

    game_data.letter_index = 0;
    game_data.try_index = 0;

    game_data.end_game = false;
    game_data.won_game = false;

    game_data.flash_timer = 0.f;//flash screen when trying to submit an invalid word

    game_data.quit = false;

    game_data.prev_ticks = SDL_GetTicks();

#if defined(__EMSCRIPTEN__)
    printf("EMSCRIPTEN STARTED!\n");
    emscripten_set_main_loop_arg(emscripten_tick, (void*)&game_data, 0, 1);
#else
    while(!game_data.quit) {
        game_tick(&game_data);
    }
#endif

    free(game_data.correct_word);
    free(game_data.all_words);
    free(game_data.full_letter_board);
    free(game_data.full_result_board);
    SDL_DestroyTexture(game_data.letters_texture);
    SDL_DestroyRenderer(game_data.renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

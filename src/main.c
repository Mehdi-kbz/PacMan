#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#define N_X_TILES 28
#define N_Y_TILES 29
#define PACMAN_SIZE 32
#define GHOST_SIZE 32

//enums for map elements
enum tile_type_e {
    WALL = 0,
    PATH,
    PACMAN_START,
    GHOSTS_START
};

//enums for directions
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Structure to hold Ghost states
enum GhostState { NORMAL, DIZZY };

// Structure to hold coordinates
struct Coordinates {
    int x;
    int y;
};

// Functions prototypes
bool can_move(enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect position, enum Direction direction);
enum Direction choose_random_direction(enum Direction previous_direction);
void teleport_pacman(SDL_Rect *pacman_position, enum Direction ouverture);
enum Direction choose_direction_red_closest_to_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position, enum tile_type_e map[N_Y_TILES][N_X_TILES]);
enum Direction choose_direction_blue_closest_to_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position, enum tile_type_e map[N_Y_TILES][N_X_TILES]);
void update_red_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position);
void update_blue_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position);
void update_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES]);
void toggleFullscreen(SDL_Window* window);
enum Direction choose_random_direction(enum Direction previous_direction);   
int check_pacman_ghost_collision(SDL_Rect pacman_position, SDL_Rect ghost_position);
void renderPacgumsEaten(int num_pacgums_eaten);
void load_maze(const char *filename, enum tile_type_e map[N_Y_TILES][N_X_TILES]);
void move_red_ghost_towards_pacman(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position);
void move_blue_ghost_towards_pacman(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position);
int SDL_RenderSetScale(SDL_Renderer * renderer, float scaleX, float scaleY);




int main(int argc, char *argv[])
{   
    if (argc != 2) {
        printf("Usage: %s <maze_file>\n", argv[0]);
        return 1;
    }

    else{
        enum tile_type_e map[N_Y_TILES][N_X_TILES];
        load_maze(argv[1], map);
    
    srand(time(NULL));
    
    int ret;
    int is_running = 1;

    const int tile_size = PACMAN_SIZE+8;
    const int window_width = N_X_TILES*tile_size;
    const int window_height = N_Y_TILES*tile_size;

    // SDL initialisation with video support
    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0)
    {
        fprintf(stderr, "Could not init SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Create the window
    SDL_Window *screen = SDL_CreateWindow(
        "Pacman",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height, 0);
    if (!screen)
    {
        fprintf(stderr, "Could not create SDL screen: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Create the renderer, can be seen as a paint brush
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
    {
        fprintf(stderr, "Could not create SDL renderer: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Rect pacman_position = {
        .w = PACMAN_SIZE,
        .h = PACMAN_SIZE,
    };
 

    SDL_Rect red_ghost_position = {
        .w = GHOST_SIZE,
        .h = GHOST_SIZE,
        .x = (N_X_TILES / 2 - 1) * tile_size + (tile_size - GHOST_SIZE) / 2,
        .y = (N_Y_TILES / 2 - 1) * tile_size + (tile_size - GHOST_SIZE) / 2
    };

    SDL_Rect blue_ghost_position = {
    .w = GHOST_SIZE,
    .h = GHOST_SIZE,
    .x = (N_X_TILES / 2) * tile_size + (tile_size - GHOST_SIZE) / 2,
    .y = (N_Y_TILES / 2) * tile_size +  (tile_size - GHOST_SIZE) / 2
    };

    SDL_Rect orange_ghost_position = {
        .w = GHOST_SIZE,
        .h = GHOST_SIZE,
        .x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2,
        .y = (N_Y_TILES / 2 + 1) * tile_size + (tile_size - GHOST_SIZE) / 2
    };

    SDL_Rect pink_ghost_position = {
        .w = GHOST_SIZE,
        .h = GHOST_SIZE,
        .x = (N_X_TILES / 2 - 1) * tile_size + (tile_size - GHOST_SIZE) / 2,
        .y = (N_Y_TILES / 2 + 1) * tile_size + (tile_size - GHOST_SIZE) / 2
    };

        
    enum Direction red_ghost_previous_direction = RIGHT;
    enum Direction blue_ghost_previous_direction = RIGHT;
    enum Direction orange_ghost_previous_direction = RIGHT;
    enum Direction pink_ghost_previous_direction = RIGHT;

    SDL_Rect map_position = {
        .x = 0,
        .y = 0,
        .w = window_width,
        .h = window_height,
    };

    // Create a texture for the map, can be seen as a layer
    SDL_Texture* map_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        window_width,
        window_height
    );

    // Now, the paint brush `renderer` will paint on the layer `map_texture`
    SDL_SetRenderTarget(renderer, map_texture);

    // Select color of the paint brush (R, G, B, alpha)
    SDL_SetRenderDrawColor(renderer, 101, 26, 166, 255);
    // Paint the the whole target of the paint brush (`map_texture` now)
    SDL_RenderClear(renderer);

    // Now, let's paint the maze walls in blue:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int y = 0; y < N_Y_TILES; y++)
    {
        for (int x = 0; x < N_X_TILES; x++)
        {
            if (map[y][x] == WALL)
            {
                SDL_Rect rectangle = {
                    .x = x*tile_size,
                    .y = y*tile_size,
                    .w = tile_size,
                    .h = tile_size,
                };
                SDL_RenderFillRect(renderer, &rectangle);
            }
            else if (map[y][x] == PACMAN_START)
            {
                // While we are at it, save what is the initial position of Pac-Man
                pacman_position.x = x*tile_size + (tile_size-PACMAN_SIZE)/2;
                pacman_position.y = y*tile_size + (tile_size-PACMAN_SIZE)/2;

                /* Don't forget to change the map tile type to consider it as a
                 * path (can be done probably in a better way, for instance
                 * with a bit fields) */
                map[y][x] = PATH;
            }
 
        }
    }

    // Draw back to window's renderer (ie the paint brush draws on the window now):
    SDL_SetRenderTarget(renderer, NULL);

    // Load the image as a texture
    SDL_Texture* pacman_texture = IMG_LoadTexture(renderer, "images/pacman-right.png");
    if (!pacman_texture)
    {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Load the image as a texture
    SDL_Texture* red_ghost_texture = IMG_LoadTexture(renderer, "images/ghost-red-right.png");
    if (!red_ghost_texture)
    {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    // Load the image as a texture for the blue ghost
    SDL_Texture* blue_ghost_texture = IMG_LoadTexture(renderer, "images/ghost-blue-right.png");
    if (!blue_ghost_texture)
    {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Load the image as a texture for the orange ghost
    SDL_Texture* orange_ghost_texture = IMG_LoadTexture(renderer, "images/ghost-orange-right.png");
    if (!orange_ghost_texture)
    {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Load the image as a texture for the pink ghost
    SDL_Texture* pink_ghost_texture = IMG_LoadTexture(renderer, "images/ghost-pink-right.png");
    if (!pink_ghost_texture)
    {
        fprintf(stderr, "Could not load image: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }



        // Loading images for Pac-Man facing different directions
        SDL_Texture* pacman_textures[4];
        pacman_textures[UP] = IMG_LoadTexture(renderer, "images/pacman-up.png");
        pacman_textures[DOWN] = IMG_LoadTexture(renderer, "images/pacman-down.png");
        pacman_textures[LEFT] = IMG_LoadTexture(renderer, "images/pacman-left.png");
        pacman_textures[RIGHT] = IMG_LoadTexture(renderer, "images/pacman-right.png");

       
        // Loading images for red ghost facing different directions
        SDL_Texture* red_ghost_textures[5];
        red_ghost_textures[UP] = IMG_LoadTexture(renderer, "images/ghost-red-up.png");
        red_ghost_textures[DOWN] = IMG_LoadTexture(renderer, "images/ghost-red-down.png");
        red_ghost_textures[LEFT] = IMG_LoadTexture(renderer, "images/ghost-red-left.png");
        red_ghost_textures[RIGHT] = IMG_LoadTexture(renderer, "images/ghost-red-right.png");
        red_ghost_textures[DIZZY] = IMG_LoadTexture(renderer, "images/ghost-blue-dizzy.png");

        // Loading images for blue ghost facing different directions
        SDL_Texture* blue_ghost_textures[5];
        blue_ghost_textures[UP] = IMG_LoadTexture(renderer, "images/ghost-blue-up.png");
        blue_ghost_textures[DOWN] = IMG_LoadTexture(renderer, "images/ghost-blue-down.png");
        blue_ghost_textures[LEFT] = IMG_LoadTexture(renderer, "images/ghost-blue-left.png");
        blue_ghost_textures[RIGHT] = IMG_LoadTexture(renderer, "images/ghost-blue-right.png");
        blue_ghost_textures[DIZZY] = IMG_LoadTexture(renderer, "images/ghost-blue-dizzy.png");

        // Loading images for orange ghost facing different directions
        SDL_Texture* orange_ghost_textures[5];
        orange_ghost_textures[UP] = IMG_LoadTexture(renderer, "images/ghost-orange-up.png");
        orange_ghost_textures[DOWN] = IMG_LoadTexture(renderer, "images/ghost-orange-down.png");
        orange_ghost_textures[LEFT] = IMG_LoadTexture(renderer, "images/ghost-orange-left.png");
        orange_ghost_textures[RIGHT] = IMG_LoadTexture(renderer, "images/ghost-orange-right.png");
        orange_ghost_textures[DIZZY] = IMG_LoadTexture(renderer, "images/ghost-blue-dizzy.png");


        // Loading images for pink ghost facing different directions
        SDL_Texture* pink_ghost_textures[5];
        pink_ghost_textures[UP] = IMG_LoadTexture(renderer, "images/ghost-pink-up.png");
        pink_ghost_textures[DOWN] = IMG_LoadTexture(renderer, "images/ghost-pink-down.png");
        pink_ghost_textures[LEFT] = IMG_LoadTexture(renderer, "images/ghost-pink-left.png");
        pink_ghost_textures[RIGHT] = IMG_LoadTexture(renderer, "images/ghost-pink-right.png");
        pink_ghost_textures[DIZZY] = IMG_LoadTexture(renderer, "images/ghost-blue-dizzy.png");


    int desired_direction = 0; // Initializing desired direction
    int move_direction = RIGHT; // Initializing move_direction 
    int is_paused = 0;
    int num_pacgums_eaten = 0; // Variable to count the number of pac-gums eaten






/////////////////////////////////////////////////////////////////////////////////////////////////













    while (is_running)
    {   
        if (is_paused) {
              // Fetch event
            SDL_Event event;
            SDL_PollEvent(&event);
            switch (event.type)
        {
            case SDL_QUIT:
                is_running = 0;
                break;

            case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
                {
                    // It was a `q`, quit the program by exiting this loop
                    case SDLK_q:
                        is_running = 0;
                        break;
                    // It was a `r`, resume the game 
                    case SDLK_r:
                        is_paused = 0; // Resume the game
                        break;
                }
        } }
        else {
        
        // Initializing the texture for the ghosts
        SDL_Texture* red_ghost_texture = red_ghost_textures[RIGHT];
        SDL_Texture* blue_ghost_texture = blue_ghost_textures[RIGHT];
        SDL_Texture* orange_ghost_texture = orange_ghost_textures[RIGHT];
        SDL_Texture* pink_ghost_texture = pink_ghost_textures[RIGHT];

        // Initializing the texture for Pac-Man
        SDL_Texture* pacman_texture = pacman_textures[RIGHT];

        // Defining the source and destination rectangles for drawing the red ghost
        SDL_Rect red_ghost_src_rect = {0, 0, GHOST_SIZE, GHOST_SIZE};
        SDL_Rect red_ghost_dst_rect = red_ghost_position;

        // Defining the source and destination rectangles for drawing the blue ghost
        SDL_Rect blue_ghost_src_rect = {0, 0, GHOST_SIZE, GHOST_SIZE};
        SDL_Rect blue_ghost_dst_rect = blue_ghost_position;
        
        // Defining the source and destination rectangles for drawing the orange ghost
        SDL_Rect orange_ghost_src_rect = {0, 0, GHOST_SIZE, GHOST_SIZE};
        SDL_Rect orange_ghost_dst_rect = orange_ghost_position;
        
        // Defining the source and destination rectangles for drawing the pink ghost
        SDL_Rect pink_ghost_src_rect = {0, 0, GHOST_SIZE, GHOST_SIZE};
        SDL_Rect pink_ghost_dst_rect = pink_ghost_position;

        move_red_ghost_towards_pacman(&red_ghost_position, &red_ghost_previous_direction, map, pacman_position);
        move_blue_ghost_towards_pacman(&blue_ghost_position,&blue_ghost_previous_direction, map, pacman_position);
        update_ghost_position(&orange_ghost_position, &orange_ghost_previous_direction, map);
        update_ghost_position(&pink_ghost_position, &pink_ghost_previous_direction, map);

        if(num_pacgums_eaten <= 150 && num_pacgums_eaten >=100){

                //making ghosts blue ( dizzy mode )
                red_ghost_texture = red_ghost_textures[DIZZY];
                blue_ghost_texture = blue_ghost_textures[DIZZY];
                orange_ghost_texture = orange_ghost_textures[DIZZY];
                pink_ghost_texture = pink_ghost_textures[DIZZY];

                if (check_pacman_ghost_collision(pacman_position, red_ghost_position)) {
                    red_ghost_texture = red_ghost_textures[RIGHT];
                    red_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    red_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                };

                if (check_pacman_ghost_collision(pacman_position, blue_ghost_position)) {
                    blue_ghost_texture = blue_ghost_textures[RIGHT];
                    blue_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    blue_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                };

                if (check_pacman_ghost_collision(pacman_position, orange_ghost_position)) {
                    orange_ghost_texture = orange_ghost_textures[RIGHT];
                    orange_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    orange_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                };

                if (check_pacman_ghost_collision(pacman_position, pink_ghost_position)) {
                    pink_ghost_texture = pink_ghost_textures[RIGHT];
                    pink_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    pink_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                };
                
                }

            // Checking for collisions between Pac-Man and the ghosts
                if (check_pacman_ghost_collision(pacman_position, red_ghost_position)) {
                    red_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    red_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    is_paused = 1; // pause the game
                    printf("Pac-Man collided with the red ghost!\n");
                    };

                if (check_pacman_ghost_collision(pacman_position, blue_ghost_position)) {
                    blue_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    blue_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    is_paused = 1; // pause the game
                    printf("Pac-Man collided with the blue ghost!\n");
                    };

                if (check_pacman_ghost_collision(pacman_position, orange_ghost_position)) {
                    orange_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    orange_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    is_paused = 1; // pause the game 
                    printf("Pac-Man collided with the orange ghost!\n");
                    };

                if (check_pacman_ghost_collision(pacman_position, pink_ghost_position)) {
                    pink_ghost_position.x = (N_X_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    pink_ghost_position.y = (N_Y_TILES / 2 + 1) * tile_size +  (tile_size - GHOST_SIZE) / 2;
                    is_paused = 1; // pause the game
                    printf("Pac-Man collided with the pink ghost!\n");
                };
            
        // Drawing the red ghost texture on top of what was previously drawn
        SDL_RenderCopy(renderer, red_ghost_texture, &red_ghost_src_rect, &red_ghost_dst_rect);
        
        // Drawing the blue ghost texture on top of what was previously drawn
        SDL_RenderCopy(renderer, blue_ghost_texture, &blue_ghost_src_rect, &blue_ghost_dst_rect);

        // Drawing the orange ghost texture on top of what was previously drawn
        SDL_RenderCopy(renderer, orange_ghost_texture, &orange_ghost_src_rect, &orange_ghost_dst_rect);

        // Drawing the pink ghost texture on top of what was previously drawn
        SDL_RenderCopy(renderer, pink_ghost_texture, &pink_ghost_src_rect, &pink_ghost_dst_rect);


        // Fetching event
        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                is_running = 0;
                break;

            case SDL_KEYDOWN:
                // A keyboard key was pressed down
                switch (event.key.keysym.sym)
                {
                    // If it's`q`, quit the program by exiting this loop
                    case SDLK_q:
                        is_running = 0;
                        break;

                    // If it's r, resume the game 
                    case SDLK_r:
                        is_paused = 0; // Resuming the game
                        break;
                    
                    // If it's p, pause the game 
                    case SDLK_p:
                        is_paused = 1; // pausing the game
                        break;

                    case SDLK_f :
                     // Toggling fullscreen mode when 'f' key is pressed
                            toggleFullscreen(screen);
                        break;

                    case SDLK_UP:
                            desired_direction = UP;
                        break;

                    case SDLK_DOWN:
                            desired_direction = DOWN;
                        break;

                    case SDLK_LEFT:
                            desired_direction = LEFT;
                        break;

                    case SDLK_RIGHT:
                            desired_direction = RIGHT;
                        break;
                }
                break;
        }

        // Checking if it's possible to move in the desired direction
        if (can_move(map, pacman_position, desired_direction)) {
        
            // Updating Pac-Man's direction to the desired direction
            move_direction = desired_direction;
        }

        // Moving Pac-Man according to the current direction
        switch (move_direction)
        {
            case UP:
                if (can_move(map, pacman_position, UP)) {
                    pacman_position.y -= 40;
                    pacman_texture = pacman_textures[UP];
                }
                break;

            case DOWN:
                if (can_move(map, pacman_position, DOWN)) {
                    pacman_position.y += 40;
                    pacman_texture = pacman_textures[DOWN];
                }
                break;

            case LEFT:
                if (can_move(map, pacman_position, LEFT)) {
                    pacman_position.x -= 40;
                    pacman_texture = pacman_textures[LEFT];
                }
                break;

            case RIGHT:
                if (can_move(map, pacman_position, RIGHT)) {
                    pacman_position.x += 40;
                    pacman_texture = pacman_textures[RIGHT];
                }
                break;
}

/////////////// Function to scale the window ///////////////
SDL_RenderSetScale(renderer, 1, 0.7);

// Calculate the tile position of Pac-Man
int pacman_tile_x = pacman_position.x / tile_size;
int pacman_tile_y = pacman_position.y / tile_size;


// Draw the pac-gums after updating the map
for (int y = 0; y < N_Y_TILES; y++) {
    for (int x = 0; x < N_X_TILES; x++) {
        if (map[y][x] == PATH) {
            // Draw a yellow rectangle to represent a pac-gum
            SDL_Rect pacgum_rect = {
                .x = x * tile_size + (tile_size - 8) / 2,
                .y = y * tile_size + (tile_size - 8) / 2,
                .w = 8,
                .h = 8
            };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); 
            SDL_RenderFillRect(renderer, &pacgum_rect);
        }
    }
}
// Check if Pac-Man is on the position of a pac-gum
if (map[pacman_tile_y][pacman_tile_x] == PATH) {
    // Pac-Man has eaten this pac-gum, so update the map
    map[pacman_tile_y][pacman_tile_x] = -1; // Update to represent an empty tile
    num_pacgums_eaten++; // Increment the number of pac-gums eaten
}
     
            
     renderPacgumsEaten(num_pacgums_eaten);

// Declaring coordinates for teleportation points
    struct Coordinates right_coordinates;
    struct Coordinates left_coordinates;
    struct Coordinates up_coordinates;
    struct Coordinates down_coordinates;


        right_coordinates.x=27;
        right_coordinates.y=14;


        left_coordinates.x=0;
        left_coordinates.y=14;


        up_coordinates.x=14;
        up_coordinates.y=0;


        down_coordinates.x=14;
        down_coordinates.y=28;


//printf("left_x=%d\n",left_coordinates.x);
//printf("pacman_tile_x=%d\n",pacman_tile_x);

//printf("left_y=%d\n",left_coordinates.y);
//printf("pacman_tile_y=%d\n",pacman_tile_y);


if((pacman_tile_x == right_coordinates.x) && (pacman_tile_y == right_coordinates.y)){
    // Teleport Pac-Man to the exit point on the right side
    teleport_pacman(&pacman_position, RIGHT);
};

if ((pacman_tile_x == left_coordinates.x) && (pacman_tile_y == left_coordinates.y)) {
    // Teleport Pac-Man to the exit point on the left side
    teleport_pacman(&pacman_position, LEFT);
};

if ((pacman_tile_x == up_coordinates.x) && (pacman_tile_y == up_coordinates.y)) {
    // Teleport Pac-Man to the exit point on the top side
    teleport_pacman(&pacman_position, UP);
};

if ((pacman_tile_x == down_coordinates.x) && (pacman_tile_y == down_coordinates.y)) {
    // Teleport Pac-Man to the exit point on the bottom side
    teleport_pacman(&pacman_position, DOWN);
};

     
         // Show on the screen what we drew so far
        SDL_RenderPresent(renderer);
        // Clear the window: remove everything that was drawn
        SDL_RenderClear(renderer);
        // Draw the map texture in the window
        SDL_RenderCopy(renderer, map_texture, NULL, &map_position);
        // Draw the pacman texture on top of what was previously drawn
        SDL_RenderCopy(renderer, pacman_texture, NULL, &pacman_position);
       
        // Wait for a short period of time to regulate the game's speed
        SDL_Delay(110);
        }
    }

    





  

/////////////////////////////////////////////////////////////////////////////////////////////////





  // Free all created resources
    SDL_DestroyTexture(pacman_texture);
    SDL_DestroyTexture(map_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(screen);

    // Quit the SDL program
    SDL_Quit();

    return EXIT_SUCCESS;
    }
}





////////////////////////////// Functions //////////////////////////////



/////////////// Function to detect if Pac-Man is able to move in a given direction ///////////////
bool can_move(enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect position, enum Direction direction) {
    
    const int tile_size = 40;

    // getting the coordinates of Pac-Man
    int pacman_x = position.x/ tile_size;
    int pacman_y = position.y / tile_size;

    switch (direction) {
        case UP:

            // Verifiying if the case on top of Pac-Man is a path 
            if (map[pacman_y - 1][pacman_x] == WALL) {
                return false;
            }
            break;
        case DOWN:
            // Vérifier si la case en-dessous de Pac-Man est un chemin
            if (map[pacman_y + 1][pacman_x] == WALL) {
                return false;
            }
            break;
        case LEFT:
            // Vérifier si la case à gauche de Pac-Man est un chemin
            if (map[pacman_y][pacman_x - 1] == WALL) {
                return false;
            }
            break;
        case RIGHT:
            // Vérifier si la case à droite de Pac-Man est un chemin
            if (map[pacman_y][pacman_x + 1] == WALL) {
                return false;
            }
            break;
    }
    return true;
}



/*old code 
  
// Update the ghost position based on the direction closest to Pac-Man
void update_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position) {
    // Choose the direction that leads the ghost closest to Pac-Man
    enum Direction new_direction = choose_direction_closest_to_pacman(*ghost_position, pacman_position, map);
    // Check if the chosen direction is valid
    if (can_move(map, *ghost_position, new_direction)) {
        // Update the ghost position
        switch (new_direction) {
            case UP:
                ghost_position->y -= GHOST_SIZE;
                break;
            case DOWN:
                ghost_position->y += GHOST_SIZE;
                break;
            case LEFT:
                ghost_position->x -= GHOST_SIZE;
                break;
            case RIGHT:
                ghost_position->x += GHOST_SIZE;
                break;
        }
        // Update the previous direction
        *previous_direction = new_direction;
    } else {
        // If the chosen direction is not valid, keep the previous direction
        new_direction = *previous_direction;
        // Attempt to move in the previous direction
        if (can_move(map, *ghost_position, new_direction)) {
            // Update the ghost position
            switch (new_direction) {
                case UP:
                    ghost_position->y -= GHOST_SIZE;
                    break;
                case DOWN:
                    ghost_position->y += GHOST_SIZE;
                    break;
                case LEFT:
                    ghost_position->x -= GHOST_SIZE;
                    break;
                case RIGHT:
                    ghost_position->x += GHOST_SIZE;
                    break;
            }
        }
    }
}
*/



/////////////// Function to calculate the Manhattan distance between two points ///////////////
int manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}



/////////////// Function to calculate the square of the Euclidian distance between two points ///////////////
int euclidian_distance(int next_x, int next_y, int pacman_center_x, int pacman_center_y) {
    int dx = pacman_center_x - next_x;
    int dy = pacman_center_y - next_y;
    int dx_squared = dx * dx;
    int dy_squared = dy * dy;
    int distance = dx_squared + dy_squared;
    return distance;
}



/*
/////////////// Function to update the blue ghost position ///////////////
void update_blue_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position) {
    
    const int tile_size = 40;
  
    // Choose a random direction for the ghost
    enum Direction random_direction = choose_direction_blue_closest_to_pacman(*ghost_position, pacman_position, map);
        
    // Update the ghost's position based on the chosen direction
    switch (random_direction) {
        case UP:
            if (can_move(map, *ghost_position, UP)) {
                ghost_position->y -= tile_size; // Assuming tile_size is defined somewhere
                *previous_direction = UP;
            }
            break;
        case DOWN:
            if (can_move(map, *ghost_position, DOWN)) {
                ghost_position->y += tile_size;
                *previous_direction = DOWN;
            }
            break;
        case LEFT:
            if (can_move(map, *ghost_position, LEFT)) {
                ghost_position->x -= tile_size;
                *previous_direction = LEFT;
            }
            break;
        case RIGHT:
            if (can_move(map, *ghost_position, RIGHT)) {
                ghost_position->x += tile_size;
                *previous_direction = RIGHT;
            }
            break;
    }
}
*/



/*
/////////////// Function to update the red ghost position ///////////////
void update_red_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position) {
    
    const int tile_size = 40;
    // Choose a random direction for the ghost
    enum Direction random_direction = choose_direction_red_closest_to_pacman(*ghost_position, pacman_position, map);
        
    // Update the ghost's position based on the chosen direction
    switch (random_direction) {
        case UP:
            if (can_move(map, *ghost_position, UP)) {
                ghost_position->y -= tile_size; // Assuming tile_size is defined somewhere
                *previous_direction = UP;
            }
            break;
        case DOWN:
            if (can_move(map, *ghost_position, DOWN)) {
                ghost_position->y += tile_size;
                *previous_direction = DOWN;
            }
            break;
        case LEFT:
            if (can_move(map, *ghost_position, LEFT)) {
                ghost_position->x -= tile_size;
                *previous_direction = LEFT;
            }
            break;
        case RIGHT:
            if (can_move(map, *ghost_position, RIGHT)) {
                ghost_position->x += tile_size;
                *previous_direction = RIGHT;
            }
            break;
    }
}
*/



/////////////// Function to update the pink and orange ghost positions ///////////////
void update_ghost_position(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES]) {
    
    const int tile_size = 40;
    
    // Choose a random direction for the ghost
    enum Direction random_direction = choose_random_direction(*previous_direction);
        
    // Update the ghost's position based on the chosen direction
    switch (random_direction) {
        case UP:
            if (can_move(map, *ghost_position, UP)) {
                ghost_position->y -= tile_size; 
                *previous_direction = UP;
            }
            break;
        case DOWN:
            if (can_move(map, *ghost_position, DOWN)) {
                ghost_position->y += tile_size;
                *previous_direction = DOWN;
            }
            break;
        case LEFT:
            if (can_move(map, *ghost_position, LEFT)) {
                ghost_position->x -= tile_size;
                *previous_direction = LEFT;
            }
            break;
        case RIGHT:
            if (can_move(map, *ghost_position, RIGHT)) {
                ghost_position->x += tile_size;
                *previous_direction = RIGHT;
            }
            break;
    }
}



// Global variables to keep track of which ghosts move
int num_ghosts_moving = 0;
int ghosts_moving[2]; // Array to store the indices of ghosts currently moving



///////////////  Function to update the position of a ghost ///////////////
void move_red_ghost_towards_pacman(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position) {
    
    const int tile_size = 40;
    
    // Choose a  direction for the ghost
    enum Direction random_direction = choose_direction_red_closest_to_pacman(*ghost_position, pacman_position, map);
    
    // Update the ghost's position based on the chosen direction
    switch (random_direction) {
        case UP:
            if (can_move(map, *ghost_position, UP)) {
                ghost_position->y -= tile_size; 
                *previous_direction = UP;
            }
            break;
        case DOWN:
            if (can_move(map, *ghost_position, DOWN)) {
                ghost_position->y += tile_size;
                *previous_direction = DOWN;
            }
            break;
        case LEFT:
            if (can_move(map, *ghost_position, LEFT)) {
                ghost_position->x -= tile_size;
                *previous_direction = LEFT;
            }
            break;
        case RIGHT:
            if (can_move(map, *ghost_position, RIGHT)) {
                ghost_position->x += tile_size;
                *previous_direction = RIGHT;
            }
            break;
    }
}




/////////////// Function to update the position of a ghost ///////////////
void move_blue_ghost_towards_pacman(SDL_Rect *ghost_position, enum Direction *previous_direction, enum tile_type_e map[N_Y_TILES][N_X_TILES], SDL_Rect pacman_position) {
    
    const int tile_size = 40;
    
    // Choose a direction for the ghost
    enum Direction random_direction = choose_direction_blue_closest_to_pacman(*ghost_position, pacman_position, map);
    
    // Update the ghost's position based on the chosen direction
    switch (random_direction) {
        case UP:
            if (can_move(map, *ghost_position, UP)) {
                ghost_position->y -= tile_size; 
                *previous_direction = UP;
            }
            break;
        case DOWN:
            if (can_move(map, *ghost_position, DOWN)) {
                ghost_position->y += tile_size;
                *previous_direction = DOWN;
            }
            break;
        case LEFT:
            if (can_move(map, *ghost_position, LEFT)) {
                ghost_position->x -= tile_size;
                *previous_direction = LEFT;
            }
            break;
        case RIGHT:
            if (can_move(map, *ghost_position, RIGHT)) {
                ghost_position->x += tile_size;
                *previous_direction = RIGHT;
            }
            break;
    }
}



/////////////// Function to choose the direction that leads the ghost closest to Pac-Man ///////////////
enum Direction choose_direction_blue_closest_to_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position, enum tile_type_e map[N_Y_TILES][N_X_TILES]) {
    int min_distance = INT_MAX; // Initialize the minimum distance to a very large value
    enum Direction best_direction = UP; // Initialize the best direction

    // Calculate the position of Pac-Man's center
    int pacman_center_x = pacman_position.x + PACMAN_SIZE / 2;
    int pacman_center_y = pacman_position.y + PACMAN_SIZE / 2;

    // Check each possible direction
    for (enum Direction dir = UP; dir <= RIGHT; dir++) {
        // Initialize the next position based on the current direction
        int next_x = ghost_position.x;
        int next_y = ghost_position.y;
        switch (dir) {
            case UP:
                next_y -= GHOST_SIZE;
                break;
            case DOWN:
                next_y += GHOST_SIZE;
                break;
            case LEFT:
                next_x -= GHOST_SIZE;
                break;
            case RIGHT:
                next_x += GHOST_SIZE;
                break;
        }

        // Check if the next position is valid
        if (can_move(map, ghost_position, dir)) {
            // Calculate the distance between the next position and Pac-Man
            int distance = manhattan_distance(next_x, next_y, pacman_center_x, pacman_center_y);
            // Update the minimum distance and best direction if needed
            if (distance < min_distance) {
                min_distance = distance;
                best_direction = dir;
            }
        }
    }

    return best_direction;
}



/////////////// Function to choose the direction that leads the ghost closest to Pac-Man ///////////////
enum Direction choose_direction_red_closest_to_pacman(SDL_Rect ghost_position, SDL_Rect pacman_position, enum tile_type_e map[N_Y_TILES][N_X_TILES]) {
    int min_distance = INT_MAX; // Initialize the minimum distance to a very large value
    enum Direction best_direction = UP; 
    // Calculating the coordinates of Pac-Man's center
    int pacman_center_x = pacman_position.x + PACMAN_SIZE / 2;
    int pacman_center_y = pacman_position.y + PACMAN_SIZE / 2;
    // Check each possible direction
    for (enum Direction dir = UP; dir <= RIGHT; dir++) {
        // Initialize the next position based on the current direction
        int next_x = ghost_position.x;
        int next_y = ghost_position.y;
        switch (dir) {
            case UP:
                next_y -= GHOST_SIZE;
                break;
            case DOWN:
                next_y += GHOST_SIZE;
                break;
            case LEFT:
                next_x -= GHOST_SIZE;
                break;
            case RIGHT:
                next_x += GHOST_SIZE;
                break;
        }
        // Check if the next position is valid
        if (can_move(map, ghost_position, dir)) {
            // Calculate the distance between the next position and Pac-Man
            int distance = euclidian_distance(next_x, next_y, pacman_center_x, pacman_center_y);
            // Update the minimum distance and best direction if needed
            if (distance < min_distance) {
                min_distance = distance;
                best_direction = dir;
            }
        }
    }
    return best_direction;
}



/////////////// Function to teleport Pac-Man ///////////////
void teleport_pacman(SDL_Rect *pacman_position, enum Direction gateway) {
    
    // Teleport depending on the specified gateway 
    switch (gateway) {
        case UP:
            pacman_position->y=1043;
            break;
        case DOWN:
            pacman_position->y=45;
            /*printf("pacman_position.y: %d\n",pacman_position->y);
            printf("verified\n");
            printf("verified\n"); debugging tests */
            break;
        case LEFT:
            pacman_position->x=1043;
            break;
        case RIGHT:
            pacman_position->x=2;
            break;
        default:
            printf("invalid gateway\n");
    }
}



/////////////// Function to toggle fullscreen mode ///////////////
void toggleFullscreen(SDL_Window* window) {
    Uint32 fullscreenFlag = SDL_WINDOW_FULLSCREEN;
    SDL_bool isFullscreen = SDL_GetWindowFlags(window) & fullscreenFlag;
    SDL_SetWindowFullscreen(window, isFullscreen ? 0 : fullscreenFlag);
}



/////////////// Function to choose a random direction for a ghost ///////////////
 enum Direction choose_random_direction(enum Direction previous_direction) {    
    
    int random_direction = rand() % 4;
    // Limiting the number of attempts to choose a valid direction
    int attempts = 0;
    int max_attempts = 10; 
    // Ensuring that the random direction is not opposite to the previous direction
    while (((random_direction == UP && previous_direction == DOWN) ||
           (random_direction == DOWN && previous_direction == UP) ||
           (random_direction == LEFT && previous_direction == RIGHT) ||
           (random_direction == RIGHT && previous_direction == LEFT)) &&
           attempts < max_attempts) {
        // Choosing another random direction
        random_direction = rand() % 4;
        attempts++;
    }
    return random_direction; 
}



/////////////// Function to check collision between Pac-Man and a ghost ///////////////
int check_pacman_ghost_collision(SDL_Rect pacman_position, SDL_Rect ghost_position) {
    // Check if the tiles of pacman ans ghosts overlap
    return ((pacman_position.x < ghost_position.x + ghost_position.w) &&
        (pacman_position.x + pacman_position.w > ghost_position.x) &&
        (pacman_position.y < ghost_position.y + ghost_position.h) &&
        (pacman_position.y + pacman_position.h > ghost_position.y));  
        // return s 1 if Collision detected and 0 otherwise
}



/////////////// Function to print the current number of pac-gums eaten into the terminal ///////////////
void renderPacgumsEaten(int num_pacgums_eaten)
{
    printf("Pac-gums eaten: %d\n", num_pacgums_eaten);
}



/////////////// Function to load the maze file ///////////////
void load_maze(const char *filename, enum tile_type_e map[N_Y_TILES][N_X_TILES]) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int y = 0;
    char line[N_X_TILES + 2]; // +2 for newline and null terminator

    while (fgets(line, sizeof(line), file)) {
        for (int x = 0; x < N_X_TILES; x++) {    
            if (line[x] == '\n') {
                continue; // Skip processing newline characters
            }

            switch (line[x]) {
                case 'W':
                    map[y][x] = WALL;
                    break;
                case 'S':
                    map[y][x] = PACMAN_START;
                    break;
                case 'G':
                    map[y][x] = GHOSTS_START;
                    break;
                case ' ':
                    map[y][x] = PATH;
                    break;
                default:
                    printf("%c is an invalid character in map file\n", line[x]);
                    break;
            }
         }
        y++;
    }
    fclose(file);
}


#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#define SQUARE_SIZE 32
#define MAP_NUMBER 7

#define CHARACTER_HORIZONTALSPEED 150
#define CHARACTER_JUMPSPEED 450
#define JOYSTICK_NEUTRALRANGE 8000

#define MINIMUM_JUMP_TIME 0
#define RUNNING_MULTIPLIER 1.8
#define CHARACTER_ACCELERATION 27

#define MAX_DOOR_RANGE 40
#define MAX_FINISH_RANGE 40

#define COEF_REST 0

#define DIR_TEXTURES "./textures/"

#define DUMP_ON 1

#define N_KEYS 6 //Quantidade de teclas para controle do personagem

#define N_CHANNELS 3 //Quantidade de canais de sons, não conta o canal de música, que é independente.
#define CHANNELMENU 1
#define CHANNELEFFECT 2
#define CHANNELMUSIC 3

// STRUCTS GLOBAIS

    int * times;
    int current;
    int finishTimer(int ID);

    // Guarda informações do arquétipo de um mapa.
    struct map{
        int xSize;
        int ySize;

        char ** mapData;

        float xGravity;
        float yGravity;

        int xSpawn;
        int ySpawn;

        int xExit;
        int yExit;

        int objectNumber;

        struct objectArchetype ** objectList;

        SDL_Surface * backgroundLayer;
        SDL_Surface * mainLayer;
    };

    // Guarda as informações de um mapa carregado.
    struct loadedMap{
        int ID;

        int xSize;
        int ySize;

        float xGravity;
        float yGravity;

        int xSpawn;
        int ySpawn;

        int xExit;
        int yExit;

        struct gridTile ** mapData;

        struct collisionHolder ** lastCollisions;

        SDL_Surface * backgroundLayer;
        SDL_Surface * mainLayer;
    };

    //Guarda informacoes de textura.
    struct blockType{
        SDL_Surface ** frames;

        int frameNumber;
        int interval;
        int animated;
        int collidable;
        int layer;
    };

    // Representa um bloco CARREGADO em um mapa
    struct gridTile{
        struct blockType * texture;

        int collidable;
    };

    struct objectType{
        int xSize;
        int ySize;

        int collidable;
        int damage;
        int physicsEnabled;

        int frameNumber;

        int leftBegin, leftEnd;
        int rightBegin, rightEnd;

        int animationInterval;
        int alwaysAnimated;

        int defaultLayer;
        int isPlatform;

        SDL_Surface ** frames;
    };

    struct positionLoop{
        float xPosition;
        float yPosition;

        float requiredTime;
        float beginTime;

        struct positionLoop * next;
        struct positionLoop * prev;
    };

    struct objectArchetype{
        float xPosition;
        float yPosition;

        float mass;
        float invMass;

        int layer;

        struct positionLoop * positionList;

        struct objectType * type;
    };

    // Contém as informações sobre algum objeto que será animado nos cálculos de física.
    struct physicsObject{
        float xPosition;
        float yPosition;

        float xProjectedPosition;
        float yProjectedPosition;

        float xDisplacement;
        float yDisplacement;

        float mass;
        float invMass;

        float xSize;
        float ySize;

        float xVelocity;
        float yVelocity;

        float xAcceleration;
        float yAcceleration;

        int collidable;
        int standing;

        struct positionLoop * currentPosition;
        int physicsEnabled;

        int active;
        int isPlatform;

        int horizontalDirection;
        int lastDirection;
        int moving;

        int frameCounter;
        int currentFrame;

        int layer;

        struct objectType * type;
    };

    typedef struct Vector2{
        float x;
        float y;
    }Vector2;

    // SDL_Rect não suporta floats. Esse suporta.
    typedef struct FloatRectangle{
        float x;
        float y;
        float w;
        float h;
    }FloatRect;

// PROTÓTIPOS DE FUNÇÕES

    // main.c           //
    int random(int min, int max);
    int max(int a, int b);
    int min(int a, int b);
    int jumpPressed();
    int jumpReleased();
    int leftPressed();
    int leftReleased();
    int rightPressed();
    int rightReleased();
    int runGameFrame();
    char * terminateString(char * string);

    // maps.c           //
    struct positionLoop * copyPositionLoop(struct positionLoop * initialNode);
    int freePositionLoop(struct positionLoop * initialNode);
    int initializeMaps();
    int freeLoadedMap();
    int loadMap(int mapID);

    // render.c         //
    int initializeGraphics();
    int initializeMapGraphics(struct map * inputMap);
    SDL_Surface * load_image( char * filename );
    SDL_Surface * returnTransparentSurface(int x_size, int y_size);
    int rawBlit(int xPos, int yPos, SDL_Surface * source, SDL_Surface * destination);
    char ** readFileList(FILE * file, char divider, char end);
    int renderText(SDL_Color color, char * text, int x_position, int y_position, int x_alignment, int y_alignment);
    SDL_Surface ** loadImageList(char ** fileList, char * directory, int fileNumber);
    int game_renderFrame(struct map * inputMap, int flip);

    // physics.c        //
    int initializePhysics();
    int calculatePhysics(struct map * inputMap);
    float returnXVelocity(struct physicsObject * object);
    float returnYVelocity(struct physicsObject * object);
    int collideAABB(struct physicsObject * obj1, struct physicsObject * obj2, Vector2 * normal, Vector2 * penetration);
    int displaceBox(struct map * inputMap, struct physicsObject * object, float xPos, float yPos, char axis);
    int rectanglesCollide(SDL_Rect rect1, FloatRect rect2);

    // menu.c
    SDLKey * Finit_menu();

// VARIÁVEIS GLOBAIS

    // Geral            //
    int QUIT_PROGRAM;
    FILE * errFile;

    int paused;

    // Mapas            //
    struct map mapArray[MAP_NUMBER];
    struct loadedMap * currentMap;

    // Renderização     //
    int xResolution;
    int yResolution;

    struct blockType * textures;
    struct objectType * objectsData;

    int maxFPS;
    int limitarFPS;

    SDL_Surface * screen;

    SDL_Surface * pauseOverlay;

    SDL_Surface * lastMainLayer;
    SDL_Surface * lastBackgroundLayer;

    TTF_Font * defaultFont;
    char * topText;

    // TEMPORÁRIO!!!!!!!

    // Física e eventos //
    SDL_Event lastEvent;

    float physics_lastRan;
    float physics_gravity;

    int physics_standing;
    int character_horizontalMovement;

    int key_up;
    int key_down;
    int key_left;
    int key_right;
    int key_running;

    int * keys_down;
    int * buttons_down;

    float lastJump;

    struct physicsObject ** physicsObjects;
    struct physicsObject * character_object;

    int physicsObjectNumber;

    int characterHealth;
    int hasDied;

    float directionalMultiplier;
    float velocityMultiplier;

    float velocityBuffer;

    float targetVelocity;

    int lastUnlockedLevel;
    float * levelScores;

    //Volume padrão
    short int vol[N_CHANNELS];

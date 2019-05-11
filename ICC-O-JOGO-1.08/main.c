#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "header.c"

// Para poder usar o console. Tirar ao terminar o jogo.
//#undef main

// CODE

// Retorna um número aleatório entre min e max.
int random(int min, int max){
    return min+(rand()%(max-min+1));
}

// Retorna o menor de a e b
int min(int a, int b){
    if(a > b)
        return b;
    else
        return a;
}

// Retorna o maior de a e b
int max(int a, int b){
    if(a > b)
        return a;
    else
        return b;
}

// Serve apenas para printar o texto msg no arquivo de debug.
int dumpMessage(char * msg){
    fprintf(errFile, "%s", msg);

    return 0;
}

// Servia para contar quanto tempo de processamento cada sistema do jogo está consumindo.
int finishTimer(int ID){
    times[ID] += SDL_GetTicks() - current;
    current = SDL_GetTicks();

    return 0;
}

// Funções de lógica de controle.
int leftPressed(){
    key_left = 1;

    character_horizontalMovement = -1;
    character_object->horizontalDirection = -1;
    character_object->moving = 1;

    return 0;
}

int leftReleased(){
    key_left = 0;

    character_object->moving = key_right == 1;
    if(key_right == 1){
        character_horizontalMovement = 1;
        character_object->horizontalDirection = 1;
    }else{
        character_horizontalMovement = 0;
    }

    return 0;
}

int rightPressed(){
    key_right = 1;

    character_horizontalMovement = 1;
    character_object->horizontalDirection = 1;
    character_object->moving = 1;

    return 0;
}

int rightReleased(){
    key_right = 0;

    character_object->moving = key_left == 1;
    if(key_left == 1){
        character_object->horizontalDirection = -1;
        character_horizontalMovement = -1;
    }else{
        character_horizontalMovement = 0;
    }

    return 0;
}

int jumpPressed(){
    key_up = 1;

    if(character_object->standing){
        character_object->yVelocity = -CHARACTER_JUMPSPEED;

        lastJump = SDL_GetTicks();
    }

    return 0;
}

int jumpReleased(){
    key_down = 0;

    float currentTime = SDL_GetTicks();
    printf("Jump time: %f\n", currentTime - lastJump);
    if(MINIMUM_JUMP_TIME < currentTime - lastJump && currentTime - lastJump < 3000 && character_object->yVelocity < 0){
        character_object->yVelocity *= 0.6;
    }

    return 0;
}

// Serve apenas para limpar os eventos na lista de eventos, para evitar que o jogo trave.
int clearEvents(){
    while(SDL_PollEvent(&lastEvent)){
        if(lastEvent.type == SDL_QUIT)
            QUIT_PROGRAM = 1;

        if(lastEvent.type == SDL_KEYDOWN)
            return 1;

        if(lastEvent.type == SDL_JOYBUTTONDOWN)
            return 1;
    }

    return 0;
}

// Retorna string com um '\0' no final.
char * terminateString(char * string){
    int newStringSize = strlen(string) + 1;
    char * newString = calloc(newStringSize, sizeof(char));

    strcat(newString, string);
    newString[newStringSize - 1] = '\0';

    free(string);
    return newString;
}

// Processa os eventos e chama as funções necessárias.
int emptyEventList(SDLKey *keys){
    fprintf(errFile, "Key pressed...\n");
    while(SDL_PollEvent(&lastEvent)){
        //printf("Joystick button type: %d\n", lastEvent.jbutton.type);
        //printf("Joystick button btn: %d\n", lastEvent.jbutton.button);
        switch(lastEvent.type){
            case SDL_QUIT:
                QUIT_PROGRAM = 1;
                break;
            case SDL_KEYDOWN:
                printf("Key pressed: %d, %c\n", (int)lastEvent.key.keysym.sym, lastEvent.key.keysym.sym);
                if((int)lastEvent.key.keysym.sym <= 320)
                    keys_down[(int)lastEvent.key.keysym.sym] = 1;
                if(lastEvent.key.keysym.sym == keys[2]){
                    if(paused)
                        return 1;
                    jumpPressed();
                }
                else if(lastEvent.key.keysym.sym == keys[0])
                    leftPressed();
                else if(lastEvent.key.keysym.sym == keys[1])
                    rightPressed();
                else if((int)lastEvent.key.keysym.sym == 27){
                    if(paused)
                        paused = 0;
                    else{
                        paused = 1;

                        SDL_BlitSurface(pauseOverlay, NULL, screen, NULL);

                        SDL_Color defaultTextColor;
                        defaultTextColor.r = 0xFF;
                        defaultTextColor.g = 0xFF;
                        defaultTextColor.b = 0xFF;

                        renderText(defaultTextColor, "Aperte pulo para sair ou", xResolution/2, yResolution/2, 0, -1);
                        renderText(defaultTextColor, "Start/ESC para despausar.", xResolution/2, yResolution/2, 0, 1);

                        SDL_Flip(screen);

                        while(paused && !QUIT_PROGRAM){
                            int mustQuit = emptyEventList(keys);

                            if(mustQuit)
                                return 1;

                            SDL_Delay(20);
                        }
                    }
                }else if((int)lastEvent.key.keysym.sym == 304)
                        velocityMultiplier = RUNNING_MULTIPLIER;
                break;
            case SDL_KEYUP:
                if((int)lastEvent.key.keysym.sym <= 320)
                    keys_down[(int)lastEvent.key.keysym.sym] = 0;
                if (lastEvent.key.keysym.sym == keys[2])
                    jumpReleased();
                else if(lastEvent.key.keysym.sym == keys[0])
                    leftReleased();
                else if(lastEvent.key.keysym.sym == keys[1])
                    rightReleased();
                else if((int)lastEvent.key.keysym.sym == 304)
                    velocityMultiplier = 1;
                break;
            case SDL_JOYAXISMOTION:
                if(lastEvent.jaxis.axis == 0){
                    if(lastEvent.jaxis.value > JOYSTICK_NEUTRALRANGE){
                        if(key_left)
                            leftReleased();

                        rightPressed();
                    }else if(lastEvent.jaxis.value < -JOYSTICK_NEUTRALRANGE){
                        if(key_right)
                            rightReleased();

                        leftPressed();
                    }else{
                        if(key_left)
                            leftReleased();
                        if(key_right)
                            rightReleased();
                    }
                }
                break;
            case SDL_JOYBUTTONDOWN:
                buttons_down[(int)lastEvent.jbutton.button] = 1;
                switch(lastEvent.jbutton.button){
                    case 0:
                        if(paused)
                            return 1;
                        jumpPressed();
                        break;
                    case 5:
                        velocityMultiplier = RUNNING_MULTIPLIER;
                        break;
                    case 7:
                        if(paused)
                            paused = 0;
                        else{
                            paused = 1;

                            SDL_BlitSurface(pauseOverlay, NULL, screen, NULL);

                            SDL_Color defaultTextColor;
                            defaultTextColor.r = 0xFF;
                            defaultTextColor.g = 0xFF;
                            defaultTextColor.b = 0xFF;

                            renderText(defaultTextColor, "Aperte pulo para sair ou", xResolution/2, yResolution/2, 0, -1);
                            renderText(defaultTextColor, "Start/ESC para despausar.", xResolution/2, yResolution/2, 0, 1);

                            SDL_Flip(screen);

                            while(paused && !QUIT_PROGRAM){
                                int mustQuit = emptyEventList(keys);

                                if(mustQuit)
                                    return 1;

                                SDL_Delay(20);
                            }
                        }
                    default:
                        break;
                }
                break;
            case SDL_JOYBUTTONUP:
                buttons_down[(int)lastEvent.jbutton.button] = 0;
                switch(lastEvent.jbutton.button){
                    case 0:
                        jumpReleased();
                        break;
                    case 5:
                        velocityMultiplier = 1;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return 0;
}

// Roda uma frame do jogo.
int runGameFrame(SDLKey *keys){
    // Processamento dos eventos.
    int mustQuit = emptyEventList(keys);

    if(mustQuit)
        return 1;

    // Atualização da física.
    calculatePhysics(&(mapArray[currentMap->ID]));

    // Lógica do controle de velocidade do personagem
    targetVelocity = character_horizontalMovement * CHARACTER_HORIZONTALSPEED * directionalMultiplier * velocityMultiplier;

    float deltaVelocity = (targetVelocity - character_object->xVelocity);

    if(deltaVelocity!=0){
        if(fabs(deltaVelocity) < CHARACTER_ACCELERATION)
            character_object->xVelocity = targetVelocity;
        else
            character_object->xVelocity += CHARACTER_ACCELERATION * deltaVelocity/fabs(deltaVelocity);
    }

    // Renderização do cenário.
    game_renderFrame(&(mapArray[currentMap->ID]), 1);

    return 0;
}

// Carrega o mapa de ID mapID.
int loadLevel(int mapID, SDLKey *keys){
    float timeCounter;
    float timeDifference;

    loadMap(mapID);

    int attempts = 1;
    float levelStart = SDL_GetTicks();

    while(QUIT_PROGRAM != 1){
        if(hasDied){
            loadMap(currentMap->ID);

            attempts++;
            levelStart = SDL_GetTicks();
        }

        timeCounter = SDL_GetTicks();

        if(currentMap->ID == 0){
            int xPos = character_object->xPosition;
            int yPos = character_object->yPosition;

            topText = terminateString("");

            // Checando (de uma forma extremamente estúpida) se o jogador está perto das portas.
            if(abs(xPos - 4*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE){
                topText = terminateString("Aperte enter para voltar ao menu.");

                if(keys_down[13] || buttons_down[5])
                    return 0;
            }
            else if(abs(xPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE){
                if(lastUnlockedLevel >= 1){
                    topText = terminateString("Level 1");

                    if(keys_down[13] || buttons_down[5])
                        return 1;
                }else
                    topText = terminateString("FASE TRAVADA");
            }
            else if(abs(xPos - 17*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE){
                if(lastUnlockedLevel >= 2){
                    topText = terminateString("Level 2");

                    if(keys_down[13] || buttons_down[5])
                        return 2;
                }else
                    topText = terminateString("FASE TRAVADA");
            }
            else if(abs(xPos - 25*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE){
                if(lastUnlockedLevel >= 3){
                    topText = terminateString("Level 3");

                    if(keys_down[13] || buttons_down[5])
                        return 3;
                }else
                    topText = terminateString("FASE TRAVADA");
            }
            /*else if(abs(xPos - 25*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 14*SQUARE_SIZE) < MAX_DOOR_RANGE){
                topText = terminateString("Level 4");

                if(keys_down[13] || buttons_down[5])
                    return 4;
            }
            else if(abs(xPos - 17*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 14*SQUARE_SIZE) < MAX_DOOR_RANGE){
                topText = terminateString("Level 5");

                if(keys_down[13] || buttons_down[5])
                    return 5;
            }
            else if(abs(xPos - 9*SQUARE_SIZE) < MAX_DOOR_RANGE && abs(yPos - 14*SQUARE_SIZE) < MAX_DOOR_RANGE){
                topText = terminateString("Level 6");

                if(keys_down[13])
                    return 6;
            }*/
        }else{
            int xPos = character_object->xPosition;
            int yPos = character_object->yPosition;

            if(abs(xPos - currentMap->xExit) < MAX_FINISH_RANGE && abs(yPos - currentMap->yExit) < MAX_FINISH_RANGE){
                break;
            }
        }

        // Roda uma frame do jogo.
        if(runGameFrame(keys))
            return 0;

        // Se o tempo decorrido na execução da frame for menor que o 1000/60 milésimos (o tempo de uma frame em 60 FPS), ele espera
        // o restante de tempo para completar 1000/60 milésimos.
        timeDifference = SDL_GetTicks() - timeCounter;

        if(timeDifference < 1000 / maxFPS){
            SDL_Delay( (1000/maxFPS) - timeDifference );
        }
    }

    return SDL_GetTicks() - levelStart;
}

// Mostra a introdução (a tela de "ICC: O JOGO", e da história inicial)
int playIntroduction(){
    float timeCounter;

    SDL_Surface * splashScreen = load_image("./textures/Main.png");
    SDL_BlitSurface(splashScreen, NULL, screen, NULL);
    SDL_Flip(screen);

    timeCounter = SDL_GetTicks();

    while(SDL_GetTicks() < timeCounter + 2500){
        clearEvents();

        if(QUIT_PROGRAM){
            SDL_Quit();

            return 0;
        }

        SDL_Delay(10);
    }

    SDL_FreeSurface(splashScreen);

    SDL_Surface * writtenBG = load_image("./textures/BG.png");
    SDL_BlitSurface(writtenBG, NULL, screen, NULL);
    SDL_Flip(screen);

    while(1){
        if(clearEvents() == 1){
            break;
        }

        if(QUIT_PROGRAM){
            SDL_Quit();

            return 0;
        }

        SDL_Delay(10);
    }

    SDL_FreeSurface(writtenBG);

    return 0;
}

// Carrega o save.dat.
int loadSave(){
    FILE * saveFile = fopen("save.dat", "r");

    fscanf(saveFile, "%d", &lastUnlockedLevel);

    int i;
    for(i=0; i<MAP_NUMBER; i++)
        fscanf(saveFile, "%f", &(levelScores[i]));

    fscanf(saveFile, "%hd", &vol[CHANNELMENU - 1]);
    fscanf(saveFile, "%hd", &vol[CHANNELEFFECT - 1]);
    fscanf(saveFile, "%hd", &vol[CHANNELMUSIC - 1]);

    fclose(saveFile);

    return 0;
}

// Salva o save.dat.
int writeSave(){
    FILE * saveFile = fopen("save.dat", "w");

    fprintf(saveFile, "%d ", lastUnlockedLevel);

    int i;
    for(i=0; i<MAP_NUMBER; i++)
        fprintf(saveFile, "%f ", levelScores[i]);

    fprintf(saveFile, "%d ", vol[0]);
    fprintf(saveFile, "%d ", vol[1]);
    fprintf(saveFile, "%d", vol[2]);

    fclose(saveFile);

    return 0;
}

// Começa a lógica do jogo.
int playGame(SDLKey *keys){
    while(!QUIT_PROGRAM){
        int selected = loadLevel(0, keys);

        topText = terminateString("");

        if(selected != 0){
            int time = loadLevel(selected, keys);

            victoryScreen(selected, time);
        }else
            break;
    }
    return 0;
}

// Mostra a tela da VITÓRIA.
int victoryScreen(int level, int time){
    float completionSeconds = (float)time / 1000;
    float score = 1000000/completionSeconds;

    if(completionSeconds == 0)
        return 1;

    int backgroundColor = SDL_MapRGBA(screen->format, 0, 0, 0, 0);

    SDL_FillRect(screen, NULL, backgroundColor);

    SDL_Color defaultTextColor;
    defaultTextColor.r = 0xFF;
    defaultTextColor.g = 0xFF;
    defaultTextColor.b = 0xFF;

    char * title;
    if(score > levelScores[level])
        title = terminateString("VOCÊ VENCEU E QUEBROU O RECORDE!");
    else
        title = terminateString("VOCÊ VENCEU!");

    char * midText = calloc(40, sizeof(char));
    char * recordText = calloc(40, sizeof(char));
    char * bottomText = terminateString("Aperte qualquer tecla para continuar...");

    sprintf(midText, "Seu score: %.2f", score);
    sprintf(recordText, "Recorde anterior: %.2f", levelScores[level]);

    while(!QUIT_PROGRAM){
        if(clearEvents()){
            break;
        }

        renderText(defaultTextColor, title, xResolution/2, 50, 0, 1);
        renderText(defaultTextColor, midText, xResolution/2, 250, 0, 1);
        renderText(defaultTextColor, recordText, xResolution/2, 300, 0, 1);
        renderText(defaultTextColor, bottomText, xResolution/2, 400, 0, 1);

        SDL_Flip(screen);

        SDL_Delay(33);
    }

    if(score > levelScores[level])
        levelScores[level] = score;

    if(level == lastUnlockedLevel){
        lastUnlockedLevel++;
    }

    writeSave();

    return 0;
}

int main( int argc, char* args[] ){

    SDLKey *keys;

    srand(time(NULL));

    lastJump = SDL_GetTicks();

    times = calloc(6, sizeof(int));

    errFile = fopen("DEBUGGER.txt", "w");

    TTF_Init();

    // GRAPHICS
    initializeGraphics();

    SDL_WM_SetCaption("ICC: O Jogo", NULL);

    // MAP
    initializeMaps();

    QUIT_PROGRAM = 0;

    SDL_Joystick * stick = NULL;
    stick = SDL_JoystickOpen(0);

    keys_down = calloc(320, sizeof(int));
    buttons_down = calloc(20, sizeof(int));

    fprintf(errFile, "Loading saves...\n");
    loadSave();

    while ( !QUIT_PROGRAM ){
        keys = Finit_menu();

        playGame( keys );

        writeSave();
    }

    // BENCHMARKING!
    /*current = SDL_GetTicks();

    int frame = 0;
    float start = SDL_GetTicks();

    while(SDL_GetTicks() <= start + 10000){

        runGameFrame();
        frame++;

    }

    printf("Frame rate: %d", frame/10);*/

    // AVALIADOR DE TEMPO DE PROCESSAMENTO!
    /*int a;

    for(a=0; a<6; a++){
        printf("ID %d: %d ms.\n", a, times[a]);
    }*/

    SDL_Quit();

    return 0;
};

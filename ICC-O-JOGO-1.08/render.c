#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "header.c"

// CODE

// Lê uma string de nomes de arquivos e as separa num vetor de nomes de arquivos.
char ** readFileList(FILE * file, char divider, char end){
    char ** fileArray;
    fileArray = (char**)malloc(1*sizeof(char*));

    int arraySize = 0;
    char * buffer = NULL;
    int bufferSize = 0;
    char current;

    int count = 0;

    // Lendo os chares dos nomes de arquivo.
    while(fscanf(file, "%c", &current)){
        fprintf(errFile, "%c", current);
        if((current != divider) && (current != end)){
            bufferSize++;
            buffer = (char*)realloc(buffer, bufferSize*sizeof(char));
            buffer[bufferSize - 1] = current;
        }else{
            // Aumentando o tamanho do buffer e finalizando-o
            bufferSize++;
            buffer = (char*)realloc(buffer, bufferSize*sizeof(char));
            buffer[bufferSize - 1] = '\0';

            // Indo para a próxima string
            arraySize++;
            fileArray = (char**)realloc(fileArray, arraySize*sizeof(char*));
            fileArray[arraySize - 1] = malloc(bufferSize*sizeof(char));
            strcpy(fileArray[arraySize - 1], buffer);

            free(buffer);
            buffer = NULL;
            bufferSize = 0;
        }
        if(current == end)
            break;
    }

    return fileArray;
}

// Lê um vetor de nomes de arquivos e os carrega para a memória.
SDL_Surface ** loadImageList(char ** fileList, char * directory, int fileNumber){
    dumpMessage("Loading image from file list string.\n");

    // Alocando um vetor de ponteiros de imagens.
    SDL_Surface ** images = (SDL_Surface *)malloc(fileNumber*sizeof(SDL_Surface *));

    char * end = malloc(sizeof(char));
    end[0] = '\0';

    int n;
    for(n=0; n<fileNumber; n++){
        fprintf(errFile, "Loading %s to memory...\n", fileList[n]);

        // Gerando o caminho do arquivo
        char * filePath = (char *)calloc(strlen(directory) + strlen(fileList[n]) + 1, sizeof(char));

        strcat(filePath, directory);
        strcat(filePath, fileList[n]);
        strcat(filePath, end);

        images[n] = load_image(filePath);

        free(fileList[n]);
        free(filePath);
    }

    free(fileList);
    free(end);

    return images;
}

// Carrega uma imagem de nome filename para a memória.
SDL_Surface * load_image(char * filename){
    printf("Carregando imagem: %s\n", filename);

    //Imagem otimizada a ser usada.
    SDL_Surface* optimizedImage = NULL;

    //Carrengando a imagem.
    SDL_Surface* loadedImage = IMG_Load( filename );

    //Se a imagem foi carregada com sucesso
    if( loadedImage != NULL ){
        //Otimizando a imagem
        optimizedImage = SDL_DisplayFormatAlpha( loadedImage );

        //Libera a imagem antiga
        SDL_FreeSurface( loadedImage );
    } else {
        printf("ERRO: Impossivel carregar %s\n", filename);
        printf("CAUSA: %s\n", IMG_GetError());

        fprintf(errFile, "ERRO: Impossivel carregar %s\n", filename);
    }

    return optimizedImage;
};

// Retorna uma superfície transparente que é usada para montar os blocos do mapa.
SDL_Surface * returnTransparentSurface(int x_size, int y_size){
    // Criando a superfície.
    SDL_Surface * tempSurface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        x_size,
        y_size,
        32,
        screen->format->Rmask,
        screen->format->Gmask,
        screen->format->Bmask,
        screen->format->Amask
    );

    // Otimizando-a.
    SDL_Surface * returnSurface = SDL_DisplayFormat(tempSurface);
    SDL_FreeSurface(tempSurface);

    SDL_LockSurface(returnSurface);

    int x, y;

    int transparentPixel = SDL_MapRGBA(screen->format, 0xFF, 0, 0xFF, 0xFF);
    int * pixels = (int *)returnSurface->pixels;

    // Preenchendo a superfície com pixels transparentes.
    for(y=0; y<y_size; y++){
        for(x=0; x<x_size; x++){
            pixels[y*x_size + x] = transparentPixel;
        }
    }

    SDL_UnlockSurface(returnSurface);

    SDL_SetColorKey(returnSurface, SDL_SRCCOLORKEY | SDL_RLEACCEL, transparentPixel);

    return returnSurface;
}

// Copia a superfície source para a destination na posição xPos, yPos, sem filtros.
int rawBlit(int xPos, int yPos, SDL_Surface * source, SDL_Surface * destination){

    int x, y;

    // Trava as superfícies para que seja possível mexer diretamente nos pixels das imagens.
    SDL_LockSurface(source);
    SDL_LockSurface(destination);

    // Convertendo os pixels para um vetor de ints.
    int * sourcePixels = (int *)source->pixels;
    int * destinationPixels = (int *)destination->pixels;

    // Calculando os mínimos X e Y.
    int minX = min(xPos + source->w, destination->w);
    int minY = min(yPos + source->h, destination->h);

    // Percorrendo as posições renderizadas.
    for(y=yPos; y<minY; y++){
        for(x=xPos; x<minX; x++){
            destinationPixels[y*(destination->w) + x] = sourcePixels[(y-yPos)*(source->w) + x - xPos];
        }
    }

    // Destravando as superfícies para poder blitar elas.
    SDL_UnlockSurface(source);
    SDL_UnlockSurface(destination);

    return 0;
}

// Monta as camadas do mapa que contém os blocos.
int initializeMapGraphics(struct map * inputMap){
    currentMap->backgroundLayer = returnTransparentSurface(SQUARE_SIZE*(inputMap->xSize), SQUARE_SIZE*(inputMap->ySize));
    currentMap->mainLayer = returnTransparentSurface(SQUARE_SIZE*(inputMap->xSize), SQUARE_SIZE*(inputMap->ySize));

    int i, j;

    for(i=0; i<inputMap->xSize; i++){
        for(j=0; j<inputMap->ySize; j++){
            fprintf(errFile, "Gerando (%d, %d)\n", i, j);

            struct blockType * texture = &(textures[(int)inputMap->mapData[i][j]]);

            if(texture->layer == 0)
                rawBlit(i*SQUARE_SIZE, j*SQUARE_SIZE, texture->frames[ random(0, texture->frameNumber - 1) ], currentMap->backgroundLayer);
            else
                rawBlit(i*SQUARE_SIZE, j*SQUARE_SIZE, texture->frames[ random(0, texture->frameNumber - 1) ], currentMap->mainLayer);
        }
    }

    return 0;
}

// Inicializa os gráficos (e importa de textures.dat as configurações das texturas).
int initializeGraphics(){
    // Resolução
    xResolution = 896;
    yResolution = 544;

    maxFPS = 60;

    SDL_Init( SDL_INIT_EVERYTHING );
    screen = SDL_SetVideoMode(xResolution, yResolution, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);

    textures = (struct blockType*)malloc(128 * sizeof(struct blockType));

    printf("CARREGANDO textures.txt\n");

    FILE * texturesFile = fopen("textures.dat", "r");

    if(texturesFile == NULL){
        printf("ERRO AO CARREGAR: textures.txt\n");
        return 0;
    }

    lastBackgroundLayer = NULL;
    lastMainLayer = NULL;

    int i;
    char j;

    while( fscanf(texturesFile, "TEXTURE %c:", &j) == 1 ){
        fscanf(texturesFile, "\n");

        i=(int)j;

        // Lendo as propriedades das texturas.
        fscanf(texturesFile, "ANIMATED: %d;", &(textures[i].animated));
        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "FRAMENUMBER: %d;", &(textures[i].frameNumber));
        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "INTERVAL: %d;", &(textures[i].interval));
        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "COLLIDABLE: %d;", &(textures[i].collidable));
        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "LAYER: %d;", &(textures[i].layer));
        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "FRAMES: ");

        // Lendo e carregando para a memória as imagens correspondentes a textura.
        char ** fileArray = readFileList(texturesFile, ',', ';');

        textures[i].frames = loadImageList(fileArray, DIR_TEXTURES, textures[i].frameNumber);

        fscanf(texturesFile, "\n");
        fscanf(texturesFile, "\n");
    }

    fclose(texturesFile);

    // Gerando a superfície cinza semi-transparente.
    pauseOverlay = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, xResolution, yResolution, 32, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    int overlayColor = SDL_MapRGBA(screen->format, 0x7E, 0x7E, 0x7E, 0);

    pauseOverlay = SDL_DisplayFormat(pauseOverlay);

    SDL_FillRect(pauseOverlay, NULL, overlayColor);
    SDL_SetAlpha(pauseOverlay, SDL_SRCALPHA, 0x7E);

    // Carregando para a memória a fonte padrão do jogo.
    defaultFont = TTF_OpenFont("gamefont.ttf", 18);

    if(defaultFont == NULL)
        printf("Error at loading default font: %s\n", TTF_GetError());

    return 0;
}

// Renderiza o texto text com cor color na posição (x_position, y_position), com alinhamento x_alignment, y_alignment.
int renderText(SDL_Color color, char * text, int x_position, int y_position, int x_alignment, int y_alignment){
    int width, height;

    SDL_Surface * temporarySurface = calloc(1, sizeof(SDL_Surface));
    SDL_Rect * temporaryRect = calloc(1, sizeof(SDL_Rect));

    TTF_SizeText(defaultFont, text, &width, &height);
    temporaryRect->x = x_position + (width/2) * (x_alignment - 1);
    temporaryRect->y = y_position + (height/2) * (y_alignment - 1);

    temporarySurface = TTF_RenderText_Solid(defaultFont, text, color);
    SDL_BlitSurface(temporarySurface, NULL, screen, temporaryRect);

    SDL_FreeSurface(temporarySurface);
    free(temporaryRect);

    return 0;
}

// Renderiza uma frame do jogo.
int game_renderFrame(struct map * inputMap, int flip){

    SDL_Rect viewArea;
    SDL_Rect screenCenter;

    // Calcula o centro da tela
    screenCenter.x = character_object->xPosition + character_object->xSize/2;
    screenCenter.y = character_object->yPosition + character_object->ySize/2;

    // Checa se o personagem do jogador está posicionado de tal forma que o jogo tentaria renderizar
    // coisas para fora dos limites do mapa (no eixo X). Se estiver, corrige o posicionamento da tela.
    if(screenCenter.x - xResolution/2 < 0){
        screenCenter.x = ceil(xResolution/2);
    }else if(screenCenter.x + xResolution/2 > inputMap->xSize * SQUARE_SIZE){
        screenCenter.x = floor(inputMap->xSize * SQUARE_SIZE - xResolution / 2);
    }

    // Checa se o personagem do jogador está posicionado de tal forma que o jogo tentaria renderizar
    // coisas para fora dos limites do mapa (no eixo Y). Se estiver, corrige o posicionamento da tela.
    if(screenCenter.y - yResolution/2 < 0){
        screenCenter.y = ceil(yResolution/2);
    }else if(screenCenter.y + yResolution/2 > inputMap->ySize * SQUARE_SIZE){
        screenCenter.y = floor(inputMap->ySize * SQUARE_SIZE - yResolution / 2);
    }

    // Calcula a área que é visualizada pelo jogador.
    viewArea.x = screenCenter.x - xResolution/2;
    viewArea.y = screenCenter.y - yResolution/2;

    viewArea.w = xResolution;
    viewArea.h = yResolution;

    // Blita a camada zero de blocos do mapa.
    SDL_BlitSurface(currentMap->backgroundLayer, &viewArea, screen, NULL);

    SDL_Rect relativePosition;

    int i;

    for(i=0; i<physicsObjectNumber; i++){
        // Processando as animações dos objetos.
        if(physicsObjects[i]->horizontalDirection == 1){
            if(physicsObjects[i]->lastDirection != 1){
                physicsObjects[i]->currentFrame = physicsObjects[i]->type->rightBegin;
                physicsObjects[i]->lastDirection = 1;
                physicsObjects[i]->frameCounter = 0;
            }

            if((physicsObjects[i]->moving && physicsObjects[i]->standing) || physicsObjects[i]->type->alwaysAnimated){
                if(physicsObjects[i]->frameCounter >= physicsObjects[i]->type->animationInterval){
                    physicsObjects[i]->frameCounter = 0;

                    physicsObjects[i]->currentFrame++;

                    if(physicsObjects[i]->currentFrame > physicsObjects[i]->type->rightEnd){
                        physicsObjects[i]->currentFrame = physicsObjects[i]->type->rightBegin;
                    }
                }
            }else{
                physicsObjects[i]->currentFrame = physicsObjects[i]->type->rightBegin;
                physicsObjects[i]->frameCounter = 0;
            }
        }else if(physicsObjects[i]->horizontalDirection == -1){
            if(physicsObjects[i]->lastDirection != -1){
                physicsObjects[i]->currentFrame = physicsObjects[i]->type->leftBegin;
                physicsObjects[i]->lastDirection = -1;
                physicsObjects[i]->frameCounter = 0;
            }

            if((physicsObjects[i]->moving && physicsObjects[i]->standing) || physicsObjects[i]->type->alwaysAnimated){
                if(physicsObjects[i]->frameCounter >= physicsObjects[i]->type->animationInterval){
                    physicsObjects[i]->frameCounter = 0;

                    physicsObjects[i]->currentFrame++;

                    if(physicsObjects[i]->currentFrame > physicsObjects[i]->type->leftEnd){
                        physicsObjects[i]->currentFrame = physicsObjects[i]->type->leftBegin;
                    }
                }
            }else{
                physicsObjects[i]->currentFrame = physicsObjects[i]->type->leftBegin;
                physicsObjects[i]->frameCounter = 0;
            }
        }

        // Renderiza os objetos na camada 0.
        if(physicsObjects[i]->active && physicsObjects[i]->layer == 0){
            // Calculando a posição do objeto na tela relativa ao centro da tela.
            relativePosition.x = physicsObjects[i]->xPosition - screenCenter.x + xResolution/2;
            relativePosition.y = physicsObjects[i]->yPosition - screenCenter.y + yResolution/2;

            // Contador interno de frames para saber quando trocar de imagem.
            physicsObjects[i]->frameCounter++;

            SDL_BlitSurface(physicsObjects[i]->type->frames[physicsObjects[i]->currentFrame], NULL, screen, &relativePosition);
        }
    }

    // Blitando a camada 1 de blocos do mapa.
    SDL_BlitSurface(currentMap->mainLayer, &viewArea, screen, NULL);

    for(i=0; i<physicsObjectNumber; i++){
        // Renderiza os objetos na camada 1.
        if(physicsObjects[i]->active && physicsObjects[i]->layer == 1){
            // Calcula a posição do objeto na tela relativa ao centro da tela.
            relativePosition.x = physicsObjects[i]->xPosition - screenCenter.x + xResolution/2;
            relativePosition.y = physicsObjects[i]->yPosition - screenCenter.y + yResolution/2;

            // Contador interno de frames para saber quando trocar de imagem.
            physicsObjects[i]->frameCounter++;

            SDL_BlitSurface(physicsObjects[i]->type->frames[physicsObjects[i]->currentFrame], NULL, screen, &relativePosition);
        }
    }

    int w, h;

    // Renderia topText no topo da tela.
    SDL_Color defaultRenderColor;
    defaultRenderColor.r = defaultRenderColor.g = defaultRenderColor.b = 0;

    TTF_SizeText(defaultFont, topText, &w, &h);
    SDL_Surface * topTextSurface = TTF_RenderText_Solid(defaultFont, topText, defaultRenderColor);

    relativePosition.x = (xResolution / 2) - (w / 2);
    relativePosition.y = 5;

    SDL_BlitSurface(topTextSurface, NULL, screen, &relativePosition);

    if(flip)
        SDL_Flip (screen);

    SDL_FreeSurface(topTextSurface);

    return 0;
}


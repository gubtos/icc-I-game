#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <stdio.h>

#include "header.c"

// Copia uma lista ligada de posições.
struct positionLoop * copyPositionLoop(struct positionLoop * initialNode){
    if(initialNode == NULL){
        return NULL;
    }

    // Criando um ponteiro de struct positionLoop para poder percorrer os nós.
    struct positionLoop * currentNode = initialNode;

    // Copiando o primeiro nó.
    struct positionLoop * copyInitialNode = (struct positionLoop *)malloc(sizeof(struct positionLoop));

    copyInitialNode->next = copyInitialNode;
    copyInitialNode->prev = copyInitialNode;

    copyInitialNode->xPosition = initialNode->xPosition;
    copyInitialNode->yPosition = initialNode->yPosition;

    copyInitialNode->requiredTime = initialNode->requiredTime;
    copyInitialNode->beginTime = 0;

    // Enquanto o próximo nó for diferente do inicial, ainda não acabaram os nós a serem copiados.
    while(currentNode->next!=initialNode){
        // Estabelecendo o próximo nó.
        currentNode = currentNode->next;

        // Copiando currentNode.
        struct positionLoop * tempNode = (struct positionLoop *)malloc(sizeof(struct positionLoop));

        tempNode->xPosition = currentNode->xPosition;
        tempNode->yPosition = currentNode->yPosition;

        tempNode->requiredTime = currentNode->requiredTime;
        tempNode->beginTime = 0;

        tempNode->next = copyInitialNode;
        tempNode->prev = copyInitialNode->prev;

        copyInitialNode->prev->next = tempNode;
        copyInitialNode->prev = tempNode;
    }

    return copyInitialNode;
}

// Libera da memória uma lista ligada de posições
int freePositionLoop(struct positionLoop * initialNode){
    // Se o nó for nulo, não há o que retornar.
    if(initialNode == NULL){
        return 0;
    }

    // Criando um ponteiro de struct positionLoop para poder percorrer os nós.
    struct positionLoop * currentNode = initialNode;

    // Percorrendo os nós e eliminando-os da memória.
    while(currentNode->next != initialNode){
        currentNode = currentNode->next;
        free(currentNode->prev);
    }

    // Liberando o último nó.
    free(currentNode);

    return 0;
}

// Libera da memória um mapa carregado.
int freeLoadedMap(int mapID){
    if(currentMap != NULL){
        fprintf(errFile, "Offloading previously loaded map...\n");

        int i;

        // Liberação da memória ocupada por cada quadrado do mapa.
        for(i=0; i<currentMap->xSize; i++){
            /*for(j=0; j<currentMap->ySize; j++){
                // Habilitar se for necessário liberar da memória algum elemento específico a cada gridTile do mapa carregado.
            }*/

            free(currentMap->mapData[i]);
        }

        // Liberação da memória ocupada pelos objetos no mapa.
        for(i=0; i<physicsObjectNumber; i++){
            freePositionLoop(physicsObjects[i]->currentPosition);
            free(physicsObjects[i]);
        }

        free(physicsObjects);

        // Liberação da memória ocupada pela imagem do mapa (caso seja necessária).
        if(mapID != currentMap->ID){
            SDL_FreeSurface(currentMap->backgroundLayer);
            SDL_FreeSurface(currentMap->mainLayer);

            lastBackgroundLayer = NULL;
            lastMainLayer = NULL;
        }else{
            lastBackgroundLayer = currentMap->backgroundLayer;
            lastMainLayer = currentMap->mainLayer;
        }

        free(currentMap->mapData);
        free(currentMap);

        fprintf(errFile, "Map offloading complete.\n");
    }

    return 0;
}

// Carrega o mapa de id mapID para a memória.
int loadMap(int mapID){

    fprintf(errFile, "Loading map of ID %d.\n", mapID);

    // Liberando a memória ocupada pelo mapa carregado previamente.
    freeLoadedMap(mapID);

    // Carregando o mapa que foi pedido à função
    currentMap = (struct loadedMap *)malloc(sizeof(struct loadedMap));

    int x, y;

    currentMap->ID = mapID;

    fprintf(errFile, "Lendo mapa...\n");
    // Copiando as informações do arquétipo do mapa para o mapa sendo carregado.
    currentMap->mapData = (struct gridTile **)malloc(mapArray[mapID].xSize * sizeof(struct gridTile *));

    for(x=0; x<mapArray[mapID].xSize; x++){
        currentMap->mapData[x] = (struct gridTile *)malloc(mapArray[mapID].ySize * sizeof(struct gridTile));

        for(y=0; y<mapArray[mapID].ySize; y++){
            currentMap->mapData[x][y].collidable = textures[ (int)mapArray[mapID].mapData[x][y] ].collidable;
        }
    }

    fprintf(errFile, "Criando vetor de objetos...\n");
    // Criando um novo vetor de objetos e determinando a quantidade de objetos.
    physicsObjects = (struct physicsObject **)malloc( mapArray[mapID].objectNumber * sizeof(struct physicsObject *) );
    physicsObjectNumber = mapArray[mapID].objectNumber;

    // ID do objeto que será carregado no vetor de objetos do mapa.
    int inputArrayID;

    fprintf(errFile, "Lendo objetos...\n");
    // Carregando objeto por objeto.
    for(inputArrayID = 0; inputArrayID<physicsObjectNumber; inputArrayID++){
        fprintf(errFile, "Lendo objeto %d...\n", inputArrayID);

        // Estabelecendo as propriedades do novo objeto.
        struct physicsObject * currentObject = (struct physicsObject *)malloc(sizeof(struct physicsObject));

        currentObject->xPosition = mapArray[mapID].objectList[inputArrayID]->xPosition;
        currentObject->yPosition = mapArray[mapID].objectList[inputArrayID]->yPosition;

        currentObject->mass = mapArray[mapID].objectList[inputArrayID]->mass;
        currentObject->invMass = mapArray[mapID].objectList[inputArrayID]->invMass;

        currentObject->xSize = mapArray[mapID].objectList[inputArrayID]->type->xSize;
        currentObject->ySize = mapArray[mapID].objectList[inputArrayID]->type->ySize;

        currentObject->xVelocity = currentObject->yVelocity = 0;
        currentObject->xAcceleration = currentObject->yAcceleration = 0;

        currentObject->type = mapArray[mapID].objectList[inputArrayID]->type;

        currentObject->collidable = mapArray[mapID].objectList[inputArrayID]->type->collidable;
        currentObject->currentPosition = copyPositionLoop(mapArray[mapID].objectList[inputArrayID]->positionList);

        currentObject->physicsEnabled = mapArray[mapID].objectList[inputArrayID]->type->physicsEnabled;

        currentObject->active = 1;
        currentObject->isPlatform = mapArray[mapID].objectList[inputArrayID]->type->isPlatform;

        currentObject->horizontalDirection = currentObject->lastDirection = 1;
        currentObject->moving = 0;

        currentObject->currentFrame = mapArray[mapID].objectList[inputArrayID]->type->rightBegin;
        currentObject->frameCounter = 0;

        currentObject->layer = mapArray[mapID].objectList[inputArrayID]->layer;

        // Adicionando o objeto no vetor de objetos.
        physicsObjects[inputArrayID] = currentObject;
    }

    fprintf(errFile, "Exportando configuracoes...\n");
    // Importanto as informações sobre o tamanho do mapa.
    currentMap->xSize = mapArray[mapID].xSize;
    currentMap->ySize = mapArray[mapID].ySize;

    currentMap->xExit = mapArray[mapID].xExit;
    currentMap->yExit = mapArray[mapID].yExit;

    // Resetando algumas informações.
    characterHealth = 1;

    hasDied = 0;

    paused = 0;

    character_object = physicsObjects[0];
    character_object->moving = 0;

    directionalMultiplier = 1;
    velocityMultiplier = 1;

    fprintf(errFile, "Inicializando fisica...\n");
    initializePhysics();

    fprintf(errFile, "Gerando background...\n");
    // Se o último mapa carregado for o mesmo do que está sendo carregado, as imagens do background são reaproveitadas.
    if(lastBackgroundLayer == NULL)
        initializeMapGraphics(&(mapArray[mapID]));
    else{
        fprintf(errFile, "Background anterior encontrado, reproveitando...\n");
        currentMap->backgroundLayer = lastBackgroundLayer;
        currentMap->mainLayer = lastMainLayer;
    }

    fprintf(errFile, "Map loading complete\n");

    return 0;
}

// Importa dos arquivos maps.dat e objects.dat os mapas e os objetos, respectivamente.
int initializeMaps(){
    currentMap = NULL;

    printf("\nCarregando objetos...\n");
    dumpMessage("LOADING OBJECTS\n");

    // Abrindo a database de objetos.
    FILE * objectsFile = fopen("objects.dat", "r");

    // Criando o vetor de tipos de objetos.
    objectsData = (struct objectType *)malloc(128*sizeof(struct objectType));

    int objectID;
    char cObjectID;

    levelScores = calloc(MAP_NUMBER, sizeof(float));

    // Leitura da database de tipos de objetos.
    while( fscanf(objectsFile, "OBJECT %c:", &cObjectID) == 1 ){
        // ID do tipo de objeto em struct objectType * objectsData.
        objectID = (int)cObjectID;
        fprintf(errFile, "OBJECT %d\n", objectID);

        // Leitura das propriedades de um tipo de objeto.
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "XSIZE: %d;", &(objectsData[objectID].xSize));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "YSIZE: %d;", &(objectsData[objectID].ySize));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "COLLIDABLE: %d;", &(objectsData[objectID].collidable));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "DAMAGE: %d;", &(objectsData[objectID].damage));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "ISPLATFORM: %d;", &(objectsData[objectID].isPlatform));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "PHYSICS_ENABLED: %d;", &(objectsData[objectID].physicsEnabled));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "FRAMENUMBER: %d;", &(objectsData[objectID].frameNumber));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "ALWAYSANIMATED: %d;", &(objectsData[objectID].alwaysAnimated));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "LEFTTEXTURE: %d ", &(objectsData[objectID].leftBegin));
        fscanf(objectsFile, "%d;", &(objectsData[objectID].leftEnd));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "RIGHTTEXTURE: %d ", &(objectsData[objectID].rightBegin));
        fscanf(objectsFile, "%d;", &(objectsData[objectID].rightEnd));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "ANIMATIONINTERVAL: %d;", &(objectsData[objectID].animationInterval));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "DEFAULTLAYER: %d;", &(objectsData[objectID].defaultLayer));
        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "FRAMES: ");

        // Quebra a lista de sprites do objeto em um vetor de nomes de arquivos de sprite.
        char ** fileArray = readFileList(objectsFile, ',', ';');

        dumpMessage("File list read.\n");

        // Lê cada nome de arquivo de fileArray, e carrega o arquivo correspondente na memória.
        objectsData[objectID].frames = loadImageList(fileArray, DIR_TEXTURES, objectsData[objectID].frameNumber);

        fscanf(objectsFile, "\n");
        fscanf(objectsFile, "\n");
    }

    fclose(objectsFile);

    printf("\nCarregando mapas...\n");
    dumpMessage("LOADING MAPS\n");

    FILE * mapsFile = fopen("maps.dat", "r");

    int mapID;

    // Leitura da database de mapas.
    while( fscanf(mapsFile, "MAP %d:", &mapID) == 1 ){
        // Leitura das propriedades de um mapa.
        fscanf(mapsFile, "\n");
        fscanf(mapsFile, "XSIZE: %d;", &(mapArray[mapID].xSize));
        fscanf(mapsFile, "\n");
        fscanf(mapsFile, "YSIZE: %d;", &(mapArray[mapID].ySize));
        fscanf(mapsFile, "\n");
        fscanf(mapsFile, "XGRAVITY: %f;", &(mapArray[mapID].xGravity));
        fscanf(mapsFile, "\n");
        fscanf(mapsFile, "YGRAVITY: %f;", &(mapArray[mapID].yGravity));
        fscanf(mapsFile, "\n");

        mapArray[mapID].mapData = (char **)malloc(mapArray[mapID].xSize * sizeof(char *));

        // Leitura dos tiles do mapa.
        char readChar;

        int currentColumn = 0;
        int currentRow = 0;

        // Transcrevendo os caracteres de map.dat para a memória do programa
        while( fscanf(mapsFile, "%c", &readChar) == 1 ){
            //printf("%c", readChar);

            if(readChar == ';')
                break;
            else if(readChar == '\n'){
                currentColumn = 0;
                currentRow++;
            }
            else{
                if(currentRow == 0){
                    mapArray[mapID].mapData[currentColumn] = (char *)malloc(mapArray[mapID].ySize * sizeof(char));
                }
                mapArray[mapID].mapData[currentColumn][currentRow] = readChar;

                currentColumn++;
            }
        }


        mapArray[mapID].objectList = NULL;
        //mapArray[mapID].objectList = (struct objectArchetype **)malloc(mapArray[mapID].objectNumber * sizeof(struct objectArchetype *));
        int objectCount = 0;

        for(fscanf(mapsFile, "%c", &readChar); readChar!=';'; fscanf(mapsFile, "%c", &readChar)){
            // 'O' identifica um objeto.
            if(readChar=='O'){
                mapArray[mapID].objectList = (struct objectArchetype **)realloc(mapArray[mapID].objectList, (objectCount+1) * sizeof(struct objectArchetype *));
                struct objectArchetype * object = (struct objectArchetype *)malloc(sizeof(struct objectArchetype));

                // Lendo o tipo de objeto e convertendo em um INT id
                fscanf(mapsFile, "%c", &readChar);
                int objectTypeID = (int)readChar;

                // Ligando o arquétipo do objeto ao seu tipo
                object->type = &objectsData[objectTypeID];

                // Atribuindo valores padrão ao objeto
                object->mass = 0;
                object->invMass = 0;

                object->positionList = NULL;
                object->layer = object->type->defaultLayer;

                for(fscanf(mapsFile, "%c", &readChar); readChar!='\n'; fscanf(mapsFile, "%c", &readChar)){
                    // 'x' é o identificador da posição x do objeto, no arquivo maps.txt.
                    if(readChar == 'x'){
                        fscanf(mapsFile, "%f", &(object->xPosition));
                        object->xPosition *= SQUARE_SIZE;
                    }

                    // 'y' é o identificador da posição y do objeto.
                    if(readChar == 'y'){
                        fscanf(mapsFile, "%f", &(object->yPosition));
                        object->yPosition *= SQUARE_SIZE;
                    }

                    // 'p' indica um nó de posição (para objetos que ficam transitando entre uma posição e outra)
                    if(readChar == 'p'){

                        // Primeiro, um nó é criado, para que posteriormente ele possa ser adicionado na corrente de posições.
                        struct positionLoop * node = (struct positionLoop *)malloc(sizeof(struct positionLoop));

                        // Posição do nó.
                        fscanf(mapsFile, "%f", &(node->xPosition));
                        fscanf(mapsFile, "%f", &(node->yPosition));

                        //printf("1Caught position: X: %f, Y: %f\n", node->xPosition, node->yPosition);

                        node->xPosition *= SQUARE_SIZE;
                        node->yPosition *= SQUARE_SIZE;

                        //printf("2Caught position: X: %f, Y: %f\n", node->xPosition, node->yPosition);

                        // Tempo que leva para chegar de um nó até outro.
                        fscanf(mapsFile, "%f", &(node->requiredTime));
                        node->beginTime = 0;

                        // Adicionando o novo nó na lista ligada de posições.
                        if(object->positionList == NULL){
                            object->positionList = node;
                            node->next = node;
                            node->prev = node;
                        }else{
                            node->prev = object->positionList->prev;
                            node->next = object->positionList;

                            object->positionList->prev->next = node;
                            object->positionList->prev = node;

                            printf("%p -> %p -> %p -> %p\n", object->positionList, object->positionList->next, object->positionList->next->next, object->positionList->next->next->next);
                        }
                    }

                    // 'm' é o indicador de massa do objeto.
                    if(readChar == 'm'){
                        fscanf(mapsFile, "%f", &(object->mass));
                        object->invMass = 1 / object->mass;
                    }

                    // 'l' é o indicador de layer do objeto.
                    if(readChar == 'l'){
                        fscanf(mapsFile, "%d", &(object->layer));
                    }
                }

                // Posição de saída do mapa (da porta final)
                if(objectTypeID == (int)'D'){
                    mapArray[mapID].xExit = object->xPosition;
                    mapArray[mapID].yExit = object->yPosition;
                }

                // Se a massa do objeto não for declarada no maps.txt, ele assume que o objeto tem densidade
                // 1 / pixel², e sua massa se torna seu volume.
                if(object->mass == 0 && object->invMass == 0){
                    object->mass = object->type->xSize * object->type->ySize;
                    object->invMass = 1 / object->mass;
                }

                mapArray[mapID].objectList[objectCount] = object;

                objectCount++;
            }
        }

        // Salvando o número de objetos
        mapArray[mapID].objectNumber = objectCount;

        fscanf(mapsFile, "\n");
        fscanf(mapsFile, "\n");
    }

    fclose(mapsFile);

    printf("Carregamento concluido.\n");
    fprintf(errFile, "Map database loading complete.\n");

    return 0;
};


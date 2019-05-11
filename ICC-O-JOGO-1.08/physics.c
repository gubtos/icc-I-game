#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <stdio.h>
#include <math.h>

#include "header.c"

// CODE

// Inicializa a f�sica.
int initializePhysics(){
    physics_lastRan = SDL_GetTicks();

    key_up = key_down = key_left = key_right = key_running = 0;

    return 0;
}

// Checa se os ret�ngulos rect1 e rect2 est�o colidindo.
int rectanglesCollide(SDL_Rect rect1, FloatRect rect2){
    // Detec��o mais simples de colis�o entre objetos.
    if((rect1.y + rect1.h <= rect2.y) || (rect2.y + rect2.h <= rect1.y))
        return 0;
    if((rect1.x + rect1.w <= rect2.x) || (rect2.x + rect2.w <= rect1.x))
        return 0;

    return 1;
}

float returnXVelocity(struct physicsObject * object){
    // C�lculo da velocidade em X de object a partir n�o diretamente de sua velocidade, mas sim de seu deslocamento.
    return object->xProjectedPosition - object->xPosition;
}

float returnYVelocity(struct physicsObject * object){
    // C�lculo da velocidade em Y de object a partir n�o diretamente de sua velocidade, mas sim de seu deslocamento.
    return object->yProjectedPosition - object->yPosition;
}

// Calcula a normal da colis�o entre obj1 e obj2, e retorna se eles est�o colidindo.
int collideAABB(struct physicsObject * obj1, struct physicsObject * obj2, Vector2 * normal, Vector2 * penetration){
    Vector2 distance;

    // C�lculo de metade do tamanho em X dos dois objetos.
    float ax_extent = (float)obj1->xSize / 2;
    float bx_extent = (float)obj2->xSize / 2;

    // Dist�ncia entre seus centros em X.
    distance.x = obj2->xProjectedPosition + bx_extent - (obj1->xProjectedPosition + ax_extent);

    // Penetra�ao absoluta em X.
    float x_overlap = ax_extent + bx_extent - fabs( distance.x );

    penetration->x = x_overlap;

    if(x_overlap > 0){
        // C�lculo da metade do tamanho em Y dos dois objetos.
        float ay_extent = (float)obj1->ySize / 2;
        float by_extent = (float)obj2->ySize / 2;

        // Dist�ncia entre seus centros em Y.
        distance.y = obj2->yProjectedPosition + by_extent - (obj1->yProjectedPosition + ay_extent);

        // Penetra��o absoluta em Y.
        float y_overlap = ay_extent + by_extent - fabs( distance.y );

        penetration->y = y_overlap;

        // Se h� penetra��o nos dois eixos, ent�o os objetos est�o se intersectando.
        if(y_overlap > 0){
            // Determina��o da normal da colis�o.
            if(x_overlap < y_overlap){
                if(distance.x < 0){
                    normal->x = -1;
                    normal->y = 0;
                }else{
                    normal->x = 1;
                    normal->y = 0;
                }

                return 1;
            }else{
                if(distance.y < 0){
                    normal->x = 0;
                    normal->y = -1;
                }else{
                    normal->x = 0;
                    normal->y = 1;
                }

                return 1;
            }
        }
    }

    return 0;
}

// displaceBox serve para calcular se o objeto object est� colidindo com os blocos do mapa, e se estiver, desloca-o para uma posi��o v�lida.
int displaceBox(struct map * inputMap, struct physicsObject * object, float xPos, float yPos, char axis){
    if(!object->collidable){
        return 0;
    }

    // Servem para checar quais quadrados da matriz de colis�o devem ser checados. Checar o mapa todo seria um disperd�cio do tempo de processamento.
    int minXTile = xPos / SQUARE_SIZE;
    int maxXTile = (ceil(xPos) + object->xSize) / SQUARE_SIZE;

    int minYTile = yPos / SQUARE_SIZE;
    int maxYTile = (ceil(yPos) + object->ySize) / SQUARE_SIZE;

    // Corresponde ao quadrado de colis�o do objeto que se quer checar.
    FloatRect collisionBox;
    collisionBox.x = xPos;
    collisionBox.y = yPos;
    collisionBox.w = object->xSize;
    collisionBox.h = object->ySize;

    // Corresponde ao quadrado de colis�o do tile da matriz de colis�o.
    SDL_Rect tileBox;
    tileBox.w = SQUARE_SIZE;
    tileBox.h = SQUARE_SIZE;

    int x, y;

    float xVelocity = xPos - object->xPosition;
    float yVelocity = yPos - object->yPosition;

    for(x = minXTile; x <= maxXTile; x++){
        for(y = minYTile; y <= maxYTile; y++){
            if(currentMap->mapData[x][y].collidable){
                tileBox.x = x*SQUARE_SIZE;
                tileBox.y = y*SQUARE_SIZE;

                // A velocidade no eixo que est� sendo checado por colis�es n�o pode ser 0, pois nesse caso n�o h� informa��o
                // sobre qual dire��o o objeto est� vindo, e portanto n�o � poss�vel reloc�-lo.
                int velocityPass = (axis == 'x' && xVelocity != 0) || (axis == 'y' && yVelocity != 0) || (axis == 'b' && xVelocity != 0 && yVelocity != 0);

                // Se o quadrado que est� sendo checado e o objeto dado se colidem.
                if(rectanglesCollide(tileBox, collisionBox) && velocityPass){
                    if(axis == 'x'){
                        if(xVelocity > 0){
                            // Se a velocidade em x � maior do que zero, o objeto s� pode estar vindo da esquerda e portanto
                            // ele � relocado para a esquerda do quadrado.
                            object->xProjectedPosition = tileBox.x - object->xSize;

                            // Se h� gravidade e ela � positiva em x, e o objeto colidiu com a superf�cie esquerda do tile,
                            // o objeto est� apoiado.
                            if(inputMap->xGravity > 0)
                                object->standing = 1;
                        }else{
                            // Se a velocidade em x � menor do que zero, o objeto s� pode estar vindo da direita e portanto
                            // ele � relocado para a direita do quadrado.
                            object->xProjectedPosition = tileBox.x + SQUARE_SIZE;

                            // Se h� gravidade e ela � negativa em x, e o objeto colidiu com a superf�cie direita do tile,
                            // o objeto est� apoiado.
                            if(inputMap->xGravity < 0)
                                object->standing = 1;
                        }
                        object->xVelocity = 0;
                    }
                    else if(axis == 'y'){
                        if(yVelocity > 0){
                            // Se a velocidade em y � maior do que zero, o objeto s� pode estar vindo de cima e portanto
                            // ele � relocado para a superf�cie do quadrado.
                            object->yProjectedPosition = tileBox.y - object->ySize;

                            // Se h� gravidade e ela � positiva em y, e o objeto colidiu com a superf�cie superior do tile,
                            // o objeto est� apoiado.
                            if(inputMap->yGravity > 0)
                                object->standing = 1;
                        }else{
                            // Se a velocidade em x � menor do que zero, o objeto s� pode estar vindo de baixo e portanto
                            // ele � relocado para baixo do quadrado.
                            object->yProjectedPosition = tileBox.y + SQUARE_SIZE;

                            // Se h� gravidade e ela � negativa em y, e o objeto colidiu com a superf�cie inferior do tile,
                            // o objeto est� apoiado.
                            if(inputMap->yGravity < 0)
                                object->standing = 1;
                        }
                        object->yVelocity = 0;
                    }
                    else if(axis == 'b'){
                        int x_distance = tileBox.x + SQUARE_SIZE/2 - (object->xProjectedPosition + object->xSize/2);
                        int y_distance = tileBox.y + SQUARE_SIZE/2 - (object->yProjectedPosition + object->ySize/2);

                        int xPenetration = SQUARE_SIZE/2 + object->xSize/2 - fabs(x_distance);
                        int yPenetration = SQUARE_SIZE/2 + object->ySize/2 - fabs(y_distance);

                        if(xPenetration > yPenetration){
                            if(yVelocity > 0){
                                // Se a velocidade em y � maior do que zero, o objeto s� pode estar vindo de cima e portanto
                                // ele � relocado para a superf�cie do quadrado.
                                object->yProjectedPosition = tileBox.y - object->ySize;

                                // Se h� gravidade e ela � positiva em y, e o objeto colidiu com a superf�cie superior do tile,
                                // o objeto est� apoiado.
                                if(inputMap->yGravity > 0)
                                    object->standing = 1;
                            }else{
                                // Se a velocidade em x � menor do que zero, o objeto s� pode estar vindo de baixo e portanto
                                // ele � relocado para baixo do quadrado.
                                object->yProjectedPosition = tileBox.y + SQUARE_SIZE;

                                // Se h� gravidade e ela � negativa em y, e o objeto colidiu com a superf�cie inferior do tile,
                                // o objeto est� apoiado.
                                if(inputMap->yGravity < 0)
                                    object->standing = 1;
                            }
                            object->yVelocity = 0;
                        }else{
                            if(xVelocity > 0){
                                // Se a velocidade em x � maior do que zero, o objeto s� pode estar vindo da esquerda e portanto
                                // ele � relocado para a esquerda do quadrado.
                                object->xProjectedPosition = tileBox.x - object->xSize;

                                // Se h� gravidade e ela � positiva em x, e o objeto colidiu com a superf�cie esquerda do tile,
                                // o objeto est� apoiado.
                                if(inputMap->xGravity > 0)
                                    object->standing = 1;
                            }else{
                                // Se a velocidade em x � menor do que zero, o objeto s� pode estar vindo da direita e portanto
                                // ele � relocado para a direita do quadrado.
                                object->xProjectedPosition = tileBox.x + SQUARE_SIZE;

                                // Se h� gravidade e ela � negativa em x, e o objeto colidiu com a superf�cie direita do tile,
                                // o objeto est� apoiado.
                                if(inputMap->xGravity < 0)
                                    object->standing = 1;
                            }
                            object->xVelocity = 0;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int calculatePhysics(struct map * inputMap){
    // Calcula quanto tempo se passou desde a �ltima atualiza��o da posi��o de objetos animados.
    // Serve para que a fun��o saiba em quanto tempo os objetos devem ser adiantados.
    float timeDifference = (float)((SDL_GetTicks() - physics_lastRan) / 1000);
    physics_lastRan = SDL_GetTicks();

    // O tempo calculado n�o pode ultrapassar 1/maxFPS segundos, pois possibilitaria a travessia de objetos nas paredes e cong�neres.
    if(timeDifference > (float)1/60)
        timeDifference = (float)1/60;

    int i, j;

    // C�lculo da velocidade e posi��o v�lida.
    for(i=0; i<physicsObjectNumber; i++){
        if(physicsObjects[i]->physicsEnabled){
            // Atualiza��o das velocidades a partir da acelera��o.
            physicsObjects[i]->xVelocity += (physicsObjects[i]->xAcceleration + inputMap->xGravity) * timeDifference;
            physicsObjects[i]->yVelocity += (physicsObjects[i]->yAcceleration + inputMap->yGravity) * timeDifference;

            // Atualiza��o da proje��o da posi��o a partir da velocidade.
            physicsObjects[i]->xProjectedPosition = physicsObjects[i]->xPosition + (physicsObjects[i]->xVelocity * timeDifference);
            physicsObjects[i]->yProjectedPosition = physicsObjects[i]->yPosition + (physicsObjects[i]->yVelocity * timeDifference);

            // S� haver� deslocamento em caso de colis�o com outros objetos;
            physicsObjects[i]->xDisplacement = 0;
            physicsObjects[i]->yDisplacement = 0;

            // Antes de testar colis�o, o padr�o � que o objeto esteja suspenso.
            physicsObjects[i]->standing = 0;

            // Checa por colis�es entre objetos e quadrados do mapa.
            displaceBox(inputMap, physicsObjects[i], physicsObjects[i]->xProjectedPosition, physicsObjects[i]->yPosition, 'x');
            displaceBox(inputMap, physicsObjects[i], physicsObjects[i]->xPosition, physicsObjects[i]->yProjectedPosition, 'y');
            displaceBox(inputMap, physicsObjects[i], physicsObjects[i]->xProjectedPosition, physicsObjects[i]->yProjectedPosition, 'b');

            // Simulando atrito com o ar.
            physicsObjects[i]->yVelocity *= 0.99;

            // Se o objeto estiver apoiado em algo, reduzir a velocidade dele para simular atrito.
            if(physicsObjects[i]->standing){
                if(inputMap->yGravity != 0){
                    physicsObjects[i]->xVelocity *= 0.9;
                }
                else if(inputMap->xGravity != 0){
                    physicsObjects[i]->yVelocity *= 0.9;
                }
            }
        }else if(physicsObjects[i]->currentPosition != NULL){
            // Apenas para resetar o deslocamento do objeto.
            physicsObjects[i]->xDisplacement = 0;
            physicsObjects[i]->yDisplacement = 0;

            // Calculando o tempo que se passou desde que o objeto come�ou a percorrer o caminho at� o pr�ximo n� do movimento.
            physicsObjects[i]->currentPosition->beginTime += timeDifference;

            // Se o tempo gasto em percorrer o caminho at� o pr�ximo n� for maior que o tempo que foi estipulado para isso,
            // come�ar a ir para o pr�ximo n�.
            if(physicsObjects[i]->currentPosition->beginTime >= physicsObjects[i]->currentPosition->requiredTime){
                float pastTime = physicsObjects[i]->currentPosition->requiredTime - physicsObjects[i]->currentPosition->beginTime;

                physicsObjects[i]->currentPosition = physicsObjects[i]->currentPosition->next;

                physicsObjects[i]->currentPosition->beginTime = pastTime;
            }

            // Dist�ncias X e Y entre um n� e outro.
            float xDifference = physicsObjects[i]->currentPosition->next->xPosition - physicsObjects[i]->currentPosition->xPosition;
            float yDifference = physicsObjects[i]->currentPosition->next->yPosition - physicsObjects[i]->currentPosition->yPosition;

            // Posi��o projetada do objeto.
            physicsObjects[i]->xProjectedPosition = (xDifference / physicsObjects[i]->currentPosition->requiredTime) * physicsObjects[i]->currentPosition->beginTime;
            physicsObjects[i]->yProjectedPosition = (yDifference / physicsObjects[i]->currentPosition->requiredTime) * physicsObjects[i]->currentPosition->beginTime;

            // Referenciando o movimento.
            physicsObjects[i]->xProjectedPosition += physicsObjects[i]->currentPosition->xPosition;
            physicsObjects[i]->yProjectedPosition += physicsObjects[i]->currentPosition->yPosition;

            // Indicador de que o objeto est� se movendo.
            physicsObjects[i]->moving = 1;
        }
    }

    // Colis�o entre objetos
    for(i=0; i<physicsObjectNumber; i++){
        if(physicsObjects[i]->collidable){
            struct physicsObject * object1 = physicsObjects[i];

            for(j=0; j<physicsObjectNumber; j++){
                struct physicsObject * object2 = physicsObjects[j];
                int keepGoing = object2->active;

                // Se o objeto for colid�vel ou uma colis�o com ele causar dano, ent�o checar por colis�es.
                if(keepGoing && (i - j < 0) && (object2->collidable || object2->type->damage)){
                    // Vetores da normal da colis�o e da penetra��o.
                    Vector2 collisionNormal;
                    Vector2 penetration;

                    // Checando por colis�o e obtendo dados sobre a penetra��o e a normal da colis�o, caso haja uma.
                    int collided = collideAABB(object1, object2, &collisionNormal, &penetration);

                    // Se eles colidiram:
                    if(keepGoing && collided){
                        // Se o objeto causar dano:
                        if(i == 0 && object2->type->damage != 0 && keepGoing){
                            characterHealth -= object2->type->damage;
                            keepGoing = 0;

                            if(characterHealth <= 0){
                                // O triste fim de ornitorrinco.
                                hasDied = 1;

                                keepGoing = 0;
                            }else if(object2->type->damage > 0){
                                object1->xVelocity *= -1;
                                object1->yVelocity *= -1;

                                keepGoing = 0;
                            }else if(object2->type->damage < 0){
                                object2->active = 0;

                                keepGoing = 0;
                            }
                        }

                        // Se o objeto for colid�vel e for elig�vel � resolu��o da colis�o:
                        if(keepGoing && object2->collidable){
                            // C�lculo da velocidade relativa entre os objetos.
                            Vector2 relativeVelocity;
                            relativeVelocity.x = (returnXVelocity(object2) - returnXVelocity(object1)) / timeDifference;
                            relativeVelocity.y = (returnYVelocity(object2) - returnYVelocity(object1)) / timeDifference;

                            float normalVelocity = collisionNormal.x * relativeVelocity.x + collisionNormal.y * relativeVelocity.y;

                            if(normalVelocity <= 0){
                                // Calculo da matem�tica por tr�s da resolu��o da colis�o.
                                float e = COEF_REST;

                                float j = -(1 + e) * normalVelocity / (object1->invMass + object2->invMass);

                                penetration.x *= collisionNormal.x / (object1->invMass + object2->invMass);
                                penetration.y *= collisionNormal.y / (object1->invMass + object2->invMass);

                                Vector2 impulse;
                                impulse.x = j * collisionNormal.x;
                                impulse.y = j * collisionNormal.y;

                                object1->xVelocity -= 1 * object1->invMass * impulse.x;
                                object1->yVelocity -= 1 * object1->invMass * impulse.y;

                                object2->xVelocity += 1 * object2->invMass * impulse.x;
                                object2->yVelocity += 1 * object2->invMass * impulse.y;

                                object1->xDisplacement -= penetration.x * object1->invMass;
                                object1->yDisplacement -= penetration.y * object1->invMass;

                                object2->xDisplacement += penetration.x * object2->invMass;
                                object2->yDisplacement += penetration.y * object2->invMass;

                                // Se houve deslocamento em Y, um objeto est� apoiado em outro.
                                if(penetration.y > 0){
                                    object1->standing = 1;

                                    if(object2->isPlatform)
                                        object1->xDisplacement += returnXVelocity(object2);
                                }else{
                                    object2->standing = 1;

                                    if(object1->isPlatform)
                                        object2->xDisplacement += returnXVelocity(object1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for(i=0; i<physicsObjectNumber; i++){
        // Atualizando a posi��o projetada.
        physicsObjects[i]->xProjectedPosition += physicsObjects[i]->xDisplacement;
        physicsObjects[i]->yProjectedPosition += physicsObjects[i]->yDisplacement;

        // Garantindo que nenhuma bizarrice seja apresentada ao jogador.
        displaceBox(inputMap, physicsObjects[i], physicsObjects[i]->xProjectedPosition, physicsObjects[i]->yPosition, 'x');
        displaceBox(inputMap, physicsObjects[i], physicsObjects[i]->xPosition, physicsObjects[i]->yProjectedPosition, 'y');
    }

    // Mudan�a definitiva da posi��o.
    for(i=0; i<physicsObjectNumber; i++){
        // Quando i = 0, o objeto em quest�o � o ornitorrinco. As propriedades dele s�o determinadas de outra forma.
        if(i!=0){
            // Determinando se o objeto est� se movendo, e, se est�, para qual dire��o.
            if(physicsObjects[i]->xProjectedPosition > physicsObjects[i]->xPosition){
                physicsObjects[i]->horizontalDirection = 1;
                physicsObjects[i]->moving = 1;
            }else if(physicsObjects[i]->xProjectedPosition < physicsObjects[i]->xPosition){
                physicsObjects[i]->horizontalDirection = -1;
                physicsObjects[i]->moving = 1;
            }else{
                physicsObjects[i]->moving = 0;
            }
        }

        // Atualizando definitivamente a posi��o.
        physicsObjects[i]->xPosition = physicsObjects[i]->xProjectedPosition;
        physicsObjects[i]->yPosition = physicsObjects[i]->yProjectedPosition;
    }

    return 0;
}

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#include <stdio.h>
#include <stdlib.h>

#include "header.c"

#define N_MENUS 5
#define MMAIN 1
#define MPLAY 11
#define MOPTIONS 12
#define MVOLUME  121
#define MCONTROLS 122
#define MCREDITS 13

typedef struct{

    SDL_Rect *button;
    SDL_Rect *underbutton;
    SDL_Rect *output;
    int *calls;//Qual submenu ou função será chamado pelo menu
    int nButtons;
    int id;//Cada função está associada a um número.
    char *imgfile; //String com o nome do arquivo correspondente ao menu.

}Menu;

//INICIO protótipos de funções
int Fmenu_generico(SDLKey *, Menu *);//Ok

int Finit_audio();//Ok

SDLKey *Finit_menu();//Ok

void Fcria_menu(Menu *, const int, const int, char *, const char);//Ok

void Fapaga_menu(Menu *);//Ok

inline void Fdraw_bar(const short int *, SDL_Rect);//Ok

inline void Fdraw_menu( const Menu *menu );

void Fgerenciador( SDLKey *, Menu * );//OK

inline int Fadjust_volume(const short int, short int *, SDLKey *, const SDL_Rect *);//Ok

inline SDLKey Fset_controls(SDLKey *, SDL_Rect *);//Ok

inline void Fdraw_key(SDL_Rect *, char *);//Ok

void Fshow_controls(SDL_Rect *, SDLKey *);//Ok

void Fclean();//Ok

inline void Fwrite_controls(SDLKey *);//Ok

void Fread_controls(SDLKey *);//Ok

//FIM protópipo de funções


int Finit_audio(){ //Função de inicializar
     //Inicializar SDL Mixer
     if ( Mix_Init( MIX_INIT_OGG ) == -1 ){
        printf("Erro ao inicializar Mixer");
        return 0;
     }
     if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ){
        printf("Erro ao definir propriedades do mixer");
        return 0;
     }
	 //Se tudo inicializou bem
    return 1;
}

void Fclean(){
    //Sair do TTF
    TTF_Quit();
    //Sair do Mixer
    Mix_CloseAudio();
    Mix_Quit();
    //Sai do SDL
    SDL_Quit();
}

//INICIO Função que inicializa o menu.
SDLKey *Finit_menu(){
    if ( Finit_audio() == 0 ){
        printf("Erro ao inicializar Som");
    }
    FILE *checkConfig;
	//INICIO Volume padrão
	checkConfig = fopen("save.dat", "r");
	if ( checkConfig == NULL ){
        vol[CHANNELMENU - 1] = 64;
        vol[CHANNELEFFECT - 1] = 64;
        vol[CHANNELMUSIC - 1] = 32;
        lastUnlockedLevel = 1;
        writeSave();
	}
	else loadSave();
	//FIM Volume padrão
    fclose(checkConfig);

    checkConfig = NULL;
    checkConfig = fopen("config.dat", "r");
    //INICIO carregar controles padrão.
    SDLKey *keys;
    keys = (SDLKey *) malloc( N_KEYS * sizeof(SDLKey));
    if ( checkConfig == NULL ){//Se não há arquivo de configuração.
        keys[0] = SDLK_LEFT;
        keys[1] = SDLK_RIGHT;
        keys[2] = SDLK_UP;
        keys[3] = SDLK_DOWN;
        keys[4] = SDLK_LSHIFT;
        keys[5] = SDLK_RETURN;
        Fwrite_controls(keys);
    }
    else{//Se há, carregar configuração de teclas.
        Fread_controls(keys);
        fclose(checkConfig);
    }
    //FIM carregar controles padrão

    //INICIO definir volume
    Mix_Volume(CHANNELEFFECT, vol[CHANNELEFFECT - 1]);
    Mix_Volume(CHANNELMUSIC, vol[CHANNELMENU - 1]);
    Mix_VolumeMusic(vol[CHANNELMUSIC - 1]);
    //FIM definir volume

    //INICIO carregar sons.
    Mix_Music *s_music;
    s_music = Mix_LoadMUS( "menu.ogg" );

    Mix_PlayMusic( s_music, -1 );
    //FIM carregar sons.

    //INICIO criar menus
    Menu menu[N_MENUS];
    /*  menu[0] - Menu principal, menu[1] - menu de opções, menu[2] - menu de ajuste de volume,
        menu[3] - Menu de controles, menu[4] - menu de créditos */

    //Menu main
    Fcria_menu( &menu[0], 4, MMAIN, "Menuproj.png", 'n' );
    //Chamadas
    menu[0].calls[0] = MPLAY; menu[0].calls[1] = MOPTIONS;  menu[0].calls[2] = MCREDITS;  menu[0].calls[3] = 2;
    //Jogar
    menu[0].button[0].x = 115;                          menu[0].button[0].y = 285;
    menu[0].button[0].w = 54;                           menu[0].button[0].h = 16;

    menu[0].underbutton[0].x = menu[0].button[0].x;     menu[0].underbutton[0].y = menu[0].button[0].y + menu[0].button[0].h;
    menu[0].underbutton[0].w = menu[0].button[0].w;     menu[0].underbutton[0].h = 5;
    //Opções
    menu[0].button[1].x = 115;                          menu[0].button[1].y = 323;
    menu[0].button[1].w = 63;                           menu[0].button[1].h = 16;

    menu[0].underbutton[1].x = menu[0].button[1].x;     menu[0].underbutton[1].y = menu[0].button[1].y + menu[0].button[1].h;
    menu[0].underbutton[1].w = menu[0].button[1].w;     menu[0].underbutton[1].h = 5;
    //Créditos
    menu[0].button[2].x = 115;                          menu[0].button[2].y = 360;
    menu[0].button[2].w = 80;                           menu[0].button[2].h = 15;

    menu[0].underbutton[2].x = menu[0].button[2].x;     menu[0].underbutton[2].y = menu[0].button[2].y + menu[0].button[2].h;
    menu[0].underbutton[2].w = menu[0].button[2].w;     menu[0].underbutton[2].h = 5;
    //Sair
    menu[0].button[3].x = 115;                          menu[0].button[3].y = 398;
    menu[0].button[3].w = 42;                           menu[0].button[3].h = 15;

    menu[0].underbutton[3].x = menu[0].button[3].x;     menu[0].underbutton[3].y = menu[0].button[3].y + menu[0].button[3].h;
    menu[0].underbutton[3].w = menu[0].button[3].w;     menu[0].underbutton[3].h = 5;

    //menu de opções
    Fcria_menu( &menu[1], 3, MOPTIONS, "Menuprojopcoes.png", 'n' );
    //Chamadas
    menu[1].calls[0] = MVOLUME; menu[1].calls[1] = MCONTROLS;  menu[1].calls[2] = MMAIN;
    //Volume
    menu[1].button[0].x = 125;                          menu[1].button[0].y = 136;
    menu[1].button[0].w = 66;                           menu[1].button[0].h = 19;

    menu[1].underbutton[0].x = menu[1].button[0].x;     menu[1].underbutton[0].y = menu[1].button[0].y + menu[1].button[0].h;
    menu[1].underbutton[0].w = menu[1].button[0].w;     menu[1].underbutton[0].h = 5;
    //Controles
    menu[1].button[1].x = 125;                          menu[1].button[1].y = 173;
    menu[1].button[1].w = 95;                           menu[1].button[1].h = 19;

    menu[1].underbutton[1].x = menu[1].button[1].x;     menu[1].underbutton[1].y = menu[1].button[1].y + menu[1].button[1].h;
    menu[1].underbutton[1].w = menu[1].button[1].w;     menu[1].underbutton[1].h = 5;
    //Voltar
    menu[1].button[2].x = 125;                          menu[1].button[2].y = 212;
    menu[1].button[2].w = 66;                           menu[1].button[2].h = 19;

    menu[1].underbutton[2].x = menu[1].button[2].x;     menu[1].underbutton[2].y = menu[1].button[2].y + menu[1].button[2].h;
    menu[1].underbutton[2].w = menu[1].button[2].w;     menu[1].underbutton[2].h = 5;

    //menu de ajuste do volume
    Fcria_menu( &menu[2], 4, MVOLUME, "Menuprojvolume.png", 'y' );
    //Chamadas
    menu[2].calls[0] = 1211; menu[2].calls[1] = menu[2].calls[0] + 1;  menu[2].calls[2] = menu[2].calls[0] + 2;
    menu[2].calls[3] = MOPTIONS;
    //Volume do menu
    menu[2].button[0].x = 126;                           menu[2].button[0].y = 155;
    menu[2].button[0].w = 90;                            menu[2].button[0].h = 21;

    menu[2].underbutton[0].x = menu[2].button[0].x;   menu[2].underbutton[0].y = menu[2].button[0].y + menu[2].button[0].h;
    menu[2].underbutton[0].w = menu[2].button[0].w;   menu[2].underbutton[0].h = 5;

    menu[2].output[0].x = 234;                           menu[2].output[0].y = menu[2].button[0].y;
    menu[2].output[0].w = 200;                           menu[2].output[0].h = menu[2].button[0].h;
    //Volume de Efeitos
    menu[2].button[1].x = 126;                           menu[2].button[1].y = 193;
    menu[2].button[1].w = 118;                           menu[2].button[1].h = 21;

    menu[2].underbutton[1].x = menu[2].button[1].x;   menu[2].underbutton[1].y = menu[2].button[1].y + menu[2].button[1].h;
    menu[2].underbutton[1].w = menu[2].button[1].w;   menu[2].underbutton[1].h = 5;

    menu[2].output[1].x = 261;                           menu[2].output[1].y = menu[2].button[1].y;
    menu[2].output[1].w = 200;                           menu[2].output[1].h = menu[2].button[1].h;
    //Volume da Música
    menu[2].button[2].x = 126;                           menu[2].button[2].y = 231;
    menu[2].button[2].w = 110;                           menu[2].button[2].h = 21;

    menu[2].underbutton[2].x = menu[2].button[2].x;   menu[2].underbutton[2].y = menu[2].button[2].y + menu[2].button[2].h;
    menu[2].underbutton[2].w = menu[2].button[2].w;   menu[2].underbutton[2].h = 5;

    menu[2].output[2].x = 251;                           menu[2].output[2].y = menu[2].button[2].y;
    menu[2].output[2].w = 200;                           menu[2].output[2].h = menu[2].button[2].h;
    //Voltar
    menu[2].button[3].x = 126;                           menu[2].button[3].y = 268;
    menu[2].button[3].w = 68;                            menu[2].button[3].h = 21;

    menu[2].underbutton[3].x = menu[2].button[3].x;   menu[2].underbutton[3].y = menu[2].button[3].y + menu[2].button[3].h;
    menu[2].underbutton[3].w = menu[2].button[3].w;   menu[2].underbutton[3].h = 5;

    //menu de controles
    Fcria_menu( &menu[3], 7, MCONTROLS, "Menuprojcontroles.png", 'y' );
    //Chamadas
    menu[3].calls[0] = 1221; menu[3].calls[1] = menu[3].calls[0] + 1;  menu[3].calls[2] = menu[3].calls[0] + 2;
    menu[3].calls[3] = menu[3].calls[0] + 3;  menu[3].calls[4] = menu[3].calls[0] + 4;
    menu[3].calls[5] = menu[3].calls[0] + 5;  menu[3].calls[6] = MOPTIONS;
    //Esquerda
    menu[3].button[0].x = 129;	                            menu[3].button[0].y = 160;
    menu[3].button[0].w = 81;                              menu[3].button[0].h = 17;

	menu[3].underbutton[0].x = menu[3].button[0].x;   menu[3].underbutton[0].y = menu[3].button[0].y + menu[3].button[0].h;
	menu[3].underbutton[0].w = menu[3].button[0].w;   menu[3].underbutton[0].h = 5;

    menu[3].output[0].x = 235;                             menu[3].output[0].y = menu[3].button[0].y;
    menu[3].output[0].w = 200;                             menu[3].output[0].h = menu[3].button[0].h;
    //Direita
    menu[3].button[1].x = 129;                             menu[3].button[1].y = 197;
    menu[3].button[1].w = 68;                              menu[3].button[1].h = 15;

    menu[3].underbutton[1].x = menu[3].button[1].x;   menu[3].underbutton[1].y = menu[3].button[1].y + menu[3].button[1].h;
    menu[3].underbutton[1].w = menu[3].button[1].w;   menu[3].underbutton[1].h = 5;

    menu[3].output[1].x = 225;                             menu[3].output[1].y = menu[3].button[1].y;
    menu[3].output[1].w = 200;                             menu[3].output[1].h = menu[3].button[1].h;
    //Pular
    menu[3].button[2].x = 129;                             menu[3].button[2].y = 236;
    menu[3].button[2].w = 52;                              menu[3].button[2].h = 15;

    menu[3].underbutton[2].x = menu[3].button[2].x;   menu[3].underbutton[2].y = menu[3].button[2].y + menu[3].button[2].h;
    menu[3].underbutton[2].w = menu[3].button[2].w;   menu[3].underbutton[2].h = 5;

    menu[3].output[2].x = 210;                             menu[3].output[2].y = menu[3].button[2].y;
    menu[3].output[2].w = 200;                             menu[3].output[2].h = menu[3].button[2].h;
    //Baixo
	menu[3].button[3].x = 128;                             menu[3].button[3].y = 274;
	menu[3].button[3].w = 51;                              menu[3].button[3].h = 11;

    menu[3].underbutton[3].x = menu[3].button[3].x;        menu[3].underbutton[3].y = menu[3].button[3].y + menu[3].button[3].h;
    menu[3].underbutton[3].w = menu[3].button[3].w;        menu[3].underbutton[3].h = 5;

    menu[3].output[3].x = 205;                             menu[3].output[3].y = menu[3].button[3].y;
    menu[3].output[3].w = 200;                             menu[3].output[3].h = menu[3].button[3].h;
    //Correr
    menu[3].button[4].x = 129;                             menu[3].button[4].y = 312;
    menu[3].button[4].w = 62;                              menu[3].button[4].h = 12;

    menu[3].underbutton[4].x = menu[3].button[4].x;        menu[3].underbutton[4].y = menu[3].button[4].y + menu[3].button[4].h;
    menu[3].underbutton[4].w = menu[3].button[4].w;        menu[3].underbutton[4].h = 5;

    menu[3].output[4].x = 220;                             menu[3].output[4].y = menu[3].button[4].y;
    menu[3].output[4].w = 200;                             menu[3].output[4].h = menu[3].button[4].h;
    //Selecionar
	menu[3].button[5].x = 128;                             menu[3].button[5].y = 342;
	menu[3].button[5].w = 102;                             menu[3].button[5].h = 12;

    menu[3].underbutton[5].x = menu[3].button[5].x;        menu[3].underbutton[5].y = menu[3].button[5].y + menu[3].button[5].h;
    menu[3].underbutton[5].w = menu[3].button[5].w;        menu[3].underbutton[5].h = 5;

    menu[3].output[5].x = 260;                             menu[3].output[5].y = menu[3].button[5].y;
    menu[3].output[5].w = 200;                             menu[3].output[5].h = menu[3].button[5].h;
    //Sair
	menu[3].button[6].x = 128;                             menu[3].button[6].y = 372;
	menu[3].button[6].w = 62;                              menu[3].button[6].h = 15;

    menu[3].underbutton[6].x = menu[3].button[6].x;   menu[3].underbutton[6].y = menu[3].button[6].y + menu[3].button[6].h;
    menu[3].underbutton[6].w = menu[3].button[6].w;   menu[3].underbutton[6].h = 5;

    //menu de créditos
    Fcria_menu( &menu[4], 1, 3, "Menucred.png", 'n' );
    //Chamadas
    *(menu[4].calls) = MMAIN;
    //Sair
    menu[4].button->x = 0;                             menu[4].button->y = 0;
    menu[4].button->w = xResolution;                   menu[4].button->h = yResolution;

    menu[4].underbutton->x = menu[4].button->x;     menu[4].underbutton->y = menu[4].button->y + menu[4].button->h;
    menu[4].underbutton->w = menu[4].button->w;     menu[4].underbutton->h = 5;

    //FIM criar menus

    Fgerenciador( keys, menu );

    //Liberar memória alocada.
    int counter;
    for ( counter = 0; counter <= N_MENUS - 1; counter++ )
        Fapaga_menu( &menu[counter] );

    Mix_FadeOutMusic( 250 );

    Mix_FreeMusic( s_music );
    return keys;
}
//FIM Função que inicializa o menu.

//INICIO Função que salva as configurações definidas pelo usuário.
void Fwrite_controls(SDLKey *keys){
    FILE *config;
    config = fopen("config.dat", "w");//Arquivo no qual é salvo as configurações do jogador.
    register int counter;

    for ( counter = 0; counter <= N_KEYS - 1; counter++)
        fprintf(config, "%d ", (int)keys[counter]);

    fclose(config);
}
//FIM Função que salva as configurações definidas pelo usuário.

/*INICIO Função que carrega as tecla salvas.
Para a função seguinte, cada tecla à qual o jogo tem suporte está associada a um inteiro, que é salvo no arquivo config.dat
a função irá "converter" de inteiro para tecla. */
void Fread_controls(SDLKey *keys){
    FILE *config;
    config = fopen("config.dat", "r");
    register int counter;
    for ( counter = 0; counter <= N_KEYS - 1; counter++)
        fscanf(config, "%d", (int*)&keys[counter]);
}
//FIM Função que carrega as teclas salvas.

void Fcria_menu(Menu *menu, const int buttonsNum, const int idNum, char *imgFileName, const char output){

    menu->button = (SDL_Rect *) calloc( buttonsNum, sizeof(SDL_Rect) );
    menu->underbutton = (SDL_Rect *) calloc( buttonsNum, sizeof(SDL_Rect) );

    if ( output == 'y' )
        menu->output = (SDL_Rect *) calloc( (buttonsNum - 1), sizeof(SDL_Rect) );
    else
        menu->output = NULL;

    menu->calls = (int *) malloc( buttonsNum * sizeof(int) );
    menu->nButtons = buttonsNum;
    menu->id = idNum;
    menu->imgfile = imgFileName;

}

void Fapaga_menu(Menu *menu){

    free(menu->button);
    free(menu->underbutton);
    if ( menu->output != NULL )
        free(menu->output);
    free(menu->calls);
    free(menu->imgfile);

}

int Fmenu_generico(SDLKey *keys, Menu *menu){

    Mix_Chunk *s_selection, *s_click;

    s_selection = Mix_LoadWAV("selecionado.ogg");
    s_click = Mix_LoadWAV("clique.ogg");

    int counter;
    int x, y; //Recebem a posição do mouse;
    short int leaveMenu = 0;
    int next;
    SDL_Event menuLastEvent;

    int *underlined, controlkeyjoy; //Controlam sublinhado e qual botão foi selecionado pelo controle.
    underlined = (int*) calloc( menu->nButtons, sizeof(int) );
    controlkeyjoy = 0;

    while ( QUIT_PROGRAM == 0 && leaveMenu == 0 ){
        while ( SDL_PollEvent( &menuLastEvent ) ){
            if ( menuLastEvent.type == SDL_MOUSEMOTION ){
                SDL_GetMouseState(&x, &y);
                for( counter = 0; counter <= (menu->nButtons - 1); counter++ ){
                    //Se estiver sobre um botão
                    if ( (x >= menu->button[counter].x) && (x <= menu->button[counter].x + menu->button[counter].w)
                        && (y >= menu->button[counter].y) && (y <= menu->button[counter].y + menu->button[counter].h) )
                    {
                        if ( underlined[counter] == 0 ){//Se não estava sublinhado
                            Mix_PlayChannel( CHANNELMENU, s_selection, 0 );
                            underlined[counter] = 1;
                        }
                        SDL_FillRect( screen, &(menu->underbutton[counter]),
                                     SDL_MapRGB( screen->format, 50, 50, 0 ) );
                        SDL_Flip( screen );
                    }
                    else if ( underlined[counter] == 1 ){//Senão, remove o sublinhado
                        SDL_FillRect( screen, &(menu->underbutton[counter]),
                                    SDL_MapRGB( screen->format, 255, 255, 255 ) );
                        SDL_Flip( screen );
                        underlined[counter] = 0;
                    }
                }
            }
            else if ( menuLastEvent.type == SDL_MOUSEBUTTONDOWN ){//Qualquer botão do mouse pressionado.
                SDL_GetMouseState(&x, &y);
                for( counter = 0; counter <= (menu->nButtons - 1); counter++ ){
                    if ( (x >= menu->button[counter].x) && (x <= menu->button[counter].x + menu->button[counter].w)
                        && (y >= menu->button[counter].y) && (y <= menu->button[counter].y + menu->button[counter].h) )
                    {
                        Mix_PlayChannel( CHANNELMENU, s_click, 0 );
                        leaveMenu = 1;
                        next = *(menu->calls + counter);
                    }
                }
            }
            else if ( menuLastEvent.type == SDL_KEYDOWN ){
                //Remove o sublinhado anterior.
                //Caso "Jogar" esteja sublinhado.
                SDL_FillRect( screen, &(menu->underbutton[controlkeyjoy]), SDL_MapRGB( screen->format, 255, 255, 255 ) );
                SDL_Flip( screen );
                if ( menuLastEvent.key.keysym.sym == keys[5] ){//Se uma opção for escolhida.
                    Mix_PlayChannel( CHANNELMENU, s_click, 0 );
                    leaveMenu = 1;
                    next = *(menu->calls + controlkeyjoy);
                }
                else if ( menuLastEvent.key.keysym.sym == keys[2] ){//Se a tecla para cima foi apertada
                    if ( controlkeyjoy == 0 )   controlkeyjoy = menu->nButtons - 1;
                    else    --controlkeyjoy;
                }
                else if ( menuLastEvent.key.keysym.sym == keys[3] ){//Se a tecla para baixo foi apertada
                    if ( controlkeyjoy == ( menu->nButtons - 1) )   controlkeyjoy = 0;
                    else    ++controlkeyjoy;
                }
                Mix_PlayChannel( CHANNELMENU, s_selection, 0 );
                //Sublinha a opção selecionada.
                SDL_FillRect( screen, &(menu->underbutton[controlkeyjoy]), SDL_MapRGB( screen->format, 50, 50, 0 ) );
                SDL_Flip( screen );
            }
            if ( menuLastEvent.type == SDL_QUIT )   QUIT_PROGRAM = 1;
        }
    }

    Mix_FreeChunk( s_selection );
    Mix_FreeChunk( s_click );
    free ( underlined );

    if ( 1 == QUIT_PROGRAM )
        next = 2;

    return next;
}

void Fgerenciador( SDLKey *keys, Menu *menu ){

    int counter, control = MMAIN, play = 0;

    while ( QUIT_PROGRAM == 0 && play != 1 ){
        switch ( control ){
            case MMAIN:
                Fdraw_menu( &menu[0] );
                control = Fmenu_generico( keys, &menu[0] );
                break;
            case MPLAY:
                play = 1;
                break;
            case MOPTIONS:
                Fdraw_menu( &menu[1] );
                control = Fmenu_generico( keys, &menu[1] );
                break;
            case MVOLUME:
                Fdraw_menu( &menu[2] );
                for( counter = 0; counter < N_CHANNELS; counter++ )
                    Fdraw_bar( &vol[counter], menu[2].output[counter] );
                control = Fmenu_generico( keys, &menu[2] );
                break;
            //Ajuste de volume
            case 1211:
                control = Fadjust_volume( CHANNELMENU, &vol[0], keys, &menu[2].output[0] );
                break;
            case 1212:
                control = Fadjust_volume( CHANNELEFFECT, &vol[1], keys, &menu[2].output[1] );
                break;
            case 1213:
                control = Fadjust_volume( CHANNELMUSIC, &vol[2], keys, &menu[2].output[2] );
                break;
            case MCONTROLS:
                Fdraw_menu( &menu[3] );
                for ( counter = 0; counter <= N_KEYS - 1; counter++ )
                    Fshow_controls( &menu[3].output[counter], &keys[counter] );
                control = Fmenu_generico( keys, &menu[3] );
                break;
            //Ajuste de controles
            case 1221:
                keys[0] = Fset_controls( keys, &menu[3].output[0] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case 1222:
                keys[1] = Fset_controls( keys, &menu[3].output[1] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case 1223:
                keys[2] = Fset_controls( keys, &menu[3].output[2] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case 1224:
                keys[3] = Fset_controls( keys, &menu[3].output[3] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case 1225:
                keys[4] = Fset_controls( keys, &menu[3].output[4] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case 1226:
                keys[5] = Fset_controls( keys, &menu[3].output[5] );
                Fwrite_controls( keys );
                control = 122;
                break;
            case MCREDITS:
                Fdraw_menu( &menu[4] );
                control = Fmenu_generico( keys, &menu[4] );
                break;
            case 2:
                QUIT_PROGRAM = 1;
                break;
        }
    }

}

void Fshow_controls(SDL_Rect *textDestRect, SDLKey *control){
    switch ( *control ){
        case ( SDLK_BACKSPACE ):{
            Fdraw_key(textDestRect, "BACKSPACE");
            break;
        }
        case ( SDLK_TAB ):{
            Fdraw_key(textDestRect, "TAB");
            break;
        }
        case ( SDLK_CLEAR ):{
            Fdraw_key(textDestRect, "CLEAR");
            break;
        }
       case ( SDLK_RETURN ):{
            Fdraw_key(textDestRect, "ENTER");
            break;
        }
        case ( SDLK_PAUSE ):{
            Fdraw_key(textDestRect, "PAUSE");
            break;
        }
        case ( SDLK_ESCAPE ):{
            Fdraw_key(textDestRect, "ESC");
            break;
        }
        case ( SDLK_SPACE ):{
            Fdraw_key(textDestRect, "ESPAÇO");
            break;
        }
        case ( SDLK_EXCLAIM ):{
            Fdraw_key(textDestRect, "!");
            break;
        }
        case ( SDLK_QUOTEDBL ):{
            Fdraw_key(textDestRect, "''");
            break;
        }
        case ( SDLK_HASH ):{
            Fdraw_key(textDestRect, "#");
            break;
        }
        case ( SDLK_DOLLAR ):{
            Fdraw_key(textDestRect, "$");
            break;
        }
        case ( SDLK_AMPERSAND ):{
            Fdraw_key(textDestRect, "&");
            break;
        }
        case ( SDLK_QUOTE ):{
            Fdraw_key(textDestRect, "'");
            break;
        }
        case ( SDLK_LEFTPAREN ):{
            Fdraw_key(textDestRect, "(");
            break;
        }
        case ( SDLK_RIGHTPAREN ):{
            Fdraw_key(textDestRect, ")");
            break;
        }
        case ( SDLK_ASTERISK ):{
            Fdraw_key(textDestRect, "*");
            break;
        }
        case ( SDLK_PLUS ):{
            Fdraw_key(textDestRect, "+");
            break;
        }
        case ( SDLK_COMMA ):{
            Fdraw_key(textDestRect, ",");
            break;
        }
        case ( SDLK_MINUS ):{
            Fdraw_key(textDestRect, "-");
            break;
        }
        case ( SDLK_PERIOD ):{
            Fdraw_key(textDestRect, ".");
            break;
        }
        case ( SDLK_SLASH ):{
            Fdraw_key(textDestRect, "/");
            break;
        }
        case ( SDLK_0 ):{
            Fdraw_key(textDestRect, "0");
            break;
        }
        case ( SDLK_1 ):{
            Fdraw_key(textDestRect, "1");
            break;
        }
        case ( SDLK_2 ):{
            Fdraw_key(textDestRect, "2");
            break;
        }
        case ( SDLK_3 ):{
            Fdraw_key(textDestRect, "3");
            break;
        }
        case ( SDLK_4 ):{
            Fdraw_key(textDestRect, "4");
            break;
        }
        case ( SDLK_5 ):{
            Fdraw_key(textDestRect, "5");
            break;
        }
        case ( SDLK_6 ):{
            Fdraw_key(textDestRect, "6");
            break;
        }
        case ( SDLK_7 ):{
            Fdraw_key(textDestRect, "7");
            break;
        }
        case ( SDLK_8 ):{
            Fdraw_key(textDestRect, "8");
            break;
        }
        case ( SDLK_9 ):{
            Fdraw_key(textDestRect, "9");
            break;
        }
        case ( SDLK_COLON ):{
            Fdraw_key(textDestRect, ":");
            break;
        }
        case ( SDLK_SEMICOLON ):{
            Fdraw_key(textDestRect, ";");
            break;
        }
        case ( SDLK_LESS ):{
            Fdraw_key(textDestRect, "<");
            break;
        }
        case ( SDLK_EQUALS ):{
            Fdraw_key(textDestRect, "=");
            break;
        }
        case ( SDLK_GREATER ):{
            Fdraw_key(textDestRect, ">");
            break;
        }
        case ( SDLK_QUESTION ):{
            Fdraw_key(textDestRect, "?");
            break;
        }
        case ( SDLK_AT ):{
            Fdraw_key(textDestRect, "@");
            break;
        }
        case ( SDLK_LEFTBRACKET ):{
            Fdraw_key(textDestRect, "[");
            break;
        }
        case ( SDLK_BACKSLASH ):{
            Fdraw_key(textDestRect, "\\");
            break;
        }
        case ( SDLK_RIGHTBRACKET ):{
            Fdraw_key(textDestRect, "]");
            break;
        }
        case ( SDLK_CARET ):{
            Fdraw_key(textDestRect, "^");
            break;
        }
        case ( SDLK_UNDERSCORE ):{
            Fdraw_key(textDestRect, "_");
            break;
        }
        case ( SDLK_BACKQUOTE ):{
            Fdraw_key(textDestRect, "`");
            break;
        }
        case ( SDLK_a ):{
            Fdraw_key(textDestRect, "a");
            break;
        }
        case ( SDLK_b ):{
            Fdraw_key(textDestRect, "b");
            break;
        }
        case ( SDLK_c ):{
            Fdraw_key(textDestRect, "c");
            break;
        }
        case ( SDLK_d ):{
            Fdraw_key(textDestRect, "d");
            break;
        }
        case ( SDLK_e ):{
            Fdraw_key(textDestRect, "e");
            break;
        }
        case ( SDLK_f ):{
            Fdraw_key(textDestRect, "f");
            break;
        }
        case ( SDLK_g ):{
            Fdraw_key(textDestRect, "g");
            break;
        }
        case ( SDLK_h ):{
            Fdraw_key(textDestRect, "h");
            break;
        }
        case ( SDLK_i ):{
            Fdraw_key(textDestRect, "i");
            break;
        }
        case ( SDLK_j ):{
            Fdraw_key(textDestRect, "j");
            break;
        }
        case ( SDLK_k ):{
            Fdraw_key(textDestRect, "k");
            break;
        }
        case ( SDLK_l ):{
            Fdraw_key(textDestRect, "l");
            break;
        }
        case ( SDLK_m ):{
            Fdraw_key(textDestRect, "m");
            break;
        }
        case ( SDLK_n ):{
            Fdraw_key(textDestRect, "n");
            break;
        }
        case ( SDLK_o ):{
            Fdraw_key(textDestRect, "o");
            break;
        }
        case ( SDLK_p ):{
            Fdraw_key(textDestRect, "p");
            break;
        }
        case ( SDLK_q ):{
            Fdraw_key(textDestRect, "q");
            break;
        }
        case ( SDLK_r ):{
            Fdraw_key(textDestRect, "r");
            break;
        }
        case ( SDLK_s ):{
            Fdraw_key(textDestRect, "s");
            break;
        }
        case ( SDLK_t ):{
            Fdraw_key(textDestRect, "t");
            break;
        }
        case ( SDLK_u ):{
            Fdraw_key(textDestRect, "u");
            break;
        }
        case ( SDLK_v ):{
            Fdraw_key(textDestRect, "v");
            break;
        }
        case ( SDLK_w ):{
            Fdraw_key(textDestRect, "w");
            break;
        }
        case ( SDLK_x ):{
            Fdraw_key(textDestRect, "x");
            break;
        }
        case ( SDLK_y ):{
            Fdraw_key(textDestRect, "y");
            break;
        }
        case ( SDLK_z ):{
            Fdraw_key(textDestRect, "z");
            break;
        }
        case ( SDLK_DELETE ):{
            Fdraw_key(textDestRect, "DEL");
            break;
        }
        case ( SDLK_KP0 ):{
            Fdraw_key(textDestRect, "KEYPAD 0");
            break;
        }
        case ( SDLK_KP1 ):{
            Fdraw_key(textDestRect, "KEYPAD 1");
            break;
        }
        case ( SDLK_KP2 ):{
            Fdraw_key(textDestRect, "KEYPAD 2");
            break;
        }
        case ( SDLK_KP3 ):{
            Fdraw_key(textDestRect, "KEYPAD 3");
            break;
        }
        case ( SDLK_KP4 ):{
            Fdraw_key(textDestRect, "KEYPAD 4");
            break;
        }
        case ( SDLK_KP5 ):{
            Fdraw_key(textDestRect, "KEYPAD 5");
            break;
        }
        case ( SDLK_KP6 ):{
            Fdraw_key(textDestRect, "KEYPAD 6");
            break;
        }
        case ( SDLK_KP7 ):{
            Fdraw_key(textDestRect, "KEYPAD 7");
            break;
        }
        case ( SDLK_KP8 ):{
            Fdraw_key(textDestRect, "KEYPAD 8");
            break;
        }
        case ( SDLK_KP9 ):{
            Fdraw_key(textDestRect, "KEYPAD 9");
            break;
        }
        case ( SDLK_KP_PERIOD ):{
            Fdraw_key(textDestRect, "KEYPAD .");
            break;
        }
        case ( SDLK_KP_DIVIDE ):{
            Fdraw_key(textDestRect, "KEYPAD /");
            break;
        }
        case ( SDLK_KP_MULTIPLY ):{
            Fdraw_key(textDestRect, "KEYPAD *");
            break;
        }
        case ( SDLK_KP_MINUS ):{
            Fdraw_key(textDestRect, "KEYPAD -");
            break;
        }
        case ( SDLK_KP_PLUS ):{
            Fdraw_key(textDestRect, "KEYPAD +");
            break;
        }
        case ( SDLK_KP_ENTER ):{
            Fdraw_key(textDestRect, "KEYPAD ENTER");
            break;
        }
        case ( SDLK_KP_EQUALS ):{
            Fdraw_key(textDestRect, "KEYPAD =");
            break;
        }
        case ( SDLK_UP ):{
            Fdraw_key(textDestRect, "SETA ACIMA");
            break;
        }
        case ( SDLK_DOWN ):{
            Fdraw_key(textDestRect, "SETA ABAIXO");
            break;
        }
        case ( SDLK_RIGHT ):{
            Fdraw_key(textDestRect, "SETA DIREITA");
            break;
        }
        case ( SDLK_LEFT ):{
            Fdraw_key(textDestRect, "SETA ESQUERDA");
            break;
        }
        case ( SDLK_INSERT ):{
            Fdraw_key(textDestRect, "INSERT");
            break;
        }
        case ( SDLK_HOME ):{
            Fdraw_key(textDestRect, "HOME");
            break;
        }
        case ( SDLK_END ):{
            Fdraw_key(textDestRect, "END");
            break;
        }
        case ( SDLK_PAGEUP ):{
            Fdraw_key(textDestRect, "PAGEUP");
            break;
        }
        case ( SDLK_PAGEDOWN ):{
            Fdraw_key(textDestRect, "PAGEDOWN");
            break;
        }
        case ( SDLK_F1 ):{
            Fdraw_key(textDestRect, "F1");
            break;
        }
        case ( SDLK_F2 ):{
            Fdraw_key(textDestRect, "F2");
            break;
        }
        case ( SDLK_F3 ):{
            Fdraw_key(textDestRect, "F3");
            break;
        }
        case ( SDLK_F4 ):{
            Fdraw_key(textDestRect, "F4");
            break;
        }
        case ( SDLK_F5 ):{
            Fdraw_key(textDestRect, "F5");
            break;
        }
        case ( SDLK_F6 ):{
            Fdraw_key(textDestRect, "F6");
            break;
        }
        case ( SDLK_F7 ):{
            Fdraw_key(textDestRect, "F7");
            break;
        }
        case ( SDLK_F8 ):{
            Fdraw_key(textDestRect, "F8");
            break;
        }
        case ( SDLK_F9 ):{
            Fdraw_key(textDestRect, "F9");
            break;
        }
        case ( SDLK_F10 ):{
            Fdraw_key(textDestRect, "F10");
            break;
        }
        case ( SDLK_F11 ):{
            Fdraw_key(textDestRect, "F11");
            break;
        }
        case ( SDLK_F12 ):{
            Fdraw_key(textDestRect, "F12");
            break;
        }
        case ( SDLK_F13 ):{
            Fdraw_key(textDestRect, "F13");
            break;
        }
        case ( SDLK_F14 ):{
            Fdraw_key(textDestRect, "F14");
            break;
        }
        case ( SDLK_F15 ):{
            Fdraw_key(textDestRect, "F15");
            break;
        }
        case ( SDLK_NUMLOCK ):{
            Fdraw_key(textDestRect, "NUMLOCK");
            break;
        }
        case ( SDLK_CAPSLOCK ):{
            Fdraw_key(textDestRect, "CAPSLOCK");
            break;
        }
        case ( SDLK_SCROLLOCK ):{
            Fdraw_key(textDestRect, "SCROLLOCK");
            break;
        }
        case ( SDLK_RSHIFT ):{
            Fdraw_key(textDestRect, "SHIFT DIR");
            break;
        }
        case ( SDLK_LSHIFT ):{
            Fdraw_key(textDestRect, "SHIFT ESQ");
            break;
        }
        case ( SDLK_RCTRL ):{
            Fdraw_key(textDestRect, "CTRL DIR");
            break;
        }
        case ( SDLK_LCTRL ):{
            Fdraw_key(textDestRect, "CTRL ESQ");
            break;
        }
        case ( SDLK_RALT ):{
            Fdraw_key(textDestRect, "ALT DIR");
            break;
        }
        case ( SDLK_LALT ):{
            Fdraw_key(textDestRect, "ALT ESQ");
            break;
        }
        case ( SDLK_RMETA ):{
            Fdraw_key(textDestRect, "META DIR");
            break;
        }
        case ( SDLK_LMETA ):{
            Fdraw_key(textDestRect, "META ESQ");
            break;
        }
        case ( SDLK_LSUPER ):{
            Fdraw_key(textDestRect, "WIN KEY ESQ");
            break;
        }
        case ( SDLK_RSUPER ):{
            Fdraw_key(textDestRect, "WIN KEY DIR");
            break;
        }
        case ( SDLK_MODE ):{
            Fdraw_key(textDestRect, "MODE SHIFT");
            break;
        }
        case ( SDLK_HELP ):{
            Fdraw_key(textDestRect, "HELP");
            break;
        }
        case ( SDLK_PRINT ):{
            Fdraw_key(textDestRect, "PRINTSCREEN");
            break;
        }
        case ( SDLK_SYSREQ ):{
            Fdraw_key(textDestRect, "SYSREQ");
            break;
        }
        case ( SDLK_BREAK ):{
            Fdraw_key(textDestRect, "BREAK");
            break;
        }
        case ( SDLK_MENU ):{
            Fdraw_key(textDestRect, "MENU");
            break;
        }
        case ( SDLK_POWER ):{
            Fdraw_key(textDestRect, "POWER");
            break;
        }
        case ( SDLK_EURO ):{
            Fdraw_key(textDestRect, "EURO");
            break;
        }
        default:
            Fdraw_key(textDestRect, "ERRO!!!");
            break;
        }
}

inline void Fdraw_bar( const short int *volume, SDL_Rect aux ){

    register int contador;
    int nBarsVol;
    SDL_Surface *volBars;
    TTF_Font *gamefont;
    SDL_Color colorOrange = { 255, 100, 0 };

    gamefont = TTF_OpenFont( "gamefont.ttf", 12 );
    volBars = TTF_RenderText_Solid( gamefont, "-", colorOrange );

    //Apaga a barra anterior
    SDL_FillRect( screen, &aux, SDL_MapRGB( screen->format, 255, 255, 255 ) );

    //Gera as novas barras
    nBarsVol = *volume / 8;
    for ( contador = 1; contador <= nBarsVol; contador++ ){
        aux.x += 8;
        SDL_BlitSurface( volBars, NULL, screen, &aux );
    }
    SDL_Flip(screen);

    TTF_CloseFont(gamefont);
    SDL_FreeSurface(volBars);

}

inline int Fadjust_volume( const short int channel, short int *volume, SDLKey *keys, const SDL_Rect *aux ){

    SDL_Event menulastEvent;
    short int leave = 0;

    while ( leave == 0 ){
        while( SDL_PollEvent(&menulastEvent) ){
            if ( menulastEvent.type == SDL_KEYDOWN || menulastEvent.type == SDL_MOUSEBUTTONDOWN ){
                if ( menulastEvent.button.button == SDL_BUTTON_MIDDLE || menulastEvent.key.keysym.sym == keys[0] ){
                    if ( *volume != 0 ) *volume -= 8;
                }
                else if ( menulastEvent.button.button == SDL_BUTTON_RIGHT || menulastEvent.key.keysym.sym == keys[1] ){
                    if ( *volume != 128 )   *volume += 8;
                }
                else if ( menulastEvent.button.button == SDL_BUTTON_LEFT || menulastEvent.key.keysym.sym == keys[5] )
                    leave = 1;
                Fdraw_bar( volume, *aux );
                if ( channel == CHANNELMUSIC )  Mix_VolumeMusic( *volume );
                else    Mix_Volume( channel, *volume );
            }
            else if ( menulastEvent.type == SDL_QUIT )  QUIT_PROGRAM = 1;
        }
    }

    return MVOLUME;
}

inline SDLKey Fset_controls(SDLKey *keys, SDL_Rect *textDestRect){
    //rect receberá a região em que deve ser escrito o texto, showkey será usada para exibir quais teclas foram definidas.
    short int leave;
    register int counter;
    SDL_Event menulastEvent;

    Mix_Chunk *s_error;
    s_error = Mix_LoadWAV("error.ogg");//Carrega o som de erro.

    SDL_FillRect( screen, textDestRect, SDL_MapRGB( screen->format, 255, 255, 255 ) );//Preenche o retângulo com branco.
    SDL_Flip(screen);

    leave = 0;
    while( leave == 0 && QUIT_PROGRAM == 0 ){
        SDL_PollEvent(&menulastEvent);
        if ( menulastEvent.type == SDL_KEYDOWN ){
            //Se uma tecla for pressionada e não estiver sendo utilizada para outra ação.
            for( counter = 0; counter <= (N_KEYS - 1); counter++ )
                if ( menulastEvent.key.keysym.sym == keys[counter] ){
                    Fdraw_key(textDestRect, "TECLA INDISPONÍVEL");
                    Mix_PlayChannel(1, s_error, 0);
                    counter = -1;
                    break;
                }
             if ( counter == -1 )
                continue;
             else{
                Fshow_controls( textDestRect, &menulastEvent.key.keysym.sym );
                leave = 1;
            }
        }
        else if ( menulastEvent.type == SDL_QUIT ){
            QUIT_PROGRAM = 1;
        }
    }

    Mix_FreeChunk(s_error);
    return menulastEvent.key.keysym.sym;
}

inline void Fdraw_key(SDL_Rect *textDestRect, char *description){//rect receberá a região em que deve ser escrito o texto.

    TTF_Font *gamefont;
    gamefont = TTF_OpenFont( "gamefont.ttf", 12 );//Inicializa a fonte a ser usada na escrita de textos

    SDL_Surface *text;//Superfície para armazenar os textos.
    SDL_Color colorOrange = { 255, 100, 0 };//Cor para ser usada em texto

    SDL_FillRect(screen, textDestRect, SDL_MapRGB(screen->format, 255, 255, 255));//"Limpa" o que foi escrito anteriormente.

    text = TTF_RenderText_Solid( gamefont, description, colorOrange );
    SDL_BlitSurface( text, NULL, screen, textDestRect );
    SDL_Flip(screen);

    TTF_CloseFont(gamefont);
    SDL_FreeSurface(text);
    free(description);

}

inline void Fdraw_menu( const Menu *menu ){

    SDL_Surface *imgmenu;
    imgmenu = load_image( menu->imgfile );
    SDL_BlitSurface( imgmenu, NULL, screen, NULL );
    SDL_Flip( screen );

    SDL_FreeSurface (imgmenu);

}

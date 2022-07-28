#include <GL/glew.h>
#include <GL/freeglut.h>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <time.h>

//tipifica naves
typedef struct{
    GLfloat x, y;  //pontos iniciais
    GLfloat length, height;  //comprimento e altura
    GLint hits;  //quantas vezes foi atingida
    GLint max_hits;  //quantas vezes pode ser atingida antes de ser destruída
                     //determina a "dificuldade" das naves inimigas e quantos vidas o jogador tem
    GLint on;  //verifica se o objeto será desenhado (não foi destruído) - inicializado com TRUE
}NAVE;

//tipifica disparos
typedef struct{
    GLfloat x, y;
    GLfloat length, height;
    GLint on;  //inicializado com FALSE
}DISPARO;

NAVE player, enemies[15];  //cria as naves
DISPARO disparo_player[5], disparo_enemies[5];  //cria os disparos - limitado por 5

GLuint texturaPlayer, texturaEnemy, texturaDisparo, texturaCenario; 

#define TRUE 1
#define FALSE 0
int incremento;
int pause = FALSE;

GLuint carregaTextura(const char* arquivo) {
    GLuint idTextura = SOIL_load_OGL_texture(
                           arquivo,
                           SOIL_LOAD_AUTO,
                           SOIL_CREATE_NEW_ID,
                           SOIL_FLAG_INVERT_Y
                       );

    if (idTextura == 0) {
        printf("Erro do SOIL: '%s'\n", SOIL_last_result());
    }

    return idTextura;
}

//inicializa as naves (player e inimigos)
void inicializaNaves(){

    incremento = 10;
    srand(time(0));  //seed para o randômico que determina qual nave inimiga disparará
    player.x = 95;
    player.y = 10;
    player.length = 10;
    player.height = 20;
    player.max_hits = 3;
    player.hits = 0;
    player.on = TRUE;
    texturaPlayer = carregaTextura("player.png");

    for(int i=0;i<15;i++){
        enemies[i].length = 20;
        enemies[i].height = 10;
        enemies[i].on = TRUE;
    }
    for(int y=130, i=0;y<=170;y+=20){
        for(int x=30;x<=150;x+=30, i++){
            enemies[i].x = x;
            enemies[i].y = y;
        }
    }
    for(int i=0, cont=1;i<15;cont++)
        for(int j=0; j<5; j++, i++){
            enemies[i].max_hits = cont;  //quanto mais atrás a nave inimiga estiver, mais 
            enemies[i].hits = 0;         //difícil é destruí-la
        }
    texturaEnemy = carregaTextura("shuttle2.png");

    for(int i=0;i<5;i++){
        disparo_player[i].on = FALSE;
        disparo_player[i].length = 5;
        disparo_player[i].height = 5;
        disparo_enemies[i].on = FALSE;
        disparo_enemies[i].length = 5;
        disparo_enemies[i].height = 5; 
    }
}

void colisao(){
    for(int i=0;i<5;i++)
        if(disparo_enemies[i].on == TRUE)  // se algum disparo inimigo atinigir o jogador
            if(player.x+player.length >= disparo_enemies[i].x && player.x <= disparo_enemies[i].x+disparo_enemies[i].length)
                if(player.y+player.height >= disparo_enemies[i].y-disparo_enemies[i].height && player.y <= disparo_enemies[i].y){
                    player.hits++;         // o contador de "dano" do jogador aumenta
                    disparo_enemies[i].on = FALSE;  // o disparo some
                }
    if(player.hits >= player.max_hits){  // se o contador de dano ultrapassar o limite
        player.on == FALSE;              // o jogador some
        pause = TRUE;                    // o jogo para   
    }

    for(int j=0;j<5;j++)
        if(disparo_player[j].on == TRUE)
            for(int i=0;i<15;i++)
                if(enemies[i].on == TRUE){  // se algum disparo do jogador atingie um inimigo
                    if(enemies[i].x+enemies[i].length >= disparo_player[j].x && enemies[i].x <= disparo_player[j].x+disparo_player[j].length)
                        if(enemies[i].y+enemies[i].height >= disparo_player[j].y && enemies[i].y <= disparo_player[j].y+disparo_player[j].height){
                            enemies[i].hits++;  // o contador de dano do inimigo aumenta
                            disparo_player[j].on = FALSE;  // o disparo some
                        }
                    if(enemies[i].hits >= enemies[i].max_hits)  // se o contador de dano ultrapassar o limite
                        enemies[i].on = FALSE;                  // o inimigo some
                }
    int cont = 0;
    for(int i=0;i<15;i++)
        if(enemies[i].on==FALSE)
            cont++;
        else
            continue;
    if(cont == 15)  // se todas as naves inimigas forem destruídas
        pause = TRUE;  // o jogo para
    glutPostRedisplay();
    glutTimerFunc(33, colisao, 0);
}

void disparoEnemies(){

    if(!pause){  // senão estiver pausado
        int r = 14;
        for(int i = 0;i<5;i++)
            if(disparo_enemies[i].on == FALSE){
                for(int j=0;j<15;j++){  // seleciona uma nave inimiga aleatória para 
                    r = rand()%15;      // realizar o disparo
                    if(enemies[r].on == TRUE)
                        break;
                }
                disparo_enemies[i].x = enemies[r].x + 7.5;  // determina a posição inicial do disparo
                disparo_enemies[i].y = enemies[r].y;
                disparo_enemies[i].on = TRUE;
                break;
            }

        for(int i=0;i<5;i++)
            if(disparo_enemies[i].on == TRUE)
                if(disparo_enemies[i].y-disparo_enemies[i].height - 10 <= 0)  // se algum disparo inimigo for ultrapassar o limite inferior da tela
                    disparo_enemies[i].on = 0;                                // ele some
                else
                    disparo_enemies[i].y -= 10;  // atualiza a altura do disparo inimigo
        glutPostRedisplay();
    }
    glutTimerFunc(100, disparoEnemies, 0);
}

void disparoPlayer(){
    if(!pause){  // senão estiver pausado
        for(int i=0;i<5;i++)
            if(disparo_player[i].on == TRUE)
                if(disparo_player[i].y + disparo_player[i].height + 10 >= 200)  // se algum disparo do jogador for ultrapassar o limite superior da tela
                    disparo_player[i].on = FALSE;                               // ele some
                else
                    disparo_player[i].y += 10;    
        glutPostRedisplay();
    }
    glutTimerFunc(60, disparoPlayer, 0);
}

void atualizaMinhaCena(int valor){

    if(!pause){  // senão estiver pausado
        int down = FALSE;
        for(int i=0; i<15; i++)
            if((enemies[i].x + enemies[i].length + incremento >= 200 || enemies[i].x + incremento <= 0) && enemies[i].on == TRUE){  // se alguma nave inimiga for ultrapassar os limtes da tela
                down = TRUE;                                                                                                     // elas descem
                break;
            }
        if(down){  // atualiza a altura das naves inimigas
            for(int i=0; i<15; i++)
                enemies[i].y -= 10;
            for(int i=0; i<15; i++)
                if(enemies[i].y <= player.y + player.height && enemies[i].on == TRUE)  // se chegarem até o jogador
                    pause = TRUE;
            incremento *= -1;
            down = FALSE;
        }else  // atualiza a posição das naves inimigas em relação ao eixo x
            for(int i=0; i<15; i++)
                 enemies[i].x += incremento;
        glutPostRedisplay();
    }
    glutTimerFunc(700, atualizaMinhaCena, 0);
}

// Rotina de desenho
void desenhaMinhaCena(){

    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);

    glBindTexture(GL_TEXTURE_2D, texturaCenario);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, 0, -1.0);
        glTexCoord2f(1, 0); glVertex3f(200, 0, -1.0);
        glTexCoord2f(1, 1); glVertex3f(200, 200, -1.0);
        glTexCoord2f(0, 1); glVertex3f(0, 200, -1.0);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texturaPlayer);
    glBegin(GL_QUADS);
        if(player.on == TRUE){
            glTexCoord2f(0, 0); glVertex3f(player.x, player.y, 0.0);
            glTexCoord2f(1, 0); glVertex3f(player.x + player.length, player.y, 0.0);
            glTexCoord2f(1, 1); glVertex3f(player.x + player.length, player.y + player.height, 0.0);
            glTexCoord2f(0, 1); glVertex3f(player.x, player.y + player.height, 0.0);
        }
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texturaEnemy);
    glBegin(GL_QUADS);
        for(int i=0; i<15; i++){
            if(enemies[i].on == TRUE){
                glTexCoord2f(1, 1); glVertex3f(enemies[i].x, enemies[i].y, 0.0);
                glTexCoord2f(0, 1); glVertex3f(enemies[i].x + enemies[i].length, enemies[i].y, 0.0);
                glTexCoord2f(0, 0); glVertex3f(enemies[i].x + enemies[i].length, enemies[i].y + enemies[i].height, 0.0);
                glTexCoord2f(1, 0); glVertex3f(enemies[i].x, enemies[i].y + enemies[i].height, 0.0);
            }
       }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glColor3f(1.0, 0.5, 0.5);
        for(int i=0;i<5;i++)
            if(disparo_player[i].on == TRUE){
                glVertex3f(disparo_player[i].x, disparo_player[i].y, 0.0);
                glVertex3f(disparo_player[i].x + disparo_player[i].length, disparo_player[i].y, 0.0);
                glVertex3f(disparo_player[i].x + disparo_player[i].length, disparo_player[i].y + disparo_player[i].height, 0.0);
                glVertex3f(disparo_player[i].x, disparo_player[i].y + disparo_player[i].height, 0.0);
            }

        glColor3f(0.0, 0.0, 0.0);
        for(int i=0;i<5;i++)
            if(disparo_enemies[i].on == TRUE){
                glVertex3f(disparo_enemies[i].x, disparo_enemies[i].y, 0.0);
                glVertex3f(disparo_enemies[i].x + disparo_enemies[i].length, disparo_enemies[i].y, 0.0);
                glVertex3f(disparo_enemies[i].x + disparo_enemies[i].length, disparo_enemies[i].y - disparo_enemies[i].height, 0.0);
                glVertex3f(disparo_enemies[i].x, disparo_enemies[i].y - disparo_enemies[i].height, 0.0);
            }
    glEnd();
    
    glutSwapBuffers();
}

// NOVIDADE: uma fun��o que vamos chamar dentro
//    do "main"
// Inicia algumas vari�veis de estado do OpenGL
void setup(){
    // define qual � a cor do fundo
    glClearColor(0.5, 0.5, 0.5, 1); // cinza

    // habilita mesclagem de cores, para termos suporte a texturas
    // com transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    texturaCenario = carregaTextura("cenario.jpg");
    inicializaNaves();
}

// NOVIDADE: callback para o evento "reshape"
void redimensionada(int width, int height)
{
   // left, bottom, right, top
   glViewport(0, 0, width, height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 200.0, 0.0, 200.0, -5.0, 5.0);

   glMatrixMode(GL_MODELVIEW);
}

// NOVIDADE: callback de "keyboard"
void teclaPressionada(unsigned char key, int x, int y)
{
    // v� qual tecla foi pressionada
    switch(key){
    case 27:      // Tecla "ESC"
        exit(0);  // Sai da aplica��o

    case 52:      // Tecla 4 ou A/a
    case 65:      // Move para esquerda
    case 97:
        if(!pause){
            if(player.x - 10 >= 0)
                player.x -= 10;
        }
        break;

    case 54:      // Tecla 6 ou D/d
    case 68:      // Move para direita
    case 100:
        if(!pause){
            if(player.x + player.length + 10 <= 200)
                player.x += 10;
        }
        break;

    case 32:     // Tecla "ESPAÇO"
        if(!pause){
            if(player.on == TRUE){  // Realiza um disparo
                for(int i=0; i<5; i++){
                    if(disparo_player[i].on == FALSE){
                        disparo_player[i].x = player.x + 2.5;  // determina a posição inicial do disparo do jogador
                        disparo_player[i].y = player.y + player.height;
                        disparo_player[i].on = TRUE;
                        break;
                    }
                }
            }
        }
        break;

    case 112:     // Tecla P/p
    case 80:      //  Pausa o jogo
        if(pause == TRUE)
            pause = FALSE;
        else
            pause = TRUE;
        break;

    case 82:     // Tecla R/r
    case 114:    // Reinicia o jogo
        pause = FALSE;
        inicializaNaves();

    default:
        break;
    }

    glutPostRedisplay();
}

// Fun��o principal
int main(int argc, char** argv){

    glutInit(&argc, argv);
    glutInitContextVersion(1, 1);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0.5*(glutGet(GLUT_SCREEN_WIDTH)-500), 0.5*(glutGet(GLUT_SCREEN_HEIGHT)-500));  //move a posição da janela para o meio da tela

    glutCreateWindow("GALAXIAN");

   // Registra callbacks para eventos
    glutDisplayFunc(desenhaMinhaCena);
    glutReshapeFunc(redimensionada);
    glutKeyboardFunc(teclaPressionada);

    glutTimerFunc(700, atualizaMinhaCena, 0);
    glutTimerFunc(60, disparoPlayer, 0);
    glutTimerFunc(700, disparoEnemies, 0);
    glutTimerFunc(33, colisao, 0);

   // Configura valor inicial de algumas
   // vari�veis de estado do OpenGL
    setup();

    glutMainLoop();
    return 0;
}

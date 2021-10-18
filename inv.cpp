#include "glut.h"
#include "glm/glm.hpp"
#include <time.h>

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

//Tamamho
#define INVADER_WIDTH (64)
#define INVADER_HEIGHT (48)

#define INVADER_COLUMN_MAX (11) //coluna
#define INVADER_ROW_MAX (5)    //linha
#define INVADER_MAX (INVADER_COLUMN_MAX * INVADER_ROW_MAX) //quantidade
#define INVADER_SPEED (INVADER_WIDTH / 8)

//Tamamho
#define PLAYER_WIDTH (64)
#define PLAYER_HEIGHT (64)

#define PLAYER_SPEED (WINDOW_WIDTH / (60*3)) //3s

#define PLAYERBEAM_HEIGHT (PLAYER_HEIGHT)
#define PLAYERBEAM_SPEED (WINDOW_HEIGHT / 60) // 60 frames para chegar de baixo para cima

#define INVADERBEAM_HEIGHT (INVADER_HEIGHT)
#define INVADERBEAM_SPEED (WINDOW_HEIGHT / (60 * 2)) // mais lento q o player

enum {
    INVADER_PHASE_RIGHT,
    INVADER_PHASE_DOWN,
    INVADER_PHASE_LEFT,
    INVADER_PHASE_MAX
};

typedef struct {
    glm::vec2 position;
    bool isDead;
}INVADER;

typedef struct{
    glm::vec2 position;
}PLAYER;

typedef struct{
    glm::vec2 position;
    glm::vec2 velocity;
    bool isFired; // para nao bugar apertando o space muitas vezes
}PLAYERBEAM;

typedef struct{
    glm::vec2 position;
    glm::vec2 velocity;
    bool isFired; 
}INVADERBEAM;

INVADER invaders[INVADER_MAX];

int invaderPhase;
int invaderNextPhase;
int currentInvader;

PLAYER player;

PLAYERBEAM playerBeam;

INVADERBEAM invaderBeam[INVADER_COLUMN_MAX];

bool keysPressed[256]; //pressionando a tecla

void init(void){
    for(int i=0; i < INVADER_MAX; i++){
        invaders[i].position = {
            (i% INVADER_COLUMN_MAX) * INVADER_WIDTH * 1.5f,
            (i/INVADER_COLUMN_MAX) * INVADER_HEIGHT * 1.25f};
        invaders[i].isDead = false;
    }

    invaderPhase = INVADER_PHASE_RIGHT;
    invaderNextPhase = INVADER_PHASE_RIGHT;
    currentInvader = 0;

    player.position = {WINDOW_WIDTH / 2 - PLAYER_WIDTH / 2, 
                        WINDOW_HEIGHT - PLAYER_HEIGHT * 2};

    playerBeam.isFired = false;

    for(int j=0; j < INVADER_COLUMN_MAX; j++){
        invaderBeam[j].isFired = false;
    }
}

void tela(void){
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //desenhar o beam do invader
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    for(int j=0; j < INVADER_COLUMN_MAX; j++){
        if(!invaderBeam[j].isFired){
            continue;
        }
        glLineWidth(WINDOW_WIDTH/256 *2); // tamanho da line
        //desenahndo o beam do invader
        glPushMatrix();
        {
            glTranslatef(invaderBeam[j].position.x, 
            invaderBeam[j].position.y, 
            0);
            glBegin(GL_LINES);{
                glColor4ub(0xff, 0xff, 0xff, 0xff); //branco
                glVertex2f(0, 0);
                glColor4ub(0xff, 0xff, 0xff, 0x00); //opacidade
                glVertex2f(0, -INVADERBEAM_HEIGHT);
            }
            glEnd();
        }
        glPopMatrix();
    }
    glDisable(GL_BLEND);

    int p = 1;

    //desenhar o invader
    glPointSize(16); // tamano do olho
    for(int i=0; i<INVADER_MAX; i++){
        if(invaders[i].isDead){// morto
            continue;
        }
        glPushMatrix();{ // desenhando os invaders
            glTranslatef(invaders[i].position.x + INVADER_WIDTH/2,
             invaders[i].position.y + INVADER_HEIGHT/2,
             0);
            //glColor3ub(0xff, 0xff, 0xff); //cor branca

            for(int c=0; c<p; c++){
                const unsigned char colors[][3] = { // cor dos invaders
                    {0xff,0x00,0x00}, //vermelho
                    {0xff,0xff,0x00}, //amarelo
                    {0x00,0xff,0x00}, //verde
                    {0x00,0xff,0xff}, //ciano
                    {0xff,0x00,0xff} //magenta
                };
                glColor3ubv((GLubyte*)&colors[c]);
            }
            
            if((i+1)/11 == p)
                p++;

            glScalef(INVADER_WIDTH, INVADER_HEIGHT, 0);
            glutSolidSphere(0.5, 8, 2);
            
            glColor3ub(0x00, 0x00, 0x00); // preto
            glBegin(GL_POINTS);{
                glVertex2f(-0.25,0.125); //ponto do olho e posicao
                glVertex2f(0.25,0.125);
            }
            glEnd();
        }
        glPopMatrix();
    }

    //desenahndo o beam do jogador
    if(playerBeam.isFired){
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);

        glLineWidth(WINDOW_WIDTH/256 *2); // tamanho da line
        //desenahndo o beam do jogador
        glPushMatrix();
        {
            glTranslatef(playerBeam.position.x, 
            playerBeam.position.y, 
            0);
            glBegin(GL_LINES);{
                glColor4ub(0xff, 0xff, 0xff, 0xff); //branco
                glVertex2f(0, 0);
                glColor4ub(0xff, 0xff, 0xff, 0x00); //opacidade
                glVertex2f(0, PLAYERBEAM_HEIGHT);
            }
            glEnd();
        }
        glPopMatrix();

        glDisable(GL_BLEND);
    }

    glColor3ub(0xff, 0xff, 0xff); //cor branca
    //desenahndo o jogador
    glPushMatrix();
    {
        glTranslatef(player.position.x + PLAYER_WIDTH/2, 
        player.position.y + PLAYER_HEIGHT/2, 
        0);
        glScalef(PLAYER_WIDTH, PLAYER_HEIGHT, 0);
        glBegin(GL_TRIANGLES);{
            glVertex2f(0, -0.5f);
            glVertex2f(-0.5f, 0.5f);
            glVertex2f(0.5f, 0.5f);
        }
        glEnd();
    }
    glPopMatrix();



    glutSwapBuffers();
}

void mov(void){
    if(currentInvader < INVADER_MAX){
        switch (invaderPhase){
        case INVADER_PHASE_RIGHT:
            invaders[currentInvader].position.x += INVADER_SPEED;
            if(invaders[currentInvader].position.x >= WINDOW_WIDTH-INVADER_WIDTH){
                invaderNextPhase = INVADER_PHASE_DOWN;
            }
            break;
        case INVADER_PHASE_DOWN:
            invaders[currentInvader].position.y += INVADER_HEIGHT;
            if(invaders[currentInvader].position.x >= WINDOW_WIDTH-INVADER_WIDTH){
                invaderNextPhase = INVADER_PHASE_LEFT;
            }
            if(invaders[currentInvader].position.x < INVADER_WIDTH){
                invaderNextPhase = INVADER_PHASE_RIGHT;
            }
            break;
        case INVADER_PHASE_LEFT:
            invaders[currentInvader].position.x -= INVADER_SPEED;
            if(invaders[currentInvader].position.x < INVADER_WIDTH){
                invaderNextPhase = INVADER_PHASE_DOWN;
            }
            break;
        }
    }
    // termina se todos estao mortos
    //vai aumentando de velocidade se diminui a quantidade de invaders
    do{
        currentInvader++;
    }while((currentInvader < INVADER_MAX) && (invaders[currentInvader].isDead));

    if(currentInvader >= INVADER_MAX){
        currentInvader =0;
        while((currentInvader < INVADER_MAX) && (invaders[currentInvader].isDead)){
            currentInvader++;
        }

        invaderPhase = invaderNextPhase;
    }

    if(keysPressed['a']){
        player.position.x -= PLAYER_SPEED;
    }
    if(keysPressed['d']){
        player.position.x += PLAYER_SPEED;
    }
    if(player.position.x < 0){
        player.position.x = 0;
    }
    if(player.position.x > WINDOW_WIDTH - PLAYER_WIDTH){
        player.position.x = WINDOW_WIDTH - PLAYER_WIDTH;
    }

    //colisao do beam do jogador
    if(playerBeam.isFired){
        playerBeam .position += playerBeam.velocity;
        if(playerBeam .position.y < -PLAYERBEAM_HEIGHT){
            playerBeam.isFired = false; //para atirar novamente depois q some da tela
        }
        //O movi de beam vai parar
        for(int i = 0; i< INVADER_MAX; i++){
            if((!invaders[i].isDead)
                && (playerBeam.position.x >= invaders[i].position.x)
                && (playerBeam.position.x < invaders[i].position.x + INVADER_WIDTH)
                && (playerBeam.position.y >= invaders[i].position.y)
                && (playerBeam.position.y < invaders[i].position.y + INVADER_HEIGHT))
            {
                invaders[i].isDead = true;
                playerBeam.isFired = false;
            }
        }   
    }

    //colisao do beam do invader
    for(int j=0; j < INVADER_COLUMN_MAX; j++){
        if(!invaderBeam[j].isFired){
            if(rand()%(60*3)){ // cada 3s atira
                continue;
            }
            int invader = j;
            //Para o invader de baixo atirar
            for(int y=0; y < INVADER_ROW_MAX; y++){
                if(!invaders[y * INVADER_COLUMN_MAX + j].isDead){
                    invader = y * INVADER_COLUMN_MAX + j;
                }
            }
            //O invader q vai estar mais embaixo q atira
            if(!invaders[invader].isDead){
                invaderBeam[j].isFired = true;
                invaderBeam[j].position = invaders[invader].position + 
                                    glm::vec2(INVADER_WIDTH/2,INVADER_HEIGHT);
                invaderBeam[j].velocity = {0, INVADERBEAM_SPEED};
            }
        }
        else{
            invaderBeam[j].position += invaderBeam[j].velocity;

            if(invaderBeam[j].position.y >= WINDOW_HEIGHT + INVADERBEAM_HEIGHT){
                invaderBeam[j].isFired = false;
            }
            //A Colisao
            if((invaderBeam[j].position.x >= player.position.x)
                && (invaderBeam[j].position.x < player.position.x + PLAYER_WIDTH)
                && (invaderBeam[j].position.y >= player.position.y)
                && (invaderBeam[j].position.y < player.position.y + PLAYER_HEIGHT))
            {
                init(); // Game Over
            }
        }
    }

    glutPostRedisplay(); // redesenhar
}

void Keyboard(unsigned char _key, int, int){
    keysPressed[_key] = true;

    if((_key == ' ') && (!playerBeam.isFired)){
        playerBeam.position = player.position + glm::vec2(PLAYER_WIDTH / 2, 0);
        playerBeam.velocity = {0, -PLAYERBEAM_SPEED};
        playerBeam.isFired = true;
    }
}

void KeyboardUP(unsigned char _key, int, int){
    keysPressed[_key] = false;
}

int main(int argc, char* argv[]){
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Invader");
    glutDisplayFunc(tela);
    glutIdleFunc(mov);  // para o processamento em tempo real
    glutKeyboardFunc(Keyboard); //tecla
    glutKeyboardUpFunc(KeyboardUP); //solta a tecla
    init();
	glutMainLoop();

}
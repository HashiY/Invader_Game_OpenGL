#include "glut.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <string.h>

//TAMANHO DA TELA
#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define BLOCK_WIDTH (128) //largura
#define BLOCK_HEIGHT (32)//altura

#define BLOCK_COLUMN_MAX (WINDOW_WIDTH / BLOCK_WIDTH) //coluna
#define BLOCK_ROW_MAX (6)    //linha

#define PADDLE_WIDTH (128) //largura
#define PADDLE_HEIGHT (32)//altura

#define BALL_SCALE (8)
#define BALL_SPEED (WINDOW_HEIGHT / (60.0f * 4))

typedef struct{
    bool isBroken;
}BLOCK;

typedef struct{
    glm::vec2 position;
}PADDLE;

typedef struct{
    glm::vec2 position;
    glm::vec2 lastPosition; // para voltar 1 frame antes
    glm::vec2 velocity;
}BALL;

BLOCK blocks[BLOCK_ROW_MAX][BLOCK_COLUMN_MAX];
PADDLE paddle;
BALL ball;

void init(void){
    //blocks
    memset(blocks, 0, sizeof blocks);   //arranjo clear

    //posicionando a raquete
    paddle.position = {WINDOW_WIDTH/2 - PADDLE_WIDTH/2, WINDOW_HEIGHT - PADDLE_HEIGHT * 3};
    //bola
    ball.lastPosition = ball.position;
    ball.position = {WINDOW_WIDTH/2, BLOCK_HEIGHT*BLOCK_ROW_MAX};
    ball.velocity = {BALL_SPEED, BALL_SPEED};
    ball.velocity = glm::normalize(ball.velocity) * BALL_SPEED;

}

void tela(void){
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //desenhando os blocos
    for(int i=0; i < BLOCK_ROW_MAX; i++){
        const unsigned char colors[][3] = { // cor dos blocos
            {0xff,0x00,0x00}, //vermelho
            {0xff,0xff,0x00}, //amarelo
            {0x00,0xff,0x00}, //verde
            {0x00,0xff,0xff}, //ciano
            {0x00,0x00,0xff}, //azul
            {0xff,0x00,0xff} //magenta
        };
        glColor3ubv((GLubyte*)&colors[i]);
        for(int j=0; j < BLOCK_COLUMN_MAX; j++){
            if(blocks[i][j].isBroken){ // se quebrou
                continue;               // pula ele
            }
            glPushMatrix();{
                glTranslatef(BLOCK_WIDTH * j, BLOCK_HEIGHT * i, 0);
                glBegin(GL_QUADS); // quadrado
                {
                    glVertex2d(0,0);
                    glVertex2d(0, BLOCK_HEIGHT-1); // -1 para separar 
                    glVertex2d(BLOCK_WIDTH-1, BLOCK_HEIGHT-1);
                    glVertex2d(BLOCK_WIDTH-1, 0);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }
    //desenhando a raquete
    glColor3ub(0xff,0xff,0xff);//branco
    glPushMatrix();{
        glTranslatef(paddle.position.x, paddle.position.y, 0);
        glBegin(GL_QUADS); // quadrado
        {
            glVertex2d(0,0);
            glVertex2d(0, PADDLE_HEIGHT);
            glVertex2d(PADDLE_WIDTH, PADDLE_HEIGHT);
            glVertex2d(PADDLE_WIDTH, 0);
        }
        glEnd();
    }
    glPopMatrix();

    //desenhar a bola
    glColor3ub(0xff,0xff,0xff);//branco
    glPushMatrix();{
        glTranslatef(ball.position.x, ball.position.y, 0);
        glScalef(BALL_SCALE, BALL_SCALE, 0);
        glBegin(GL_TRIANGLE_FAN); 
        {
            glVertex2f(0,0);
            int n = 32;  //poligono
            for(int i =0; i<=n; i++){
                float r=glm::pi<float>() * 2 * i /n; //angulo
                glVertex2f(cosf(r), -sinf(r));
            }
            
        }
        glEnd();
    }
    glPopMatrix();

    glutSwapBuffers();
}

void mov(void){
    ball.lastPosition = ball.position; // lembrar a posicao antes de ter movi
    ball.position += ball.velocity;

    // se bater no chao Game Over
    if(ball.position.y >= WINDOW_HEIGHT){
        init();
        glutPostRedisplay();
        return;
    }
    // colisao nas paredes
    if(ball.position.x < 0 || ball.position.x >= WINDOW_WIDTH){//na parede
        ball.position = ball.lastPosition; // a bola vai parar
        ball.velocity.x *= -1.0f;   // vai voltar
    }
    if(ball.position.y < 0 || ball.position.y >= WINDOW_HEIGHT){
        ball.position = ball.lastPosition; // a bola vai parar
        ball.velocity.y *= -1.0f;   // vai voltar para cima
    }
    //colisao na raquete
    if((ball.position.x >= paddle.position.x) 
        && (ball.position.x < paddle.position.x + PADDLE_WIDTH)
        && (ball.position.y >= paddle.position.y)
        && (ball.position.y < paddle.position.y + PADDLE_HEIGHT))
    {
        ball.position = ball.lastPosition; // a bola vai parar
        float paddleCenterX = paddle.position.x + PADDLE_WIDTH/2;
        ball.velocity.x = (ball.position.x - paddleCenterX) / (PADDLE_WIDTH/2) * 2; // angulo
        ball.velocity.y = -glm::sign(ball.velocity.y);   // vira +1 ou -1
        ball.velocity = glm::normalize(ball.velocity) * BALL_SPEED; // normaliza e corrigi a vel
    }

    //Colisao dos blocos
    {
        int x = (int)ball.position.x / BLOCK_WIDTH;
        int y = (int)ball.position.y / BLOCK_HEIGHT;
        if((x >= 0) && (x < BLOCK_COLUMN_MAX) 
            && (y >= 0) && (y < BLOCK_ROW_MAX)
            && (!blocks[y][x].isBroken))
        {
            ball.position = ball.lastPosition; 
            blocks[y][x].isBroken = true; //quebrou

            //se colidiu de cima e baixo
            if((ball.lastPosition.y < BLOCK_HEIGHT*y)
                || ball.lastPosition.y >= BLOCK_HEIGHT*(y+1))
            {
                ball.velocity.y *= -1.0f;
            }
            else {  // se colidiu de lado
                ball.velocity.x *= -1.0f;
            }
        } 
    }

    glutPostRedisplay(); // redesenhar
}

void PassiveMotion (int _x, int _y){
    paddle.position.x = (float)_x - PADDLE_WIDTH/2; // coordenadas x do mouse
                                    // - para o mouse ficar no meio
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("ARKANOID");
    glutDisplayFunc(tela);
    glutIdleFunc(mov); // para o processamento em tempo real
    glutPassiveMotionFunc(PassiveMotion);   //mouse
    init();
    glutMainLoop();
}
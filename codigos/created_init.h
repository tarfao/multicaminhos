//
// Created by root on 17/06/19.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

double raio = 50;
//DIMENSÃOO DE BUSCA
const int MAXX = 600;
const int MAXY = 400;

const int LARGURA_TELA = 1400;
const int ALTURA_TELA = 800;

typedef struct sensor
{
    int Id;/*id = <posicao_vetor>*/
    int Pwi; /*Energia inicial do sensor. Gerada entre 70 e 99 */
    int Pwa; /* Energia atual, Gerada entre Pwi e (Pwi - 30)*/

    /*dois auxiliares para recuperar o valor original/inicial de pwi e pwa */
    int auxPwi;
    int auxPwa;

    /*x e y representa a posicao do sensor no plano */
    int x;
    int y;

    float **mtAdjMet; /*matriz [2][N], onde na primeira linha temos os sensores adjacentes com 
    sua respectiva distancia ao sensor referencia. E na segunda linha temos as metricas para esses sensores.*/

    float **mtDijks; /*representa a matriz gerada quando o algoritmo de dijkstra eh executado */

    int color; /*Representa as cores dos dispositivos na parte grafica
                 0 - preta      - dispositivo qualquer
                 1 - verde      - representa o inicial
                 2 - azul       - representa o final 
                 3 - rosado     - participa da comunicacao fim a fim 
                 4 - vermelho    - dispositivo sem carga
                 */

    int sendData; /* Essa variavel auxilia no descobrimento de multicaminhos 
                    1 - se enviou dados 
                     0 - caso nao envou*/

    int countRoutes; /*representa a quantidade de rotas encontradas */


}NODE;

/*AUXILIA O ALGORITMO DE DIJKSTRA*/
typedef struct queue{
    int id;
    float d;
    struct queue *prox;
}ENCADEADA;

//======================================= ALOCACAO DE NOHS, E MATRIZES =====================================

NODE* alocaVetorNode (int N)
{
    NODE *V;

    V = malloc(sizeof(NODE) * N);
    return V;
}

int** alocaMatriz(int N)
{
    int i;
    int **M;

    M = malloc (sizeof(int *) * N);

    for( i = 0; i < N; i++)
    {
        M[i] = malloc (sizeof(int) * N);
    }
    return M;
}

float** alocaMatrizf(int N, int K)
{
    int i;
    float **M;

    M = malloc (sizeof(float *) * N);

    for( i = 0; i < N; i++)
    {
        M[i] = malloc (sizeof(float) * K);
    }
    return M;
}

//======================================= FIM DAS ALOCA��ES =====================================


//======================================= INICIALIZA��ES DOS NOHS E MATRIZES =====================================
//objetivo: inicializar a matriz toda com 0 e para cada n� Ni, atribuir o id i+1
//inicializa as adjac�ncias, o contador de passos at� o �ltimo n�, o ID e a bateria
void inicializaMatriz (int N, int** Matriz)
{
    int i, j;
    for (i = 0; i < N; i++)
    {
        for(j = 0; j < N; j++)
        {
            Matriz[i][j] = 0;
        }
    }
}

//objetivo: inicializar a matriz toda com 0 e para cada n� Ni, atribuir o id i+1
//inicializa as adjac�ncias, o contador de passos at� o �ltimo n�, o ID e a bateria
void inicializaMatrizf (int N, int m, float valor, float** Matriz)
{
    int i, j;
    for (i = 0; i < N; i++)
    {
        for(j = 0; j < m; j++)
        {
            Matriz[i][j] = valor;
        }
    }
}


void inicializaNos(int N, NODE *Nos, int sink)
{
    int i;

    srand(time(NULL));
    for(i = 0; i < N; i++){
        Nos[i].Id = i;
        Nos[i].Pwi = rand() % 30 + 70; 
        Nos[i].Pwa = rand() % 30 + (Nos[i].Pwi - 30);/*rand() % Nos[i].Pwi + 1 */
        Nos[i].mtAdjMet = alocaMatrizf(2,N);
        inicializaMatrizf(2, N, 0.0, Nos[i].mtAdjMet);
        Nos[i].mtDijks = alocaMatrizf(N,N+2);
        inicializaMatrizf(N, N+2, -1.0, Nos[i].mtDijks);
        Nos[i].color = 0;
        Nos[i].sendData = 0;
        Nos[i].countRoutes = 0;
        Nos[i].auxPwa = Nos[i].Pwa;
        Nos[i].auxPwi = Nos[i].Pwi;
 
    }
}

void inicializa_color(int N, NODE *nos){
    int i;

    for(i = 0; i < N; i++)
    {
        nos[i].color=0;
    }
}

//======================================= FIM DAS INICIALIZA��ES =====================================
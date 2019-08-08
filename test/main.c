#include "router.h"
#include "gnuplot_i.h"
#include <pthread.h>
#include <GL/glut.h>

#define PI 3.14159265358979323846 

NODE *Nos;
int N;/*representa a quantidade de dispositivos */
int dispMenu = -1;/*representa o dispositivo que foi clicado */
int inicial = -1, final = -1;/*representa o dispositivo inicial e final */
int ax = 1020, ay = 640; /*representa os eixos x e y do plano do opengl. Ele nao eh o tamanho da janela */
int click = -1;/*representa o dispositivo eu devo atribuir a cor de inicial ou final
            se -1 -> nao foi atribuido o incial
            se 0 -> ja foi atribuido o inicial, resta o dispositivo final
            se 1 -> os dois dispositivos ja foram definidos e pode iniciar o roteamento */

/*médias  */
double med_temp_dijks = 0.0;
int qt_temp_dijks = 0;

int med_energia = 0; /*media das energias utilizadas no caminho */
int med_dispositivo = 0;/*media da quantidade de dispositivo utilizado no caminho */

int num_ciclos = 0; /*representa o numero de vezes que o caminho pode ser encontrado com sucesso */

typedef struct dados_thread{
    double *x;
    double *y;
    int cont;
}DATE_THREAD;

typedef struct data_rede{
    NODE *Nos;
    int N;
}Data; 

void Desenha(void);
void organiza();
 
//======================================= EXIBE O CONTE�DO DAS MATRIZES =====================================
//mostra uma matriz de adjac�ncia
void mostra (int N, int** matriz)
{
    int i, j;

    printf("  ");
    for (int i = 0; i < N; ++i)
    {
        printf("%i ",i);
    }
    printf("\n------------------------------\n");
    for (i = 0; i < N; i++)
    {
        printf("%i|",i );
        for(j = 0; j < N; j++)
        {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("------------------------------\n");
}

//mostra uma matriz de adjacencia
void mostraf (int N, float** matriz)
{
    int i, j;

    printf("  ");
    for (int i = 0; i < N; ++i)
    {
        printf("%i     ",i);
    }
    printf("\n------------------------------\n");
    for (i = 0; i < N; i++)
    {
        printf("%i|",i );
        for(j = 0; j < N; j++)
        {
            printf("%.2f ", matriz[i][j]);
        }
        printf("\n");
    }
    printf("------------------------------\n");
}
//======================================= FIM DAS EXIBICOES =====================================

/*=======================================GRAVACAO DA MEDIA DE BATERIAS E NUMERO DE NÓS================================= */
void grava_energia()
{
    FILE *fp;
    int qtbarra_ene;
    char arq;
    double media_energ = med_energia/med_dispositivo;
    double media_dispos = med_dispositivo/num_ciclos;

    if((fp = fopen("media_energia_dispositivo.txt", "a+")) == NULL)
    {
        puts("Erro na abertura do arquivo de log para construcao do grafico de tempo dijk!\n");
        exit(1);
    }

    rewind(fp);

    qtbarra_ene = 0;
    while((arq = getc(fp)) != EOF)
    {
        if(arq == '\n')
            qtbarra_ene++;
    }
    qtbarra_ene++;
    
    fprintf(fp,"%d %.6f %.6f %d\n", qtbarra_ene, media_energ, media_dispos, num_ciclos);

    fclose(fp);
}

/*=======================================GRAVACAO DA MEDIA DE TEMPO DE EXECUCAO PARA O ALGORITMO DE DIJKSTRA================================= */

void grava_medias_dijkstra()
{
    FILE *fp;
    int qtbarra_ene;
    char arq;
    double media = med_temp_dijks/qt_temp_dijks;

    if((fp = fopen("media_exec_dijks.txt", "a+")) == NULL)
    {
        puts("Erro na abertura do arquivo de log para construcao do grafico de tempo dijk!\n");
        exit(1);
    }

    rewind(fp);

    qtbarra_ene = 0;
    while((arq = getc(fp)) != EOF)
    {
        if(arq == '\n')
            qtbarra_ene++;
    }
    qtbarra_ene++;
    
    fprintf(fp,"%d %.6f\n", qtbarra_ene, media);

    fclose(fp);

    grava_energia();
}

void *print_time(void *args){

    DATE_THREAD date;
    struct timeval time_inicial, time_final;
    double tIni, tFim;
    int i, j, aux;
    int cargaRetirada; /*potencia de carga que será retirado da bateria residual*/
    gnuplot_ctrl * graph = gnuplot_init();
    char msg[100]; /*msg para enviar no plot*/
    FILE *fp;
    char arq;
    int qtbarra_ene;
    int num_arq;
    char nome_arq[20];

    int med_aux_local;


    date.x = malloc(sizeof(double) * N);
    date.y = malloc(sizeof(double) * N);
    date.cont = 0;
    printf("Resposta para rota em dijkstra:::::\n");

    if(Nos[inicial].mtDijks[final][N+1] < 0.0){//o N+1 representa o proximo dispositivo, se ele é -1 significa que nao há rotas até o proximo nó -> rede desconexa
        printf("Falha na comunicação com o dispositivo de id = %i",final);
        Nos[inicial].color = 4;
        grava_medias_dijkstra();
    }

    num_arq = 0;

    while(1) {

        // to refresh the window it calls display() function
        glutPostRedisplay();

        gettimeofday(&time_inicial, NULL);
        tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;

        tFim = 0;
        while (tFim - tIni < 5.0) { 
            gettimeofday(&time_final, NULL);
            tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;
        }
        /*for (i = 0; i < N; i++) {
            cargaRetirada = rand() % 8;
            if(Nos[i].Pwa - cargaRetirada < 0){
                Nos[i].Pwa = 0;
                Nos[i].color = 4;
            }else{
                Nos[i].Pwa -= cargaRetirada;
            }
        }*/
        click = 1;
        if(click == 1){           
            /*for (i = 0; i < N; i++) {
                cargaRetirada = rand() % 8;
                if(Nos[i].Pwa - cargaRetirada < 0){
                    Nos[i].Pwa = 0;
                    Nos[i].color = 4;
                }else{
                    Nos[i].Pwa -= cargaRetirada;
                }
            }*/
            conhece_metrica(N, Nos);/*Conhece a nova metrica dos dispositivos*/

            for(i = 0; i < N; i++){
                inicializaMatrizf(N, N+2, -1.0, Nos[i].mtDijks);
            }

            gettimeofday(&time_inicial, NULL);
            tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;
            for (i = 0; i < N; i++) {
                if(Nos[i].Pwa > 0)
                    dijkstra(&Nos[i], Nos, N);
            } 
            gettimeofday(&time_final, NULL);
            tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;

            med_temp_dijks += (tFim - tIni);
            qt_temp_dijks++;

            printf("Resposta para rota em dijkstra:::::\n");
            date.cont = 0;

            if(Nos[inicial].Pwa != 0){ 
                Nos[inicial].color = 1;
                if(Nos[final].Pwa != 0){ 
                    Nos[final].color = 2;
                    aux = inicial;
                    med_aux_local = 0;
                    while (aux != final){
                        if(Nos[aux].Pwa == 0){
                            printf("Dispositivo descarregado de id = %i\n", aux);
                            inicializa_color(N, Nos);
                            Nos[aux].color = 4;
                            aux = final;
                            grava_medias_dijkstra();
                            /*gravacao do arquivo de log do tempo de execucao do algoritmo de dijks */
                        }else{
                            date.x[date.cont] = (double)Nos[aux].Id;
                            date.y[date.cont] = (double)Nos[aux].Pwa;
                            med_energia += date.y[date.cont];/*acumula a energia para gerar uma media para o arquivo */
                            date.cont++;
                            
                            if(aux != inicial){
                                Nos[aux].sendData = 1;
                                Nos[aux].color = 3;
                            }
                            if((int)Nos[aux].mtDijks[final][N+1] == -1){
                                if(Nos[aux].countRoutes == 0)
                                    printf("Rede desconexa! %d\n",aux);
                                else
                                    printf("Numero total de rotas = %d\n", Nos[aux].countRoutes);
                                aux = final;
                                //grava_medias_dijkstra();
                            } else{
                                if(aux == inicial){
                                    Nos[aux].countRoutes++;
                                }
                                aux = (int)Nos[aux].mtDijks[final][N+1];
                            }
                        }
                        

                    }
                    if(date.cont != 0){
                        date.x[date.cont] = (double)Nos[aux].Id;
                        date.y[date.cont] = (double)Nos[aux].Pwa;
                        med_energia += date.y[date.cont];

                        date.cont++;

                        med_dispositivo += date.cont;
                        num_ciclos++;
                    }
                } else {
                    puts("final descarregado!\n");
                    grava_medias_dijkstra();
                }
            } else {
                puts("inicial descarregado!!\n");
                grava_medias_dijkstra();
            }

            sprintf(nome_arq,"histog.txt");
            
            if((fp = fopen("hitog.txt", "w")) == NULL)
            {
                puts("Erro na abertura do arquivo de log para constru��o do caminho!\n");
            }

            for(j = 0; j < date.cont; j++){
                fprintf(fp, "%.1f %.1f\n",date.x[j], date.y[j]);
            }
            fclose(fp);

            gnuplot_resetplot(graph);
            if(date.cont != 0){
                gnuplot_cmd(graph, "set style data histograms");
                gnuplot_cmd(graph, "set yrange[0:100]");
                gnuplot_set_xlabel(graph, "ID node");
                gnuplot_set_ylabel(graph, "Energy");
                gnuplot_setstyle(graph, "fill solid");
                sprintf(msg,"plot './histog.txt' using 2:xtic(1) title 'Trafego de %i para %i'", inicial, final);
                gnuplot_cmd(graph, msg);

                
            }else{
                gnuplot_cmd(graph, "plot title 'no data'");
            }

            click = -1;
        }
    }

    gnuplot_close(graph);

    free(date.x); 
    free(date.y);
}

/*funcao para gravar a matriz de distancia do tipo float em um arquivo de nome igual a file_name */
void file_rec_matrix(char *file_name, float **mt){
    char fname[20];
    FILE *fp;/*Arquivo para geração de logs*/
    int i, j;

    sprintf(fname,"%s.txt", file_name);
    if((fp = fopen(fname, "w")) == NULL)
    {
        puts("Erro na abertura do arquivo de log para construcao do caminho!\n");
        exit(1);
    }

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            fprintf(fp, "%.3f ",mt[i][j]);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}

/*funcao para gravar a matriz de dijkstra de cada dispositivo 
do tipo float em um arquivo de nome igual a file_name */
void file_rec_matrix_dij(char *file_name){
    char fname[20];
    FILE *fp;/*Arquivo para geração de logs*/
    int i, j,l;

    sprintf(fname,"%s.txt", file_name);
    if((fp = fopen(fname, "w")) == NULL)
    {
        puts("Erro na abertura do arquivo de log para construcao do caminho!\n");
        exit(1);
    }

    for(l =0; l < N; l++){
        fprintf(fp, "Noh %d\n\n",l); 
        for(i = 0; i < N; i++){
            for(j = 0; j < N; j++){
                fprintf(fp, "%.3f ",Nos[l].mtDijks[i][j]);
            }
            fprintf(fp,"\n");
        }
    }
    fclose(fp);
}

/*funcao para gravar a matriz de metricas do tipo float
 de cada dispositivo em um arquivo de nome igual a file_name */
void file_rec_matrix_met(char *file_name){
    char fname[20];
    FILE *fp;/*Arquivo para geração de logs*/
    int i, j,l;

    sprintf(fname,"%s.txt", file_name);
    if((fp = fopen(fname, "w")) == NULL)
    {
        puts("Erro na abertura do arquivo de log para construcao do caminho!\n");
        exit(1);
    }

    for(l =0; l < N; l++){
        fprintf(fp, "Noh %d\n\n",l); 
        for(i = 0; i < 2; i++){
            for(j = 0; j < N; j++){
                fprintf(fp, "%.3f ",Nos[l].mtAdjMet[i][j]);
            }
            fprintf(fp,"\n");
        }
    }
    fclose(fp);
}

void init(void){

    /*Obtencao do tempo de execucao*/
    struct timeval time_inicial, time_final;
    double tIni, tFim;
    float **MatrizDistancia;
    int i, j, l;

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_POINT_SMOOTH);

    Nos = alocaVetorNode(N);
    MatrizDistancia = alocaMatrizf(N, N);

    inicializaNos(N, Nos, N);
    inicializaMatrizf(N, N, 0.0, MatrizDistancia);

    gettimeofday(&time_inicial,NULL);
    espalhaDisp(Nos, N, ax, ay);
    gettimeofday(&time_final,NULL);  

    gettimeofday(&time_inicial,NULL);
    procuraConexoes(Nos, N, MatrizDistancia);
    gettimeofday(&time_final,NULL);

    conheceAdjacente(N, MatrizDistancia, Nos);
    conhece_metrica(N, Nos);

    gettimeofday(&time_inicial, NULL);
    tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;
    for(j = 0; j < N; j++)
    {
        dijkstra(&Nos[j], Nos, N);
    }
    gettimeofday(&time_final, NULL);
    tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;

    qt_temp_dijks += (tFim - tIni);
    qt_temp_dijks++;

    file_rec_matrix("MatrizDistancia", MatrizDistancia);
    file_rec_matrix_dij("Dijkstra");
    file_rec_matrix_met("Metricas");

}

/*funcao que de fato cria os dispositvos(circulos)*/
void desenharCirculo(GLint x, GLint y, GLint raio, int num_linhas, double R, double G, double B) { 
	double angle; 
	int i,j; 

	glColor3f(R,G,B);
	glBegin(GL_LINE_LOOP); 
		for (i = 0; i < num_linhas; i++) {
			angle = 2*PI*i/num_linhas;
			glVertex2f((cos(angle)*raio) + x, (sin(angle)*raio) + y);
		}
	glEnd();
} 

void organiza()
{
    int i; 
    
    for(i = 0; i < N; i++)
    {
        switch(Nos[i].color){
            case 0:/*dispositivo qualquer, cor preta */
                desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 0.0, 0.0, 0.0);
            break;

            case 1:/*dispositivo inicial, cor verde */
                desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 0.0, 1.0, 0.0);
            break;

            case 2:/*dispositivo final, cor azul */
                desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 0.0, 0.0, 1.0);
            break;

            case 3: /*dispositivo da rota dos dados, branco */
                desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 1.0, 1.0, 1.0);
            break;

            case 4:/*dispositivo descarregado, vermelho */
                desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 1.0, 0.0, 0.0);
            break;
        }
    }
}

/*funcao para analisar e executar uma acao correspondente as opcoes do menu */
void GoMenu(int value){

    switch(value){
        case 1:
            Nos[dispMenu].color = 1;
            if( click == -1){
                inicial = dispMenu;
                click++;
            }else{
                Nos[inicial].color = 0;
            }
        break;

        case 2:
            if(click == 0){
                Nos[dispMenu].color = 2;
                click++;
            }
        break;

        case 3:
            Nos[dispMenu].Pwa = Nos[dispMenu].Pwi;

    }
    dispMenu = -1;
    glutPostRedisplay();
}

/*funcao para criacao do menu se o usuario clicar em um dispositivo */
void menu(){
    int sub1;
    
    printf("click\n");
    glutCreateMenu(GoMenu);
    glutAddMenuEntry("Define source", 1);
    glutAddMenuEntry("Define sink", 2);
    glutAddMenuEntry("Recharge device", 3);
    glutAttachMenu(GLUT_LEFT_BUTTON);
}

// Função callback chamada para gerenciar eventos do mouse
void GerenciaMouse(int button, int state, int x, int y)
{
	int i = 0, x1, y1;
	int encontrou = 0;
	float distancia;
	
	x1 = x;
	y1 = ay - y;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){  
         while(i < N && !encontrou){
			distancia = calcDist(x1, y1, Nos[i].x, Nos[i].y);
			if(distancia <= 5.0){
                printf("click");
			    encontrou = 1;
                dispMenu = i;
                
                menu();
			 	//desenharCirculo((GLint)Nos[i].x, (GLint)Nos[i].y, 5,10, 0.0, 1.0, 0.0);
			}
			i++;
         }
    }
}

// Função callback chamada para fazer o desenho
void Desenha(void)
{
    glClearColor(0.3, 0.3, 0.3, 0); // sets the backgraound color to black
    glClear(GL_COLOR_BUFFER_BIT); // clears the frame buffer and set values defined in glClearColor() function call
    glLoadIdentity();

	gluOrtho2D(0.0, ax, 0.0, ay);
	//Limpa a janela de visualização com a cor de fundo especificada 

	organiza();
	
    //glutSwapBuffers();
    glFlush();
}




int main(int argc, char **argv)
{

    pthread_t threads;
    int rc,i;
    /*variáveis para thread de teste*/

    if(argc != 4)
    {
        printf(" Error\n Forma de execucao: %s <num_sensores> <inicial_int> <final_int>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);
    inicial = atoi(argv[2]);
    final = atoi(argv[3]);

    printf("%d %d %d", N, inicial, final);

    glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE );
	glutInitWindowSize(ax,ay);
	glutCreateWindow("Dijkstra Based Router Protocol");
	glutDisplayFunc(Desenha);
    glutIdleFunc(Desenha);
    init();
    //glutMouseFunc(GerenciaMouse);
    rc = pthread_create(&threads, NULL, print_time, NULL);
    
    if(rc){
        printf("Error, return code from pthread_create() is %d\n",rc);
        exit(-1);
    }
    
   	glutMainLoop();

    pthread_join(threads, NULL);
    free(Nos);

    return 0;
}


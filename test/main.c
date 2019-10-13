/*"This software uses the gnuplot_i library written by N.Devillard" */

#include "router.h"
#include "../src/gnuplot_i.h"
#include <pthread.h>
#include <GL/glut.h>

#define PI 3.14159265358979323846 

/*lista para obter as rotas encontradas, utilizando uma matriz
2xN, na qual a primeira linha representa os id's/ip's dos 
 dispositivos, e a segunda linha representa as baterias dos dispositivos*/
typedef struct routes{
    double id;
    double energy;
    int fim; /*1 para fim do primeiro caminho, 0 para dispositivo intermediario
            assim podemos fazer graficos limitados, definir quando parar de procurar,
            ou pode ser desnecessario em alguns momentos. */
    struct routes * prox; 
} ROTAS;

NODE *Nos;
int N;/*representa a quantidade de dispositivos */
int dispMenu = -1;/*representa o dispositivo que foi clicado */
int inicial = -1, final = -1;/*representa o dispositivo inicial e final */
int ax = LARGURA_TELA - 10, ay = ALTURA_TELA - 10; /*representa os eixos x e y do plano do opengl. Ele nao eh o tamanho da janela */
int click = -1;/*representa o dispositivo eu devo atribuir a cor de inicial ou final
            se -1 -> nao foi atribuido o incial
            se 0 -> ja foi atribuido o inicial, resta o dispositivo final
            se 1 -> os dois dispositivos ja foram definidos e pode iniciar o roteamento */

/*médias  */
double med_temp_dijks = 0.0;
int qt_temp_dijks = 0;

int med_energia = 0; /*media das energias utilizadas no caminho */
int med_dispositivo = 0;/*media da quantidade de dispositivo utilizado no caminho */
int med_caminhos = 0;
int num_ciclos = 0; /*representa o numero de vezes que os caminhos poderam ser encontrados com sucesso */

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
    double media_energ = 0.0;
    double media_dispos = 0.0; 
    double media_caminhos = 0.0;
                            

    if(num_ciclos){
        media_dispos = med_dispositivo/num_ciclos;
        media_caminhos = med_caminhos/num_ciclos;
    }
    else{
        puts("num_ciclo == 0");
    }

    if(med_dispositivo)
        media_energ = med_energia/med_dispositivo;
    else
        puts("med_dispositivo == 0");

    if((fp = fopen("./dados/media_energia_dispositivo.txt", "a+")) == NULL)
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
    
    fprintf(fp,"%.6f %.6f %.6f %d\n",media_caminhos, media_dispos, media_energ, num_ciclos);

    fclose(fp);
}

/*=======================================GRAVACAO DA MEDIA DE TEMPO DE EXECUCAO PARA O ALGORITMO DE DIJKSTRA================================= */

void grava_medias_dijkstra()
{
    FILE *fp;
    int qtbarra_ene;
    char arq;
    double media;

    if(qt_temp_dijks)
        media = med_temp_dijks/qt_temp_dijks;
    else
        puts("zerooo!");
    

    if((fp = fopen("./dados/media_exec_dijks.txt", "a+")) == NULL)
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
    
    fprintf(fp,"%.6f\n", media);

    fclose(fp);

    grava_energia();
}

void teste(ROTAS *p){
    puts("encontrou rota\n");

    while(p != NULL){
        printf("(%.3f , %.3f) ",p->id, p->energy);
        if(p->fim && p->prox != NULL)
            puts("outro caminho\n");
        p=p->prox;
    }
}

void *print_time(void *args){

    struct timeval time_inicial, time_final;
    double tIni, tFim;
    int i, j, aux;
    int cargaRetirada; /*potencia de carga que será retirado da bateria residual*/
    gnuplot_ctrl **graph; /*para funcionar o grafico, basta descomentar tudo que tem relação com graph */
    char msg[100]; /*msg para enviar no plot*/
    FILE *fp;
    char arq;
    int qtbarra_ene;
    int num_arq;
    char nome_arq[20];
    int med_aux_local;
    ROTAS *rotas; /*obtem os caminhos encontrados */
    ROTAS *auxRotas; 
    /*variaveis para contar a quantidade de caminhos e auxiliar 
    na plotagem do grafico */
    int total_caminhos = 0;
    int conta_dispositivos;
    int multicaminhos = 1; /*identifica se o algoritmo a ser executado é o multicaminhos ou o de caminho unico, incialmente 
                            eh o algoritmo de caminhos multiplos */
    int controla_qt_caminhos = 0; /*ele controla a quantidade de caminhos para limitar para apenas um caminho quando nao for o 
                            o algoritmo de caminhos multiplos que estiver executando, inicialmente nao necessita de controle
                            pois o primeiro algoritmo a iniciar eh o de caminhos multiplos */
    DATE_THREAD date; /*utilizado pelo caminho unico para salvar os dados e plotar */

    printf("Resposta para rota em dijkstra:::::\n");

    if(Nos[inicial].mtDijks[final][N+1] < 0.0){//o N+1 representa o proximo dispositivo, se ele é -1 significa que nao há rotas até o proximo nó -> rede desconexa
        printf("Falha na comunicação com o dispositivo de id = %i",final);
        Nos[inicial].color = 4;
        grava_medias_dijkstra();
    }

    num_arq = 0;
    rotas = NULL; /*como nao foi encontrado nenhum caminho, inicializamos a variavel */

    while(1) {

        // to refresh the window it calls display() function
        glutPostRedisplay();

        gettimeofday(&time_inicial, NULL);
        tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;

        tFim = 0;
        while (tFim - tIni < 3.0) { 
            gettimeofday(&time_final, NULL);
            tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;
        }

        multicaminhos = 1;
    
        click = 1;
        if(click == 1){           
            inicializa_color(N, Nos);
            Nos[inicial].color = 1;
            Nos[final].color = 2;
            for (i = 0; i < N; i++) {
                cargaRetirada = rand() % 8;
                if(Nos[i].Pwa - cargaRetirada < 0){
                    Nos[i].Pwa = 0;
                    Nos[i].color = 4;
                }else{
                    Nos[i].Pwa -= cargaRetirada;
                    Nos[i].sendData = 0;
                }
            }
            conhece_metrica(N, Nos);/*Conhece a nova metrica dos dispositivos*/

            for(i = 0; i < N; i++){
                inicializaMatrizf(N, N+2, -1.0, Nos[i].mtDijks);
            } 

            for(i = 0; i < total_caminhos; i++)
            {
                gnuplot_close(graph[i]);
            }

            if(total_caminhos != 0)
                free(graph);

            total_caminhos = 0;

            gettimeofday(&time_inicial, NULL);
            tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;
            for (i = 0; i < N; i++) {
                if(Nos[i].Pwa > 0) /*executa o dijkstra somente para os sensores com 
                                    bateria positiva */
                    dijkstra(&Nos[i], Nos, N);
            }
            gettimeofday(&time_final, NULL);
            tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;

            med_temp_dijks += (tFim - tIni); /*variavel global, para acumular o tempo da execucao do algoritmo de dijkstra e calculo de rota */
            qt_temp_dijks++;/*variavel global para controlar quantas vezes foi rodado o algoritmo e encontrado o caminho, consideramos como quantidade de ciclos tambem */

            while(Nos[inicial].mtDijks[final][N+1] != -1 && multicaminhos == 1){
                printf("Resposta para rota em dijkstra:::::\n");
                total_caminhos++;  
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
                        if((int)Nos[aux].mtDijks[final][N+1] == -1){
                            teste(auxRotas);
                            if(Nos[aux].countRoutes == 0)
                                printf("Rede desconexa! %d\n",aux);
                            else
                                printf("Numero total de rotas = %d\n", Nos[aux].countRoutes);
                            aux = final;
                            //grava_medias_dijkstra();
                        } else{
                            if(rotas == NULL){
                                rotas = malloc(sizeof(ROTAS));
                                rotas->prox = NULL;
                                auxRotas = rotas;
                            }
                            else{
                                rotas->prox = malloc(sizeof(ROTAS));
                                rotas = rotas->prox;
                                rotas->prox = NULL;
                            }
                            
                            rotas->id = (double)Nos[aux].Id; /*double pois eh o que o grafico pede */
                            rotas->energy = (double)Nos[aux].Pwa;
                            rotas->fim = 0;

                            if(rotas->id != inicial && rotas->id != final)/*somo as baterias dos dispositivos inicial e final somente apos sair do laco mais externo */
                                med_energia += rotas->energy;/*acumula a energia para gerar uma media para o arquivo */

                            /*se o dispositivo analisado eh o inicial, entao encontramos uma rota a mais */
                            if(aux == inicial){
                                Nos[aux].countRoutes++;
                            }else{
                                Nos[aux].sendData = 1;
                                Nos[aux].color = 3;
                            }

                            aux = (int)Nos[aux].mtDijks[final][N+1];

                            if(aux == final){
                                rotas->prox = malloc(sizeof(ROTAS));
                                rotas = rotas->prox;
                                rotas->prox = NULL;

                                rotas->id = (double)Nos[aux].Id; /*double pois eh o que o grafico pede */
                                rotas->energy = (double)Nos[aux].Pwa;
                                rotas->fim = 1;
                                med_energia += rotas->energy;
                                if(controla_qt_caminhos)
                                    multicaminhos = 0;
                            }
                        }
                    }

                }

                conhece_metrica(N, Nos);/*Conhece a nova metrica dos dispositivos*/

                for(i = 0; i < N; i++){
                    inicializaMatrizf(N, N+2, -1.0, Nos[i].mtDijks);
                }

                if(multicaminhos){ /*caso for o algoritmo de multicaminhos que esta executando, procuramos mais caminhos e guardamos o tempo */
                    gettimeofday(&time_inicial, NULL);
                    tIni = (double) time_inicial.tv_usec / 1000000 + (double) time_inicial.tv_sec;
                    for (i = 0; i < N; i++) {
                        if(Nos[i].Pwa > 0)
                            dijkstra(&Nos[i], Nos, N);
                    } 

                    gettimeofday(&time_final, NULL);
                    tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;
                }

                med_temp_dijks += (tFim - tIni); /*variavel global, para acumular o tempo da execucao do algoritmo de dijkstra e calculo de rota */
                qt_temp_dijks++;/*variavel global para controlar quantas vezes foi rodado o algoritmo e encontrado o caminho, consideramos como quantidade de ciclos tambem */
            }

            if(total_caminhos == 0) {
                if(Nos[inicial].Pwa == 0) {
                    puts("\nDispositivo inicial descarregado");
                } else {
                    if (Nos[final].Pwa == 0)
                        puts("\nDispositivo final descarregado");
                    else
                        puts("\nRede desconexa");
                } 
                    
                //grava_medias_dijkstra();
                controla_qt_caminhos = 1;

                for(i = 0; i < N; i++){
                    Nos[i].Pwa = Nos[i].auxPwa;
                    Nos[i].Pwi = Nos[i].auxPwi;
                }
            }
            
            num_ciclos++;
            rotas = auxRotas;
            conta_dispositivos = 0;
            while(rotas != NULL){
                rotas = rotas->prox;
                conta_dispositivos++;
            }
            
            med_caminhos += total_caminhos;
            conta_dispositivos -= (total_caminhos - 1)*2; /*conto somente 1 vez o dispositivo inicial e o final */

            med_dispositivo += conta_dispositivos;
            /*unica vez que somo as baterias do dispositivo inicial e do final eh aqui */
            med_energia += Nos[inicial].Pwa;
            med_energia += Nos[final].Pwa;
            

            conta_dispositivos = 0;

            rotas = auxRotas;

            //arquivo para mostrar o grafico de baterias comentado
            
            for(i = 0; i < total_caminhos; i++){

                sprintf(nome_arq,"histograma/histog_%d.txt",i);
            
                if((fp = fopen(nome_arq, "w")) == NULL)
                {
                    puts("Erro na abertura do arquivo de log para constru do caminho!\n");
                }

                while(!rotas->fim){
                    fprintf(fp, "%.1f %.1f\n",rotas->id, rotas->energy);
                    rotas=rotas->prox;
                    conta_dispositivos++;
                }

                fprintf(fp, "%.1f %.1f\n",rotas->id, rotas->energy);
                conta_dispositivos++;
                if(rotas->prox != NULL)
                    rotas=rotas->prox;

                fclose(fp);
            }
            
            graph = malloc(sizeof(gnuplot_ctrl*) * total_caminhos);

            for(i = 0; i < total_caminhos; i++)
                graph[i] = gnuplot_init();


            for(i = 0; i < total_caminhos; i++){
                sprintf(nome_arq,"histog_%d.txt",i);
                gnuplot_resetplot(graph[i]);
                if(auxRotas != NULL){
                    gnuplot_cmd(graph[i], "set style data histograms");
                    gnuplot_cmd(graph[i], "set yrange[0:100]");
                    gnuplot_set_xlabel(graph[i], "ID node");
                    gnuplot_set_ylabel(graph[i], "Energy");
                    gnuplot_setstyle(graph[i], "fill solid");
                    gnuplot_cmd(graph[i],"set key above right");
                    sprintf(msg,"plot './histograma/histog_%d.txt' using 2:xtic(1) title 'Trafego de %i para %i -> %s' ",i, inicial, final,nome_arq);
                    gnuplot_cmd(graph[i], msg);

                    
                }else{
                    gnuplot_cmd(graph[i], "plot title 'no data'");
                }
            }

            click = -1;
        }
        
    }

    while(auxRotas->prox != NULL){
        rotas = auxRotas->prox;
        free(auxRotas);
        auxRotas=rotas;
    }

    free(auxRotas);
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

    gettimeofday(&time_final, NULL);
    tFim = (double) time_final.tv_usec / 1000000 + (double) time_final.tv_sec;

    med_temp_dijks += (tFim - tIni); /*variavel global, para acumular o tempo da execucao do algoritmo de dijkstra e calculo de rota */
    qt_temp_dijks++;/*variavel global para controlar quantas vezes foi rodado o algoritmo e encontrado o caminho, consideramos como quantidade de ciclos tambem */

    /*file_rec_matrix("MatrizDistancia", MatrizDistancia);
    file_rec_matrix_dij("Dijkstra");
    file_rec_matrix_met("Metricas");*/

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


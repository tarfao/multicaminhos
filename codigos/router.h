//
// Created by root on 17/06/19.
//

#include "created_init.h"


//======================================= CONFIGURA A DISPOSI��O DOS NOHS =====================================

float calcDist(int x, int y, int x1, int y1){
    float soma1, soma2, resultado;

    soma1 = x1 - x;
    soma1 = pow(soma1, 2.0);

    soma2 = y1 - y;
    soma2 = pow(soma2, 2.0);

    resultado = soma1+soma2;
    resultado = pow(resultado, 0.5);

    return resultado;
}

/*
OBJETIVO: VERIFICA SE AS COORDENADAS J� EST�O SENDO USADA,
EM CASO AFIRMATIVO PROCURA UMA COORDENADA V�LIDA E RETORNA
*/
void verificaCoordenadas (NODE *conjNos, NODE *No, int dimX, int dimY, int N)
{
    int i;
    float distancia = 0;
    int conectado = 0;/*supomos que a rede estah inicialmente conexa*/
    int xmax;
    int ymax;
	int cont = 0;

	/*enquanto o grafo nao for conexo, e os dispositivos nao estiverem a uma distancia de pelo menos 10px de cada um nao sai do laco*/
	if(N > 0){
		while(!conectado){
			i = 0;
			while(i < N)
			{
				if (No->x == conjNos[i].x && No->y == conjNos[i].y)
				{
				    i = 0;
			/*gera numeros entre xmin - xmax*/
				    No->x = (rand() % (dimX - 10)) + 5;
				    No->y = (rand() % (dimY - 10)) + 5;
				}else
				{
				    distancia = calcDist(conjNos[i].x, conjNos[i].y, No->x, No->y);
				    if(distancia <= raio && distancia >= 20){/* 20 <= distancia <= raio para nao ficar sobreposto*/
				    	i = N;
				    	conectado = 1;
				    }
				    else{
				    	i++;
				    }
				}
			}
			
			if(conectado){
				conectado = 0;
				i = 0;
				while(i < N && !conectado){
					distancia = calcDist(conjNos[i].x, conjNos[i].y, No->x, No->y);
					if(distancia < 20.0){
		  	            No->x = (rand() % (dimX - 10)) + 5;
						No->y = (rand() % (dimY - 10)) + 5;
						conectado = 1;
				    }else{
				    	i++;
				    }
				}
				
				if(!conectado){
					conectado = 1;
				}
				else{
	 	            No->x = (rand() % (dimX - 10)) + 5;
					No->y = (rand() % (dimY - 10)) + 5;
					conectado = 0;
				}
			}else{
				No->x = (rand() % (dimX - 10)) + 5;
				No->y = (rand() % (dimY - 10)) + 5;
			}
		}
	}
}

/*
FUN��O: ESPALHAR OS NOHS EM UM PLANO 2D, COM DIMENSOES MAXIMAS dimX E dimY
*/
void espalhaDisp (NODE *conjNos, int N, int dimX, int dimY){
    NODE no1;
    int i;

    for(i = 0; i < N; i++){
        no1.x = (rand() % (dimX - 10)) + 5; /*gera numeros entre xmin - xmax*/
        no1.y = (rand() % (dimY - 10)) + 5;
        verificaCoordenadas(conjNos, &no1, dimX, dimY, i);
        conjNos[i].x = no1.x;
        conjNos[i].y = no1.y;
    }
}
//======================================= FIM DAS DISPOSI��ES DOS NOHS ===========================================

//======================================= CONFIGURA AS LIGA��ES DOS NOHS =========================================
void defineConexoes(NODE *conjNos, int pos, int N, float **matriz){
    float distancia;
    int i = 0;
    NODE *aux;

    for(i = 0; i < N; i++){
        if(i != pos){
            distancia = calcDist(conjNos[pos].x, conjNos[pos].y, conjNos[i].x, conjNos[i].y);
            if(distancia <= raio){
                matriz[pos][i] = distancia;
                matriz[i][pos] = distancia;
            }
        }
    }
}

void procuraConexoes (NODE *conjNos, int N, float **matriz){
    int i;

    for(i = 0; i < N; i++){
        defineConexoes(conjNos, i, N, matriz);
    }
}
//======================================= FIM DAS CONFIGURA��ES DE LIGACOES  =====================================

/*======================================= PREPARANDO OS N�S PARA REALIZAREM ROTEAMENTO ===========================*/

/*OBJETIVO: FAZER COM QUE CADA N� CONHE�A SEUS VIZINHOS*/
void conheceAdjacente(int N, float **matriz, NODE *conjNos){
    int i, j;

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            conjNos[i].mtAdjMet[0][j] = matriz[i][j];
        }
    }
}

float calcula_metrica(NODE *no, int adj){

    float divisao, denominador, numerador;

    divisao = ((float)no->Pwa / (float)no->Pwi);
    denominador = pow(divisao,1.0/3.0);
    numerador = pow(no->mtAdjMet[0][adj], 2.0)*0.00025;//o raio est� com 50

    if(denominador > 0.0)
        return numerador/denominador;
    else
        return 0.0;
}

/*Objetivo: CONHECER A M�TRICA DE CADA N� VIZINHO*/
void conhece_metrica(int N, NODE *conjNos){
    int i, j;
    float metrica;

    for(i = 0; i < N; i++){
        for(j = 0; j < N; j++){
            if(conjNos[i].mtAdjMet[0][j] > 0){
                metrica = calcula_metrica(&conjNos[j], i);
                conjNos[i].mtAdjMet[1][j] = metrica;
            }else{
                conjNos[i].mtAdjMet[1][j] = 100.0;
            }
        }
    }
}
//======================================= FIM DAS CONFIGURAÇÕES DE LIGACOES  =====================================

/* ==================================== INICIA AS TABELAS DE ROTEAMENTOS ========================================*/

//essa funcao verifica se o nó pertence a fila de visitados
//ele retorna 1 caso positivo, e 0 caso negativo
int pertenceVisitadosFila (ENCADEADA *Visitados, NODE *NO)
{
    ENCADEADA *run;

    //percorremos a lista de visitados para ver se não já foi veriricado
    run = Visitados;

    while(run!= NULL)
    {
        if(run->id == NO->Id)
        {
            return 1;
        }else{
            run = run->prox;
        }
    }
    return 0;
}

//objetivo: adicionar o dispositivo No(varíável) a lista de visitados
void AdcListaVisitados(ENCADEADA **visitados, ENCADEADA *No)
{
    ENCADEADA *run;

    run = *visitados;
    if(run != NULL){
        while(run->prox != NULL) run = run->prox;
        No->prox = NULL;
        run->prox = No;
    }
    else{
        *visitados = No;
    }


}

void se_existe_remove(ENCADEADA **pt, int id){

    ENCADEADA *del;

    if(*pt != NULL){
        if((*pt)->id == id){
            del = (*pt);
            *pt = del->prox;
            free(del);
        }else{
            se_existe_remove(&(*pt)->prox, id);
        }
    }
}

/*Fila de prioridade mínima
Está ordenada pela menor distância,
se houver distâncias iguais, o primeiro
dos iguais será o primeiro que entrou.

 OBJETIVO: CONTINUAR COM A PROPRIEDADE DA FILA, ADICIONANDO UM NÓ
 EM SUA POSIÇÃO, E CASO ESSE NÓ JÁ PERTENCE A FILA (POIS ADICIONAMOS ELE NOVAMENTE
 QUANDO ENCONTRAMOS UM NOVO CAMINHO MENOR) TEMOS QUE DELETAR O ANTIGO, ENTÃO ADICIONAMOS O
 NOVO, E REMOVEMOS O ANTIGO*/
void adc_fila(ENCADEADA **fila, int id, float d){
    ENCADEADA *novo;

    if(*fila == NULL){//se nao tiver nada na fila, eu aloco o primeiro nó
        assert(((*fila)=malloc(sizeof(ENCADEADA))) != NULL);
        if((*fila)!=NULL)/*seguranca*/
        {
            (*fila)->id=id;
            (*fila)->d=d;
            (*fila)->prox=NULL;
        }else{
            printf("Abort!\n");
            exit(1);
        }

    }else{
        if((*fila)->d <= d){
            adc_fila(&(*fila)->prox, id, d);
        }else{
            novo = malloc(sizeof(ENCADEADA));
            novo->id = id;
            novo->d = d;
            novo->prox = *fila;
            *fila = novo;

        }

    }
}

ENCADEADA* remove_min(ENCADEADA **fila){
    ENCADEADA *F;

    F = (*fila);
    (*fila) = (*fila)->prox;

    return F;
}

void inicia_d_p(float *d, int *p, int id, int N){
    int i;

    for(i = 0; i < N; i++){
        d[i] = 10000.0;
        p[i] = -1;
    }
    d[id] = 0.0;
}

void dijkstra(NODE *INI, NODE *conjNos, int N){
    ENCADEADA *FILA;
    ENCADEADA *u;
    ENCADEADA *visitados;
    ENCADEADA *ax;
    FILE *fp;

    int i, aux;
    float d[N];//representa a distancia acumulada, iniciada com 10000,
    //pois a probabilidade de ter essa distancia é muito baixa, pois é um número alto.
    // O inicial é 0;
    int p[N];//representa o predecessor, iniciada com -1, pois nenhum pai terá o id -1;
    float metrica;
    int pos_u, pos_pai;//obtem a posição de u e do pai

    inicia_d_p(d, p, INI->Id, N);/*inicializacao semelhante ao dijkstra padrao*/
    FILA = NULL;
    visitados = NULL;
    adc_fila(&FILA, INI->Id, d[INI->Id]);
    if(FILA == NULL){
        printf("Fila nao alocada!");
        exit(2);
    }

    /*if((fp = fopen("algoDijks", "a+")) == NULL)
    {
        puts("erro na abertura/criacao do arquivo algoDijks");
        exit(2);
    }*/

    //fprintf(fp, "Anahlise noh %d com predecessor = %d\n", pos_u, p[pos_u]);

    while(FILA != NULL){
        u = remove_min(&FILA); /*remove da fila o sensor de menor metrica */
        pos_u = u->id; /*salvo o identificador dele, que representa a sua posicao no vetor */
        for(i = 0; i < N; i++){
            if(conjNos[u->id].mtAdjMet[1][i] > 0.0 /*metrica maior que 0 sao vizinhos */ && conjNos[u->id].mtAdjMet[1][i] < 100.0 && !pertenceVisitadosFila(visitados, &conjNos[i]) && conjNos[i].sendData == 0){
                metrica = conjNos[pos_u].mtAdjMet[1][i];

                /*fprintf(fp,"Noh = %d Pwa = %d Pwi = %d dist = %.3f\n", i, conjNos[i].Pwa, conjNos[i].Pwi, conjNos[pos_u].mtAdjMet[0][i]);*/
                
                if(d[i] > d[pos_u] + metrica){
                    if(p[i] != -1){//aqui significa que ele possuia um pai, e o seu pai deverá ser corrigido
                        //pois uma distância menor foi encontrada
                        aux = p[i];
                        INI->mtDijks[aux][i] = 0.0;
                    }
                    if(u->id == INI->Id){//se for o primeiro que estamos analisando, o destino são os próprios filhos.
                        //A posição N+1 é o destino do pacote
                        INI->mtDijks[i][N+1] = i;
                    }else{//o destino do pacote será o mesmo destino do pai dele, ou seja, o nó atual que está analisando
                        INI->mtDijks[i][N+1] = INI->mtDijks[pos_u][N+1];
                    }
                    INI->mtDijks[i][N] = u->id;//identifica o predecessor dele, (FAMOSO PI DE U)

                    //atualiza as variáveis internas da função
                    d[i] = d[pos_u] + metrica;
                    p[i] = pos_u;

                    INI->mtDijks[pos_u][i] = d[i];//atualiza a matriz de u, adicionando a distancia acumulada para o destino i

                    se_existe_remove(&FILA, i);

                    adc_fila(&FILA, i, d[i]);//adiciona i na fila, (i representa não só a posição mas também é o identificador do nó)

                }
            }
        }
        AdcListaVisitados(&visitados, u);
    }
    //fprintf(fp,"==================================================\n");

//    fclose(fp);

}
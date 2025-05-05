#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM char mem[512][17] = {{'\0'}}
#define REGISTRADOR int registrador[8] = {0}
#define PC int pc = 0

//STRUCTS e ENUMS
typedef enum tipo {
        Tipo_R=0,
        Tipo_I=1,
        Tipo_J=2,
        Tipo_OUTROS=3
} Tipo_Instrucao;

typedef struct instrucao {
        char opcode[5];
        char rs[4];
        char rt[4];
        char rd[4];
        char funct[4];
        char imm[7];
        char addr[8];
} Instrucao;

typedef struct decodificador {
        int opcode;
        int rs;
        int rt;
        int rd;
        int funct;
        int imm;
        int addr;
        Tipo_Instrucao tipo;
} Decodificador;

typedef struct Nodo {
        int ra[8];
        int da[256];
        int pca;
        struct Nodo *prox;
} Nodo;

typedef struct pilha {
        Nodo *topo;
} Pilha;

void inicia_pilha(Pilha *p);
void menu();
int carrega_mem(char mem[512][17]);
void print_mem_dat(char mem[512][17]);
void print_mem_inst(char mem[512][17]);
void printReg(int *reg);
int executa_step(char mem[512][17],Instrucao *in,Decodificador *d,int *pc,int *registrador,Pilha *p,int est);
void empilha(Pilha *p, int *r,char m[][17], int *pc);
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);

int main() {
        Instrucao in;
        Decodificador d;
        Pilha p;
        MEM;
        REGISTRADOR;
        PC;
        int op,resul,est = 0;
        inicia_pilha(&p);

        do {
                menu();
                printf("Sua escolha: ");
                scanf("%d", &op);
                printf("\n");
                switch (op) {
                case 1:
                        carrega_mem(mem);
                        break;
                case 2:
                        print_mem_inst(mem);
                        print_mem_dat(mem);
                        break;
                case 3:
                        printReg(registrador);
                        break;
                case 4:
                        print_mem_inst(mem);
                        print_mem_dat(mem);
                        printReg(registrador);
                        printf("\n\nPC: %d", pc);
                        break;
                case 5:
                        est = executa_step(mem,&in,&d,&pc,registrador,&p,est);
                        break;
                case 6:
                        printf("Voce saiu!!!");
                        break;
                default:
                        printf("Opcao invalida!");
                }
        } while(op != 6);
        return 0;
}

void inicia_pilha(Pilha *p) {
        p->topo=NULL;
}

void menu() {
        printf("\n\n *** MENU *** \n");
        printf("1 - Carregar memoria");
        printf("2 - Imprimir memoria\n");
        printf("3 - Imprimir banco de registradores\n");
        printf("4 - Imprimir todo o simulador\n");
        printf("5 - Executar step\n");
        printf("6 - Executar programa\n");
        printf("7 - Salvar .asm\n");
        printf("8 - Salvar .dat\n");
        printf("9 - Volta uma instrucao\n");
        printf("10 - Sair\n\n");
}

// carrega memoria de instrucoes a partir de um "arquivo.mem"
int carrega_mem(char mem[512][17]) {
        char arquivo[20];
        // abre o arquivo em modo leitura
        printf("Nome do arquivo: ");
        scanf("%s", arquivo);
        FILE *arq = fopen (arquivo, "r");
        if (!arq)
        {
                perror ("Erro ao abrir arquivo") ;
                exit (1) ;
        }
        int i = 0;
        char linha[20]; // Buffer para leitura
        while (i < 256 && fgets(linha, sizeof(linha), arq)) {
                // Remover quebras de linha e caracteres extras
                linha[strcspn(linha, "\r\n")] = '\0';

                // Ignorar linhas vazias
                if (strlen(linha) == 0) {
                        continue;
                }

                strncpy(mem[i], linha, 16); // Copia atC) 16 caracteres
                mem[i][16] = '\0'; // Garante terminaC'C#o de string
                i++; // AvanC'a corretamente para a prC3xima posiC'C#o
        }
        while (fscanf(arq, "%s", mem[i]) != EOF) {
                i++;
        }
        fclose(arq);
}

// imprime memoria de instrucoes
void print_mem_inst(char mem[512][17]) {
        printf("\n############## INSTRUCOES ##############\n");
        for (int i = 0; i < 256; i++)
        {
                printf("\n[%d]: %s\n", i,mem[i]);
                printf("\n");
        }
}

// imprime memoria de dados
void print_mem_dat(char mem[512][17]) {
        printf("\n############## DADOS ##############\n\n");
        for(int i=256; i<512; i++) {
                printf("[%d]. %s   ", i, mem[i]);
                if (i % 8 == 7)
                {
                        printf("\n");
                }
        }
}

// imprime banco de registradores
void printReg(int *reg) {
        for(int i=0; i<8; i++) {
                printf("\nREGISTRADOR [%d] - %d", i, reg[i]);
        }
}

int executa_step(char mem[512][17], Instrucao *in, Decodificador *d, int *pc, int *registrador,Pilha *p, int est) {
        if (strcmp(mem[*pc], "0000000000000000") == 0 || *pc > 255) {
                printf("\nFim do programa!");
                return 12;
        } else {
                switch(est) {
                case 0:
                        empilha(p,registrador,mem, pc);
												return 1;
                        break;
                case 1:
            break;
                }
        }
}

void empilha(Pilha *p,int *r,char m[][17],int *pc) {
        Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
        int i;
        for(i=0; i<8; i++) {
                nNodo->ra[i] = r[i];
        }
        for(i=256; i<512; i++) {
                nNodo->da[i] = atoi(m[i]);
        }
        nNodo->pca=*pc;
        nNodo->prox=p->topo;
        p->topo=nNodo;
}
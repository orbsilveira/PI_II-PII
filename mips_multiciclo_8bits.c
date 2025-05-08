#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM char mem[256][17] = {{'\0'}}
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
	char dado[9];
} Instrucao;

typedef struct decodificador {
	int opcode;
	int rs;
	int rt;
	int rd;
	int funct;
	int imm;
	int addr;
	int dado;
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
int carrega_mem(char mem[256][17]);
void print_mem_dat(char mem[256][17]);
void print_mem_inst(char mem[256][17]);
void printReg(int *reg);
int executa_step(char mem[256][17],Instrucao *in,Decodificador *d,int *pc,int *registrador,Pilha *p,int est);
void empilha(Pilha *p, int *r,char m[][17], int *pc);
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);
int ULA_op2(int est, Decodificador *d);
int ULA(int op1,int op2,int opULA,int *flag,int *overflow);
void printInstrucao(Decodificador *d);
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d);

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
	printf("1 - Carregar memoria\n");
	printf("2 - Imprimir memoria\n");
	printf("3 - Imprimir banco de registradores\n");
	printf("4 - Imprimir todo o simulador\n");
	printf("5 - Executar step\n");
	printf("6 - Sair\n");
	//printf("6 - Executar instrucao\n");
	//printf("7 - Salvar .asm\n");
	//printf("8 - Salvar .dat\n");
	//printf("9 - Volta um step\n");
}

// carrega memoria
int carrega_mem(char mem[256][17]) {
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
	while (i < 128 && fgets(linha, sizeof(linha), arq)) {
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
void print_mem_inst(char mem[256][17]) {
	printf("\n############## INSTRUCOES ##############\n");
	for (int i = 0; i < 128; i++)
	{
		printf("\n[%d]: %s\n", i,mem[i]);
		printf("\n");
	}
}

// imprime memoria de dados
void print_mem_dat(char mem[256][17]) {
	printf("\n############## DADOS ##############\n\n");
	for(int i=128; i<256; i++) {
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

int executa_step(char mem[256][17], Instrucao *in, Decodificador *d, int *pc, int *registrador,Pilha *p, int est) {
	int flag = 0, overflow = 0;
	switch(est) {
	case 0:
		if (strcmp(mem[*pc], "0000000000000000") == 0 || *pc > 127) {
			printf("\nFim do programa!");
			return 0;
		} else {
			empilha(p,registrador,mem,pc);
			decodificarInstrucao(mem[*pc],in,d);
			*pc = ULA(*pc,ULA_op2(est,d),0,&flag,&overflow);
			printInstrucao(d);
			return 1;
		}
		break;
	case 1:
		return 0;
		break;
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

void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d) {
	copiarBits(bin, in->opcode, 0, 4);    // Copia os 4 bits do opcode (4 bits)
	d->opcode = binarioParaDecimal(in->opcode, 0);
	copiarBits(bin, in->rs, 4, 3);        // Copia os 3 bits do rs
	d->rs = binarioParaDecimal(in->rs, 0);
	copiarBits(bin, in->rt, 7, 3);        // Copia os 3 bits do rt
	d->rt = binarioParaDecimal(in->rt, 0);
	copiarBits(bin, in->rd, 10, 3);       // Copia os 3 bits do rd
	d->rd = binarioParaDecimal(in->rd, 0);
	copiarBits(bin, in->funct, 13, 3);    // Copia os 3 bits do funct
	d->funct = binarioParaDecimal(in->funct, 0);
	copiarBits(bin, in->imm, 10, 6);      // Copia os 6 bits do imm
	d->imm = binarioParaDecimal(in->imm, 1);
	copiarBits(bin, in->addr, 9, 7);     // Copia os 7 bits do addr
	d->addr = binarioParaDecimal(in->addr, 0);
}

// copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho)
{
	strncpy(destino, instrucao + inicio, tamanho); // Copia os bits desejados
	destino[tamanho] = '\0';  // Adiciona o terminador de string
}

// converte de binario para decimal
int binarioParaDecimal(const char *bin, int sinal) {
	int valor = (int)strtol(bin, NULL, 2);
	int bits = strlen(bin);

	if (sinal && bits > 0 && (valor & (1 << (bits - 1)))) {
		valor = valor - (1 << bits);
	}
	return valor;
}

int ULA_op2(int est,Decodificador *d) {
	int saida;	
	switch(est) {
		case 0:
			saida = 1;
			break;
		case 1:
	   		saida = d->imm;
	   		break;
	}
	return saida;
}

int ULA(int op1,int op2,int opULA,int *flag,int *overflow) {
	int resultado;
	if(opULA == 0) {
		resultado = op1 + op2;

		if ((op1 > 0 && op2 > 0 && resultado < 0) || (op1 < 0 && op2 < 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - ADD: %d + %d = %d (fora do intervalo de 8 bits!)\n", op1, op2, resultado);
		}
	}
	else if(opULA == 2) {
		resultado = op1 - op2;
		if(resultado == 0) {
			*flag = 1;
		}
		if ((op1 > 0 && op2 < 0 && resultado < 0) || (op1 < 0 && op2 > 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - SUB: %d - %d = %d (fora do intervalo de 8 bits!)\n", op1, op2, resultado);
		}
	}
	else if(opULA == 4) {
		resultado = op1 & op2;
	}
	else if(opULA == 5) {
		resultado = op1 | op2;
	}
	return resultado;
}

// funcao para imprimir a instrucao
void printInstrucao(Decodificador *d) {
	switch (d->opcode) {
	case 0: // Tipo R (add, sub, and, or)
		switch (d->funct) {
		case 0:
			printf("add $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 2:
			printf("sub $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 4:
			printf("and $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 5:
			printf("or $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		}
		break;
	case 4: // addi
		printf("addi $%d, $%d, %d", d->rt, d->rs, d->imm);
		break;
	case 11: // lw
		printf("lw $%d, %d($%d)", d->rt, d->imm, d->rs);
		break;
	case 15: // sw
		printf("sw $%d, %d($%d)", d->rt, d->imm, d->rs);
		break;
	case 8: // beq
		printf("beq $%d, $%d, %d", d->rs, d->rt, d->imm);
		break;
	case 2: // j
		printf("j %d", d->addr);
		break;
	}
}

void decodifica_dado(const char *data,Instrucao *in,Decodificador *d) {
	copiarBits(bin, in->dado, 8, 8);    // Copia os 4 bits do opcode (4 bits)
	d->dado = binarioParaDecimal(in->dado, 1);
}

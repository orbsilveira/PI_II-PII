#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM char mem[256][17] = {{'\0'}}
#define REGISTRADOR int registrador[8] = {0}
#define PC int pc = 0
#define RI char ri[17] = {'\0'}
#define SAIDA_A int saida_a = 0
#define SAIDA_B int saida_b = 0
#define ULA_SAIDA int ula_saida = 0

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
	char da[128][17];
	int pca;
	char ria[9];
	struct Nodo *prox;
} Nodo;

typedef struct pilha {
	Nodo *topo;
} Pilha;

void inicia_pilha(Pilha *p);
void empilha(Pilha *p,Decodificador *d,char mem[256][17],char *ri,int *r,int *pc);

int carrega_mem(char mem[256][17]);

void menu();
void print_mem(char mem[256][17]);
void printReg(int *reg);
void printInstrucao(Decodificador *d);

int executa_step(char mem[256][17],Instrucao *in,Decodificador *d,int *pc,int *br,Pilha *p,int est,char *ri,int *saida_a,int *saida_b,int *ula_saida);
int controle(int opcode, int est);

void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d);

int ULA_op1(int est,int pc,int saida_a);
int ULA_op2(int est,int saida_b,int imm);
int ULA(int op1,int op2,int opULA,int *flag,int *overflow);

int main() {
	Instrucao in;
	Decodificador d;
	Pilha p;
	MEM;
	REGISTRADOR;
	PC;
	RI;
	SAIDA_A;
	SAIDA_B;
	ULA_SAIDA;
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
			print_mem(mem);
			break;
		case 3:
			printReg(registrador);
			break;
		case 4:
			print_mem(mem);
			printReg(registrador);
			printf("\n\nPC: %d", pc);
			printf("\n\nRI: %s", ri);
			printf("\n\nA: %d", saida_a);
			printf("\n\nB: %d", saida_b);
			printf("\n\nULA-SAIDA: %d", ula_saida);
			break;
		case 5:
			est = executa_step(mem,&in,&d,&pc,registrador,&p,est,ri,&saida_a,&saida_b,&ula_saida);
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

// Carrega memoria
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

// Imprime memoria
void print_mem(char mem[256][17]) {
	int i = 0;
	printf("\n############## INSTRUCOES ##############\n");
	while(i<128) {
		printf("\n[%d]: %s\n", i,mem[i]);
		printf("\n");
		i++;
	}
	while(i<256) {
		printf("[%d]: %s\n", i, mem[i]);
		i++;
	}
}

// Imprime banco de registradores
void printReg(int *reg) {
	for(int i=0; i<8; i++) {
		printf("\nREGISTRADOR [%d]:  %d", i, reg[i]);
	}
}

int executa_step(char mem[256][17], Instrucao *in, Decodificador *d, int *pc, int *br,Pilha *p, int est,char *ri,int *saida_a,int *saida_b,int *ula_saida) {
	int flag = 0, overflow = 0;
	switch(est) {
	case 0: //Estado 0
		if (strcmp(mem[*pc], "0000000000000000") == 0 || *pc > 127) {
			printf("\nFim do programa!");
			return 0;
		} else {
			empilha(p,d,mem,ri,br,pc);
			strcpy(ri,mem[*pc]);
			*pc = ULA(ULA_op1(est,*saida_a,*pc),ULA_op2(est,*saida_b,d->imm),0,&flag,&overflow);
			return 1;
		}
		break;
	case 1: //Estado 1
		decodificarInstrucao(ri,in,d);
		printInstrucao(d);
		*saida_a = br[d->rs];
		*saida_b = br[d->rt];
		*ula_saida = ULA(ULA_op1(est,*saida_a,*pc),ULA_op2(est,*saida_b,d->imm),0,&flag,&overflow);
		return controle(d->opcode,est);
		break;
	}
}

void empilha(Pilha *p,Decodificador *d,char mem[256][17],char *ri,int *r,int *pc) {
	Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
	int i;
	for(i=0; i<8; i++) {
		nNodo->ra[i] = r[i];
	}
	for(i=128; i<256; i++) {
		strcpy(nNodo->da[i],mem[i]);
	}
	strcpy(nNodo->ria,ri);
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

// Copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho)
{
	strncpy(destino, instrucao + inicio, tamanho); // Copia os bits desejados
	destino[tamanho] = '\0';  // Adiciona o terminador de string
}

// Converte de binario para decimal
int binarioParaDecimal(const char *bin, int sinal) {
	int valor = (int)strtol(bin, NULL, 2);
	int bits = strlen(bin);

	if (sinal && bits > 0 && (valor & (1 << (bits - 1)))) {
		valor = valor - (1 << bits);
	}
	return valor;
}

// Controle para o proximo estado
int controle(int opcode, int est) {
	if(est == 0) {
		return 1;
	}
	else if(est == 1) {
		if (opcode == 11 || opcode == 15 || opcode == 4) {
			return 2;
		}
		else if(opcode == 0) {
			return 7;
		}
		else if(opcode == 8) {
			return 9;
		}
		else if(opcode == 2) {
			return 10;
		}
	}
	else if(est == 2) {
		if(opcode == 11) {
			return 3;
		}
		else if(opcode == 15) {
			return 5;
		}
		else if(opcode == 4) {
			return 6;
		}
	}
	else if(est == 3) {
		return 4;
	}
	else if(est == 7) {
		return 8;
	}
	else if(est == 4 || est == 5 || est == 6 || est == 8 || est == 9 || est == 10) {
		return 0;
	}
}

// MUX para o operando 1 da ULA
int ULA_op1(int est,int pc,int saida_a) {
	int saida;
	if(est == 0) {
		saida = pc;
	}
	else if(est == 2 || est == 7 || est == 9) {
		saida = saida_a;
	}
	return saida;
}

// MUX para o operando 2 da ULA
int ULA_op2(int est,int saida_b,int imm) {
	int saida;
	if(est == 7 || est == 9) {
		saida = saida_b;
	}
	else if(est == 0) {
		saida = 1;
	}
	else if(est == 1 || est == 2) {
		saida = imm;
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

// Funcao para imprimir a instrucao
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

//Funcao para converter os bits do imediato para decimal
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d) {
	copiarBits(data, in->dado, 8, 8);    // Copia os 4 bits do opcode (4 bits)
	d->dado = binarioParaDecimal(in->dado, 1);
}

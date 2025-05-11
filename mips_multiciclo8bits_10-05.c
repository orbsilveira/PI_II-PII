#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM char mem[256][17] = {{'\0'}}

typedef struct sinais {
	int EscPC,
	    IouD,
	    EscMem,
	    EscRI,
	    RegDest,
	    MemParaReg,
	    EscReg,
	    ULAFontA,
	    ULAFontB,
	    ControleULA,
	    FontePC,
	    Branch;
} Sinais;

typedef struct registradores {
	int br[8];
	int pc;
	char ri[17];
	int a;
	int b;
	int ula_saida;
	char rdm[17];
} Registradores;

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
	int bra[8];
	int pca;
	char ria[17];
	int aa;
	int ba;
	int ula_saidaa;
	char rdma[17];
	char da[128][17];
	int est_a;
	struct Nodo *prox;
} Nodo;

typedef struct pilha {
	Nodo *topo;
} Pilha;

//Sinal para autorizar escrita no RI
void escreve_ri(Registradores *r,int EscRI,char inst[17]);

void inicia_pilha(Pilha *p);
void empilha(Pilha *p,Decodificador *d,char mem[256][17],Registradores *r,int *est);

int carrega_mem(char mem[256][17]);

void menu();
void print_mem(char mem[256][17]);
void printReg(int *reg);
void printInstrucao(Decodificador *d);

int executa_step(char mem[256][17],Instrucao *in,Decodificador *d,Registradores *,Pilha *p,Sinais *s,int *est);
int controle(int opcode, int *est,Sinais *s);

void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d);

int ULA_op1(int est,int pc,int a);
int ULA_op2(int est,int b,int imm);
int ULA(int op1,int op2,int opULA,int *flag,int *overflow);

int main() {
	Sinais s;
	Instrucao in;
	Decodificador d;
	Registradores r = {0};
	Pilha p;
	MEM;
	int op,est = 0;
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
			printReg(r.br);
			break;
		case 4:
			print_mem(mem);
			printReg(r.br);
			printf("\n\nPC: %d", r.pc);
			printf("\n\nRI: %s", r.ri);
			printf("\n\nA: %d", r.a);
			printf("\n\nB: %d", r.b);
			printf("\n\nULA-SAIDA: %d", r.ula_saida);
			break;
		case 5:
			est = executa_step(mem,&in,&d,&r,&p,&s,&est);
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
	return 1;
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

int executa_step(char mem[256][17], Instrucao *in, Decodificador *d,Registradores *r,Pilha *p,Sinais *s,int *est) {

	int flag = 0, overflow = 0, jump=0;

	switch(*est) {
	case 0: //Estado 0
		if (strcmp(mem[r->pc], "0000000000000000") == 0 || r->pc > 127) {
			printf("\nFim do programa!");
			return 0;
		}

		else {
			empilha(p,d,mem,r,est);
			controle(d->opcode,est,s);
			escreve_ri(r,s->EscRI,mem[r->pc]);
			r->pc =  ULA(ULA_op1(*est,r->pc,r->a),ULA_op2(*est,r->b,d->imm),0,&flag,&overflow);
			return 1;
		}
		break;

	case 1: //Estado 1
		decodificarInstrucao(r->ri,in,d);
		printInstrucao(d);
		r->a = r->br[d->rs];
		r->b = r->br[d->rt];
		r->ula_saida = ULA(ULA_op1(*est,r->a,r->pc),ULA_op2(*est,r->b,d->imm),0,&flag,&overflow);

		if(d->opcode == 2) {
			copiarBits(r->ri, in->addr, 8, 8);
			jump = binarioParaDecimal(in->addr, 0);
		}

		return controle(d->opcode,est,s);
		break;

	case 2: //Estado 2 - executa lw,sw e addi
		if(d->opcode == 11 || d->opcode == 15) {
			r->ula_saida = ULA(r->a, d->imm, 0, &flag, &overflow);
		}

		if(d->opcode == 4) {
			r->ula_saida = ULA(r->a, d->imm, 0, &flag, &overflow);
		}

		return controle(d->opcode, est,s);
		break;

	case 3: //Estado 3 - acessa a memC3ria (lw)
		strcpy(r->rdm, mem[r->ula_saida]);
		return 4;
		break;

	case 4: //Estado 4 - escreve no registrador (lw)
		r->br[d->rt] = binarioParaDecimal(r->rdm, 1);
		return 0;
		break;

	case 5:
		break;

	case 6: //Estado 6 - escreve no registrador (addi)
		r->br[d->rt] = r->ula_saida;
		return 0;
		break;

	case 7: //Estado 7 - executa r
		r->ula_saida = ULA(r->a, r->b, d->funct, &flag, &overflow);
		return 8;
		break;

	case 8: //Estado 8 - escreve no registrador (r)
		r->br[d->rd] = r->ula_saida;
		return 0;
		break;

	case 9:
		break;

	case 10: //Estado 10 - executa jump
		r->pc = jump;
		return 0;
		break;

	default:
		printf("ESTADO DESCONHECIDO");
		return 0;
		break;
	}
}

void empilha(Pilha *p,Decodificador *d,char mem[256][17],Registradores *r,int *est) {
	Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
	int i;
	nNodo->est_a = *est;
	nNodo->pca=r->pc;
	for(i=128; i<256; i++) {
		strcpy(nNodo->da[i],mem[i]);
	}
	strcpy(nNodo->ria,r->ri);
	strcpy(nNodo->rdma,r->rdm);
	for(i=0; i<8; i++) {
		nNodo->bra[i] = r->br[i];
	}
	nNodo->aa = r->a;
	nNodo->ba = r->b;
	nNodo->ula_saidaa = r-> ula_saida;
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
int controle(int opcode, int *est,Sinais *s) {
	switch(*est) {
	case 0:
		s->EscPC = 1;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 1;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 0;
		s->ULAFontB = 1;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		return 1;

	case 1:
		if(opcode == 11 || opcode == 15 || opcode == 4)
			return 2;

		if(opcode == 0)
			return 7;

		if(opcode == 8)
			return 9;

		if(opcode == 2)
			return 10;

	case 2:
		if(opcode == 11)
			return 3;

		if(opcode == 15)
			return 5;

		if(opcode == 4)
			return 6;

	case 3:
		return 4;

	case 7:
		return 8;

	default:
		return 0;
	}
}

// MUX para o operando 1 da ULA
int ULA_op1(int est,int pc,int a) {
	int saida;
	if(est == 0) {
		saida = pc;
	}
	else if(est == 2 || est == 7 || est == 9) {
		saida = a;
	}
	return saida;
}

// MUX para o operando 2 da ULA
int ULA_op2(int est,int b,int imm) {
	int saida;
	if(est == 7 || est == 9) {
		saida = b;
	}
	else if(est == 0) {
		saida = 1;
	}
	else if(est == 1 || est == 2) {
		saida = imm;
	}
	return saida;
}

int ULA(int op1, int op2, int opULA, int *flag, int *overflow) {

	int resultado;
	*flag = 0;
	*overflow = 0;

	switch(opULA) {
	case 0:
		resultado = op1 + op2;

		if ((op1 > 0 && op2 > 0 && resultado < 0) || (op1 < 0 && op2 < 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - ADD: %d + %d = %d\n", op1, op2, resultado);
		}
		break;

	case 2:
		resultado = op1 - op2;

		if(resultado == 0) {
			*flag = 1;
		}

		if ((op1 > 0 && op2 < 0 && resultado < 0) || (op1 < 0 && op2 > 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - SUB: %d - %d = %d\n", op1, op2, resultado);
		}
		break;

	case 4:
		resultado = op1 & op2;
		break;

	case 5:
		resultado = op1 | op2;
		break;

	default:
		resultado = 0;
		printf("OPERACCO DESCONHECIDA: %d\n", opULA);
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

//Sinal para autorizar escrita no RI
void escreve_ri(Registradores *r,int EscRI,char inst[17]) {
	if(EscRI == 1) {
		strcpy(r->ri,inst);
	}
}

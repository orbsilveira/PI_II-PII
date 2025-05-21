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
	int pc,
	    br[8],
	    a,
	    b,
	    ula_saida;
	char ri[17],
	     rdm[17];
} Registradores;

typedef enum tipo {
	Tipo_R=0,
	Tipo_I=1,
	Tipo_J=2,
	Tipo_OUTROS=3
} Tipo_Instrucao;

typedef struct instrucao {
	char opcode[5],
	     rs[4],
	     rt[4],
	     rd[4],
	     funct[4],
	     imm[7],
	     addr[8],
	     dado[9];
} Instrucao;

typedef struct decodificador {
	int opcode,
	    rt,
	    rs,
	    rd,
	    funct,
	    imm,
	    addr,
	    dado;
	Tipo_Instrucao tipo;
} Decodificador;

typedef struct Nodo {
	int pca,
	    bra[8],
	    aa,
	    ba,
	    ula_saidaa,
	    est_a;
	char ria[17],
	     rdma[17],
	     da[128][17];
	struct Nodo *prox;
} Nodo;

typedef struct pilha {
	Nodo *topo;
} Pilha;

typedef struct ALUout {
	int resultado,
	    flag_zero,
	    overflow;
} ALUout;

//Controle de escrita
void escreve_ri(char *ri,int EscRI,char inst[17]);
void escreve_pc(int *pc, int EscPC, int FontePC, int Branch, int flag_zero);

//MUX
int IOuD(int IouD, int pc, int ula_saida); //Seleciona se o endereco a ser acessado vem do PC ou do IMEDIATO
int PCFonte(int resul, int reg_ula, int FontePC); // Seleciona se o incremento do PC vem da soma com o IMEDIATO 1 ou do BRANCH EQUAL
int ULA_fontA(int pc,int a,int ULAFontA); //Seleciona o primeiro operando da ULA
int ULA_fontB(int b,int imm,int ULAFontB);//Seleciona o segundo operando da ULA
int RegiDest(int rt, int rd, int RegDest);
int MemReg(int ula_saida, int dado, int MemParaReg);
void escreve_br(int *reg, int EscReg, int dado);

void controle_acesso_memoria(char (*mem)[17],Instrucao *in,Decodificador *d,Registradores *r,Pilha *p,Sinais *s,ALUout *saida,int *est);

//Funcoes
void inicia_pilha(Pilha *p);
void empilha(Pilha *p,Decodificador *d,char (*mem)[17],Registradores *r,int *est);
int carrega_mem(char mem[256][17]);
void menu();
void print_mem(char mem[256][17]);
void print_br(int *r);
void printReg(Registradores *r);
void printInstrucao(Decodificador *d);
int executa_step(char (*mem)[17],Instrucao *in,Decodificador *d,Registradores *,Pilha *p,Sinais *s,ALUout *saida,int *est, int *p_c);
int controle(int opcode, int *est,Sinais *s);
void estado(int *est,int opcode);
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d);
void ULA(int op1,int op2,int opULA,ALUout *saida);
void int_para_binario_recursiva(int valor, char *binario, int pos);
void int_para_binario(int valor, char *binario);
void executa_run(char (*mem)[17], Instrucao *in, Decodificador *d, Registradores *r, Pilha *p, Sinais *s, ALUout *saida, int *est);
void salvarAssembly(char mem[256][17]);
int step_back(Pilha *p, Registradores *r, char (*mem)[17], int *est, int sinal);
int limite_back(Pilha *p);
void infoEstado(int *est, Decodificador *d, ALUout *saida);

int main() {
	Sinais s = {0};
	Instrucao in;
	Decodificador d = {0};
	Registradores r = {0};
	strcpy(r.ri, "0000000000000000");
	strcpy(r.rdm, r.ri);
	Pilha p = {0};
	ALUout saida = {0};
	MEM;
	int op, est = 0;

	inicia_pilha(&p);

	do {
		menu();
		printf(" Sua escolha: ");
		scanf("%d", &op);
		printf("\n");
		switch (op) {
		case 1:
			if (carrega_mem(mem) == 1)
			{
				printf("Arquivo carregado!\n");
			}
			break;
		case 2:
			print_mem(mem);
			break;
		case 3:
			print_br(r.br);
			break;
		case 4:
			print_mem(mem);
			print_br(r.br);
			printReg(&r);
			printf("\nEstado atual: %d\n", est);
			break;
		case 5:
			if(strlen(mem[0]) > 0) {
				controle_acesso_memoria(mem,&in,&d,&r,&p,&s,&saida,&est);
			} else {
				printf("Nenhuma memoria carregada!\n");
			}
			break;
		case 6:
			if(strlen(mem[0]) > 0) {
				executa_run(mem, &in, &d, &r, &p, &s, &saida, &est);
			}
			else {
				printf("Nenhuma memoria carregada!\n");
			}
			break;
		case 7:
			salvarAssembly(mem);
			break;
		case 8:
			printf("Step Back");
			step_back(&p, &r, mem, &est,0);
			break;
		case 9:
			printf("Voce saiu!!!");
			break;
		default:
			printf("Opcao invalida!");
		}
	} while(op != 9);
	return 0;
}

// Funcao para iniciar pilha
void inicia_pilha(Pilha *p) {
	p->topo=NULL;
}

void menu() {
	printf("\n ******* MENU *******\n");
	printf("\n 1 - Carregar memoria\n");
	printf(" 2 - Imprimir memoria\n");
	printf(" 3 - Imprimir banco de registradores\n");
	printf(" 4 - Imprimir todo o simulador\n");
	printf(" 5 - Executar step\n");
	printf(" 6 - Executar run\n");
	printf(" 7 - Salvar .asm\n");
	printf(" 8 - Volta um step\n");
	printf(" 9 - Sair\n");
}

// Carrega memoria
int carrega_mem(char mem[256][17]) {
	char arquivo[20],extensao[5];
	int tam;
	// abre o arquivo em modo leitura
	printf("Nome do arquivo: ");
	scanf("%s", arquivo);
	tam = strlen(arquivo);
	strncpy(extensao,arquivo + tam - 4,4);
	extensao[4] = '\0';
	if(strcmp(extensao, ".mem") != 0) {
		printf("Extensao de arquivo nao suportada!\n");
	} else {
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
}

// Imprime memoria
void print_mem(char mem[256][17]) {
	int i = 0;
	printf("\n ============= MEMORIA =============\n");
	printf("\n ------------ INSTRUCOES ------------\n\n");
	while(i<128) {
		printf(" [%d]: %s\n", i,mem[i]);
		i++;
	}
	printf("\n   ------------ DADOS ------------\n\n");
	while(i<256) {
		printf(" [%d]: %s\n", i, mem[i]);
		i++;
	}
}

// Imprime banco de registradores
void print_br(int *r) {
	printf("\n ------ BANCO DE REGISTRADORES ------ \n\n");
	for(int i=0; i<8; i++) {
		printf(" REGISTRADOR [%d]: %d\n", i, r[i]);
	}
}

// Imprime todos registradores (menos os do Banco de Registradores)
void printReg(Registradores *r) {
	printf("\n PC: %d", r->pc);
	printf("\n RI: %s", r->ri);
	printf("\n RDM: %s", r->rdm);
	printf("\n A: %d", r->a);
	printf("\n B: %d", r->b);
	printf("\n ULA-SAIDA: %d\n", r->ula_saida);
}

void controle_acesso_memoria(char (*mem)[17],Instrucao *in,Decodificador *d,Registradores *r,Pilha *p,Sinais *s,ALUout *saida,int *est) {
	if(d->opcode == 11 || d->opcode == 15) {
		if(*est == 1) {
			int temp_rs,tem_pc,temp_a,temp_b,temp_saidaULA, p_c = 0;
			temp_rs = r->br[d->rs];
			/*temp_a = r->a;
			temp_b = r->b;
			temp_saidaULA = r->ula_saida;*/
			char addi[9][17];
			strcpy(addi[0],"0100000001010000");
			strcpy(addi[8],mem[r-pc]);
			addi[0][16] = '\0';
			addi[0][4] = in->rs[0];
			addi[0][5] = in->rs[1];
			addi[0][6] = in->rs[2];
			for(int i=1; i<8; i++) {
				strcpy(addi[i],addi[0]);
			}
			for(int i=0)
				executa_step(addi, in, d, r, p, s, saida, est,&p_c) != 1) {
			}
			r->br[d->rs] = temp_rs;
			*est = 2;
			/*r->ula_saida = temp_saidaULA;
			r->a = temp_a;
			r->b = temp_b;*/
		}
	} else {
		executa_step(mem, in, d, r, p, s, saida, est,&r->pc);
	}
}

int executa_step(char (*mem)[17], Instrucao *in, Decodificador *d, Registradores *r, Pilha *p, Sinais *s, ALUout *saida, int *est, int *p_c) {

	int endereco_dados;
	char binario[17];

	if(r->pc > 127) {
		printf("Posicao invalida apontada pelo PC!\n");
		return 0;
	}

	if(*est == 0 && strcmp(mem[*p_c], "0000000000000000") == 0) {
		printf("########## EXECUCAO CONCLUCDA! ##########\n");
		return 1;
	}

	empilha(p, d, mem, r, est);
	controle(d->opcode, est, s);
	escreve_ri(r->ri, s->EscRI, mem[r->pc]);

	if(s->IouD == 1) {
		if(r->br[d->rs] >= 128 && r->br[d->rs] < 256) {
			strcpy(r->rdm, mem[r->br[d->rs]]);
			decodifica_dado(r->rdm, in, d);
		}
	}

	r->a = r->br[d->rs];
	r->b = r->br[d->rt];

	//executa operaC'C#o da ula

	escreve_br(&r->br[RegiDest(d->rt, d->rd, s->RegDest)], s->EscReg, MemReg(r->ula_saida, d->dado, s->MemParaReg));
	
	decodificarInstrucao(r->ri, in, d);
	
	ULA(ULA_fontA(*p_c, r->a, s->ULAFontA), ULA_fontB(r->b, d->imm, s->ULAFontB), s->ControleULA, saida);
	r->ula_saida = saida->resultado;

	if(s->EscMem == 1) {
		if(r->br[d->rs] >= 128 && r->br[d->rs] < 256) {
			int_para_binario(r->b, binario);
			strcpy(mem[r->br[d->rs]], binario);
		}
	}

	escreve_pc(p_c, s->EscPC, PCFonte(saida->resultado, r->ula_saida, s->FontePC), s->Branch, saida->flag_zero);

	infoEstado(est, d, saida);

	estado(est, d->opcode);

	return 0;
}


//Funcao para informaC'C5es do estado
void infoEstado(int *est, Decodificador *d, ALUout *saida) {
	switch (*est)
	{
	case 0:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Acesso a memoria para busca de instrucao.\n");
		printf(" Incrementa PC em 1.\n");
		break;
	case 1:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Decodificacao da instrucao.\n");
		printInstrucao(d);
		printf(" Leitura dos registradores Rs e Rt.\n");
		printf(" Rs: %d\n", d->rs);
		printf(" Rt: %d\n", d->rt);
		printf(" Calculo do endereco de desvio (cond.)\n");
		printf(" Flag zero = %d\n", saida->flag_zero);
		printf(" Endereco de desvio: %d\n", saida->resultado);
		break;
	case 2:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Calculo do endereco efetivo para LW/SW/ADDI\n");
		printf(" Endereco calculado: %d\n", saida->resultado);
		break;
	case 3:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Acesso C memCoria para LW (leitura)\n");
		printf(" EndereC'o acessado: %d\n", saida->resultado);
		printf(" Dado lido: %d\n", d->dado);
		break;
	case 4:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Conclusao da instrucao LW\n");
		printf(" Escrita no registrador $%d: %d\n", d->rt, d->dado);
		break;
	case 5:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Acesso a memoria para SW (escrita)\n");
		printf(" Endereco acessado: %d\n", saida->resultado);
		printf(" Dado escrito: %d\n", d->rt);
		break;
	case 6:
		printf(" Estado [%d] executado.\n", *est);
		printf(" ConclusC#o da instrucao ADDI\n");
		printf(" Escrita no registrador $%d: %d\n", d->rt, saida->resultado);
		break;
	case 7:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Execucao da instrucao tipo R\n");
		printf(" Operacao realizada: ");
		printInstrucao(d);
		printf(" Resultado: %d\n", saida->resultado);
		break;
	case 8:
		printf(" Estado [%d] executado.\n", *est);
		printf(" Conclusao da instrucao tipo R\n");
		printf(" Escrita no registrador $%d: %d\n", d->rd, saida->resultado);
		break;
	case 9:
		printf(" Estado [%d] executado.\n", *est);
		printf(" ConclusC#o do desvio condicional (BEQ)\n");
		printf(" Comparacao: $%d == $%d\n", d->rs, d->rt);
		if (saida->flag_zero != 0) {
			printf(" Resultado: Verdadeiro\n");
		} else {
			printf(" Resultado: Falso\n");
		}
		break;
	case 10:
		printf(" Estado [%d] executado.\n", *est);
		printf(" ConclusC#o do desvio incondicional (J)\n");
		printf(" Novo PC: %d\n", d->addr);
		break;
	}
}


int RegiDest(int rt, int rd, int RegDest) {
	switch (RegDest) {
	case 0:
		return rt;
	case 1:
		return rd;
	}
}

void escreve_br(int *reg, int EscReg, int dado) {
	if(EscReg == 1) {
		*reg = dado;
	}
}

int MemReg(int ula_saida, int dado, int MemParaReg) {
	switch (MemParaReg) {
	case 0:
		return ula_saida;
	case 1:
		return dado;
	}
}

// Funcao para pilha usada no step back
void empilha(Pilha *p, Decodificador *d, char (*mem)[17], Registradores *r, int *est) {
	Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
	if (nNodo == NULL) {
		printf("Erro ao alocar memC3ria para novo nC3\n");
		exit(1);
	}
	int i;
	nNodo->est_a = *est;
	nNodo->pca = r->pc;
	for(i = 0; i < 128; i++) {
		strncpy(nNodo->da[i], mem[i+128], 16);
		nNodo->da[i][16] = '\0';
	}
	strncpy(nNodo->ria, r->ri, 16);
	nNodo->ria[16] = '\0';
	strncpy(nNodo->rdma, r->rdm, 16);
	nNodo->rdma[16] = '\0';
	for(i = 0; i < 8; i++) {
		nNodo->bra[i] = r->br[i];
	}
	nNodo->aa = r->a;
	nNodo->ba = r->b;
	nNodo->ula_saidaa = r->ula_saida;
	nNodo->prox = p->topo;
	p->topo = nNodo;
}

// Funcao para decodificar instrucao
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
	copiarBits(bin, in->addr, 8, 8);      // Copia os 8 bits do addr
	d->addr = binarioParaDecimal(in->addr, 0);
}

// Copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho) {
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
int controle(int opcode, int *est, Sinais *s) {

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
		break;
	case 1:
		s->EscPC = 0;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 1;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 2:
		s->EscPC = 0;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 3:
		s->EscPC = 0;
		s->IouD = 1;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 4:
		s->EscPC = 0;
		s->IouD = 1;
		s->EscMem = 1;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 1;
		s->EscReg = 1;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 5:
		s->EscPC = 0;
		s->IouD = 1;
		s->EscMem = 1;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 6:
		s->EscPC = 0;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 1;
		s->ULAFontA = 1;
		s->ULAFontB = 2;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 7:
		s->EscPC = 0;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 1;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 0;
		s->ControleULA = 1;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 8:
		s->EscPC = 0;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 1;
		s->MemParaReg = 0;
		s->EscReg = 1;
		s->ULAFontA = 1;
		s->ULAFontB = 0;
		s->ControleULA = 0;
		s->FontePC = 0;
		s->Branch = 0;
		break;
	case 9:
		s->EscPC = 1;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 1;
		s->ULAFontB = 0;
		s->ControleULA = 2;
		s->FontePC = 1;
		s->Branch = 1;
		break;
	case 10:
		s->EscPC = 1;
		s->IouD = 0;
		s->EscMem = 0;
		s->EscRI = 0;
		s->RegDest = 0;
		s->MemParaReg = 0;
		s->EscReg = 0;
		s->ULAFontA = 0;
		s->ULAFontB = 0;
		s->ControleULA = 0;
		s->FontePC = 2;
		s->Branch = 0;
		break;
	}
}

// Funcao para atualizacao do estado
void estado(int *est, int opcode) {
	switch(*est) {
	case 0:
		*est = 1;
		break;
	case 1:
		if(opcode == 11 || opcode == 15 || opcode == 4)
			*est = 2;

		if(opcode == 0)
			*est = 7;

		if(opcode == 8)
			*est = 9;

		if(opcode == 2)
			*est = 10;
		break;
	case 2:
		if(opcode == 11)
			*est = 3;

		if(opcode == 15)
			*est = 5;

		if(opcode == 4)
			*est = 6;
		break;
	case 3:
		*est = 4;
		break;
	case 7:
		*est = 8;
		break;
	default:
		*est = 0;
	}
}

// MUX para o operando 1 da ULA
int ULA_fontA(int pc,int a,int ULAFontA) {
	switch(ULAFontA) {
	case 0:
		return pc;
	case 1:
		return a;
	}
}

// MUX para o operando 2 da ULA
int ULA_fontB(int b,int imm, int ULAFontB) {
	switch(ULAFontB) {
	case 0:
		return b;
	case 1:
		return 1;
	case 2:
		return imm;
	}
}

// Funcao ULA
void ULA(int op1, int op2, int opULA, ALUout *saida) {
	switch(opULA) {
	case 0:
		saida->resultado = op1 + op2;

		if(saida->resultado == 0) {
			saida->flag_zero = 1;
		}

		if ((op1 > 0 && op2 > 0 && saida->resultado < 0) || (op1 < 0 && op2 < 0 && saida->resultado > 0)) {
			saida->overflow = 1;
			printf("OVERFLOW - ADD: %d + %d = %d\n", op1, op2, saida->resultado);
		}
		break;

	case 2:
		saida->resultado = op1 - op2;

		if(saida->resultado == 0) {
			saida->flag_zero = 1;
		}

		if ((op1 > 0 && op2 < 0 && saida->resultado < 0) || (op1 < 0 && op2 > 0 && saida->resultado > 0)) {
			saida->overflow = 1;
			printf("OVERFLOW - SUB: %d - %d = %d\n", op1, op2, saida->resultado);
		}
		break;

	case 4:
		saida->resultado = op1 & op2;
		break;

	case 5:
		saida->resultado = op1 | op2;
		break;

	default:
		saida->resultado = 0;
		printf("OPERACCO DESCONHECIDA: %d\n", opULA);
	}
}

// Funcao para imprimir a instrucao
void printInstrucao(Decodificador *d) {
	switch (d->opcode) {
	case 0: // Tipo R (add, sub, and, or)
		switch (d->funct) {
		case 0:
			printf(" add $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
			break;
		case 2:
			printf(" sub $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
			break;
		case 4:
			printf(" and $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
			break;
		case 5:
			printf(" or $%d, $%d, $%d\n", d->rd, d->rs, d->rt);
			break;
		}
		break;
	case 4: // addi
		printf(" addi $%d, $%d, %d\n", d->rt, d->rs, d->imm);
		break;
	case 11: // lw
		printf(" lw $%d, %d($%d)\n", d->rt, d->imm, d->rs);
		break;
	case 15: // sw
		printf(" sw $%d, %d($%d)\n", d->rt, d->imm, d->rs);
		break;
	case 8: // beq
		printf(" beq $%d, $%d, %d\n", d->rs, d->rt, d->imm);
		break;
	case 2: // j
		printf(" j %d\n", d->addr);
		break;
	}
}

//Funcao para converter os bits do imediato para decimal
void decodifica_dado(const char *data,Instrucao *in,Decodificador *d) {
	copiarBits(data, in->dado, 8, 8);    // Copia os 4 bits do opcode (4 bits)
	d->dado = binarioParaDecimal(in->dado, 1);
}

//Sinal para autorizar escrita no RI
void escreve_ri(char *ri,int EscRI,char inst[17]) {
	if(EscRI == 1) {
		strcpy(ri,inst);
	}
}

void escreve_pc(int *pc, int EscPC, int FontePC, int Branch, int flag_zero) {
	if (EscPC == 1) {
		if (Branch == 1) {
			if (flag_zero == 1) {
				*pc = FontePC;
			}
		}
		else {
			*pc = FontePC;
		}
	}
}

int IOuD(int IouD, int pc, int ula_saida) {
	if (IouD == 1) {
		return ula_saida + 128;
	}
	return pc;
}

int PCFonte(int resul, int reg_ula, int FontePC) {
	switch (FontePC) {
	case 0:
		return resul;
	case 1:
		return reg_ula;
	case 2:
		return reg_ula;
	default:
		return resul;
	}
}

// Funcao auxiliar recursiva
void int_para_binario_recursiva(int valor, char *binario, int pos) {
	if(pos < 0) {
		return;
	}

	if(valor & 1) {
		binario[pos] = '1';
	}
	else {
		binario[pos] = '0';
	}

	int_para_binario_recursiva(valor >> 1, binario, pos-1);
}

// Funcao principal
void int_para_binario(int valor, char *binario) {
	int i;

	for(i = 0; i < 16; i++) {
		binario[i] = '0';
	}

	binario[16] = '\0';

	int_para_binario_recursiva(valor, binario, 15);
}

// Funcao para executar o run
void executa_run(char (*mem)[17], Instrucao *in, Decodificador *d, Registradores *r, Pilha *p, Sinais *s, ALUout *saida, int *est) {
	while(executa_step(mem, in, d, r, p, s, saida, est,&r->pc) != 1) {
	}
}

// Funcao para conversao das instrucoes para assembly e salvar "arquivo.asm"
void salvarAssembly(char mem[256][17]) {
	char arquivo[20];

	printf("Nome do arquivo .asm: ");
	scanf("%s", arquivo);

	FILE *arq = fopen(arquivo, "w");
	if (!arq) {
		perror("Erro ao criar arquivo");
		return;
	}

	for (int i = 0; i < 128; i++) {
		if (mem[i][0] == '\0') continue; // Ignora posiC'C5es vazias

		struct instrucao inst;
		Decodificador d;
		decodificarInstrucao(mem[i], &inst, &d);

		// Converte para assembly e escreve no arquivo
		switch (d.opcode) {
		case 0: // Tipo R (add, sub, and, or)
			switch (d.funct) {
			case 0:
				fprintf(arq, "add $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 2:
				fprintf(arq, "sub $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 4:
				fprintf(arq, "and $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 5:
				fprintf(arq, "or $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			}
			break;
		case 4: // addi
			fprintf(arq, "addi $%d, $%d, %d\n", d.rt, d.rs, d.imm);
			break;
		case 11: // lw
			fprintf(arq, "lw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
			break;
		case 15: // sw
			fprintf(arq, "sw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
			break;
		case 8: // beq
			fprintf(arq, "beq $%d, $%d, %d\n", d.rs, d.rt, d.imm);
			break;
		case 2: // j
			fprintf(arq, "j %d\n", d.addr);
			break;
		}
	}
	fclose(arq);
	printf("Arquivo %s salvo com sucesso!\n", arquivo);
}

// Funcao de execucao do step back
int step_back(Pilha *p, Registradores *r, char (*mem)[17], int *est,int sinal) {
	int i;

	if(limite_back(p) == 1) {
		return 1;
	} else {
		Nodo *remover = p->topo;
		*est = remover->est_a;
		r->pc = remover->pca;
		r->a = remover->aa;
		r->b = remover->ba;
		r->ula_saida = remover->ula_saidaa;
		for(i = 0; i < 8; i++) {
			r->br[i] = remover->bra[i];
		}
		strncpy(r->ri, remover->ria, 16);
		r->ri[16] = '\0';
		strncpy(r->rdm, remover->rdma, 16);
		r->rdm[16] = '\0';
		for(i = 0; i < 128; i++) {
			strncpy(mem[i+128], remover->da[i], 16);
			mem[i][16] = '\0';
		}
		p->topo = remover->prox;
		free(remover);
		return 0;
	}
}

// Limite do step back, termina desempilhamento na primeira instrucao executada
int limite_back(Pilha *p) {
	if(p->topo==NULL) {
		printf("\nVoce voltou ao inicio!");
		return 1;
	}
}
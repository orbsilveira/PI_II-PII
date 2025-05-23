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

//Funcoes
void inicia_pilha(Pilha *p);
void empilha(Pilha *p,Decodificador *d,char (*mem)[17],Registradores *r,int *est);
int carrega_mem(char mem[256][17]);
void menu();
void print_mem(char mem[256][17]);
void print_br(int *r);
void printReg(Registradores *r);
void printInstrucao(Decodificador *d);
int executa_step(char (*mem)[17],Instrucao *in,Decodificador *d,Registradores *,Pilha *p,Sinais *s,ALUout *saida,int *est);
int controle(int opcode, int *est,Sinais *s, Decodificador *d);
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
int step_back(Pilha *p, Registradores *r, char (*mem)[17], int *est);
int limite_back(Pilha *p);
void infoEstado(int *est, Decodificador *d, ALUout *saida, Registradores *r);

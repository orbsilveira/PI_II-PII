#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct inst {
		char opcode[5];
		char rs[4];
		char rt[4];
		char rd[4];
		char funct[4];
		char imm[7];
		char addr[8];
	};

struct inst *instrucao(const char *instr);
void unidadeDeControle(struct inst *instrucao);

int main() {
	
	const char *inst = "0010010101101010";
    struct inst *regInst;

    regInst = instrucao(inst);

    printf("opcode: %s\n", regInst->opcode);
    printf("rs:     %s\n", regInst->rs);
    printf("rt:     %s\n", regInst->rt);
    printf("rd:     %s\n", regInst->rd);
    printf("funct:  %s\n", regInst->funct);
    printf("imm:    %s\n", regInst->imm);
    printf("addr:   %s\n", regInst->addr);
    
    unidadeDeControle(regInst);
    
    free(regInst);

    return 0;
}

struct inst *instrucao(const char *instr) {
	struct inst *inst = (struct inst *) malloc(sizeof(struct inst));
	
    strncpy(inst->opcode, instr, 4);
    inst->opcode[4] = '\0';

    strncpy(inst->rs, instr + 4, 3);
    inst->rs[3] = '\0';

    strncpy(inst->rt, instr + 7, 3);
    inst->rt[3] = '\0';

    strncpy(inst->rd, instr + 10, 3);
    inst->rd[3] = '\0';

    strncpy(inst->funct, instr + 13, 3);
    inst->funct[3] = '\0';

    strncpy(inst->imm, instr + 10, 6);
    inst->imm[6] = '\0';

    strncpy(inst->addr, instr + 9, 7);
    inst->addr[7] = '\0';
    
    return inst;
}

void unidadeDeControle(struct inst *instrucao) {
    const char *opcode = instrucao->opcode;
    int ciclos = 0;
    const char *tipo = "";

    if (strcmp(opcode, "0000") == 0 || strcmp(opcode, "0001") == 0) {
        tipo = "R";
        ciclos = 4;
    } else if (strcmp(opcode, "0010") == 0) {
        tipo = "LW";
        ciclos = 5;
    } else if (strcmp(opcode, "0011") == 0) {
        tipo = "BEQ";
        ciclos = 3;
    } else if (strcmp(opcode, "1100") == 0) {
		tipo = "JUMP";
		ciclos = 3;
    } else {
        tipo = "DESCONHECIDO";
        ciclos = 0;
    }

    printf("\n[Unidade de Controle]\n");
    printf("Tipo: %s\n", tipo);
    printf("Executando em %d ciclos:\n", ciclos);
    
    getchar(); //por enquanto usado para clock

    for (int i = 1; i <= ciclos; i++) {
        printf(" Ciclo %d: ", i);
        switch (i) {
            case 1:
                printf("Busca da instrução\n");
                break;
            case 2:
                printf("Decodificação / leitura de registradores\n");
                break;
            case 3:
                if (strcmp(tipo, "BEQ") == 0) {
                    printf("Execução de comparação + desvio\n");
                } else if (strcmp(tipo, "LW") == 0) {
                    printf("Cálculo do endereço\n");
                } else if (strcmp(tipo, "R") == 0) {
                    printf("Execução da operação ULA\n");
                } else if (strcmp(tipo, "JUMP") == 0) {
					printf("Execução de JUMP\n");
				} else {
                    printf("Operação desconhecida\n");
                }
                break;
            case 4:
                if (strcmp(tipo, "R") == 0) {
                    printf("Escrita no registrador de destino\n");
                } else if (strcmp(tipo, "LW") == 0) {
                    printf("Leitura da memória\n");
                }
                break;
            case 5:
                if (strcmp(tipo, "LW") == 0) {
                    printf("Escrita no registrador\n");
                }
                break;
            default:
                printf("N/A\n");
        }
        getchar(); //por enquanto usado para clock
    }
}

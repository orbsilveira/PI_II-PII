#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tad.h"

#define MEM char mem[256][17] = {{'\0'}}

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
        executa_step(mem,&in,&d,&r,&p,&s,&saida,&est);
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
            step_back(&p, &r, mem, &est); 
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

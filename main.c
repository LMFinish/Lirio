#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "DolFile.h"
#include <stdio.h>
#include <stdlib.h>

uint32_t Big_Endian(uint32_t VTF)
{
    // Tentar não usar em supostas plataformas big endian
    uint32_t Fixed_Val = ((((VTF & 0xFF000000) >> 24) + ((VTF & 0x00FF0000) >> 8)) + ((VTF & 0xFF00) << 8)) + ((VTF << 24));
    return Fixed_Val;
}

int main(int argc, char *argv[])
{
    printf("\n");
    printf("Lírio v1.1 (c) 2024 Kevin Andrade @ LMFinish\n");
    printf("Injetor de códigos Gecko para Nintendo GameCube / decompilador dos chips Gekko e Broadway\n");
	printf("\n");
	
    FILE *dolfile;
    FILE *codefile;
    codefile = fopen("code.txt", "r");
    if (codefile == NULL) {
        printf("code.txt não encontrado! Abortando.\n");
        return 0;
    } else printf("Processando code.txt\n");

    int tool_arg;
    int Linhas_Processadas = 0,
    Simulation_Mode = 0;

    uint32_t Inst, Inj_Addr, Entry_Point;
    char dolname[30];
	
    // Normalmente as metadas das trapaças Action Replay tem 8 digitos
    char Code_Half1[9];
    char Bytecode[9];

    // Escanear argumentos
    while ((tool_arg = getopt(argc, argv, "sd")) != -1) {
        switch(tool_arg) {

            case 'd':
            printf("Modo de Decompilação\n");
            printf("Endereço (hexadecimal): ");
            scanf("%x", &Entry_Point);
            int Inst_Read = 0;
            if (Entry_Point > 0) { Linhas_Processadas = 1;
            }

            while (fgets(Bytecode, 9, codefile) != NULL) {

                if (Entry_Point >= 0x100000000) {
                    printf("Valor fora do limite de 32 bits, abortando.\n");
                    break;
                }

                if (Linhas_Processadas == 1) { printf("%02X:\t", Entry_Point); }
                Inst_Read++;
                fseek(codefile, 8*Inst_Read, SEEK_SET);
                Inst = (uint32_t)strtoul(Bytecode, NULL, 16);

                if (Inst < 0x8000000) { printf("Possíveis dados nulos ou inválidos: ");
                }   DisASM(Inst, Entry_Point);
                    Entry_Point = Entry_Point + 4;

            } printf("Concluído!\n");
              return 0;

            case 's':
            printf("Executando no modo de simulação (sem injeção)\n");
            Simulation_Mode = 1;
            break;

        }
    }
    printf("Informe o executável DOL de destino: ");
    scanf("%29s", dolname);

    dolfile = fopen(dolname, "rb+");

    if (dolfile == NULL) {
        printf("Executável não encontrado! Abortando.\n");
        return 0;
      } else printf("%s Aberto", dolname);

    printf("\n");
    struct Dol_File Dol;
    fread(&Dol, sizeof(Dol), 1, dolfile);

    int Inst_Shift = 19;

    // Iniciar a leitura da linha, ler o endereço de injeção
    while (fgets(Code_Half1, 9, codefile) != NULL) {

    // Realizar a conversão da string no code.txt para o endereço de injeção
    Inj_Addr = (uint32_t)strtoul(Code_Half1, NULL, 16);
	
    // Obter a string da instrução e converter, ou parar se atingir o limite
    fseek(codefile, (9 + (Inst_Shift * Linhas_Processadas)), SEEK_SET);

    if (fgets(Code_Half1, 9, codefile) == NULL) {
        printf("Não foi possível ler instrução/valor da linha %d. Abortando.\n", Linhas_Processadas + 1);
        break;
    }
    Inst = (uint32_t)strtoul(Code_Half1, NULL, 16);

    // Deslocar 3 bytes para isolar o tipo da linha de trapaça
    uint8_t Code_Type = Inj_Addr >> 24;
	
    // Tornar o endereço de injeção um local de memória adequado
    Inj_Addr = ((Inj_Addr << 8) >> 8) + 0x80000000;
    printf("L%03d: ", Linhas_Processadas +1);

    // Criar uns valores pra começar a escanear as seções do executável
    int Analyzed_Sections = 0,
        Use_Data = 0;

    // Iniciar o scan; o limite de seções de Texto é 7, mas tem código dentro desse loop pra mudar pro scan de seções de Dados
    while (Analyzed_Sections < 11) {
    uint32_t CurSect_Virtual, CurSect_MaxBounds = 0;

    if (Use_Data == 0) {
    CurSect_Virtual = Big_Endian(Dol.Text_Virtual[Analyzed_Sections]);
    CurSect_MaxBounds = CurSect_Virtual + Big_Endian(Dol.Text_Lengths[Analyzed_Sections]);
    } else {
    CurSect_Virtual = Big_Endian(Dol.Data_Virtual[Analyzed_Sections]);
    CurSect_MaxBounds = CurSect_Virtual + Big_Endian(Dol.Data_Lengths[Analyzed_Sections]);
    }

    // Considerado não injetável quando: após escanear todas as seções de texto, quando uma seção de dados nula é alcançada ou quando nenhuma seção de dados adequada é encontrada
    if ((CurSect_Virtual == 0 && Use_Data == 1) || (Analyzed_Sections == 10 && Use_Data == 1) ) {
        printf("Não injetável; fora dos limites do executável\n");
        break;
    }

    if (Inj_Addr >= CurSect_Virtual && Inj_Addr <= CurSect_MaxBounds) {

            // obter endereço físico no executável
            uint32_t Inj_Addr_Physical = 0;

            // Alternar entre usar o endereço físico das seções de Dados/Texto
            if (Use_Data == 0) {
            Inj_Addr_Physical = (Inj_Addr - CurSect_Virtual) + Big_Endian(Dol.Text_Physical[Analyzed_Sections]);
            } else Inj_Addr_Physical = (Inj_Addr - CurSect_Virtual) + Big_Endian(Dol.Data_Physical[Analyzed_Sections]);

            uint32_t Spot = (Inj_Addr_Physical << 28) >> 28;

            if (Code_Type == 4 && (Spot << 30) >> 30 >= 1) {
            printf("Tentativa de escrever 32 bits a endereço não alinhado, ignorando.\n");
            break;
            }

            if ((Code_Type == 1 && Inst > 0xFF) || (Code_Type == 2 && Inst > 0xFFFF)) {
            printf("Valor fora do limite de %d bits, abortando.\n", Code_Type == 1? 8 : 16);
            break;
            }
			
            if (Code_Type == 2) {
                struct Dol_File Access;

                if (Spot == 0 || Spot == 4 || Spot == 8 || Spot == 0xC) {
                fseek(dolfile, Inj_Addr_Physical, SEEK_SET);
                fread(&Access, sizeof(Access), 1, dolfile);
                Inst = (Big_Endian(Access.Text_Physical[0]) << 16) >> 16 | Inst << 16;

                } else if (Spot == 2 || Spot == 6 || Spot == 0xA || Spot == 0xE) {
                Inj_Addr_Physical = Inj_Addr_Physical - 2;
                fseek(dolfile, Inj_Addr_Physical, SEEK_SET);
                fread(&Access, sizeof(Access), 1, dolfile);
                Inst = (Big_Endian(Access.Text_Physical[0]) >> 16) << 16 | Inst;
                } else { printf("Tentativa de escrever 16 bits a endereço não alinhado, ignorando.\n");
                break;
                }
            }

            printf("Aplicando %08X, tipo %d, endereço 0x%06X | %X", Inst, Code_Type, Inj_Addr_Physical, Inj_Addr);
            if (Code_Type == 4) { printf(": ");
            DisASM(Inst, Inj_Addr);
            } else printf("\n");
            // Decompilar e printar instrução

            if (Simulation_Mode == 0) {

            // A mensagem acima printa a instrução, mas será necessário “desfazer a correção” pra injetar, a menos que o código seja do tipo 1
            if (Code_Type != 1) {
            Inst = Big_Endian(Inst);
            }
            fseek(dolfile, Inj_Addr_Physical, SEEK_SET);
            fwrite(&Inst, Code_Type == 1? 1 : sizeof(Inst), 1, dolfile);

            if(ferror(dolfile)) {
                printf("Não foi possível aplicar mudanças ao executável. Abortando.\n");
                fclose(dolfile);
                fclose(codefile);
                return 0;
            }
        }   break;
    }

    if (Analyzed_Sections++ == 7 && Use_Data == 0) {
        Use_Data = 1;
        Analyzed_Sections = 0;
    }
}
        Linhas_Processadas++;
        fseek(codefile, (19 * Linhas_Processadas), SEEK_SET);
}
    printf("Concluído!\n");
    fclose(dolfile);
    fclose(codefile);
    return 0;
}

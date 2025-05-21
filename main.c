// Gustavo Henrique de Sousa Santos - 10721355
// Felipe Kanamaru de Oliveira - 10435742
// Felipe Silva Siqueira - 10445036

#include <stdio.h>
#include <stdlib.h>

/**
 * Essa função ler_int_little_endian serve pra pegar um número inteiro que está dividido em vários bytes, seguindo o formato "little endian", que é o padrão usado em arquivos BMP.
 *
 * No formato little endian, os bytes vêm na ordem "invertida", ou seja:
 * o byte menos importante (o que vale menos) vem primeiro, e o mais importante (o que vale mais) vem depois.
 *
 * Exemplo: se a matriz tiver assim:
 * matriz[10] = 0xF5
 * matriz[11] = 0xA1
 * matriz[12] = 0x33
 * matriz[13] = 0x44
 *
 * Esses 4 bytes representam o número 0x4433A1F5 (que é 1144234485 em decimal), mas estão armazenados em ordem invertida (F5 A1 33 44).
 *
 * A função abaixo lê esses bytes da matriz (entre as posições 'inicio' e 'fim'), e vai montando o número do jeito certo, somando os bytes com seus pesos:
 * O primeiro byte vale 1, o segundo vale 256, o terceiro vale 256², e assim por diante.
 */

int ler_int_little_endian(unsigned char *dados, int inicio, int fim) {
    int resultado = 0;
    int deslocamento = 0;

    // a partir do segundo par de bytes é necessario deslocar eles 8 bits à esquerda e assim sucessivamente para ler o número de maneira certa e não invertida
    for (int i = inicio; i <= fim; i++) {
        resultado += dados[i] << deslocamento;
        deslocamento += 8;
    }

    return resultado;
}


void compactar_quadrante(unsigned char imagem[][1000],int linha_inicio, int linha_fim, int coluna_inicio, int coluna_fim, int bytes_por_pixel, unsigned char vetorR[1000], unsigned char vetorG[1000], unsigned char vetorB[1000], int *indiceCompactado) {
    
    int num_linhas = linha_fim - linha_inicio + 1;
    int num_colunas = coluna_fim - coluna_inicio + 1;

    if (num_linhas <= 3 || num_colunas <= 3) {
        int linha_central = (linha_inicio + linha_fim) / 2;
        int coluna_central = (coluna_inicio + coluna_fim) / 2;

        int base = coluna_central * bytes_por_pixel;

        vetorR[*indiceCompactado] = imagem[linha_central][base + 2];
        vetorG[*indiceCompactado] = imagem[linha_central][base + 1];
        vetorB[*indiceCompactado] = imagem[linha_central][base + 0];
        (*indiceCompactado)++;

        return;
    }

    int meio_linha = (linha_inicio + linha_fim) / 2;
    int meio_coluna = (coluna_inicio + coluna_fim) / 2;

    compactar_quadrante(imagem, linha_inicio, meio_linha, coluna_inicio, meio_coluna, bytes_por_pixel, vetorR, vetorG, vetorB, indiceCompactado);
    compactar_quadrante(imagem, linha_inicio, meio_linha, meio_coluna + 1, coluna_fim, bytes_por_pixel, vetorR, vetorG, vetorB, indiceCompactado);
    compactar_quadrante(imagem, meio_linha + 1, linha_fim, coluna_inicio, meio_coluna, bytes_por_pixel, vetorR, vetorG, vetorB, indiceCompactado);
    compactar_quadrante(imagem, meio_linha + 1, linha_fim, meio_coluna + 1, coluna_fim, bytes_por_pixel, vetorR, vetorG, vetorB, indiceCompactado);
}

void reconstruir_imagem(unsigned char imagem[][1000], int linha_inicio, int linha_fim, int coluna_inicio, int coluna_fim, int bytes_por_pixel, unsigned char vetorR[], unsigned char vetorG[], unsigned char vetorB[], int *indiceRGB) {
    int num_linhas = linha_fim - linha_inicio + 1;
    int num_colunas = coluna_fim - coluna_inicio + 1;

    if (num_linhas <= 3 || num_colunas <= 3) {
        for (int linha = linha_inicio; linha <= linha_fim; linha++) {
            for (int coluna = coluna_inicio; coluna <= coluna_fim; coluna++) {
                int base = coluna * bytes_por_pixel;
                imagem[linha][base + 2] = vetorR[*indiceRGB];
                imagem[linha][base + 1] = vetorG[*indiceRGB];
                imagem[linha][base + 0] = vetorB[*indiceRGB];
            }
        }
        (*indiceRGB)++;
        return;
    }

    int meio_linha = (linha_inicio + linha_fim) / 2;
    int meio_coluna = (coluna_inicio + coluna_fim) / 2;

    reconstruir_imagem(imagem, linha_inicio, meio_linha, coluna_inicio, meio_coluna, bytes_por_pixel, vetorR, vetorG, vetorB, indiceRGB);
    reconstruir_imagem(imagem, linha_inicio, meio_linha, meio_coluna + 1, coluna_fim, bytes_por_pixel, vetorR, vetorG, vetorB, indiceRGB);
    reconstruir_imagem(imagem, meio_linha + 1, linha_fim, coluna_inicio, meio_coluna, bytes_por_pixel, vetorR, vetorG, vetorB, indiceRGB);
    reconstruir_imagem(imagem, meio_linha + 1, linha_fim, meio_coluna + 1, coluna_fim, bytes_por_pixel, vetorR, vetorG, vetorB, indiceRGB);
}


int main() {

	const char *FILENAME = "/workspaces/ProjetoImagemCaptacao/imagem22x20.bmp";
	const char *FILENAME_SAIDA = "e:\\saida.BMP";
    FILE *arquivo = fopen(FILENAME, "rb"); 
    if(arquivo == NULL) {
		printf("Erro na abertura do arquivo de entrada");
		return -1;
	}

	FILE *arquivo_saida = fopen(FILENAME_SAIDA, "wb");
	if(arquivo_saida == NULL) {
		printf("Erro na abertura do arquivo de saída");
		return -1;
	}

	unsigned char byte;
	unsigned char vetorTotal[10000];    
	int percorrendo = 0;    
    printf("PRINTANDO TODOS OS BYTES:\n");
	while(!feof(arquivo)) {
		fread(&byte, 1, 1, arquivo);
        vetorTotal[percorrendo] = byte;
        percorrendo ++; 
        printf("%02x ", byte);
	}  
    
    // lendo as informacoes principais
    int tamanho_total = ler_int_little_endian(vetorTotal,2,5);
    int offset  = ler_int_little_endian(vetorTotal, 10, 13);
    int qtde_colunas_por_linhas = ler_int_little_endian(vetorTotal, 18, 21);
    int qtde_linhas_pixels  = ler_int_little_endian(vetorTotal, 22, 25);
    int bits_por_pixel = ler_int_little_endian(vetorTotal, 28,28);
    printf("\n\nPrincipais informações:\ntamanho = %d bytes; offset = %d; pixels por linha = %d; qtde linhas = %d; bits por pixel = %d\n", tamanho_total, offset, qtde_colunas_por_linhas, qtde_linhas_pixels, bits_por_pixel);

    int bytes_por_pixel = bits_por_pixel / 8;


    int bytes_por_linha_sem_acrescimo = qtde_colunas_por_linhas * 3; 
    int acrescimo = (4 - (bytes_por_linha_sem_acrescimo % 4)) % 4;
    int bytes_por_linha = bytes_por_linha_sem_acrescimo + acrescimo;

    printf("\nQtde de bytes por linha (já com o acrescimo) (colunas) = %d\n", bytes_por_linha);

    unsigned char cabecalho[offset];        
    unsigned char matrizImagem[qtde_linhas_pixels][bytes_por_linha]; 
    unsigned char temp;
    
    // preenchendo o cabeçalho e ja printando
    printf("\nPRINTANDO CABECALHO:\n");
    for (int i = 0; i < offset; i++){
        temp = vetorTotal[i];
        cabecalho[i] = temp;
        printf("%02x ", cabecalho[i]);
    }

    // preenchendo e printando a matriz apenas da imagem
    printf("\n\nPRINTANDO MATRIZ APENAS PIXELS DA IMAGEM:\n");
    int aux = offset;
    for (int linha = 0; linha < qtde_linhas_pixels; linha++){
        for (int coluna = 0; coluna < bytes_por_linha; coluna++){
            temp = vetorTotal[aux++];
            if (coluna >= bytes_por_linha_sem_acrescimo)
                matrizImagem[linha][coluna] = 0;
            else
                matrizImagem[linha][coluna] = temp;
            printf("%02x ", matrizImagem[linha][coluna]);
        }
        printf("\n");
    }

    unsigned char vetorR[1000];
    unsigned char vetorG[1000]; 
    unsigned char vetorB[1000];
    int indiceCompactado = 0;

    compactar_quadrante(matrizImagem, 0, qtde_linhas_pixels - 1, 0, qtde_colunas_por_linhas - 1, 3, vetorR, vetorG, vetorB, &indiceCompactado);
    
    printf("\n\nPRINTANDO VETOR R:\n");
    for (int i = 0; i <  indiceCompactado; i++){
        printf("%02x ", vetorR[i]);
    }
    printf("\n\nPRINTANDO VETOR G:\n");
    for (int i = 0; i <  indiceCompactado; i++){
        printf("%02x ", vetorG[i]);
    }
    printf("\n\nPRINTANDO VETOR B:\n");
    for (int i = 0; i < indiceCompactado; i++){
        printf("%02x ", vetorB[i]);
    }

    FILE *arquivo_compactado = fopen("imagemCompactada.zmp", "wb");
    if (arquivo_compactado == NULL) {
    printf("Erro ao criar imagemCompactada.zmp\n");
    return -1;
    }
    
    fwrite(cabecalho, sizeof(unsigned char), offset, arquivo_compactado);
    
    for (int i = 0; i < indiceCompactado; i++) {
    fwrite(&vetorR[i], sizeof(unsigned char), 1, arquivo_compactado);
    fwrite(&vetorG[i], sizeof(unsigned char), 1, arquivo_compactado);
    fwrite(&vetorB[i], sizeof(unsigned char), 1, arquivo_compactado);
    }

    fclose(arquivo_compactado);
    printf("\n\nArquivo imagemCompactada.zmp gerado com sucesso!\n\n");


    
    // Descompactação
    unsigned char cabecalho_zmp[offset];
    fread(cabecalho_zmp, 1, offset, arquivo_compactado);
    unsigned char vetorRzmp[1000], vetorGzmp[1000], vetorBzmp[1000];
    for (int i = 0; i < indiceCompactado; i++) {
        fread(&vetorRzmp[i], 1, 1, arquivo_compactado);
        fread(&vetorGzmp[i], 1, 1, arquivo_compactado);
        fread(&vetorBzmp[i], 1, 1, arquivo_compactado);
    }
    fclose(arquivo_compactado);

    // Reconstrução da matriz
    unsigned char novaImagem[qtde_linhas_pixels][1000];
    int indiceRGB = 0;
    reconstruir_imagem(novaImagem, 0, qtde_linhas_pixels - 1, 0, qtde_colunas_por_linhas - 1, bytes_por_pixel, vetorRzmp, vetorGzmp, vetorBzmp, &indiceRGB);

    // Gravação da nova imagem BMP
    FILE *bmp_saida = fopen("imagemDescompactada.bmp", "wb");
    if (bmp_saida == NULL) {
        printf("Erro ao criar imagemDescompactada.bmp\n");
        return -1;
    }
    fwrite(cabecalho_zmp, 1, offset, bmp_saida);
    for (int i = 0; i < qtde_linhas_pixels; i++) {
        fwrite(novaImagem[i], 1, bytes_por_linha, bmp_saida);
    }
    fclose(bmp_saida);

    printf("Compactação e descompactação concluídas com sucesso!\n");

	fclose(arquivo);
	fclose(arquivo_saida);

    FILE *f_origem = fopen("imagem22x20.bmp", "rb");
    FILE *f_compactado = fopen("imagemCompactada.zmp", "rb");
    FILE *f_descompactado = fopen("imagemDescompactada.bmp", "rb");

    fseek(f_origem, 0, SEEK_END);
    fseek(f_compactado, 0, SEEK_END);
    fseek(f_descompactado, 0, SEEK_END);

    long tamanho_original = ftell(f_origem);
    long tamanho_zmp = ftell(f_compactado);
    long tamanho_final = ftell(f_descompactado);

    fclose(f_origem);
    fclose(f_compactado);
    fclose(f_descompactado);

    float taxa = 100.0 * (1.0 - ((float)tamanho_zmp / tamanho_original));

    printf("\n\n===== RELATÓRIO FINAL =====\n\n");
    printf("Tamanho original         : %ld bytes\n", tamanho_original);
    printf("Tamanho compactado (.zmp): %ld bytes\n", tamanho_zmp);
    printf("Tamanho final (.bmp)     : %ld bytes\n", tamanho_final);
    printf("Taxa de compactação      : %.2f%%\n\n", taxa);

    return 0;
}

// CADA LINHA AQUI TEM 46 VALORES (monitor gustavo)
// do 42 até o 00HD ==> header

/*
42 4d 86 05 00 00 00 00 00 00 36 00 00 00 28 00 00 00 16 00 00 00 14 00 00 00 01 00 18 00 00 00 00 00 50 05 00 00 13 0b 00 00 13 0b 00 00 00 00 00 00 00 00 00 00HD 00 00 00 0c 00 00 18 00 00 24 00 00 30 00 00 3c 00 00 48 00 00 55 00 00 61 00 00 6d 00 00 79 00 00 85 00 00 91 00 00 9d 00 00 aa 00 00 b6 00 00 c2 00 00 ce 00 00 da 00 00 e6 00 00 f2 00 00 ff 00 00 00 00 00 0d 00 0c 0d 00 18 0d 00 24 0d 00 30 0d 00 3c 0d 00 48 0d 00 55 0d 00 61 0d 00 6d 0d 00 79 0d 00 85 0d 00 91 0d 00 9d 0d 00 aa 0d 00 b6 0d 00 c2 0d 00 ce 0d 00 da 0d 00 e6 0d 00 f2 0d 00 ff 0d 00 00 00 00 1a 00 0c 1a 00 18 1a 00 24 1a 00 30 1a 00 3c 1a 00 48 1a 00 55 1a 00 61 1a 00 6d 1a 00 79 1a 00 85 1a 00 91 1a 00 9d 1a 00 aa 1a 00 b6 1a 00 c2 1a 00 ce 1a 00 da 1a 00 e6 1a 00 f2 1a 00 ff 1a 00 00 00 00 28 00 0c 28 00 18 28 00 24 28 00 30 28 00 3c 28 00 48 28 00 55 28 00 61 28 00 6d 28 00 79 28 00 85 28 00 91 28 00 9d 28 00 aa 28 00 b6 28 00 c2 28 00 ce 28 00 da 28 00 e6 28 00 f2 28 00 ff 28 00 00 00 00 35 00 0c 35 00 18 35 00 24 35 00 30 35 00 3c 35 00 48 35 00 55 35 00 61 35 00 6d 35 00 79 35 00 85 35 00 91 35 00 9d 35 00 aa 35 00 b6 35 00 c2 35 00 ce 35 00 da 35 00 e6 35 00 f2 35 00 ff 35 00 00 00 00 43 00 0c 43 00 18 43 00 24 43 00 30 43 00 3c 43 00 48 43 00 55 43 00 61 43 00 6d 43 00 79 43 00 85 43 00 91 43 00 9d 43 00 aa 43 00 b6 43 00 c2 43 00 ce 43 00 da 43 00 e6 43 00 f2 43 00 ff 43 00 00 00 00 50 00 0c 50 00 18 50 00 24 50 00 30 50 00 3c 50 00 48 50 00 55 50 00 61 50 00 6d 50 00 79 50 00 85 50 00 91 50 00 9d 50 00 aa 50 00 b6 50 00 c2 50 00 ce 50 00 da 50 00 e6 50 00 f2 50 00 ff 50 00 00 00 00 5d 00 0c 5d 00 18 5d 00 24 5d 00 30 5d 00 3c 5d 00 48 5d 00 55 5d 00 61 5d 00 6d 5d 00 79 5d 00 85 5d 00 91 5d 00 9d 5d 00 aa 5d 00 b6 5d 00 c2 5d 00 ce 5d 00 da 5d 00 e6 5d 00 f2 5d 00 ff 5d 00 00 00 00 6b 00 0c 6b 00 18 6b 00 24 6b 00 30 6b 00 3c 6b 00 48 6b 00 55 6b 00 61 6b 00 6d 6b 00 79 6b 00 85 6b 00 91 6b 00 9d 6b 00 aa 6b 00 b6 6b 00 c2 6b 00 ce 6b 00 da 6b 00 e6 6b 00 f2 6b 00 ff 6b 00 00 00 00 78 00 0c 78 00 18 78 00 24 78 00 30 78 00 3c 78 00 48 78 00 55 78 00 61 78 00 6d 78 00 79 78 00 85 78 00 91 78 00 9d 78 00 aa 78 00 b6 78 00 c2 78 00 ce 78 00 da 78 00 e6 78 00 f2 78 00 ff 78 00 00 00 00 86 00 0c 86 00 18 86 00 24 86 00 30 86 00 3c 86 00 48 86 00 55 86 00 61 86 00 6d 86 00 79 86 00 85 86 00 91 86 00 9d 86 00 aa 86 00 b6 86 00 c2 86 00 ce 86 00 da 86 00 e6 86 00 f2 86 00 ff 86 00 00 00 00 93 00 0c 93 00 18 93 00 24 93 00 30 93 00 3c 93 00 48 93 00 55 93 00 61 93 00 6d 93 00 79 93 00 85 93 00 91 93 00 9d 93 00 aa 93 00 b6 93 00 c2 93 00 ce 93 00 da 93 00 e6 93 00 f2 93 00 ff 93 00 00 00 00 a1 00 0c a1 00 18 a1 00 24 a1 00 30 a1 00 3c a1 00 48 a1 00 55 a1 00 61 a1 00 6d a1 00 79 a1 00 85 a1 00 91 a1 00 9d a1 00 aa a1 00 b6 a1 00 c2 a1 00 ce a1 00 da a1 00 e6 a1 00 f2 a1 00 ff a1 00 00 00 00 ae 00 0c ae 00 18 ae 00 24 ae 00 30 ae 00 3c ae 00 48 ae 00 55 ae 00 61 ae 00 6d ae 00 79 ae 00 85 ae 00 91 ae 00 9d ae 00 aa ae 00 b6 ae 00 c2 ae 00 ce ae 00 da ae 00 e6 ae 00 f2 ae 00 ff ae 00 00 00 00 bb 00 0c bb 00 18 bb 00 24 bb 00 30 bb 00 3c bb 00 48 bb 00 55 bb 00 61 bb 00 6d bb 00 79 bb 00 85 bb 00 91 bb 00 9d bb 00 aa bb 00 b6 bb 00 c2 bb 00 ce bb 00 da bb 00 e6 bb 00 f2 bb 00 ff bb 00 00 00 00 c9 00 0c c9 00 18 c9 00 24 c9 00 30 c9 00 3c c9 00 48 c9 00 55 c9 00 61 c9 00 6d c9 00 79 c9 00 85 c9 00 91 c9 00 9d c9 00 aa c9 00 b6 c9 00 c2 c9 00 ce c9 00 da c9 00 e6 c9 00 f2 c9 00 ff c9 00 00 00 00 d6 00 0c d6 00 18 d6 00 24 d6 00 30 d6 00 3c d6 00 48 d6 00 55 d6 00 61 d6 00 6d d6 00 79 d6 00 85 d6 00 91 d6 00 9d d6 00 aa d6 00 b6 d6 00 c2 d6 00 ce d6 00 da d6 00 e6 d6 00 f2 d6 00 ff d6 00 00 00 00 e4 00 0c e4 00 18 e4 00 24 e4 00 30 e4 00 3c e4 00 48 e4 00 55 e4 00 61 e4 00 6d e4 00 79 e4 00 85 e4 00 91 e4 00 9d e4 00 aa e4 00 b6 e4 00 c2 e4 00 ce e4 00 da e4 00 e6 e4 00 f2 e4 00 ff e4 00 00 00 00 f1 00 0c f1 00 18 f1 00 24 f1 00 30 f1 00 3c f1 00 48 f1 00 55 f1 00 61 f1 00 6d f1 00 79 f1 00 85 f1 00 91 f1 00 9d f1 00 aa f1 00 b6 f1 00 c2 f1 00 ce f1 00 da f1 00 e6 f1 00 f2 f1 00 ff f1 00 00 00 00 ff 00 0c ff 00 18 ff 00 24 ff 00 30 ff 00 3c ff 00 48 ff 00 55 ff 00 61 ff 00 6d ff 00 79 ff 00 85 ff 00 91 ff 00 9d ff 00 aa ff 00 b6 ff 00 c2 ff 00 ce ff 00 da ff 00 e6 ff 00 f2 ff 00 ff ff 00 00 00 00
*/


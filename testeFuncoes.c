  #include <stdio.h>
  #include <stdlib.h>
  
  int ler_int_little_endian(unsigned char *dados, int inicio, int fim) {
      int resultado = 0, deslocamento = 0;
      for (int i = inicio; i <= fim; i++) {
          resultado += dados[i] << deslocamento;
          deslocamento += 8;
      }
      return resultado;
  }
  
  void compactar_quadrante(unsigned char imagem[][1000], int linha_inicio, int linha_fim, int coluna_inicio, int coluna_fim, int bytes_por_pixel, unsigned char vetorR[], unsigned char vetorG[], unsigned char vetorB[], int *indiceCompactado) {
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
  
  void processar_imagem(const char *input_bmp, const char *saida_bmp, const char *compactado_zmp, const char *descompactado_bmp) {
    printf("\nPROCESSANDO IMAGEM: %s", input_bmp);
      FILE *arquivo = fopen(input_bmp, "rb");
      if (!arquivo) {
          printf("\nErro ao abrir %s\n", input_bmp);
          return;
      }
  
      unsigned char vetorTotal[10000];
      int percorrendo = 0;
      unsigned char byte;
      while (fread(&byte, 1, 1, arquivo)) vetorTotal[percorrendo++] = byte;
      fclose(arquivo);
  
      int offset = ler_int_little_endian(vetorTotal, 10, 13);
      int tipo_bmp = ler_int_little_endian(vetorTotal, 0, 1);
      int bits_por_pixel = ler_int_little_endian(vetorTotal, 28, 28);
      if (tipo_bmp != 0x4D42 || bits_por_pixel != 24) {
          printf("\nErro: arquivo %s não é um BMP 24 bits suportado.\n", input_bmp);
          return;
      }
      int qtde_colunas = ler_int_little_endian(vetorTotal, 18, 21);
      int qtde_linhas = ler_int_little_endian(vetorTotal, 22, 25);
      int bytes_por_pixel = bits_por_pixel / 8;
      int bytes_sem_padding = qtde_colunas * bytes_por_pixel;
      int padding = (4 - (bytes_sem_padding % 4)) % 4;
      int bytes_por_linha = bytes_sem_padding + padding;
  
      unsigned char cabecalho[offset];
      for (int i = 0; i < offset; i++) cabecalho[i] = vetorTotal[i];
  
      unsigned char imagem[qtde_linhas][1000];
      int aux = offset;
      for (int i = 0; i < qtde_linhas; i++)
          for (int j = 0; j < bytes_por_linha; j++)
              imagem[i][j] = (j >= bytes_sem_padding) ? 0 : vetorTotal[aux++];
  
      unsigned char vetorR[1000], vetorG[1000], vetorB[1000];
      int indiceCompactado = 0;
      compactar_quadrante(imagem, 0, qtde_linhas - 1, 0, qtde_colunas - 1, bytes_por_pixel, vetorR, vetorG, vetorB, &indiceCompactado);
  
      FILE *zmp = fopen(compactado_zmp, "wb");
      fwrite(cabecalho, 1, offset, zmp);
      for (int i = 0; i < indiceCompactado; i++) {
          fwrite(&vetorR[i], 1, 1, zmp);
          fwrite(&vetorG[i], 1, 1, zmp);
          fwrite(&vetorB[i], 1, 1, zmp);
      }
      fclose(zmp);
  
      zmp = fopen(compactado_zmp, "rb");
      fread(cabecalho, 1, offset, zmp);
      unsigned char vetorRzmp[1000], vetorGzmp[1000], vetorBzmp[1000];
      for (int i = 0; i < indiceCompactado; i++) {
          fread(&vetorRzmp[i], 1, 1, zmp);
          fread(&vetorGzmp[i], 1, 1, zmp);
          fread(&vetorBzmp[i], 1, 1, zmp);
      }
      fclose(zmp);
  
      unsigned char imagemReconstruida[qtde_linhas][1000];
      int indiceRGB = 0;
      reconstruir_imagem(imagemReconstruida, 0, qtde_linhas - 1, 0, qtde_colunas - 1, bytes_por_pixel, vetorRzmp, vetorGzmp, vetorBzmp, &indiceRGB);
  
      FILE *bmp_saida = fopen(descompactado_bmp, "wb");
      fwrite(cabecalho, 1, offset, bmp_saida);
      for (int i = 0; i < qtde_linhas; i++)
          fwrite(imagemReconstruida[i], 1, bytes_por_linha, bmp_saida);
      fclose(bmp_saida);
  
      FILE *f1 = fopen(input_bmp, "rb");
      FILE *f2 = fopen(compactado_zmp, "rb");
      FILE *f3 = fopen(descompactado_bmp, "rb");
      fseek(f1, 0, SEEK_END); fseek(f2, 0, SEEK_END); fseek(f3, 0, SEEK_END);
      long t1 = ftell(f1), t2 = ftell(f2), t3 = ftell(f3);
      fclose(f1); fclose(f2); fclose(f3);
  
      float taxa = 100.0 * (1.0 - ((float)t2 / t1));
      printf("\nRELATÓRIO FINAL PARA %s\n", input_bmp);
      printf("Tamanho original         : %ld bytes\n", t1);
      printf("Tamanho compactado (.zmp): %ld bytes\n", t2);
      printf("Tamanho final (.bmp)     : %ld bytes\n", t3);
      printf("Taxa de compactação      : %.2f%%\n", taxa);
  }
  
  int main() {
      processar_imagem("imagem22x20.bmp", "saida.bmp", "imagemCompactada.zmp", "imagemDescompactada.bmp");

      return 0;
  }

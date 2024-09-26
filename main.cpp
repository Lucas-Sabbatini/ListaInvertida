/*
 * Alunos:
 * Lucas Sabbatini Janot Procópio 12211BCC019
 * Gustavo Alves Kuabara
 * João Caio Pereira Melo 12211BCC052
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>

using namespace std;

// remove pontuacao de uma palavra
void removePontuacao (char *palavra) {
    int length = strlen(palavra);
    if (
        (palavra[length-1] == '.') || (palavra[length-1] == ',') || (palavra[length-1] == ';') ||
        (palavra[length-1] == ':') || (palavra[length-1] == '?') || (palavra[length-1] == '!')
       )
        palavra[length-1] = '\0';
}

// imprime linha do arquivo com base no offset da palavra
void imprimeLinha(int offset,FILE *f) {
    int pos = ftell(f);
    char linha[2048];
    while (pos < offset) {
        fgets(linha,2047,f);
        pos = ftell(f);
    }
    printf("%s",linha);
}

// classe que implementa a lista invertida
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;
class listaInvertida {
private:
    unordered_map<string, vector<int>> indice; // Índice em memória
    unordered_map<string, vector<int>> indiceSecundario; // Índice secundário para busca rápida no disco

public:
    // Construtor
    listaInvertida() { }

    // Destrutor
    ~listaInvertida() { }

    // Adiciona palavra na estrutura com o offset
    void adiciona(char *palavra, int offset) {
        string palavraStr(palavra); // Convertendo o char* para string
        indice[palavraStr].push_back(offset); // Adiciona o offset ao vetor correspondente à palavra

        // Adiciona ao arquivo binário
        ofstream out("indice.bin", ios::binary | ios::app);
        if (out.is_open()) {
            int length = palavraStr.size();
            out.write(reinterpret_cast<char*>(&length), sizeof(int));
            out.write(palavraStr.c_str(), length);
            out.write(reinterpret_cast<char*>(&offset), sizeof(int));
            out.close();
        }
    }

    // Carrega o índice secundário
    void carregaIndiceSecundario() {
        ifstream in("indice.bin", ios::binary);
        if (in.is_open()) {
            while (!in.eof()) {
                int length;
                in.read(reinterpret_cast<char*>(&length), sizeof(int));
                if (in.eof()) break;
                char *palavra = new char[length + 1];
                in.read(palavra, length);
                palavra[length] = '\0';
                int offset;
                in.read(reinterpret_cast<char*>(&offset), sizeof(int));
                indiceSecundario[palavra].push_back(offset);
                delete[] palavra;
            }
            in.close();
        }
    }

    // Realiza busca, retornando vetor de offsets que referenciam a palavra
    int* busca(char *palavra, int *quantidade) {
        string palavraStr(palavra); // Convertendo o char* para string

        // Procurar pela palavra no índice em memória
        auto it = indice.find(palavraStr);
        if (it != indice.end()) {
            // Palavra encontrada, retornamos os offsets
            *quantidade = it->second.size();
            int *offsets = new int[*quantidade];
            
            // Copiando os valores do vetor de offsets
            for (int i = 0; i < *quantidade; i++) {
                offsets[i] = it->second[i];
            }
            return offsets;
        }

        // Se não encontrado em memória, procurar no índice secundário
        auto itSec = indiceSecundario.find(palavraStr);
        if (itSec != indiceSecundario.end()) {
            // Palavra encontrada no índice secundário, retornamos os offsets
            *quantidade = itSec->second.size();
            int *offsets = new int[*quantidade];
            
            // Copiando os valores do vetor de offsets
            for (int i = 0; i < *quantidade; i++) {
                offsets[i] = itSec->second[i];
            }
            return offsets;
        }

        // Palavra não encontrada
        *quantidade = 0;
        return nullptr;
    }
};  map<string, vector<int>> indice;
};


// programa principal
int main(int argc, char** argv) {
    // abrir arquivo
    ifstream in("biblia.txt");
    if (!in.is_open()){
        printf("\n\n Nao consegui abrir arquivo biblia.txt. Sinto muito.\n\n\n\n");
    }
    else{
        // vamos ler o arquivo e criar a lista invertida com as palavras do arquivo
        char *palavra = new char[100];
        int offset, contadorDePalavras = 0;
        listaInvertida lista;
        // ler palavras
        while (!in.eof()) {
            // ler palavra
            in >> palavra;
            // pegar offset
            offset = in.tellg();
            // remover pontuacao
            removePontuacao(palavra);
            // desconsiderar palavras que sao marcadores do arquivo
            if (!((palavra[0] == '#') || (palavra[0] == '[') || ((palavra[0] >= '0') && (palavra[0] <= '9')))) {
                //printf("%d %s\n", offset,palavra); fflush(stdout); // debug :-)
                lista.adiciona(palavra, offset);
                contadorDePalavras++;
                if (contadorDePalavras % 1000 == 0) { printf(".");  fflush(stdout); }
            }
        }
        in.close();

        // agora que ja construimos o indice, podemos realizar buscas
        do {
            printf("\nDigite a palavra desejada ou \"SAIR\" para sair: ");
            scanf("%s",palavra);
            if (strcmp(palavra,"SAIR") != 0) {
                int quantidade;
                // busca na lista invertida
                int *offsets = lista.busca(palavra,&quantidade);
                // com vetor de offsets, recuperar as linhas que contem a palavra desejada
                if (quantidade > 0) {
                    FILE *f = fopen("biblia.txt","rt");
                    for (int i = 0; i < quantidade; i++)
                        imprimeLinha(offsets[i],f);
                    fclose(f);
                }
                else
                    printf("nao encontrou %s\n",palavra);
            }
        } while (strcmp(palavra,"SAIR") != 0);

        printf("\n\nAte mais!\n\n");
    }

    return (EXIT_SUCCESS);
}


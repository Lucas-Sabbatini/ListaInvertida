/*
 * Alunos:
 * Lucas Sabbatini Janot Procópio 12211BCC019
 * Gustavo Alves Kuabara 12211BCC035
 * João Caio Pereira Melo 12211BCC052
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

// remove pontuação de uma palavra
void removePontuacao(char *palavra) {
    int length = strlen(palavra);
    if (
        (palavra[length - 1] == '.') || (palavra[length - 1] == ',') || (palavra[length - 1] == ';') ||
        (palavra[length - 1] == ':') || (palavra[length - 1] == '?') || (palavra[length - 1] == '!')
    )
        palavra[length - 1] = '\0';
}

// imprime linha do arquivo com base no número da linha
void imprimeLinha(int linhaNum, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        printf("Nao foi possivel abrir o arquivo.\n");
        return;
    }
    string linha;
    int currentLine = 1;
    while (getline(file, linha)) {
        if (currentLine == linhaNum) {
            cout << linha << endl; //saida padrao + \n e limpa buffer
            break;
        }
        currentLine++;
    }
    file.close();
}

// classe que implementa a lista invertida
class listaInvertida {
public:
    // Construtor
    listaInvertida(const string& filename) : arquivoLista(filename) {
        // Carrega o índice do arquivo, se existir
        ifstream file(arquivoLista);
        if (file.is_open()) {
            string palavra;
            int linhaNum;
            while (file >> palavra) {
                vector<int> linhas;
                while (file.peek() != '\n' && file >> linhaNum) {
                    linhas.push_back(linhaNum);
                }
                indice[palavra] = linhas;
            }
            file.close();
        }
    }

    // Destrutor
    ~listaInvertida() {
        // Salva o índice no arquivo ao final
        ofstream file(arquivoLista, ios::trunc);
        if (file.is_open()) {
            for (const auto& par : indice) {
                file << par.first;
                for (int linhaNum : par.second) {
                    file << " " << linhaNum;
                }
                file << "\n";
            }
            file.close();
        }
    }

    // Adiciona palavra na estrutura com o número da linha
    void adiciona(char *palavra, int linhaNum) {
        string palavraStr(palavra); // Convertendo o char* para string
        indice[palavraStr].push_back(linhaNum); // Adiciona o número da linha ao vetor correspondente à palavra
    }

    // Realiza busca, retornando vetor de números de linha que referenciam a palavra
    int* busca(char *palavra, int *quantidade) {
        string palavraStr(palavra); // Convertendo o char* para string

        // Procurar pela palavra no índice
        auto it = indice.find(palavraStr);
        if (it != indice.end()) {
            // Palavra encontrada, retornamos os números das linhas
            *quantidade = it->second.size();
            int *linhas = new int[*quantidade];
            
            // Copiando os valores do vetor de números de linha
            for (int i = 0; i < *quantidade; i++) {
                linhas[i] = it->second[i];
            }
            return linhas;
        } else {
            // Palavra não encontrada
            *quantidade = 0;
            return nullptr;
        }
    }

private:
    string arquivoLista;
    map<string, vector<int>> indice;
};

// programa principal
int main(int argc, char** argv) {
    // Caminho do arquivo de dados (Bíblia) e da lista invertida
    const string bibliaPath = "biblia.txt";
    const string listaInvertidaPath = "lista_invertida.txt";

    // abrir arquivo da bíblia
    ifstream in(bibliaPath);
    if (!in.is_open()) {
        printf("\n\n Não consegui abrir arquivo biblia.txt. Sinto muito.\n\n\n\n");
    } else {
        // Vamos ler o arquivo e criar a lista invertida com as palavras do arquivo
        char *palavra = new char[100];
        int linhaNum = 1, contadorDePalavras = 0;
        listaInvertida lista(listaInvertidaPath);
        string linha;

        // Ler linhas
        while (getline(in, linha)) {
            // Quebrar a linha em palavras
            char *palavraPtr = strtok(&linha[0], " ");
            while (palavraPtr != nullptr) {
                // Remover pontuação
                removePontuacao(palavraPtr);
                // Desconsiderar palavras que são marcadores do arquivo
                if (!((palavraPtr[0] == '#') || (palavraPtr[0] == '[') || ((palavraPtr[0] >= '0') && (palavraPtr[0] <= '9')))) {
                    lista.adiciona(palavraPtr, linhaNum);
                    contadorDePalavras++;
                    if (contadorDePalavras % 1000 == 0) {
                        printf(".");
                        fflush(stdout);
                    }
                }
                palavraPtr = strtok(nullptr, " ");
            }
            linhaNum++; // Incrementa o número da linha
        }
        in.close();

        // Agora que já construímos o índice, podemos realizar buscas
        do {
            printf("\nDigite a palavra desejada ou \"SAIR\" para sair: ");
            scanf("%s", palavra);
            if (strcmp(palavra, "SAIR") != 0) {
                int quantidade;
                // Busca na lista invertida
                int *linhas = lista.busca(palavra, &quantidade);
                // Com vetor de números de linha, recuperar as linhas que contêm a palavra desejada
                if (quantidade > 0) {
                    for (int i = 0; i < quantidade; i++) {
                        imprimeLinha(linhas[i], bibliaPath);
                    }
                    delete[] linhas;
                } else {
                    printf("não encontrou %s\n", palavra);
                }
            }
        } while (strcmp(palavra, "SAIR") != 0);

        printf("\n\nAté mais!\n\n");
    }

    return (EXIT_SUCCESS);
}

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
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <zlib.h>

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
void imprimeLinha(int offset, FILE *f) {
    int pos = ftell(f);
    char linha[2048];
    while (pos < offset) {
        fgets(linha, 2047, f);
        pos = ftell(f);
    }
    printf("%s", linha);
}

std::vector<char> compressData(const std::string& data) {
    uLongf compressedSize = compressBound(data.size());
    std::vector<char> compressedData(compressedSize);

    if (compress(reinterpret_cast<Bytef*>(compressedData.data()), &compressedSize, 
                 reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK) {
        throw std::runtime_error("Compression failed");
    }

    compressedData.resize(compressedSize);
    return compressedData;
}

std::string decompressData(const std::vector<char>& compressedData, uLongf originalSize) {
    std::vector<char> decompressedData(originalSize);

    if (uncompress(reinterpret_cast<Bytef*>(decompressedData.data()), &originalSize, 
                   reinterpret_cast<const Bytef*>(compressedData.data()), compressedData.size()) != Z_OK) {
        throw std::runtime_error("Decompression failed");
    }

    return std::string(decompressedData.begin(), decompressedData.end());
}

class listaInvertida {
private:
    unordered_map<string, vector<int>> indice; // Índice em memória
    unordered_map<string, vector<int>> indiceSecundario; // Índice secundário para busca rápida no disco
    vector<pair<string, int>> buffer; // Buffer para batch writing
    const size_t BUFFER_SIZE = 1000; // Tamanho do buffer

    // Função para escrever o buffer no arquivo
    void flushBuffer() {
        ofstream out("indice.bin", ios::binary | ios::app);
        if (out.is_open()) {
            for (const auto& entry : buffer) {
                std::string data = entry.first + std::to_string(entry.second);
                auto compressedData = compressData(data);
                uLongf originalSize = data.size();
                out.write(reinterpret_cast<const char*>(&originalSize), sizeof(uLongf));
                out.write(compressedData.data(), compressedData.size());
            }
            out.close();
            buffer.clear(); // Limpar o buffer após escrever
        }
    }

public:
    // Construtor
    listaInvertida() { }

    // Destrutor
    ~listaInvertida() {
        flushBuffer(); // Garantir que todos os dados sejam escritos no arquivo
    }

    // Adiciona palavra na estrutura com o offset
    void adiciona(char *palavra, int offset) {
        string palavraStr(palavra); // Convertendo o char* para string
        indice[palavraStr].push_back(offset); // Adiciona o offset ao vetor correspondente à palavra

        // Adiciona ao buffer
        buffer.emplace_back(palavraStr, offset);

        // Se o buffer atingir o tamanho definido, escrever no arquivo
        if (buffer.size() >= BUFFER_SIZE) {
            flushBuffer();
        }
    }

    // Carrega o índice secundário
    void carregaIndiceSecundario() {
        ifstream in("indice.bin", ios::binary);
        if (in.is_open()) {
            while (!in.eof()) {
                uLongf originalSize;
                in.read(reinterpret_cast<char*>(&originalSize), sizeof(uLongf));
                if (in.eof()) break;
                std::vector<char> compressedData(originalSize);
                in.read(compressedData.data(), originalSize);
                std::string data = decompressData(compressedData, originalSize);
                std::string palavra = data.substr(0, data.size() - sizeof(int));
                int offset = std::stoi(data.substr(data.size() - sizeof(int)));
                indiceSecundario[palavra].push_back(offset);
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
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <queue>
#include <unistd.h>
#include <cstring>
#include <fstream>
#define FALSE 0
#define TRUE 1

#define ORDEM 3
#define DOBRO_ORDEM 6

using namespace std;

typedef struct Indice{
    char Chave[50];
    int posicaoArquivo;
}Indice;

typedef struct Pagina *Apontador;

typedef struct Pagina{
    short qtdRegistros;
    Indice indices[DOBRO_ORDEM];
    Apontador ponteiros[DOBRO_ORDEM + 1];
}Pagina;

typedef struct Registro{
    char Chave[50];
    char Significado[100];
}Registro;


//Função de pesquisa na árvore B.

void PesquisaEspecifica(char chave[], Apontador pontPagina, Registro *regRetorno) {
    long i = 1;
    Indice indiceTemp;
    fstream fd1;
    fd1.open("registros.bin", fstream::binary | fstream::in | fstream::out);

    if (pontPagina == NULL) {
        return;
    }

    while (i < pontPagina->qtdRegistros && strcmp(chave, pontPagina->indices[i-1].Chave) > 0) i++;

    if (strcmp(chave, pontPagina->indices[i-1].Chave) == 0) {
        indiceTemp = pontPagina->indices[i - 1];
        fd1.seekg(indiceTemp.posicaoArquivo*sizeof(Registro), ios::beg);
        int x = sizeof(*regRetorno);
        fd1.read((char*)regRetorno, x);
        fd1.close();
        return;
    }

    if (strcmp(chave, pontPagina->indices[i-1].Chave) < 0)
        PesquisaEspecifica(chave, pontPagina->ponteiros[i - 1], regRetorno);
    else
        PesquisaEspecifica(chave, pontPagina->ponteiros[i], regRetorno);

}

void printLevelOrder(Apontador root) {
  if (root == NULL){
      return;
  } 
  queue<Apontador> filaPaginas;
  int paginasNivelAtual = 1;
  int paginasProximoNivel = 0;
  filaPaginas.push(root);
  while (!filaPaginas.empty()) {
    Apontador paginaAtual = filaPaginas.front();
    filaPaginas.pop();
    paginasNivelAtual--;
    if (paginaAtual){
        for(int i=0;i<paginaAtual->qtdRegistros;i++){
            cout << paginaAtual->indices[i].Chave << " ";
        }
        for(int i=0;i<(DOBRO_ORDEM+1);i++){
            filaPaginas.push(paginaAtual->ponteiros[i]);
        }
        paginasProximoNivel += DOBRO_ORDEM+1;
    }
    if (paginasNivelAtual == 0) {
      cout << endl;
      paginasNivelAtual = paginasProximoNivel;
      paginasProximoNivel = 0;
    }
  }
}

//Retorna as palavras da árvore que estão dentro do intervalo

void devolveIntervalos(char intervalo1[], char intervalo2[], Apontador raiz, vector<Indice> &regs) {
    if (raiz == NULL) {
        return;
    }
    for (int i = 0; i < raiz->qtdRegistros; i++) {
        devolveIntervalos(intervalo1, intervalo2, raiz->ponteiros[i], regs);
        if (strcmp(raiz->indices[i].Chave, intervalo1) >= 0 && strcmp(raiz->indices[i].Chave, intervalo2)<= 0){
            regs.push_back(raiz->indices[i]); 
        } else if (raiz->indices[i].Chave > intervalo2)
            return;
    }
    devolveIntervalos(intervalo1, intervalo2, raiz->ponteiros[raiz->qtdRegistros], regs);
}

//Retorna as palavras da árvore que iniciam com uma determinada letra
void devolvePorLetra(char letra, Apontador raiz, vector<Indice> &regs) {
    if (raiz == NULL) {
        return;
    }
    for (int i = 0; i < raiz->qtdRegistros; i++) {
        devolvePorLetra(letra, raiz->ponteiros[i], regs);
        if (raiz->indices[i].Chave[0] == letra) {
            regs.push_back(raiz->indices[i]);
            cout << "achou palavra com a letra" << endl;
        } else if (raiz->indices[i].Chave[0] > letra) {
            cout << "nao achou porra nenhuma" << endl;
            return;
        }
    }
    devolvePorLetra(letra, raiz->ponteiros[raiz->qtdRegistros], regs);
}



void InsereNaPagina(Apontador Ap, Indice indiceRetorno, Apontador ApDir) {
    short NaoAchouPosicao;
    int k, posArquivo;
    k = Ap->qtdRegistros;
    FILE *fd1;
    fd1 = fopen("registros.bin", "rb");
    fseek(fd1, 0, SEEK_END);

    NaoAchouPosicao = (k > 0);
    while (NaoAchouPosicao) {
        if (indiceRetorno.Chave >= Ap->indices[k - 1].Chave) {
            NaoAchouPosicao = FALSE;
            break;
        }
        Ap->indices[k] = Ap->indices[k - 1];
        Ap->ponteiros[k + 1] = Ap->ponteiros[k];
        k--;
        if (k < 1)
            NaoAchouPosicao = FALSE;
    }
    Ap->ponteiros[k + 1] = ApDir;
    Ap->qtdRegistros++;
    posArquivo = ftell(fd1) / sizeof (Registro);
    indiceRetorno.posicaoArquivo = posArquivo;
    Ap->indices[k] = indiceRetorno;
    cout << "inseriu" << endl;
    fclose(fd1);
}

void Ins(Indice indice, Apontador Pagina, short *Cresceu, Indice *indiceRetorno, Apontador *ApRetorno, short *insArq) {
    long i = 1;
    long J;
    Apontador ApTemp;

    if (Pagina == NULL) {
        *Cresceu = TRUE;
        *indiceRetorno = indice;
        *ApRetorno = NULL;
        return;
    }
    while (i < Pagina->qtdRegistros && strcmp(indice.Chave, Pagina->indices[i-1].Chave) > 0) i++;
//indice.Chave == Pagina->indices[i - 1].Chave
    if (strcmp(indice.Chave, Pagina->indices[i-1].Chave) == 0) {
        printf("Erro: Registro ja esta presente\n");
        *Cresceu = 0;
        *insArq = 0;
        return;
    }

    if (strcmp(indice.Chave, Pagina->indices[i-1].Chave) < 0)i--;
    Ins(indice, Pagina->ponteiros[i], Cresceu, indiceRetorno, ApRetorno, insArq);

    if (!*Cresceu)
        return;

    if (Pagina->qtdRegistros < DOBRO_ORDEM) {
        InsereNaPagina(Pagina, *indiceRetorno, *ApRetorno);
        *Cresceu = FALSE;
        return;
    }//Overflow: Pagina tem que ser dividida

    ApTemp = (Apontador) malloc(sizeof (Pagina));
    ApTemp->qtdRegistros = 0;
    ApTemp->ponteiros[0] = NULL;

    if (i < ORDEM + 1){
        InsereNaPagina(ApTemp, Pagina->indices[DOBRO_ORDEM - 1], Pagina->ponteiros[DOBRO_ORDEM]);
        Pagina->qtdRegistros--;
        InsereNaPagina(Pagina, *indiceRetorno, *ApRetorno);
    }else
        InsereNaPagina(ApTemp, *indiceRetorno, *ApRetorno);
    for (J = ORDEM + 2; J <= DOBRO_ORDEM; J++)
        InsereNaPagina(ApTemp, Pagina->indices[J - 1], Pagina->ponteiros[J]);

    Pagina->qtdRegistros = ORDEM;
    ApTemp->ponteiros[0] = Pagina->ponteiros[ORDEM + 1];
    *indiceRetorno = Pagina->indices[ORDEM];
    *ApRetorno = ApTemp;
}

int Insere(Indice Insere, Apontador *Ap, short inserir) {
    short Cresceu;
    short insArq;
    Indice indiceRetorno;
    int posArq;
    FILE *fd1, *fd2;
    Pagina *ApRetorno, *ApTemp;
    //cout << "chegou aqui" << endl;

    Ins(Insere, *Ap, &Cresceu, &indiceRetorno, &ApRetorno, &insArq);
    
    //cout << "chegou aqui2" << endl;
    
    if (Cresceu) {
        ApTemp = new Pagina();
        ApTemp->qtdRegistros = 1;
        ApTemp->indices[0] = indiceRetorno;
        ApTemp->ponteiros[1] = ApRetorno;
        ApTemp->ponteiros[0] = *Ap;
        *Ap = ApTemp;
    }
    if (inserir && insArq) {
        fd1 = fopen("indices.bin", "a+b");
        fd2 = fopen("registros.bin", "r+b");
        fseek(fd2, -sizeof (Registro), SEEK_END);
        posArq = ftell(fd2) / sizeof (Registro);
        Insere.posicaoArquivo = posArq;
        fseek(fd1, 0, SEEK_END);
        fwrite(&Insere, sizeof (Indice), 1, fd1);
        cout << Insere.posicaoArquivo << endl;
        fclose(fd1);
        fclose(fd2);
    }

    return insArq;
}

void insereRegistro(Registro reg, Apontador *Arv) {
    Indice insere;
    FILE *fd1;
    short devoInserir;

    fd1 = fopen("registros.bin", "ab+");
    strcpy(insere.Chave, reg.Chave);
    //insere.Chave = reg.Chave;
    devoInserir = Insere(insere, Arv, 1);

    if (devoInserir != 0) {
        fwrite(&reg, sizeof(Registro), 1, fd1);
    }
    fclose(fd1);
}

void carregaIndices(Apontador *raiz) {
    FILE *fd1;
    int inseridos = 0;
    fd1 = fopen("indices.bin", "r+b");
    Indice ind;
    
    while (!feof(fd1)){    
        fd1 = fopen("indices.bin", "r+b");
        fseek(fd1, inseridos*sizeof(ind), SEEK_SET);
        fread(&ind, sizeof(ind), 1, fd1);
        inseridos++;
        //cout << ind.Chave << endl;
        //cout << ind.posicaoArquivo << endl;
        Insere(ind, raiz, 0);
    }
    
    fclose(fd1);
    cout << "Carregou Indices" << endl;
}

//void limpaArquivos(){
//    FILE *fd1, *fd2;
//    Indice indApaga;
//    Registro regApaga;
//    
//    indApaga.Chave = "VAZIO";
//    indApaga.posicaoArquivo = 9999;
//    
//    regApaga.Chave = "VAZIO";
//    regApaga.Significado = "VAZIO";
//    
//    fd1 = fopen("registros.bin", "wb");
//    fd2 = fopen("indices.bin", "wb");
//    
//    fwrite(&regApaga, sizeof(Registro), 50, fd1);
//    fwrite(&indApaga, sizeof(Indice), 50, fd2);
//    
//    fclose(fd1);
//    fclose(fd2);
//}
void inicializa(Apontador *arv){
    *arv = NULL;   
}

void especifico(Apontador *arv){
    system("cls");
    char palavra[50];
    Registro *regRetorno = new Registro();
    cout << endl;
    cout << "Digite a palavra a ser buscada: ";            
    cin >> palavra;
    PesquisaEspecifica(palavra, *arv, regRetorno);
    cout << endl << "Palavra: " << regRetorno->Chave << endl;
    cout << "Significado: " << regRetorno->Significado << endl;
}

void intervalos(Apontador *arv){
    system("cls");
    char palavra1[50], palavra2[50];
    vector<Indice> buscados;
    cout << "Digite a primeira palavra: ";
    cin >> palavra1;
    cout << endl << "Digite a segunda palavra: ";
    cin >> palavra2;
    devolveIntervalos(palavra1, palavra2, *arv, buscados);
    cout << endl << "Palavras Buscadas: " << endl;
    for(int i=0;i<buscados.size();i++){
        cout << buscados.at(i).Chave << endl;
    }
}

void letra(Apontador *arv){
    system("cls");
    char letra;
    cout << "Digite a letra para buscar as palavras: ";
    cin >> letra;
    vector<Indice> buscados2;
    devolvePorLetra(letra, *arv, buscados2);
    cout << endl << "Palavras buscadas: " << endl;
    for(int i=0;i<buscados2.size();i++){
        cout << buscados2.at(i).Chave << endl;
    }
}

void nivel(Apontador *arv){
    system("cls");
    cout << "Impressao da arvore por nivel: " << endl;
    printLevelOrder(*arv);  
}

void insert (Apontador *arv){
    system("cls");
    Registro regInserido;
    
    cout << "Digite a palavra: ";
    cin >> regInserido.Chave;
    cout << endl << "Digite o significado: ";
    cin >> regInserido.Significado;
    
    insereRegistro(regInserido, arv);   
}

int main() {
    Apontador *arv;
    arv = (Apontador*)malloc(sizeof(Apontador));
    inicializa(arv);
    int escolha;
    carregaIndices(arv);
    
    while(1){
        printf("////////////////////////////////////////////////////////\n");   
        printf("////////////////////Dicionario B-Tree///////////////////\n");
        printf("////////////////////////////////////////////////////////\n");     
        
        
        printf("\n-=-=-=-=-=--=-=-==--=--=Opcoes-=-=-=-=-=-=-=--=-=-=-=\n");
        printf("Digite 1 para a busca de uma palavra especifica no arquivo.\n");
        printf("Digite 2 para fazer uma busca por intervalos.\n");
        printf("Digite 3 para buscar todas as palavras que iniciam com uma letra.\n");
        printf("Digite 4 para imprimir toda a arvore ordenada por nivel!\n");
        printf("Digite 5 para fazer uma insercao!\n");
        printf("Digite qualquer outra tecla para sair!\n");
        
        printf("\n\nOpcao: ");
        cin >> escolha;
        switch(escolha){
            case 1:
                especifico(arv);
                break;
            case 2:
                intervalos(arv);
                break;
            case 3:
                letra(arv);
                break;
            case 4:
                nivel(arv);
                break;
            case 5:
                insert(arv);
                break;
            default:
                exit(1);               
        }        
    }

    return 0;
}


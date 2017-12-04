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

void PesquisaEspecifica(char *chave, Apontador pontPagina, Registro *regRetorno) {
    long i = 1;
    Indice indiceTemp;
    fstream fd1;
    
    if(pontPagina == NULL){
        cout << "registro nao esta presente" << endl;
        return;
    }

    while (i < pontPagina->qtdRegistros && strcmp(chave, pontPagina->indices[i-1].Chave) > 0) i++;

    if (!strcmp(chave, pontPagina->indices[i-1].Chave)){
        fd1.open("registros.bin", fstream::in | fstream::out | fstream::binary);
        indiceTemp = pontPagina->indices[i-1];
        cout << pontPagina->indices[i-1].Chave << endl;
        cout << pontPagina->indices[i-1].posicaoArquivo << endl;
        fd1.seekg(indiceTemp.posicaoArquivo, fd1.beg);
        fd1.read((char*)regRetorno, sizeof(Registro));
        cout << " -=-=-=-=- Chegou aqui -=--=-=-=" << endl;
        fd1.close();
        return;
    }

    if (strcmp(chave, pontPagina->indices[i-1].Chave) < 0)
        PesquisaEspecifica(chave, pontPagina->ponteiros[i - 1], regRetorno);
    else
        PesquisaEspecifica(chave, pontPagina->ponteiros[i], regRetorno);
}

void printLevelOrder(Apontador root) {
  if(root == NULL){
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
    int k;
    k = Ap->qtdRegistros;

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
    Ap->indices[k] = indiceRetorno;
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

    if (strcmp(indice.Chave, Pagina->indices[i-1].Chave) == 0) {
        printf("Erro: Registro ja esta presente\n");
        *Cresceu = FALSE;
        *insArq = FALSE;
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

int Insere(Indice Insere, Apontador *Ap, short inserir, Registro *reg) {
    short Cresceu;
    short insArq;
    Indice indiceRetorno;
    int posArq;
    fstream fd1, fd2;
    Pagina *ApRetorno, *ApTemp;

    if(inserir == 0)
        insArq = 0;
    else{
        fd2.open("registros.bin", fstream::in | fstream::out | fstream::binary | fstream::app);
        fd2.seekg(0, fd2.end);
        posArq = fd2.tellp();
        Insere.posicaoArquivo = posArq;
        insArq = 1;
    }
        
    Ins(Insere, *Ap, &Cresceu, &indiceRetorno, &ApRetorno, &insArq);
    
    if(Cresceu){
        cout << "cresceu arvore" << endl;
        ApTemp = (Apontador)malloc(sizeof(Pagina));
        ApTemp->qtdRegistros = 1;
        ApTemp->indices[0] = indiceRetorno;
        ApTemp->ponteiros[1] = ApRetorno;
        ApTemp->ponteiros[0] = *Ap;
        *Ap = ApTemp;
        cout << "fim do crescimento" << endl;
    }
    
    if(inserir && insArq && reg){
        fd1.open("indices.bin", fstream::in | fstream::out | fstream::binary | fstream::app);
        cout << Insere.posicaoArquivo << endl;
        fd1.write((char*)&Insere, sizeof(Indice));
        fd2.write((char*)reg, sizeof(Registro));
        fd1.close();    
        fd2.close();       
    }

    return insArq;
}

void insereRegistro(Registro reg, Apontador *Arv) {
    Indice insere;
    strcpy(insere.Chave, reg.Chave);
    //insere.Chave = reg.Chave;
    Insere(insere, Arv, 1, &reg);
}

void carregaIndices(Apontador *raiz) {
    FILE *fd1;
    int inseridos = 0;
    fd1 = fopen("indices.bin", "a+b");
    Indice *ind = new Indice();
    
    while (!feof(fd1)){
        ind = new Indice();
        fread(ind, sizeof(Indice), 1, fd1);
        inseridos++;
        Insere(*ind, raiz, 0, NULL);
        delete(ind);
    }
    fclose(fd1);
    cout << "Carregou Indices" << endl;
}

void inicializa(Apontador *arv){
    *arv = NULL;   
}

void especifico(Apontador *arv){
    system("cls");
    char palavra[50];
    Registro *regRetorno = new Registro();
    cout << endl;
    cout << "Digite a palavra a ser buscada: ";
    fflush(stdin);
    fgets(palavra, 49, stdin);
    PesquisaEspecifica(palavra, *arv, regRetorno);
    printf("Palavra: %s", regRetorno->Chave);
    cout << endl << "Palavra: " << regRetorno->Chave << endl;
    cout << "Significado: " << regRetorno->Significado << endl;
}

void intervalos(Apontador *arv){
    system("cls");
    char palavra1[50], palavra2[50];
    vector<Indice> buscados;
    fflush(stdin);
    cout << "Digite a primeira palavra: ";
    fgets(palavra1, 49, stdin);
    cout << endl << "Digite a segunda palavra: ";
    fflush(stdin);
    fgets(palavra2, 49, stdin);
    devolveIntervalos(palavra1, palavra2, *arv, buscados);
    cout << endl << "Palavras Buscadas: " << endl;
    for(int i=0;i<buscados.size();i++){
        cout << buscados.at(i).Chave << endl;
    }
}


void letra(Apontador *arv){
    system("cls");
    char letra;
    fflush(stdin);
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
    fflush(stdin);
    cout << "Digite a palavra: ";
    fgets(regInserido.Chave, 49, stdin);
    cout << endl << "Digite o significado: ";
    fgets(regInserido.Significado, 99, stdin);
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


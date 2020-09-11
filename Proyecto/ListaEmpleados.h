#include <string.h>

using namespace std;

struct Empleado {
  int id_empleado;
  int pass_empleado;
  Empleado *siguiente, *anterior;
  Empleado(int id, int pass) {
    this->id_empleado = id;
    this->pass_empleado = pass;
    this->siguiente = this->anterior = NULL;
  }
};

struct lista {
  Empleado *raiz, *ultimo;
  int tamanio;//numero de asiento
  int id_correlativo;
  //constructor de struct
  lista() {
    this->raiz = this->ultimo = NULL;
    this->tamanio = 0;
    this->id_correlativo = 1;
  }

  void agregar(int id, int pass) {
    Empleado * nuevo = new Empleado(id, pass);    
    if (this->raiz == NULL) {
      this->raiz = this->ultimo = nuevo;
      tamanio++;
      return;
    } else {
      this->ultimo->siguiente = nuevo;
      nuevo->anterior = this->ultimo;
      ultimo = nuevo;
      tamanio++;
      return;
    }
  }

  Empleado *buscar_empleado(int ide, int passe) {    
    if (this->raiz == NULL) {
      return NULL;
    } else {
      Empleado *aux = raiz;
      while (aux != NULL) {
        if (aux->id_empleado == ide && aux->pass_empleado == passe) {
          return aux;
        }
        aux = aux->siguiente;
      }
      return NULL;
    }
  }


  void print() {
    Empleado *aux = this->raiz;
    while (aux != NULL) {
      printf("ID Empleado: %d \n", aux->id_empleado);
      aux = aux->siguiente;
    }
  }

};
/**
  int main()
  {
    int passw[8] = { -7, -7, -7, -7, -7, -7, -7, -7};
    int emm[8] = { -6, -6, -6, -6, -6, -6, -6, -6};
    lista *lst = new lista();
    lst->agregar(666, emm, passw);
    //lst->add(5);
    //lst->add(8);
    //lst->add(2);
    printf("Se debe imprimir de primero-ultimo\n");
    lst->print();
    Empleado *encontrado = lst->buscar_empleado(1);
    for (int i = 0; i < 8; i++) {
        cout<<"Id"<<endl;;
        cout<<encontrado->id_employ[i]<<endl;;
        cout<<"Pass"<<endl;;
        cout<<encontrado->pass_empleado[i]<<endl;;
    }
    return 0;
  }**/

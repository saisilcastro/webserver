#include <iostream>
using namespace std;

class A
{
protected:
    int a;

public:
    A() : a(0) {} // Construtor para inicializar a

    int getA() const
    {
        return a;
    }

    void setA(int value)
    {
        a = value;
    }
};

class B
{
private:
    A* aRef; // Ponteiro para a classe A

public:
    B(A* a) : aRef(a) {} // Construtor que recebe um ponteiro para A

    int getA() const
    {
        return aRef->getA(); // Acessa o valor de A
    }

    void setA(int value)
    {
        aRef->setA(value); // Altera o valor de A
    }
};

int main(void)
{
    A a; // Instância de A
    B b(&a); // Instância de B, passando a referência de A

    a.setA(5);
    cout << "Valor de A: " << a.getA() << endl; // Acesso direto
    cout << "Valor de B: " << b.getA() << endl; // Acesso via B

    b.setA(10); // Altera o valor de A através de B
    cout << "Valor de A: " << a.getA() << endl; // Acesso direto
    cout << "Valor de B: " << b.getA() << endl; // Acesso via B

    return 0;
}

// Однострочный комментарий

#include <iostream>

/*
    Это пример
    многострочного комментария.
    Он нужен для проверки
    работы препроцессора.
*/

using namespace std;

int sum(int a, int b)
{
    return a + b;
}

int main()
{
    int x = 5;      // первое число
    int y = 10;     // второе число

    int result = sum(x, y);

    if (result > 10)
    {
        cout << "Result > 10" << endl;
    }
    else
    {
        cout << "Result <= 10" << endl;
    }

    for (int i = 0; i < 5; i++)
    {
        cout << i << endl;
    }

    return 0;
}
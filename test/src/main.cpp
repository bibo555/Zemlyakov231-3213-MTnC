#include <iostream>
#include <fstream>
#include <string>
#include "../include/preprocessor.h"
#include <windows.h>


using namespace std;

int main()
{
    ifstream file("test.cpp");

    if (!file.is_open())
    {
        cerr << "Ошибка открытия файла!" << endl;
        return 1;
    }

    cout << "Файл успешно открыт." << endl;
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
   
    string code((istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>());

    string processed = preprocessCode(code);

    cout << "\nОбработанный код:\n";
    cout << processed << endl;

    

    return 0;
}
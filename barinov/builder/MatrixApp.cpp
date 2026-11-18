#pragma hdrstop

#pragma comment(lib, "rtl.bpi")   // подключает Delphi RTL (DynamicArray, String, SysUtils, IOUtils)

#include <System.hpp>

#include <System.SysUtils.hpp>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <windows.h>

#include "MatrixModule.h"

#pragma argsused
using namespace std;

// Вывод String в консоль (кодовая страница консоли - 1251)
void vyvod(const String& s)
{
    cout << AnsiStringT<1251>(s).c_str();
}

void zavershit_pri_konce_vvoda()
{
    cout << "\nВвод завершён. Выход из программы.\n";
    exit(0);
}

int proverka_v(const string& zapros, int min_v, int max_v)
{
    int znach = 0;
    while (true) {
        cout << zapros;
        cin >> znach;
        if (cin.eof())
            zavershit_pri_konce_vvoda();
        bool chisto = !cin.fail() && cin.peek() == '\n';
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (!chisto)
            cout << "Ошибка ввода! Ожидается корректное целое число.\n";
        else if (znach < min_v || znach > max_v)
            cout << "Ошибка! Значение должно быть в диапазоне от " << min_v
                 << " до " << max_v << ".\n";
        else
            return znach;
    }
}

String vvod_im()
{
    string put;
    cout << "Введите имя текстового файла с матрицей: ";
    if (!getline(cin, put))
        zavershit_pri_konce_vvoda();
    // консольный ввод в cp1251 -> UnicodeString
    return String(AnsiStringT<1251>(put.c_str()));
}

void soobshchit_ob_oshibke(const TErrorInfo& err)
{
    cout << "\n[ОШИБКА] ";
    switch (err.code) {
    case errFileNotFound:
        cout << "Не удалось открыть файл. Проверьте имя или путь.\n";
        break;
    case errEmptyFile:
        cout << "Файл пуст или не содержит числовых данных.\n";
        break;
    case errNotRectangular:
        cout << "Матрица не прямоугольная: в строке " << err.notRectangular.line
             << " найдено элементов: " << err.notRectangular.found
             << ", а ожидалось: " << err.notRectangular.expected << ".\n";
        break;
    case errInvalidElement:
        cout << "Некорректный элемент (не число): строка " << err.invalidElement.line
             << ", позиция " << err.invalidElement.column << ".\n";
        break;
    default:
        cout << "Неизвестная ошибка.\n";
    }
}

int main()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    bool povtor = true;
    while (povtor) {
        TMatrix ishodnaya;

        while (true) {
            String imya = vvod_im();
            TErrorInfo err;
            if (LoadMatrixFromFile(imya, ishodnaya, &err))
                break;
            soobshchit_ob_oshibke(err);
            cout << "Исправьте файл или укажите другой.\n\n";
        }

        cout << "\nУспешно загружена матрица размером " << ishodnaya.Length
             << "x" << ishodnaya[0].Length << ".\n";

        cout << "\nИсходная матрица:\n";
        vyvod(MatrixToString(ishodnaya));

        double minV = 0, maxV = 0;
        FindMinMax(ishodnaya, minV, maxV);
        double P = ComputeP(minV, maxV);

        cout << "\nМинимальный элемент: " << minV
             << "\nМаксимальный элемент: " << maxV
             << "\nP = " << P << "\n";

        TMatrix rezultat = AddToRowsExcept(ishodnaya, P, 3);

        cout << "\nНовая матрица (ко всем строкам, кроме 4-й, прибавлено P):\n";
        vyvod(MatrixToString(rezultat));

        cout << "\n";
    }
    return 0;
}

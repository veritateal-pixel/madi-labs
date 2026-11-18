//---------------------------------------------------------------------------
// Модуль для загрузки, обработки и преобразования матрицы.
// Не содержит ввода/вывода в консоль: годится и для консольного, и для
// GUI-приложения без изменений.
//---------------------------------------------------------------------------
#pragma once
#ifndef MATRIX_MODULE_H_INCLUDED
#define MATRIX_MODULE_H_INCLUDED

#include <System.hpp>

// Двумерный динамический массив. ВНИМАНИЕ: присваивание (=) копирует только
// ссылку; полная копия делается построчно (см. AddToRowsExcept).
typedef DynamicArray< DynamicArray<double> > TMatrix;

// Результат загрузки матрицы из файла.
enum TErrorCode {
    errSuccess = 0,       // ошибок нет
    errFileNotFound,      // файл не удалось открыть
    errEmptyFile,         // в файле нет ни одного числа
    errNotRectangular,    // строки содержат разное число элементов
    errInvalidElement     // элемент не является числом
};

// Дополнительная информация об ошибке (компактно, через union).
// Какое поле объединения верно - определяет code.
struct TErrorInfo {
    TErrorCode code;
    union {
        struct {
            int line;       // номер строки файла (с 1), считая пустые строки
            int column;     // номер элемента в строке (с 1)
        } invalidElement;   // при code == errInvalidElement
        struct {
            int line;       // номер строки файла (с 1), считая пустые строки
            int expected;   // сколько элементов должно быть (по первой строке)
            int found;      // сколько элементов оказалось в этой строке
        } notRectangular;   // при code == errNotRectangular
    };
};

// Загрузка матрицы из текстового файла.
// Возвращает true при успехе. errInfo - необязательный (можно nullptr).
// При ошибке matrix пуста (Length = 0), а в errInfo - причина.
bool LoadMatrixFromFile(const String& fileName, TMatrix& matrix,
                        TErrorInfo* errInfo = nullptr);

// Поиск минимального и максимального элементов. false, если матрица пуста.
bool FindMinMax(const TMatrix& matrix, double& minVal, double& maxVal);

// P = (Amin + Amax)^2
double ComputeP(double minVal, double maxVal);

// Обработка по варианту: возвращает НОВУЮ матрицу, в которой ко всем строкам,
// кроме строки skipRow (нумерация с 0), прибавлено p. Исходная не меняется.
TMatrix AddToRowsExcept(const TMatrix& source, double p, int skipRow);

// Форматированное текстовое представление матрицы: каждый элемент занимает
// width позиций (по правому краю), precision знаков после точки.
String MatrixToString(const TMatrix& matrix, int width = 12, int precision = 4);

#endif

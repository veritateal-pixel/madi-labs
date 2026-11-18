#pragma once
#ifndef MATRIX_MODULE_H_INCLUDED
#define MATRIX_MODULE_H_INCLUDED

#include <System.hpp>

// Двумерный динамический массив. ВНИМАНИЕ: присваивание (=) копирует только
// ссылку; полную копию нужно делать построчно (см. AddToRowsExcept).
typedef DynamicArray< DynamicArray<double> > TMatrix;

// Результат загрузки матрицы из файла.
enum TErrorCode {
    errSuccess = 0,       // ошибок нет
    errFileNotFound,      // файл не удалось открыть
    errEmptyFile,         // в файле нет ни одного числа
    errNotRectangular,    // строки содержат разное число элементов
    errInvalidElement     // элемент не является числом
};

struct TErrorInfo {
    TErrorCode code;
    union {
        struct {
            int line;       // номер строки файла (с 1), считая пустые строки
            int column;     // номер элемента в строке (с 1)
        } invalidElement;
        struct {
            int line;       // номер строки файла (с 1), считая пустые строки
            int expected;   // сколько элементов должно быть (по первой строке)
            int found;      // сколько элементов оказалось в этой строке
        } notRectangular;
    };
};

// Загрузка матрицы из текстового файла. При ошибке матрица пуста (Length = 0).
bool LoadMatrixFromFile(const String& fileName, TMatrix& matrix,
                        TErrorInfo* errInfo = nullptr);

// Поиск минимального и максимального элементов.
bool FindMinMax(const TMatrix& matrix, double& minVal, double& maxVal);

// P = (Amin + Amax)^2
double ComputeP(double minVal, double maxVal);

// Возвращает НОВУЮ матрицу: ко всем строкам, кроме skipRow (с 0), прибавлено p.
TMatrix AddToRowsExcept(const TMatrix& source, double p, int skipRow);

// Форматированное текстовое представление матрицы.
String MatrixToString(const TMatrix& matrix, int width = 12, int precision = 4);

#endif

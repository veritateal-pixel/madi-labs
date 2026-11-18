#pragma once
#ifndef MATRIX_MODULE_H_INCLUDED
#define MATRIX_MODULE_H_INCLUDED

#include <string>
#include <vector>

typedef std::vector< std::vector<double> > TMatrix;

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

bool LoadMatrixFromFile(const std::string& fileName, TMatrix& matrix,
                        TErrorInfo* errInfo = nullptr);

bool FindMinMax(const TMatrix& matrix, double& minVal, double& maxVal);

double ComputeP(double minVal, double maxVal);

TMatrix AddToRowsExcept(const TMatrix& source, double p, size_t skipRow);

std::string MatrixToString(const TMatrix& matrix, int width = 12, int precision = 4);

#endif

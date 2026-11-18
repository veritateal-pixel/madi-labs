//---------------------------------------------------------------------------
#pragma hdrstop

#include "MatrixModule.h"

#include <System.SysUtils.hpp>
#include <System.IOUtils.hpp>

#pragma package(smart_init)
//---------------------------------------------------------------------------

// ---- Внутренние (невидимые снаружи) вспомогательные функции ----

// Десятичный разделитель - всегда точка, независимо от региональных настроек.
static TFormatSettings MakeFormatSettings()
{
    TFormatSettings fs = TFormatSettings::Create();
    fs.DecimalSeparator = L'.';
    return fs;
}

static bool IsSpace(wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n' ||
           c == L'\v' || c == L'\f';
}

// Разбор массива строк в матрицу. Вся информация об ошибке - в info.
static void ReadMatrix(const TStringDynArray& lines, TMatrix& matrix, TErrorInfo& info)
{
    const TFormatSettings fs = MakeFormatSettings();
    int width = 0;

    for (int lineIdx = 0; lineIdx < lines.Length; ++lineIdx) {
        const int lineNo = lineIdx + 1;
        const String& line = lines[lineIdx];
        const int n = line.Length();

        DynamicArray<double> row;
        int i = 1;                          // символы String нумеруются с 1

        while (true) {
            while (i <= n && IsSpace(line[i]))
                ++i;
            if (i > n)
                break;

            int start = i;
            while (i <= n && !IsSpace(line[i]))
                ++i;
            String token = line.SubString(start, i - start);

            double value = 0;
            if (!TryStrToFloat(token, value, fs)) {
                info.code = errInvalidElement;
                info.invalidElement.line = lineNo;
                info.invalidElement.column = row.Length + 1;
                return;
            }
            row.Length = row.Length + 1;
            row[row.Length - 1] = value;
        }

        if (row.Length == 0)
            continue;                       // пустая строка пропускается

        if (matrix.Length == 0) {
            width = row.Length;
        }
        else if (row.Length != width) {
            info.code = errNotRectangular;
            info.notRectangular.line = lineNo;
            info.notRectangular.expected = width;
            info.notRectangular.found = row.Length;
            return;
        }

        matrix.Length = matrix.Length + 1;
        matrix[matrix.Length - 1] = row;
    }

    if (matrix.Length == 0)
        info.code = errEmptyFile;
}

// ---- Экспортируемые функции ----

bool LoadMatrixFromFile(const String& fileName, TMatrix& matrix, TErrorInfo* errInfo)
{
    TErrorInfo info = TErrorInfo();      // обнуляет и code (= errSuccess), и объединение
    matrix.Length = 0;

    TStringDynArray lines;
    bool opened = false;
    try {
        if (TFile::Exists(fileName)) {
            lines = TFile::ReadAllLines(fileName);
            opened = true;
        }
    }
    catch (const Exception&) {
        opened = false;
    }

    if (!opened)
        info.code = errFileNotFound;
    else
        ReadMatrix(lines, matrix, info);

    if (info.code != errSuccess)
        matrix.Length = 0;               // при ошибке матрица всегда пуста

    if (errInfo != nullptr)
        *errInfo = info;                 // выходные данные согласованы для каждого исхода

    return info.code == errSuccess;      // динамические массивы освобождаются сами
}

bool FindMinMax(const TMatrix& matrix, double& minVal, double& maxVal)
{
    if (matrix.Length == 0 || matrix[0].Length == 0)
        return false;

    minVal = maxVal = matrix[0][0];
    for (int i = 0; i < matrix.Length; i++) {
        for (int j = 0; j < matrix[i].Length; j++) {
            if (matrix[i][j] < minVal) minVal = matrix[i][j];
            if (matrix[i][j] > maxVal) maxVal = matrix[i][j];
        }
    }
    return true;
}

double ComputeP(double minVal, double maxVal)
{
    double s = minVal + maxVal;
    return s * s;   // P = (Amin + Amax)^2
}

TMatrix AddToRowsExcept(const TMatrix& source, double p, int skipRow)
{
    // Полная (глубокая) копия: каждая строка выделяется заново,
    // поэтому исходная матрица не затрагивается.
    TMatrix result;
    result.Length = source.Length;

    for (int i = 0; i < source.Length; i++) {
        result[i].Length = source[i].Length;
        for (int j = 0; j < source[i].Length; j++)
            result[i][j] = (i == skipRow) ? source[i][j] : source[i][j] + p;
    }
    return result;
}

String MatrixToString(const TMatrix& matrix, int width, int precision)
{
    const TFormatSettings fs = MakeFormatSettings();
    String text;

    for (int i = 0; i < matrix.Length; i++) {
        for (int j = 0; j < matrix[i].Length; j++) {
            String num = FloatToStrF(static_cast<Extended>(matrix[i][j]),
                                     TFloatFormat::ffFixed, 18, precision, fs);
            if (num.Length() < width)               // выравнивание по правому краю
                num = StringOfChar(L' ', width - num.Length()) + num;
            text += num;
        }
        text += sLineBreak;                         // конец строки матрицы
    }
    return text;
}

#include "MatrixModule.h"

#include <fstream>
#include <iomanip>
#include <sstream>


static bool ParseDouble(const std::string& token, double& value)
{
    std::istringstream ss(token);
    ss >> value;
    return !ss.fail() && ss.eof();
}


static void ReadMatrix(std::istream& in, TMatrix& matrix, TErrorInfo& info)
{
    std::string line;
    int lineNo = 0;
    size_t width = 0;

    while (std::getline(in, line)) {
        ++lineNo;

        std::istringstream ls(line);
        std::vector<double> row;
        std::string token;

        while (ls >> token) {
            double value = 0;
            if (!ParseDouble(token, value)) {
                info.code = errInvalidElement;
                info.invalidElement.line = lineNo;
                info.invalidElement.column = static_cast<int>(row.size()) + 1;
                return;
            }
            row.push_back(value);
        }

        if (row.empty())
            continue;

        if (matrix.empty()) {
            width = row.size();
        }
        else if (row.size() != width) {
            info.code = errNotRectangular;
            info.notRectangular.line = lineNo;
            info.notRectangular.expected = static_cast<int>(width);
            info.notRectangular.found = static_cast<int>(row.size());
            return;
        }
        matrix.push_back(row);
    }

    if (matrix.empty())
        info.code = errEmptyFile;
}


// Экспортируемые функции

bool LoadMatrixFromFile(const std::string& fileName, TMatrix& matrix, TErrorInfo* errInfo)
{
    TErrorInfo info = TErrorInfo();      // обнуляет и code (= errSuccess), и всё объединение
    matrix.clear();

    std::ifstream f(fileName.c_str());   // закроется сам при выходе из функции
    if (!f.is_open())
        info.code = errFileNotFound;
    else
        ReadMatrix(f, matrix, info);

    if (info.code != errSuccess)
        matrix.clear();                  // при ошибке матрица всегда пуста

    if (errInfo != nullptr)
        *errInfo = info;                 // выходные данные согласованы для каждого исхода

    return info.code == errSuccess;
}

bool FindMinMax(const TMatrix& matrix, double& minVal, double& maxVal)
{
    if (matrix.empty() || matrix[0].empty())
        return false;

    minVal = maxVal = matrix[0][0];
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix[i].size(); j++) {
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

TMatrix AddToRowsExcept(const TMatrix& source, double p, size_t skipRow)
{
    TMatrix result = source;
    for (size_t i = 0; i < result.size(); i++) {
        if (i == skipRow)
            continue;
        for (size_t j = 0; j < result[i].size(); j++)
            result[i][j] += p;
    }
    return result;
}

std::string MatrixToString(const TMatrix& matrix, int width, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision);
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix[i].size(); j++)
            oss << std::setw(width) << matrix[i][j];
        oss << "\n";////////////////////////// 
    }
    return oss.str();
}

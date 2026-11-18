#include <cerrno>      // errno - системный код ошибки при работе с файлом
#include <cmath>       // std::pow
#include <cstring>     // std::strerror
#include <fstream>     // std::ifstream, std::ofstream
#include <iomanip>     // std::setw, std::setprecision
#include <iostream>    // std::cout, std::cerr
#include <limits>      // std::numeric_limits
#include <new>         // std::nothrow
#include <string>      // std::string

#include <windows.h>


// Матрица хранится как динамический двумерный массив.
// Тип не содержит логики ввода/вывода и поэтому может использоваться повторно.
struct Matrix {
    std::size_t rows = 0;
    std::size_t cols = 0;
    double** data = nullptr;
};

// Коды завершения функций: main сможет отдельно обработать каждый вид ошибки.
enum class Status {
    Ok,
    FileOpenError,
    MemoryAllocationError,
    InvalidElement,
    ExtraData,
    EmptyMatrix,
    OutputFileError
};

// Объединение хранит дополнительную информацию именно о той ошибке,
// которая произошла. Его передача в функции необязательна.
union ErrorDetails {
    struct {
        int systemCode;               // значение errno при ошибке открытия файла
    } fileOpen;

    struct {
        std::size_t row;              // индекс строки ошибочного элемента (с нуля)
        std::size_t col;              // индекс столбца ошибочного элемента (с нуля)
        std::streamoff position;      // позиция элемента в файле, если известна
    } invalidElement;

    struct {
        int step;                     // -1: массив строк; >= 0: конкретная строка
    } allocation;
};

// Освобождает всю память, занятую матрицей, и приводит объект
// к безопасному пустому состоянию.
void destroyMatrix(Matrix& matrix) noexcept {
    if (matrix.data != nullptr) {
        for (std::size_t i = 0; i < matrix.rows; ++i) {
            delete[] matrix.data[i];
        }
        delete[] matrix.data;
    }

    matrix.rows = 0;
    matrix.cols = 0;
    matrix.data = nullptr;
}

// Выделяет память под матрицу rows x cols.
// При ошибке result не получает частично созданную матрицу.
Status allocateMatrix(std::size_t rows,
                      std::size_t cols,
                      Matrix& result,
                      ErrorDetails* details = nullptr) {
    if (rows == 0 || cols == 0) {
        return Status::EmptyMatrix;
    }

    Matrix temporary;
    temporary.rows = rows;
    temporary.cols = cols;

    // Сначала создается массив указателей на строки.
    temporary.data = new (std::nothrow) double*[rows];
    if (temporary.data == nullptr) {
        if (details != nullptr) {
            details->allocation.step = -1;
        }
        return Status::MemoryAllocationError;
    }

    // Обнуляем указатели, чтобы destroyMatrix был безопасен при частичном создании.
    for (std::size_t i = 0; i < rows; ++i) {
        temporary.data[i] = nullptr;
    }

    // Затем выделяется память для каждой строки.
    for (std::size_t i = 0; i < rows; ++i) {
        temporary.data[i] = new (std::nothrow) double[cols];
        if (temporary.data[i] == nullptr) {
            if (details != nullptr) {
                details->allocation.step = static_cast<int>(i);
            }
            destroyMatrix(temporary);
            return Status::MemoryAllocationError;
        }
    }

    // Выходной параметр изменяется только после полного успеха операции.
    result = temporary;
    return Status::Ok;
}

// Загружает прямоугольную матрицу заданного размера из текстового файла.
// В файле должны располагаться ровно rows * cols чисел, разделенных пробелами.
Status loadMatrixFromFile(const std::string& filename,
                          std::size_t rows,
                          std::size_t cols,
                          Matrix& result,
                          ErrorDetails* details = nullptr) {
    errno = 0;
    std::ifstream input(filename);
    if (!input.is_open()) {
        if (details != nullptr) {
            details->fileOpen.systemCode = errno;
        }
        return Status::FileOpenError;
    }

    Matrix temporary;
    Status status = allocateMatrix(rows, cols, temporary, details);
    if (status != Status::Ok) {
        return status;
    }

    // Читаем элементы. При ошибке временная матрица будет уничтожена,
    // а result останется неизменным.
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            input >> std::ws;
            const std::streampos position = input.tellg();

            if (!(input >> temporary.data[i][j])) {
                if (details != nullptr) {
                    details->invalidElement.row = i;
                    details->invalidElement.col = j;
                    details->invalidElement.position =
                        (position == std::streampos(-1))
                            ? static_cast<std::streamoff>(-1)
                            : static_cast<std::streamoff>(position);
                }
                destroyMatrix(temporary);
                return Status::InvalidElement;
            }
        }
    }

    // После ожидаемых 15 чисел в файле допустимы только пробелы и переводы строк.
    input >> std::ws;
    if (!input.eof()) {
        destroyMatrix(temporary);
        return Status::ExtraData;
    }

    result = temporary;
    return Status::Ok;
}

// Находит минимальный и максимальный элементы произвольной непустой матрицы.
Status findMinMax(const Matrix& matrix, double& minValue, double& maxValue) {
    if (matrix.data == nullptr || matrix.rows == 0 || matrix.cols == 0) {
        return Status::EmptyMatrix;
    }

    double localMin = matrix.data[0][0];
    double localMax = matrix.data[0][0];

    for (std::size_t i = 0; i < matrix.rows; ++i) {
        for (std::size_t j = 0; j < matrix.cols; ++j) {
            if (matrix.data[i][j] < localMin) {
                localMin = matrix.data[i][j];
            }
            if (matrix.data[i][j] > localMax) {
                localMax = matrix.data[i][j];
            }
        }
    }

    minValue = localMin;
    maxValue = localMax;
    return Status::Ok;
}

// Вычисляет значение P по формуле варианта: P = (Amin + Amax)^2.
double calculateP(double minValue, double maxValue) {
    return std::pow(minValue + maxValue, 2.0);
}

// Формирует новую матрицу: к элементам всех строк, кроме unchangedRow,
// прибавляет P. Номер unchangedRow передается как индекс с нуля.
Status createProcessedMatrix(const Matrix& source,
                             double p,
                             std::size_t unchangedRow,
                             Matrix& result,
                             ErrorDetails* details = nullptr) {
    if (source.data == nullptr || source.rows == 0 || source.cols == 0) {
        return Status::EmptyMatrix;
    }

    Matrix temporary;
    Status status = allocateMatrix(source.rows, source.cols, temporary, details);
    if (status != Status::Ok) {
        return status;
    }

    for (std::size_t i = 0; i < source.rows; ++i) {
        for (std::size_t j = 0; j < source.cols; ++j) {
            // В задании 4-я строка не изменяется; ее индекс в C++ равен 3.
            temporary.data[i][j] =
                (i == unchangedRow) ? source.data[i][j] : source.data[i][j] + p;
        }
    }

    result = temporary;
    return Status::Ok;
}

// Выводит матрицу в произвольный поток: это может быть cout или файл.
// Значения width и precision имеют значения по умолчанию.
void printMatrix(std::ostream& output,
                 const Matrix& matrix,
                 int width = 12,
                 int precision = 2) {
    // Запоминаем прежние настройки потока, чтобы функция не изменяла их навсегда.
    const std::ios::fmtflags oldFlags = output.flags();
    const std::streamsize oldPrecision = output.precision();

    output << std::fixed << std::setprecision(precision);
    for (std::size_t i = 0; i < matrix.rows; ++i) {
        for (std::size_t j = 0; j < matrix.cols; ++j) {
            output << std::setw(width) << matrix.data[i][j];
        }
        output << '\n';
    }

    output.flags(oldFlags);
    output.precision(oldPrecision);
}

// Обрабатывает и выводит ошибки загрузки матрицы.
void printLoadError(Status status,
                    const std::string& filename,
                    const ErrorDetails& details) {
    switch (status) {
        case Status::FileOpenError:
            std::cerr << "Не удалось открыть файл \"" << filename << "\"";
            if (details.fileOpen.systemCode != 0) {
                std::cerr << ": " << std::strerror(details.fileOpen.systemCode);
            }
            std::cerr << "\n";
            break;

        case Status::MemoryAllocationError:
            std::cerr << "Ошибка выделения памяти, шаг: "
                      << details.allocation.step << "\n";
            break;

        case Status::InvalidElement:
            std::cerr << "В файле отсутствует число или содержится некорректный "
                         "элемент в строке "
                      << (details.invalidElement.row + 1)
                      << ", столбце " << (details.invalidElement.col + 1);
            if (details.invalidElement.position >= 0) {
                std::cerr << ", позиция в файле: "
                          << details.invalidElement.position;
            }
            std::cerr << "\n";
            break;

        case Status::ExtraData:
            std::cerr << "В файле есть лишние данные после матрицы 5 x 3.\n";
            break;

        default:
            std::cerr << "Неизвестная ошибка загрузки матрицы.\n";
            break;
    }
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    // Размер из условия задания: матрица A[5 x 3].
    constexpr std::size_t rows = 5;
    constexpr std::size_t cols = 3;

    // Четвертая строка в человеческой нумерации имеет индекс 3 в C++.
    constexpr std::size_t unchangedRow = 3;

    if (argc < 2 || argc > 3) {
        std::cerr << "Использование: " << argv[0]
                  << " <входной_файл> [выходной_файл]\n";
        std::cerr << "Пример: " << argv[0] << " matrix.txt result.txt\n";
        return 1;
    }

    const std::string inputFilename = argv[1];
    Matrix source;
    Matrix processed;
    ErrorDetails details{};

    // 1. Загружаем исходную матрицу из файла.
    Status status = loadMatrixFromFile(inputFilename, rows, cols, source, &details);
    if (status != Status::Ok) {
        printLoadError(status, inputFilename, details);
        return 1;
    }

    // 2. Находим минимальный и максимальный элементы матрицы.
    double minValue = 0.0;
    double maxValue = 0.0;
    status = findMinMax(source, minValue, maxValue);
    if (status != Status::Ok) {
        std::cerr << "Матрица пуста; обработка невозможна.\n";
        destroyMatrix(source);
        return 1;
    }

    // 3. Вычисляем P по формуле из задания.
    const double p = calculateP(minValue, maxValue);

    // 4. Создаем новую матрицу с измененными строками.
    status = createProcessedMatrix(source, p, unchangedRow, processed, &details);
    if (status != Status::Ok) {
        std::cerr << "Не удалось сформировать новую матрицу.\n";
        destroyMatrix(source);
        return 1;
    }

    // 5. Если задан второй аргумент командной строки, пишем в файл,
    // иначе выводим результат в окно терминала.
    std::ofstream outputFile;
    std::ostream* output = &std::cout;

    if (argc == 3) {
        outputFile.open(argv[2]);
        if (!outputFile.is_open()) {
            std::cerr << "Не удалось открыть выходной файл \"" << argv[2]
                      << "\".\n";
            destroyMatrix(processed);
            destroyMatrix(source);
            return 1;
        }
        output = &outputFile;
    }

    *output << "Исходная матрица A:\n";
    printMatrix(*output, source);

    *output << "\nМинимальный элемент Amin = " << minValue << '\n';
    *output << "Максимальный элемент Amax = " << maxValue << '\n';
    *output << "P = (Amin + Amax)^2 = " << p << "\n\n";

    *output << "Новая матрица (4-я строка без изменений):\n";
    printMatrix(*output, processed);

    // 6. Освобождаем динамически выделенную память.
    destroyMatrix(processed);
    destroyMatrix(source);

    return 0;
}

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>
#include <limits.h>

// Константы программы
const int K = 10;                      // Коэффициент повторений для усреднения времени
const int N_MIN = 256;                 // Минимальный размер массива (1KB при int=4байта)
const int N_MAX = 8 * 1024 * 1024;     // Максимальный размер массива (32MB при int=4байта)

/**
 * Создает "прямой" список - каждый элемент указывает на следующий
 * arr[0] → 1, arr[1] → 2, ..., arr[N-1] → 0
 */
void forward(int *arr, int N) {
    for (int i = 0; i < N - 1; i++) {
        arr[i] = i + 1;  // Текущий элемент указывает на следующий
    }
    arr[N - 1] = 0;      // Последний элемент замыкает на первый
}

/**
 * Создает "обратный" список - каждый элемент указывает на предыдущий
 * arr[0] → N-1, arr[1] → 0, ..., arr[N-1] → N-2
 */
void reverse(int *arr, int N) {
    for (int i = N - 1; i > 0; i--) {
        arr[i] = i - 1;  // Текущий элемент указывает на предыдущий
    }
    arr[0] = N - 1;      // Первый элемент замыкает на последний
}

// Вспомогательная функция для обмена значений двух переменных
void swap(int *a, int *b) {
    int c = *a;
    *a = *b;
    *b = c;
}

/**
 * Создает случайную перестановку указателей
 * Эмулирует случайный доступ к памяти
 */
void random_(int *arr, int N) {
    // Инициализация массива последовательными значениями
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }

    // Алгоритм Фишера-Йейтса для случайной перестановки
    for (int i = N - 1; i > 0; i--) {
        swap(&arr[i], &arr[rand() % (i + 1)]);
    }

    // Преобразование перестановки в связный список
    int current = arr[0];  // Начальный элемент
    
    for (int i = 1; i < N; i++) {
        int next = arr[i];      // Следующий элемент в перестановке
        arr[current] = next;    // Текущий элемент указывает на следующий
        current = next;         // Переходим к следующему элементу
    }
    
    // Замыкание последнего элемента на первый
    arr[current] = arr[0];
}

const int COUNT = 100;  // Количество итераций измерений для усреднения

/**
 * Измеряет такты процессора для прохода по списку
 * Возвращает минимальное среднее время доступа к одному элементу
 */
long double tacts(int *arr, int N, FILE *log) {
    long double min_avg = ULLONG_MAX;  // Инициализация максимальным значением

    // Многократные измерения для статистической значимости
    for (int it = 0; it < COUNT; it++) {
        // "Прогрев" кэша - предварительный проход по данным
        volatile int x = 0;
        for (int i = 0; i < N; i++) {
            x = arr[x];  // Проход по всему списку
        }

        // Измерение времени с помощью счетчика тактов процессора
        uint64_t start, end;
        start = __rdtsc();  // Read Time-Stamp Counter - начало измерения
        
        // Основной измеряемый цикл - K проходов по всему списку
        for (int i = 0; i < N * K; i++) {
            x = arr[x];  // Последовательный доступ по указателям
        }

        end = __rdtsc();  // Конец измерения

        // Запись сырых данных в лог-файл
        fprintf(log, "%d;%d;%d;%lu;%lu;%lu\n", N, K, it, start, end, end - start);

        // Расчет среднего времени доступа к одному элементу
        long double avg = (long double)(end - start) / (N * K);

        // Сохранение минимального значения (исключаем выбросы)
        if (min_avg > avg)
            min_avg = avg;
    }

    return min_avg;
}

int main() {
    srand(time(NULL));  // Инициализация генератора случайных чисел

    // Открытие файлов для результатов и детального лога
    FILE *out = fopen("resultO1.csv", "w");
    FILE *log = fopen("log.csv", "w");

    // Заголовки CSV файлов
    fprintf(out, "Size;Forward;Reverse;Random\n");
    fprintf(log, "N;K;count_it;start;end;result\n");
    
    // Основной цикл по разным размерам массивов
    for (int N = N_MIN; N <= N_MAX; N *= 2) {
        // Выделение памяти для массива указателей
        int *arr = (int *)malloc(N * sizeof(int));

        if (arr == NULL) {
            fprintf(stderr, "Memory allocation failed for N: %d\n", N);
            exit(EXIT_FAILURE);
        }

        // Измерение для прямого списка
        forward(arr, N);
        long double time_forward = tacts(arr, N, log);
        
        // Измерение для обратного списка
        reverse(arr, N);
        long double time_reverse = tacts(arr, N, log);

        // Измерение для случайного списка
        random_(arr, N);
        long double time_random = tacts(arr, N, log);

        // Запись результатов: размер в байтах и времена доступа
        fprintf(out, "%d;%.2Lf;%.2Lf;%.2Lf\n", 
                N * (int)sizeof(int),  // Размер в байтах
                time_forward,          // Время для прямого обхода
                time_reverse,          // Время для обратного обхода  
                time_random);          // Время для случайного обхода

        free(arr);  // Освобождение памяти
    }
    
    // Закрытие файлов
    fclose(out);
    fclose(log);

    return 0;
}

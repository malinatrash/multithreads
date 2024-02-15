// N=10
// Первый поток - генерирует в первый буфер 100 чисел от 100000 до 1000 с шагом 1000.
// Второй поток - извлекает числа из первого буфера. Обозначим извлечённое число переменной х. Поток вычисляет значение функции х/1000 и помещает его во второй буфер
// Третий поток - извлекает из второго буфера числа, начиная с максимального и выводит их на экран.

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

const int N = 10;
std::vector<int> buffer1(N); // Первый буфер
std::vector<double> buffer2(N); // Второй буфер
int buffer1_count = 0; // Количество элементов в первом буфере
int buffer2_count = 0; // Количество элементов во втором буфере
std::mutex mtx; // Мьютекс для синхронизации доступа к буферам
std::condition_variable cv1, cv2; // Условные переменные для ожидания заполнения буферов

// Первый поток: генерация чисел и помещение их в первый буфер
void producer() {
    for (int i = 0; i < 100; ++i) {
        int number = 100000 - i * 1000;
        std::unique_lock<std::mutex> lock(mtx);
        cv1.wait(lock, []{ return buffer1_count < N; });
        buffer1[buffer1_count++] = number;
        lock.unlock();
        cv2.notify_one();
    }
}

// Второй поток: извлечение чисел из первого буфера, вычисление функции и помещение результата во второй буфер
void calculator() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv2.wait(lock, []{ return buffer1_count > 0; });
        int number = buffer1[--buffer1_count];
        lock.unlock();
        double result = number / 1000.0;
        lock.lock();
        cv1.notify_one();
        cv2.wait(lock, []{ return buffer2_count < N; });
        buffer2[buffer2_count++] = result;
        lock.unlock();
    }
}

// Третий поток: извлечение чисел из второго буфера и вывод на экран
void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv2.wait(lock, []{ return buffer2_count > 0; });
        double number = buffer2[--buffer2_count];
        lock.unlock();
        std::cout << number << std::endl;
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread calculator_thread(calculator);
    std::thread consumer_thread(consumer);

    producer_thread.join();
    calculator_thread.join();
    consumer_thread.join();

    return 0;
}

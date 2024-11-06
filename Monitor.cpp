#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <fstream>

class MonitorQueue {
private:
    std::deque<int> queue;
    std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    size_t max_size;
    bool producers_done = false;
    int consumer_wait_time;
    std::ofstream log_file;

public:
    MonitorQueue(size_t initial_size, int wait_time) 
        : max_size(initial_size), consumer_wait_time(wait_time), log_file("log.txt") {}

    ~MonitorQueue() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    // FunciÃ³n para que los productores agreguen elementos
    void produce(int item) {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.size() >= max_size) {
            not_full.wait(lock); // Espera si la cola estÃ¡ llena
        }
        queue.push_back(item);
        log_file << "Produced: " << item << ", Queue Size: " << queue.size() << std::endl;

        // Dobla el tamaÃ±o de la cola si estÃ¡ llena
        if (queue.size() == max_size) {
            max_size *= 2;
            log_file << "Queue doubled to: " << max_size << std::endl;
        }

        not_empty.notify_all(); // Notifica a los consumidores
    }

    // FunciÃ³n para que los consumidores extraigan elementos
    bool consume(int &item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Espera hasta que haya elementos o los productores hayan terminado
        while (queue.empty() && !producers_done) {
            not_empty.wait(lock);
        }

        // Verifica si ya no quedan elementos para consumir y los productores terminaron
        if (queue.empty() && producers_done) {
            return false;
        }

        // Extrae un elemento si la cola no estÃ¡ vacÃ­a
        item = queue.front();
        queue.pop_front();
        log_file << "Consumed: " << item << ", Queue Size: " << queue.size() << std::endl;

        // Reduce el tamaÃ±o de la cola si estÃ¡ ocupada al 25% o menos
        if (queue.size() <= max_size / 4 && max_size > 1) {
            max_size /= 2;
            log_file << "Queue reduced to: " << max_size << std::endl;
        }

        not_full.notify_all(); // Notifica a los productores
        return true;
    }

    // Llamada cuando todos los productores han terminado
    void setProducersDone() {
        std::lock_guard<std::mutex> lock(mtx);
        producers_done = true;
        not_empty.notify_all(); // Notifica a los consumidores en espera
    }

    // Tiempo de espera para consumidores despuÃ©s de que los productores terminan
    bool waitConsumersToFinish() {
        std::this_thread::sleep_for(std::chrono::seconds(consumer_wait_time));
        return queue.empty(); // Retorna verdadero si la cola estÃ¡ vacÃ­a
    }
};

void producer(MonitorQueue &mq, int id) {
    
        mq.produce(id * 100); // Producir un nÃºmero Ãºnico
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simula tiempo de producciÃ³n
    
}

void consumer(MonitorQueue &mq) {
    int item;
    if (mq.consume(item)) {
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simula tiempo de consumo
    }
    // El consumidor termina después de consumir un solo elemento o si no hay nada que consumir.
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        std::cerr << "Usage: ./simulapc -p <producers> -c <consumers> -s <initial_queue_size> -t <consumer_wait_time>" << std::endl;
        return 1;
    }

    int num_producers = std::stoi(argv[2]);
    int num_consumers = std::stoi(argv[4]);
    size_t initial_queue_size = std::stoi(argv[6]);
    int consumer_wait_time = std::stoi(argv[8]);

    MonitorQueue mq(initial_queue_size, consumer_wait_time);

    // Crea hebras de productores
    std::vector<std::thread> producers;
    for (int i = 0; i <= num_producers; ++i) {
        producers.push_back(std::thread(producer, std::ref(mq), i));
    }

    // Crea hebras de consumidores
    std::vector<std::thread> consumers;
    for (int i = 0; i <= num_consumers; ++i) {
        consumers.push_back(std::thread(consumer, std::ref(mq)));
    }

    // Espera a que todos los productores terminen
    for (auto &p : producers) {
        p.join();
    }

    // Notifica a los consumidores que la producciÃ³n terminÃ³
    mq.setProducersDone();

    // Espera a que todos los consumidores terminen
    for (auto &c : consumers) {
        c.join();
    }

    std::cout << "Simulation completed. Check log.txt for details." << std::endl;

    return 0;
}

#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <fstream>

using namespace std;

class MonitorQueue {
private:
    deque<int> queue;
    mutex mtx;
    condition_variable not_empty;
    condition_variable not_full;
    size_t max_size;
    bool producers_done = false;
    int consumer_wait_time;
    ofstream log_file;

public:
    MonitorQueue(size_t initial_size, int wait_time) 
        : max_size(initial_size), consumer_wait_time(wait_time), log_file("log.txt") {}


    
    void produce(int item) {
        unique_lock<mutex> lock(mtx);
        while (queue.size() >= max_size) {
            not_full.wait(lock); 
        }
        queue.push_back(item);
        log_file << "Produced: " << item << ", Queue Size: " << queue.size() << endl;

        
        if (queue.size() == max_size) {
            max_size *= 2;
            log_file << "Queue doubled to: " << max_size << endl;
        }

        not_empty.notify_all(); // Notifica a los consumidores
    }

   
    bool consume(int &item) {
        unique_lock<mutex> lock(mtx);
        
        
        while (queue.empty() && !producers_done) {
            not_empty.wait(lock);
        }

        
        if (queue.empty() && producers_done) {
            return false;
        }

        
        item = queue.front();
        queue.pop_front();
        log_file << "Consumed: " << item << ", Queue Size: " << queue.size() << endl;

       
        if (queue.size() <= max_size / 4 && max_size > 1) {
            max_size /= 2;
            log_file << "Queue reduced to: " << max_size << endl;
        }

        not_full.notify_all(); 
        return true;
    }

    
    void setProducersDone() {
        lock_guard<mutex> lock(mtx);
        producers_done = true;
        not_empty.notify_all(); 
    }

    
    bool waitConsumersToFinish() {
        this_thread::sleep_for(chrono::seconds(consumer_wait_time));
        return queue.empty(); 
    }
};

void producer(MonitorQueue &mq, int id, int products) {
    for (int i = 0; i < products; ++i) {
        mq.produce(id * 100 + i); 
        this_thread::sleep_for(chrono::milliseconds(50)); 
    }
}

void consumer(MonitorQueue &mq) {
    int item;
    while (mq.consume(item)) {
        
        this_thread::sleep_for(chrono::milliseconds(100)); // Simula tiempo de consumo
    }
    
    if (!mq.waitConsumersToFinish()) {
        cout << "Consumer timed out waiting for items." << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        cerr << "Usage: ./simulapc -p <producers> -c <consumers> -s <initial_queue_size> -t <consumer_wait_time>" << endl;
        return 1;
    }

    int num_producers = stoi(argv[2]);
    int num_consumers = stoi(argv[4]);
    size_t initial_queue_size = stoi(argv[6]);
    int consumer_wait_time = stoi(argv[8]);

    MonitorQueue mq(initial_queue_size, consumer_wait_time);

    // Crea hebras de productores
    vector<thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.push_back(thread(producer, ref(mq), i, 10));
    }

    // Crea hebras de consumidores
    vector<thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.push_back(thread(consumer, ref(mq)));
    }

    // Espera a que todos los productores terminen
    for (auto &p : producers) {
        p.join();
    }

    
    mq.setProducersDone();

    
    for (auto &c : consumers) {
        c.join();
    }

    

    return 0;
}

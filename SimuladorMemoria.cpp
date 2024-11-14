
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

class PageTable
{
private:
    std::unordered_map<int, std::list<int>::iterator> table;
    std::list<int> pages;
    int capacity;

public:
    PageTable(int size) : capacity(size) {}

    bool accessPage(int page)
    {
        if (table.find(page) == table.end())
        {
            if (pages.size() == capacity)
            {
                int last = pages.back();
                pages.pop_back();
                table.erase(last);
            }
            pages.push_front(page);
            table[page] = pages.begin();
            return false;
        }
        else
        {
            pages.erase(table[page]);
            pages.push_front(page);
            table[page] = pages.begin();
            return true;
        }
    }
};

int simulateFIFO(const std::vector<int> &references, int frames)
{
    std::unordered_map<int, int> pageTable;
    std::queue<int> fifoQueue;
    int pageFaults = 0;

    for (int page : references)
    {
        if (pageTable.find(page) == pageTable.end())
        {
            pageFaults++;
            if (fifoQueue.size() == frames)
            {
                int oldPage = fifoQueue.front();
                fifoQueue.pop();
                pageTable.erase(oldPage);
            }
            fifoQueue.push(page);
            pageTable[page] = 1;
        }
    }

    return pageFaults;
}

int simulateLRU(const std::vector<int> &references, int frames)
{
    PageTable pageTable(frames);
    int pageFaults = 0;

    for (int page : references)
    {
        if (!pageTable.accessPage(page))
        {
            pageFaults++;
        }
    }

    return pageFaults;
}

int simulateSecondChance(const std::vector<int> &references, int frames)
{
    std::unordered_map<int, bool> pageTable;
    std::queue<int> fifoQueue;
    int pageFaults = 0;

    for (int page : references)
    {
        if (pageTable.find(page) == pageTable.end())
        {
            pageFaults++;
            while (fifoQueue.size() == frames)
            {
                int oldPage = fifoQueue.front();
                fifoQueue.pop();
                if (pageTable[oldPage])
                {
                    pageTable[oldPage] = false;
                    fifoQueue.push(oldPage);
                }
                else
                {
                    pageTable.erase(oldPage);
                }
            }
            fifoQueue.push(page);
            pageTable[page] = false;
        }
        else
        {
            pageTable[page] = true;
        }
    }

    return pageFaults;
}

int simulateOptimal(const std::vector<int> &references, int frames)
{
    std::unordered_map<int, int> pageTable;
    int pageFaults = 0;

    for (size_t i = 0; i < references.size(); ++i)
    {
        int page = references[i];
        if (pageTable.find(page) == pageTable.end())
        {
            pageFaults++;
            if (pageTable.size() == frames)
            {
                int farthest = i;
                int pageToRemove = -1;
                for (const auto &entry : pageTable)
                {
                    int nextUse = std::find(references.begin() + i + 1, references.end(), entry.first) - references.begin();
                    if (nextUse > farthest)
                    {
                        farthest = nextUse;
                        pageToRemove = entry.first;
                    }
                }
                pageTable.erase(pageToRemove);
            }
            pageTable[page] = i;
        }
        else
        {
            pageTable[page] = i;
        }
    }

    return pageFaults;
}

std::vector<int> readReferences(const std::string &filename)
{
    std::ifstream file(filename);
    std::vector<int> references;
    int ref;

    while (file >> ref)
    {
        references.push_back(ref);
    }

    return references;
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " -m <frames> -a <algorithm> -f <file>" << std::endl;
        return 1;
    }

    int frames = 0;
    std::string algorithm;
    std::string filename;

    for (int i = 1; i < argc; i += 2)
    {
        std::string arg = argv[i];
        if (arg == "-m")
        {
            frames = std::stoi(argv[i + 1]);
        }
        else if (arg == "-a")
        {
            algorithm = argv[i + 1];
        }
        else if (arg == "-f")
        {
            filename = argv[i + 1];
        }
    }

    std::vector<int> references = readReferences(filename);
    int pageFaults = 0;

    if (algorithm == "FIFO")
    {
        pageFaults = simulateFIFO(references, frames);
    }
    else if (algorithm == "LRU")
    {
        pageFaults = simulateLRU(references, frames);
    }
    else if (algorithm == "LRUR")
    {
        pageFaults = simulateSecondChance(references, frames);
    }
    else if (algorithm == "Optimo")
    {
        pageFaults = simulateOptimal(references, frames);
    }
    else
    {
        std::cerr << "Unknown algorithm: " << algorithm << std::endl;
        return 1;
    }

    std::cout << "Page faults: " << pageFaults << std::endl;

    return 0;
}

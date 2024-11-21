
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

// probablemente haya librerias que no se usaron porque a lo largo del trabajo intentamos varias cosas y olvide que era necesario y que no :P
using namespace std;

// En todas las implementaciones se usa un unordered map (porque este cuenta con hashing)con su key int y los valores list<int> para simular el chaining
// PARA EJECUTAR EL ALGORITMO DEBE DE USARSE "FIFO", "LRU", "RELOJ" O "OPTIMO" COMO ARGUMENTO DE LA LINEA DE COMANDOS
int simulateLRU(const vector<int> &references, int frames)
{
    unordered_map<int, list<int>> PageTable;
    int pageFaults = 0;
    list<int> pages; // Lista de paginas que se usa para mantener el orden de las paginas(para saber que pagina se inserto hace mas tiempo)

    for (int page : references) // Se recorre el vector de referencias
    {
        if (PageTable.find(page) == PageTable.end()) // Se ejecuta si la pagina no esta en la table
        {
            pageFaults++;               // Se aumenta el contador de fallos
            if (pages.size() == frames) // Si la tabla esta llena/no queda espacio para insertar se elimina la pagina que se inserto hace mas tiempo
            {
                int last = pages.back();
                pages.pop_back();
                PageTable.erase(last);
            }
            pages.push_front(page);
            PageTable[page].push_back(page);
        }
        else
        {
            pages.erase(find(pages.begin(), pages.end(), page)); // si la pagina ya esta en la tabla se elimina de la lista de paginas y se la reincerta para que quede al principio
            pages.push_front(page);                              // esto marca que la pagina fue usada recientemente
        }
    }

    return pageFaults;
}

int simulateFIFO(const vector<int> &references, int frames)
{
    int pageFaults = 0;

    unordered_map<int, list<int>> pageTable;
    list<int> fifoList;

    for (int page : references) // Se recorre el vector de referencias
    {
        if (pageTable.find(page) == pageTable.end()) // Analogo a la implementacion anterior
        {
            pageFaults++;                  // Se aumenta el contador de fallos
            if (fifoList.size() == frames) // Si la tabla esta llena se elimina la pagina que se inserto hace mas tiempo
            {
                int oldPage = fifoList.back();
                // printf("Poped page: %d\n", oldPage);
                fifoList.pop_back();
                pageTable.erase(oldPage);
            }
            fifoList.push_front(page); // Se inserta la pagina al principio de la lista y a la tabla
            pageTable[page].push_back(page);
        }
    }

    return pageFaults;
}

int simulateLRUReloj(const vector<int> &references, int frames)
{
    int pageFaults = 0;
    unordered_map<int, list<int>> pageTable;
    vector<pair<int, bool>> fifoList; // Se usa un vector de pair para simular el bit de referencia

    for (int page : references)
    { // Se recorre el vector de referencias

        bool found = false;

        for (auto &entry : fifoList) // Se busca si la pagina ya esta en la lista
        {
            if (entry.first == page)
            {
                entry.second = true; // Si la pagina ya esta en la lista se cambia su bit de referencia a 1
                found = true;
                break;
            }
        }

        if (!found) // La pagina no esta en la lista
        {
            pageFaults++;                     // Se aumenta el contador de fallos
            while (fifoList.size() == frames) // mientras la tabla no tiene frames disponibles
            {
                auto &entry = fifoList.front();
                if (entry.second) // Si el bit de referencia de la pagina es 1 se le da una segunda oportunidad
                {
                    entry.second = false;
                    fifoList.push_back(entry);
                    // printf("Gave second chance to page: %d\n", entry.first);
                    fifoList.erase(fifoList.begin());
                }
                else // Si el bit de referencia es 0 se elimina la pagina de la tabla
                {

                    // printf("Kicked page: %d, to put in page: %d \n", entry.first, page);
                    pageTable[entry.first].remove(entry.first);
                    fifoList.erase(fifoList.begin());
                    break;
                }
            }
            fifoList.push_back({page, false});
            // printf("Inserted page: %d \n", page);    //Si la tabla no esta llena se inserta la pagina con bit de referencia 0
            pageTable[page].push_back(page);
        }
    }
    return pageFaults;
}

int simulateOptimal(const vector<int> &references, int frames)
{
    int pageFaults = 0;
    vector<pair<int, int>> OptList; // Se usa un vector de pair para guardar las paginas que estan en la tabla y su proximo uso
    unordered_map<int, list<int>> pageTable;

    for (size_t i = 0; i < references.size(); ++i) // Se itera sobre los elementos del vector de referencias
    {
        int page = references[i];
        if (pageTable.find(page) == pageTable.end()) // chekea si la pagina no esta en la tabla
        {
            pageFaults++; // Se aumenta el contador de fallos

            if (pageTable.size() == frames) // chekea si la tabla esta llena
            {

                int farthest = OptList[0].second; // Se busca la pagina que se usara mas tarde y se la elimina de la tabla y de la lista de paginas en la tabla
                int pageToRemove = 0;
                for (size_t j = 0; j < OptList.size(); ++j)
                {
                    if (OptList[j].second > farthest)
                    {
                        farthest = OptList[j].second;
                        pageToRemove = j;
                    }
                }
                // printf("Replaced furthes use page: %d With next use: %d\n", OptList[pageToRemove].first, farthest);
                pageTable.erase(OptList[pageToRemove].first);
                OptList.erase(OptList.begin() + pageToRemove);
            }
            int nextUse = references.size(); // si la tabla no esta llena, se inserta el elemento a la tabla y se calcula el proximo uso del elemento a insertar para guardarlo en la lista de paginas
            for (int j = i + 1; j < references.size(); j++)
            {
                if (references[j] == page)
                {
                    nextUse = j;
                    break;
                }
            }
            pageTable[page].push_back(page);
            OptList.push_back({page, nextUse});
            // printf("inserted page: %d With next use: %d\n", page, nextUse);
        }
        else
        {
            for (auto &entry : OptList) // Si la pagina ya esta en la tabla se busca en la lista de paginas y se actualiza el proximo uso
            {
                if (entry.first == page)
                {
                    int nextUse = references.size();
                    for (int j = i + 1; j < references.size(); ++j)
                    {
                        if (references[j] == page)
                        {
                            nextUse = j;
                            break;
                        }
                    }
                    entry.second = nextUse;
                    // printf("uptaded page: %d next use value to: %d\n", page, nextUse);
                    break;
                }
            }
        }
    }

    return pageFaults;
}

vector<int> readReferences(const string &filename) // Funcion para leer el archivo de referencias
{
    ifstream file(filename);
    vector<int> references;
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
        cerr << "Entrada valida: " << argv[0] << " -m <numero de frames> -a <algoritmo> -f <archivo>" << endl;
        return 1;
    }

    int frames = 0;
    string algorithm;
    string filename;

    for (int i = 1; i < argc; i += 2) // Se recorre los argumentos de la linea de comandos
    {
        string arg = argv[i];
        if (arg == "-m") // numero de frames
        {
            frames = stoi(argv[i + 1]);
        }
        else if (arg == "-a") // algoritmo
        {
            algorithm = argv[i + 1];
        }
        else if (arg == "-f") // nombre del archivo
        {
            filename = argv[i + 1];
        }
    }

    vector<int> references = readReferences(filename);
    int pageFaults = 0;

    if (algorithm == "FIFO")
    {
        pageFaults = simulateFIFO(references, frames);
    }
    else if (algorithm == "LRU")
    {
        pageFaults = simulateLRU(references, frames);
    }
    else if (algorithm == "RELOJ")
    {
        pageFaults = simulateLRUReloj(references, frames);
    }
    else if (algorithm == "OPTIMO")
    {
        pageFaults = simulateOptimal(references, frames);
    }
    else
    {
        cerr << "Unknown algorithm: " << algorithm << endl;
        return 1;
    }

    cout << "Page faults: " << pageFaults << endl;

    return 0;
}

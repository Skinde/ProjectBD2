#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

namespace SeqFile {
    // Lista de constantes
    const int RAW_RECORD_SIZE = 114;
    const long END_OF_LIST = -1;
    const long NO_DELETED = -2;
    const long ID_OFFSET = 2 * sizeof(long);
    const long NEXTDEL_OFFSET = sizeof(long);
    const long METADATA = sizeof(long) + 2 * sizeof(int) + sizeof(long);
    const long SIZE_OFFSET = sizeof(long);
    const long HEAP_OFFSET = SIZE_OFFSET + sizeof(int);
    const long MIN_OFFSET = HEAP_OFFSET + sizeof(int);

    stringstream fixedRead(ifstream& file, size_t size){
        char buffer [size];
        file.read((char*) &buffer, size);
        stringstream ss(buffer);
        return ss;
    }
    
    struct Record { 
        long id;                    
        long avg_players;
        long gain;
        long peak_players;
        int year;           
        char URL [35];           
        char game_name [43];
    };

    struct SeqRecord {
        long next;              //ordenamiento temporal
        long nextDel;           //LIFO FreeList
        long id;                    
        long avg_players;
        long gain;
        long peak_players;
        int year;           
        char URL [35];            
        char game_name [43];

        void writeWithoutMetadata(ofstream& file){
            file.write((char*) &id, sizeof(long));
            file.write((char*) &avg_players, sizeof(long));
            file.write((char*) &gain, sizeof(long));
            file.write((char*) &peak_players, sizeof(long));
            file.write((char*) &year, sizeof(int));
            file.write((char*) &URL, 35);
            file.write((char*) &game_name, 43);
        }
    };

    bool recordComp(const Record& r1, const Record& r2){ return r1.id < r2.id; };

    bool SeqRecordComp(const SeqRecord& r1, const SeqRecord& r2){ return r1.id < r2.id; };

    //Dataset para pruebas
    SeqRecord dataset [10] = {
        {0, 0, 2, 1248, -745, 2290, 2014, "https://steamcharts.com/app/306130",  "The Elder Scrolls Online"},
        {0, 0, 3, 1035, -1111, 2277, 2017, "https://steamcharts.com/app/275850",  "No Man's Sky"},
        {0, 0, 4, 1012, -1468, 2262, 2018, "https://steamcharts.com/app/594650",  "Hunt: Showdown"},
        {0, 0, 5, 668, -277, 2221, 2019, "https://steamcharts.com/app/835570",  "Conqueror's Blade"},
        {0, 0, 6, 870, -1275, 2220, 2017, "https://steamcharts.com/app/275850",  "No Man's Sky"},
        {0, 0, 7, 940, -223, 2180, 2020, "https://steamcharts.com/app/123881",  "Battlefield V"},
        {0, 0, 8, 774, -2209, 2172, 2016, "https://steamcharts.com/app/275850",  "No Man's Sky"},
        {0, 0, 9, 1006, 162, 2095, 2015, "https://steamcharts.com/app/306130",  "The Elder Scrolls Online"},
        {0, 0, 10, 1004, -231, 2051, 2013, "https://steamcharts.com/app/227300",  "Euro Truck Simulator 2"},
        {0, 0, 11, 1035, 253, 2047, 2018, "https://steamcharts.com/app/275850",  "No Man's Sky"}
    };
    //Partimos a la mitad el arreglo, solo para testear sin reordenamiento
    const int TEST_SIZE = sizeof(dataset) / (sizeof(dataset[0]) * 2 );
}
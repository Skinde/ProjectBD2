#include "SeqFile/test_seqfile.cpp"
#include "hash/hash.cpp"
#include <chrono>
using namespace SeqFile;
using namespace std::chrono;

/*
template <class ExtendibleHash> 
*/





class ChronoTester {
private:
    SequentialFile* SF = nullptr;
    Hash* ExH = nullptr;
    SeqRecord new_record;
    Registro record_hash;
   
    
public:
    ChronoTester(const std::string & fileName) {
        SequentialFile SFinstance(fileName);
        Hash ExHinstance("index.cpp", fileName);
        SF = &SFinstance;
        ExH = &ExHinstance;
        
        
        
    }

    void test(){
        new_record.id = 999;
        record_hash.primary_key = 999;
        add();
        search(999);
        rangeSearch(998,999);
        remove(999);
    }
    void add(){
        auto start = high_resolution_clock::now();
        SF->add(new_record);
        
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación add tomó " << duration.count() << "ms.\n";
        start = high_resolution_clock::now();
        ExH->insertion(record_hash);         
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        cout << "[Hash]: La operación insert tomó " << duration.count() << "ms.\n"; 
    }

    void search(const long& key){
        auto start = high_resolution_clock::now();
        SF->search(key);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación search tomó " << duration.count() << "ms.\n";
        start = high_resolution_clock::now();
        ExH->search(key);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        cout << "[Hash]: La operación search tomó " << duration.count() << "ms.\n";
    }

    // que cubra todos los datos 
    void rangeSearch(long begin, long end){
        auto start = high_resolution_clock::now();
        SF->rangeSearch(begin,end);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación range search tomó " << duration.count() << "ms.\n";
        start = high_resolution_clock::now();
        ExH->rangeSearch(begin,end);
        stop = high_resolution_clock::now();
        cout << "[Hash]: La operación range search tomó " << duration.count() << "ms.\n";
    }

    void remove(const long& key){
        auto start = high_resolution_clock::now();
        SF->remove(key);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación remove tomó " << duration.count() << "ms.\n";
        start = high_resolution_clock::now();
        ExH->deleteThis(key);
        stop = high_resolution_clock::now();
        duration = duration_cast<microseconds>(stop - start);
        cout << "[Hash]: La operación remove tomó " << duration.count() << "ms.\n";
    }



    ~ChronoTester() { delete SF; };
};
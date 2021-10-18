#include "SeqFile/seqfile.cpp"
#include <chrono>
using namespace SeqFile;
using namespace std::chrono;

template <class ExtendibleHash> 
class ChronoTester {
private:
    SequentialFile* SF = nullptr;
    ExtendibleHash* ExH = nullptr;
    /*
    Record sampleRecord (
        10000, 0, 0, 0, 0, "este record tiene que tener exactamente 43"
    )
    SeqRecord sampleRecord(
        -1, -2 , ....
    )
    */
public:
    ChronoTester(const string& fileName) {
        SequentialFile SFinstance(fileName);
        ExtendibleHash ExHinstance(fileName);
        SF = &SFinstance;
        ExH = &ExHinstance;
    }

    void add(){
        auto start = high_resolution_clock::now();
        SF->add(record);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación add tomó " << duration.count() << "ms.\n";
    }

    void search(const long& key){
        auto start = high_resolution_clock::now();
        SF->search(key);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación search tomó " << duration.count() << "ms.\n";
    }

    // que cubra todos los datos 
    void rangeSearch(long begin, long end){
        auto start = high_resolution_clock::now();
        SF->rangeSearch(begin,end);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación range search tomó " << duration.count() << "ms.\n";
    }

    void remove(const long& key){
        auto start = high_resolution_clock::now();
        SF->remove(key);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "[SeqFile]: La operación remove tomó " << duration.count() << "ms.\n";
    }

    ~ChronoTester() { delete SF; };
};
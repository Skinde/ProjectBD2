#include "seqfile.cpp"
using namespace SeqFile;

int seqFileTest(){
    SequentialFile SF = SequentialFile("Data/9_9k.dat");
    
    vector<SeqRecord> testData {};
    for(const auto& Seqrecord: dataset) testData.push_back(Seqrecord);
    int errCount {}, testIdx {};

    cout << "\t\t\t ==== TESTS FUNCIONALES: SEQUENTIAL FILE ===\n";
    
    cout << "Probando search... \n";
    for(const auto& record: testData){
        auto searched = SF.search(record.id); 
        if(searched.id == record.id) continue;
        else {
            errCount++;
            cout << "(" << SF.search(record.id).id << " - " << record.id << "): Hubo un problema con el search.\n";
        }
    }
    if(errCount == 0) cout <<  "PASSED: search esta ok.\n";
    
    cout << "Probando rangeSearch... \n";
    vector<SeqRecord> someRecords = SF.rangeSearch(1, 999);
    for(const auto& record: someRecords){
        if(record.id == testData[testIdx].id) continue;
        else {
            cout << "(" << ++errCount << "): Hubo un problema con el rangeSearch.\n";
            cout << "Actual: " << record.id << ", Expected: " << testData[testIdx++].id <<endl;
        }
    }
    if(errCount == 0) cout <<  "PASSED: rangeSearch esta ok.\n";
    
    
    cout << "Añadiendo registros... \n";
    SeqRecord newRecords [7] = {
        {-1, -2, 1100, 1248, -745, 2290, 2014, "https://steamcharts.com/app/306130",  "The Elder Scrolls Online"},
        {-1, -2, 1103, 1035, -1111, 2277, 2017, "https://steamcharts.com/app/275850",  "No Man's Sky"},
        {-1, -2, 1101, 1012, -1468, 2262, 2018, "https://steamcharts.com/app/594650",  "Hunt: Showdown"},
        {-1, -2, -2, 668, -277, 2221, 2019, "https://steamcharts.com/app/835570",  "Conqueror's Blade"},
        {-1, -2, 18, 870, -1275, 2220, 2017, "https://steamcharts.com/app/275850",  "No Man's Sky"},
    };

    for (const auto& record: newRecords) {
        SF.add(record);
        testData.push_back(record);
    }
    sort(testData.begin(), testData.end(), SeqRecordComp);

    
    cout << "Probando search (luego de add)... \n";
    for(const auto& record: testData){
        if(SF.search(record.id).id == record.id) continue;
        else {
             errCount++;
            cout << "(" << SF.search(record.id).id << " - " << record.id << "): Hubo un problema con el add o el search.\n";
        }
    }
    if(errCount == 0) cout <<  "PASSED: search (luego de add) esta ok.\n";
    
    
    cout << "Probando rangeSearch (luego de add)... \n";
    someRecords = SF.rangeSearch(1, 999);
    testIdx = 0;
    for(const auto& record: someRecords){
        if(SF.search(record.id).id == record.id) continue;
        else {
            errCount++;
            cout << "(" << ++errCount << "): Hubo un problema con el add o el rangeSearch.\n";
        }
    }
    if(errCount == 0) cout <<  "PASSED: rangeSearch (luego de add) esta ok.\n"; 

    cout << "Eliminando archivos...\n";
    vector<SeqRecord> removedData;
    for (int i=0; i < TEST_SIZE; i++) {
        auto record = newRecords[i];
        SF.remove(record.id);
        removedData.push_back(record);
    }
    sort(removedData.begin(), removedData.end(), SeqRecordComp);

    cout << "Probando search (sobre un elemento eliminado)\n";
    for(const auto record: removedData){
        if(SF.search(record.id).id == record.id) continue;
        else {
             errCount++;
            cout << "(" << SF.search(record.id).id << " - " << record.id << "): Hubo un problema con el remove.\n";
        }
    }
    if(errCount == 0) cout <<  "PASSED: search (sobre un elemento eliminado) esta ok.\n";

    //TODO: Probando rangeSearch con elementos en delete
    cout << "Probando rangeSearch (con elementos eliminados)... \n";
    someRecords = SF.rangeSearch(0, 34);
    testIdx = 0;
    for(const auto& record: someRecords){
        if(SF.search(record.id).id == record.id) continue;
        else {
            errCount++;
            cout << "(" << ++errCount << "): Hubo un problema con el remove.\n";
        }
    }
    if(errCount == 0) cout << "PASSED: rangeSearch (con elementos eliminados) esta ok.\n";
        
    if (errCount == 0) cout << "SEQFILE: PASSED ALL TESTS.\n";
    return 0;
}
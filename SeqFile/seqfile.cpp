#include <iostream>
#include <map>
#include "lib.hpp" 
#define HEAP_LIMIT 8 // Tamaño de aux
#define RECORD_SIZE 130
using namespace SeqFile;

//NOTA: Para simplificar el código, el 'archivo' aux se comporta como un heap en el mismo main file
//La idea es unir ambas partes del main file (ordered y heap) cada cierto tiempo

class SequentialFile {
private:
    string fileName;
    fstream writeMetadata(int size);
    void load();
    void reorganize();
    int binarySearch(const long& key);
    long findPrev(long pos);
    void addMin(SeqRecord record); 
public:
    SequentialFile(string filename): fileName(filename) { load(); };
    ~SequentialFile() { reorganize(); };
    void add(SeqRecord record);
    SeqRecord search(const long& key);
    vector<SeqRecord> rangeSearch(long begin, long end);
    void remove(const long& key);
};

// === FUNCIONES DE R/W ===

//O(1): Setea el espacio de las flags y define el tamaño del archivo.
fstream SequentialFile::writeMetadata(int size) {
    fstream file(fileName, ios::in | ios::out | ios::trunc | ios::binary);
    file.seekg(0, ios::beg);
    long freeListHeader = -1;
    file.write((char*) &freeListHeader, sizeof(long));
    file.write((char*) &size, sizeof(int));
    int heapSize = 0;
    file.write((char*) &heapSize, sizeof(int));
    long minIndex = 0;
    file.write((char*) &METADATA, sizeof(long));
    return file;
}

// O(n): Lee n registros, escribe metadata y los escribe de forma ordenada.
//       El usuario percibirá el ordenamiento sin recibir metadata que le es innecesaria
void SequentialFile::load(){
    ifstream read(fileName, ios::binary);
    if(!read.is_open()){
        std::cout << "No se puede abrir/encontrar el archivo, abortando el programa.\n";
        exit(-1);
    }

    read.seekg(0, ios::end);
    int size = read.tellg();
    size /= RAW_RECORD_SIZE;
    read.seekg(0, ios::beg);
    vector<Record> all;
    Record record;
    while(read.read((char*) &record, RAW_RECORD_SIZE)) all.push_back(record);
    read.close();

    sort(all.begin(), all.end(), recordComp);
    fstream write = writeMetadata(size);
    int i {};
    for(Record record: all) {
        long currNext = -1;
        if (++i < size) currNext = i * RECORD_SIZE;
        write.write((char*) &currNext, sizeof(long));
        write.write((char*) &NO_DELETED, sizeof(long));
        write.write((char*) &record, RAW_RECORD_SIZE);
    }

    write.close();
}

// O(n): Escribe todos los archivos de forma ordenada cuando se excede el heap.
void SequentialFile::reorganize(){
    ifstream read(fileName, ios::binary);
    if (!read.is_open()) {
        cout << "No se ha podido abrir el archivo. Se abortará el programa.\n";
        exit(-1);
    }
    
    vector<SeqRecord> all {};
    SeqRecord srecord;
    read.seekg(METADATA);
    while(read.read((char*) &srecord, RECORD_SIZE)) 
        if(srecord.nextDel == NO_DELETED) all.push_back(srecord);
    read.close();

    sort(all.begin(), all.end(), SeqRecordComp);
    ofstream write(fileName, ios::trunc | ios::binary);
    for(SeqRecord srecord: all) srecord.writeWithoutMetadata(write);
    write.close();
}

// === FUNCIONES DE BÚSQUEDA ===

// O(lgn): Busca un registro en el archivo principal. Si no lo encuentra, retorna el previo  
int SequentialFile::binarySearch(const long& key){
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cout << "No se ha podido abrir el archivo. Se abortará el programa.\n";
        exit(-1);
    }

    int l {}, u {};
    long pos {};
    file.seekg(SIZE_OFFSET);
    file.read((char*) &u, sizeof(int));
    while (l <= u){
        int m = (u + l) / 2;
        file.seekg(m * RECORD_SIZE + METADATA + ID_OFFSET);
        file.read((char*) &pos, sizeof(long));
        if (pos == key) {
            file.close();
            return m;
        } 
        else if (key < pos) u = m - 1;
        else l = m + 1;
    }
    file.close();
    return u;
}

//O(n): Dada una key, busca el registro menor más cercano (que no este eliminado)
long SequentialFile::findPrev(long pos){
    fstream file(fileName, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        cout << "No se puede abrir/encontrar el archivo, la operación insertAll no pudo realizarse.\n";
        exit(-1);
    }

    long prevPos = pos, posKey;
    file.seekg(pos + ID_OFFSET);
    file.read((char*) &posKey, sizeof(long));
    long status;
    do {
        //Si llegamos al primer elemento, debemos buscar el prev partiendo del minimo.
        if (prevPos = METADATA) {
            file.seekg(MIN_OFFSET);
            file.read((char*) &prevPos, sizeof(long));
            break;
        }

        //Revisamos si el registro esta eliminado.
        prevPos -= RECORD_SIZE;
        file.seekg(pos + NEXTDEL_OFFSET);
        file.read((char*) &status, sizeof(long));
    } while(status != NO_DELETED);

    //Recorremos el next para encontrar el previo más cercano
    long tempPos {}, actKey {};
    do {
        file.seekg(prevPos);
        tempPos = prevPos;
        file.read((char*) &prevPos, sizeof(long));
        file.seekg(prevPos + ID_OFFSET);
        file.read((char*) &actKey, sizeof(long));
    } while(actKey < posKey && prevPos != END_OF_LIST);

    file.close();
    return tempPos;
}

//O(lgn + k): Se usa binarySearch y luego se busca secuencialmente a partir del punto de retorno.
SeqRecord SequentialFile::search(const long& key){
    SeqRecord record;
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cout << "No se puede abrir/encontrar el archivo, la operación insertAll no pudo realizarse.\n";
        return record;
    }

    //Busca en main
    long recordKey;
    long recordIdx = binarySearch(key);
    if (recordIdx != -1) {
        file.seekg(recordIdx * RECORD_SIZE + METADATA + ID_OFFSET);
        file.read((char*) &recordKey, sizeof(long));   
    }

    //Si entra en este bloque es porque no lo encontro
    if (recordIdx == -1 | recordKey != key){

        //Revisa si puede buscar en heap
        int heapSize;
        file.seekg(HEAP_OFFSET);
        file.read((char*) &heapSize, sizeof(int));
        if(heapSize == 0){
            cout << "ERROR: No se encontro el registro.\n";
            return record;
        }

        //Busca en heap
        int size;
        file.seekg(SIZE_OFFSET);
        file.read((char*) &size, sizeof(int));
        file.seekg(size * RECORD_SIZE + METADATA);
        SeqRecord record;
        while(file.read((char*) &record, RECORD_SIZE)) {
            if(record.id == key){
                file.close();
                return record;
            }
        }
        file.close();
        cout << "ERROR: No se encontro el registro.\n";
        return record;
    } 

    //El record esta en el archivo principal                           
    file.seekg(recordIdx * RECORD_SIZE + METADATA);
    file.read((char*) &record, RECORD_SIZE);
    file.close();
    return record;
}

//O(lgn + n): Costo de search más procesar secuencialmente n o menos registros
vector<SeqRecord> SequentialFile::rangeSearch(long begin, long end){
    ifstream file(fileName, ios::binary);
    if (!file.is_open()) {
        cout << "No se puede abrir/encontrar el archivo, la operación rangeSearch no pudo realizarse.\n";
        return {};
    }

    //Solo nos aseguramos que el rango sea de menor a mayor y no al reves.
    file.seekg(min(begin,end) * RECORD_SIZE + METADATA + ID_OFFSET);
    file.read((char*) &begin, sizeof(long));
    file.seekg(max(begin,end) * RECORD_SIZE + METADATA + ID_OFFSET);
    file.read((char*) &end, sizeof(long));

    //Búsqueda puntual.
    SeqRecord record = search(begin);
    vector<SeqRecord> inRange = {};
    if(record.nextDel == NO_DELETED) inRange.push_back(record);

    //Recorrido en punteros next.
    SeqRecord buffer;
    long recordIdx;
    file.seekg(record.next);
    file.read((char*) &recordIdx, sizeof(long));
    while (recordIdx <= end){
        file.seekg(record.next);
        file.read((char*) &buffer, RECORD_SIZE);
        inRange.push_back(buffer);
        file.seekg(buffer.next + ID_OFFSET);
        file.read((char*) &recordIdx, sizeof(long));
    }
    file.close();
    return inRange;
}

// == FUNCIONES DE ACTUALIZACIÓN DE DATOS ==

//O(lgn + k): Busqueda binaria + busqueda secuencial en k o menos elementos.
void SequentialFile::add(SeqRecord record){
    fstream file(fileName, ios::binary | ios::in | ios::out);
    if (!file.is_open()) {
        cout << "No se puede abrir/encontrar el archivo, la operación insertAll no pudo realizarse.\n";
        return;
    }

    //Búsqueda puntual. Delegamos el caso a addMin si el registro es menor que el menor del archivo principal.
    int searched = binarySearch(record.id);
    if (searched == -1) addMin(record);
    
    else {
        //Si el nuevo elemento es mayor al último
        searched *= RECORD_SIZE;
        searched += METADATA;
        file.seekg(0, ios::end);
        if(searched == file.tellg()) {
            record.next = -1;
            file.seekg(searched - RECORD_SIZE);
            file.write((char*) &searched, sizeof(long));
        }

        else {
            //Busqueda secuencial en aux
            file.seekg(searched);
            long nextIdx;
            file.read((char*) &nextIdx, sizeof(long));
            while (nextIdx < record.id && nextIdx != END_OF_LIST) {
                file.seekg(nextIdx);
                searched = nextIdx;
                file.read((char*) &nextIdx, sizeof(long));
            } 

            //Actualización de next y size
            record.next = nextIdx;
            file.seekg(0, ios::end);
            long endPos = file.tellg();
            file.seekg(searched);
            file.write((char*) &endPos, sizeof(long));
        }
    }

    //Escritura en heap
    file.seekg(0, ios::end);
    file.write((char*) &record, RECORD_SIZE);

    //Revisamos si debemos reorganizar
    int heapSize;
    file.seekg(HEAP_OFFSET);
    file.read((char*) &heapSize, sizeof(int));
    file.seekg(HEAP_OFFSET);
    ++heapSize;
    file.write((char*) &heapSize, sizeof(int));
    file.close();
    if(heapSize == HEAP_LIMIT) {
        cout << "REESTABLECIENDO ARCHIVO. SE AGREGARAN "<< HEAP_LIMIT << " REGISTROS\n";
        reorganize();
        load();
    }
}


// O(K): Dado que el elmento es menor a todos los del archivo principal, no llamaremos más de K veces al puntero next.
void SequentialFile::addMin(SeqRecord record){
    fstream file(fileName, ios::binary | ios::in | ios::out);

    //Lee el menor elemento
    long minIndex;
    file.seekg(MIN_OFFSET);
    file.read((char*) &minIndex, sizeof(long));
    long minKey;
    file.seekg(minIndex * RECORD_SIZE + METADATA + ID_OFFSET);
    file.read((char*) &minKey, sizeof(long));

    //Si es el menor de toda la colección entra en el if
    if (record.id <= minKey){
        record.next = minIndex * RECORD_SIZE +  METADATA;
        file.seekg(0, ios::end);
        long endPos = file.tellg();
        file.seekg(MIN_OFFSET);
        file.write((char*) &endPos, sizeof(long));
    }
    else {
        //Podriamos simplemente revisar todo el heap, pero con esto nos ahorraremos tiempo la mayoría de las veces
        long nextKey, nextIdx = minIndex * RECORD_SIZE + METADATA, prevIdx = nextIdx;
        file.seekg(nextIdx + ID_OFFSET);
        file.read((char*) &nextKey, sizeof(long));
        while(record.id > nextKey){
            file.seekg(nextIdx);
            prevIdx = nextIdx;
            file.read((char*) &nextIdx, sizeof(long));
            file.seekg(nextIdx + ID_OFFSET);
            file.read((char*) &nextKey, sizeof(long));
        }
        record.next = nextIdx;
        file.seekg(0, ios::end);
        long endPos = file.tellg();
        file.seekg(prevIdx);
        file.write((char*) &endPos, sizeof(long));
    }
    file.seekg(0, ios::end);
    file.write((char*) &record, RECORD_SIZE);
    file.close();
}

void SequentialFile::remove(const long& key){
    fstream file(fileName, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        cout << "No se puede abrir/encontrar el archivo, la operación remove no pudo realizarse.\n";
        return;
    }

    //Buscar el registro a eliminar, primero en el main y si es necesario en el heap
    long pos = binarySearch(key);
    if (pos == -1){
        file.seekg(MIN_OFFSET);
        file.read((char*) &pos, sizeof(long));
        file.seekg(0, ios::end);
        if(pos == file.tellg()) pos -= RECORD_SIZE;
        //Count evita que haya segfault si no existe un record con la key a eliminar.
        int count {};
        long posKey;
        long temp = pos;
        do {
            pos = temp;
            file.seekg(temp + ID_OFFSET);
            file.read((char*) &posKey, sizeof(long));
            file.seekg(temp);
            file.read((char*) &temp, sizeof(long));
        } while(posKey != key && count++ < HEAP_LIMIT);
    }
    else {
        pos *= RECORD_SIZE;
        pos += METADATA;
    }

    file.seekg(0, ios::end);
    if(pos == file.tellg()) pos -= RECORD_SIZE;
    file.seekg(pos + ID_OFFSET);
    long posKey;
    file.read((char*) &posKey, sizeof(long));

    //Count evita que haya segfault si no existe un record con la key a eliminar.
    int count {};
    while(posKey != key && count++ < HEAP_LIMIT){
        file.seekg(pos);
        file.read((char*) &pos, sizeof(long));
        file.seekg(pos + ID_OFFSET);
        file.read((char*) &posKey, sizeof(long));
    }
    if(posKey != key){
        cout << key << ": El registro que se pretende eliminar no existe en la base de datos.\n";
        return;
    }

    //Buscar registro anterior no eliminado: debe cumplirse que tiene a key en su next
    long prev = findPrev(pos), nextIdx;

    //Cambio de valores en next
    file.seekp(pos);
    file.read((char*) &nextIdx, sizeof(long));
    file.seekp(prev);
    file.write((char*) &nextIdx, sizeof(long));

    //Actualización de freeList
    file.seekg(0, ios::beg);
    long header;
    file.read((char*) &header, sizeof(long));
    file.seekg(0, ios::beg);
    file.write((char*) &key, sizeof(long));
    file.seekg(pos + NEXTDEL_OFFSET);
    file.write((char*) &header, sizeof(long));

    //Actualización de MinIndex si es necesario
    long minIndex;
    file.seekg(MIN_OFFSET);
    file.read((char*) &minIndex, sizeof(long));
    long minKey;
    file.seekg(minIndex * RECORD_SIZE + METADATA);
    file.read((char*) &minKey, sizeof(long));
    if(minKey == key){
        int nextMin;
        file.seekg(minIndex);
        file.read((char*) &nextMin, sizeof(long));
        nextMin -= (nextMin == 0)? 0:METADATA;
        nextMin /= RECORD_SIZE;
        file.seekg(MIN_OFFSET);
        file.write((char*) &nextMin, sizeof(long));
    }

    file.close();
}
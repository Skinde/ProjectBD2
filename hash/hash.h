#ifndef HASH_H
#define HASH_H
#include "lib.h"
#define D 5


const long M = (4096-sizeof(int) - 3*sizeof(long))/sizeof(Registro);

const Registro vacio{-1,0,0,0,0,"url","rimwoerl"};

struct Bucket{
    Registro registros[M];
    int size=0;
    long next= -1;
    long prev = -1;
    long freeNext = -1;
    Bucket(){
        for(int i = 0 ; i < M;i++){
            registros[i] = vacio;
        }
    }
};

void printBucket(Bucket& b);

struct Index{
    bool created = false;
    int profundidad[(long) pow(2,D)] = {0};
    long positions[(long) pow(2,D)] = {0};
};


class Hash{

    public:

        Hash(std::string indexName, std::string fileName);
        Registro search(long key);
        std::vector<Registro> rangeSearch(long begin, long end);
        void insertion(Registro nuevo);
        bool deleteThis(long key);
        void printIndex(){
            Index index = loadIndex();
            for(int i = 0 ; i < (long) pow(2 ,D); i++){
                std::cout <<" hash: "<<i <<" posicion: "<<index.positions[i]<<" profundidad: "<<index.profundidad[i]<<std::endl;
            }
        }
        void appendBuckets(){
            Bucket b;
            appendBucket(b);
            appendBucket(b);
            appendBucket(b);
        }
        void showBucket(long position){
            Bucket temp = loadBucket(position);
            printBucket(temp);
        }
        void showAllRootBuckets(){
            Index index = loadIndex();
            for(int i= 0; i < (long) pow(2,D);i++){
                std::cout<<i<<" "<<index.positions[i]<<std::endl;
                showBucket(index.positions[i]);
                std::cout<<"\n\n";
            }
        }

    private:
        std::string fileName;
        std::string indexName;
        long freeList;

        Index loadIndex();
        void updateIndex(Index& i);

        void loadFreeList();
        void updateFreeList(long& i);
            
        Bucket loadBucket(long& postion);
        void updateBucket(long& position, Bucket& b);
        
        long insertInFreeList(Bucket& b);
        void fillingBucket(long& position, Bucket& bucket , Registro& reg);
        long appendBucket(Bucket& b);
        void insertNewBucket(Bucket& b);
        void partitionateBucket(long& positionOfFull, Bucket& full, Bucket& empty, int indexFull, int indexEmpty);
        
        void eliminateRegisterFromBucket(Bucket& b, int position);

        long getHash(long& key);
        long readNBits(int N, long& number);
        Registro linearSearchInBucket(Bucket& bucket, long& key);
        Bucket SearchBucketInList(Bucket& bucket, long& key);

        long seekAndDestroyByMetallica(long& positionOfBucket, Bucket& bucket, long& key);
};


#endif

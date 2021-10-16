#include "hash.h"
bool compareKeys(const Registro& a, const Registro& b){
  return a.primary_key < b.primary_key;
}
void printBucket(Bucket& b){
  for(int i= 0; i< b.size; i++ ){
    std::cout<<b.registros[i].primary_key<<" "<<b.registros[i].avg_players<<" "<<b.registros[i].gain
      <<" "<<b.registros[i].peak_players<<" "<<b.registros[i].year<<" "<<b.registros[i].URL<<" "
      <<b.registros[i].game_name<<std::endl;
  }  
  std::cout<<b.size<<std::endl;
  std::cout<<b.next<<std::endl;
  std::cout<<b.prev<<std::endl;
  std::cout<<b.freeNext<<std::endl;
}
 //" Hash(string indexName, string fileName) {{{1
Hash::Hash(std::string indexName, std::string fileName){
    std::fstream index, file;
    this->indexName = indexName;
    this->fileName = fileName;
    //checkea si existe un indice
    Index check = loadIndex();
    //si no existe el indice lo crea
    if(check.created == false){
        check.created = true;
        check.profundidad[0] = check.profundidad[1] = 1; 
        //como aún no hay buckets llenas todas las posiciones con -1
        for(int i= 0; i<(long) pow(2,D);i++)
          check.positions[i] = -1;
        updateIndex(check);
    }
    // checkea si hay un main creado
    freeList = -2;
    loadFreeList();
    std::cout<<freeList<<std::endl;
    // crea el archivo main
    if(freeList == -2){
      std::cout<<"updateo el freelist"<<std::endl;
      freeList = -1;
      std::fstream file;
      file.open(fileName,std::fstream::app | std::fstream::binary);
      file.write((char*)& freeList, sizeof(freeList));
      file.close();
    }
}

//" loadIndex() {{{1
Index Hash::loadIndex(){
  std::fstream index;
  Index i;
  index.open(indexName, std::fstream::in | std::fstream::binary);
  index.read((char*) &i, sizeof(i));
  index.close();
  return i;
}
//" updateIndex(Index& i) {{{1
void Hash::updateIndex(Index& i){
  std::fstream index;
  index.open(indexName, std::fstream::out | std::fstream::binary);
  index.seekg(0);
  index.write((char*) &i, sizeof(i));
  index.close();
}
//" loadFreeList() {{{1
void Hash::loadFreeList(){
  std::fstream file;
  file.open(fileName, std::fstream::in | std::fstream::binary);
  file.read((char*)& freeList, sizeof(freeList));
  file.close(); 
}
//" updateFreeList(long& i) {{{1 
void Hash::updateFreeList(long& i){
  std::fstream file;
  file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::binary);
  file.seekg(0, std::ios::beg);
  file.write((char*)& i, sizeof(i));
  file.close();
  freeList = i;
} 
//" loadBuck et(long& position) {{{1
Bucket Hash::loadBucket(long& position){
  std::fstream file;
  file.open(fileName, std::fstream::in | std::fstream::binary);
  Bucket b;
  file.seekg(position ,std::ios::beg);
  file.read((char*) &b, sizeof(b));
  file.close();
  return b;
}
//" updateBucket(long& position, Bucket& b) {{{1 
void Hash::updateBucket(long& position, Bucket& b){
  std::fstream file;
  file.open(fileName, std::fstream::in | std::fstream::out | std::fstream::binary);
  file.seekg(position);
  std::cout<<"actualizado en posicion: "<<file.tellg()<<std::endl;
  file.write((char*) &b, sizeof(b));
  file.close();
}
//" getHash(long& key) {{{1
long Hash::getHash(long& key){
  long hashed = key % (long) pow(2, D);
  Index index = loadIndex();
  int cuantosLeer=0;
  for(int p = 1; p < D; p++){
    if(cuantosLeer < index.profundidad[readNBits(p, hashed)])
      cuantosLeer = index.profundidad[readNBits(p, hashed)];
  }
  return readNBits(cuantosLeer, hashed);
} 
//"  readNBits(int N , long& number) {{{1
long Hash::readNBits(int N, long& number){
  std::string binary = std::bitset<sizeof(long)>(number).to_string();
  binary = binary.substr(binary.size() - N,binary.size());
  return stoi(binary, 0, 2);
}

//" linearSearchInBucket(Bucket& bucket, long& key) {{{1
Registro Hash::linearSearchInBucket(Bucket& bucket, long& key){
  for(int i = 0 ; i < bucket.size; i++)
    if(bucket.registros[i].primary_key == key)
      return bucket.registros[i];
    if(bucket.next != -1){
      std::fstream file;
      file.open(fileName, std::fstream::in | std::fstream::binary);
      while(bucket.next >= -1){
        file.seekg(bucket.next);
        file.read((char*)& bucket, sizeof(bucket));
        for(int i = 0 ; i < bucket.size; i++)
          if(bucket.registros[i].primary_key == key)
            return bucket.registros[i];
      }
      file.close();
    }
    return {-1,0,0,0,0};
}
 
//" search(long key) {{{1
Registro Hash::search(long key){
  Index index = loadIndex();
  long positionOfBucket = index.positions[getHash(key)];
  Bucket temp = loadBucket(positionOfBucket);
  return linearSearchInBucket(temp, key);
} 
// " insertion(Registro nuevo) {{{1
void Hash::insertion(Registro nuevo){
  Index index = loadIndex();
  long positionOfBucket = index.positions[getHash(nuevo.primary_key)];
  std::cout<<"position of first bucket: "<<positionOfBucket<<std::endl;
      //si no tiene bucket
      if(positionOfBucket == -1){
        // lo creas e insertas el bucket
        std::cout<<"Primer bucket creado"<<std::endl;
        Bucket nuevoBucket;
        nuevoBucket.registros[0] = nuevo;
        nuevoBucket.size++;
        printBucket(nuevoBucket);
        insertNewBucket(nuevoBucket);
        
      }
      else{
        //si hay un bucket lo cargas
        Bucket current = loadBucket(positionOfBucket);
        std::cout<<"Bucket root cargado:"<<std::endl;
        printBucket(current);
        int indexValue = getHash(current.registros[0].primary_key);
        //si el bucket esta lleno
        if(current.size == M){
          std::cout<<"bucket lleno"<<std::endl;
          // si ya esta en la profundidad maxima solo queda hacer colisiones
          if(index.profundidad[indexValue] == D){
            
            Bucket nuevoBucket;
            nuevoBucket.registros[0] = nuevo;
            nuevoBucket.size++;

            insertNewBucket(nuevoBucket);
          }

          else{

            index.profundidad[indexValue]++;
            Bucket nuevoBucket;
        
            int nuevoIndexValue = indexValue + (int) pow(2,index.profundidad[indexValue]-1);
            std::cout<<"index complementario: "<<nuevoIndexValue<<std::endl;
            index.profundidad[nuevoIndexValue] = index.profundidad[indexValue];
            updateIndex(index);
            partitionateBucket(index, current, nuevoBucket, indexValue, nuevoIndexValue);
            std::cout<<"nuevo Bucket a insertarse: "<<std::endl;
            printBucket(nuevoBucket);
            if(nuevoBucket.size > 0){
              insertNewBucket(nuevoBucket);
              insertion(nuevo);
              updateBucket(positionOfBucket, current);
            }
            else{
              index.profundidad[indexValue]--;
              index.profundidad[nuevoIndexValue] = 0;
              updateIndex(index);
              nuevoBucket.registros[0] = nuevo;
              insertNewBucket(nuevoBucket);
            }

          }
        }
        //en caso no este lleno el bucket solo le agregas el registro al final
        else{
          std::cout<<"llenando el bucket"<<std::endl;
          current.registros[current.size] = nuevo;
          current.size++;
          updateBucket(positionOfBucket, current);
        }        
      }
}
//" partitionateBucket {{{1
    void Hash::partitionateBucket(Index& index,Bucket& full, Bucket& empty, int indexFull, int indexEmpty){
        std::cout<<"partition begins"<<std::endl;
        std::cout<<"full: "<<indexFull<<" empty:"<<indexEmpty<<std::endl;
        for(int i = 0; i < full.size; i++){
          std::cout<<"hash of element:"<<getHash(full.registros[i].primary_key)<<std::endl;
          while(getHash(full.registros[i].primary_key) == indexEmpty){
            std::cout<<"hash of element:"<<getHash(full.registros[i].primary_key)<<std::endl;
            empty.registros[empty.size] = full.registros[i];
            empty.size++;
            eliminateRegisterFromBucket(full,i);
            std::cout<<"se redujo: "<<indexFull<<" "<<full.size<<std::endl;
            if(i==full.size)
              break;
          }

        }
    }
     
    //" insertNewBucket {{{1
    void Hash::insertNewBucket(Bucket& b){
      Index index = loadIndex();
      int indexValue = getHash(b.registros[0].primary_key);
      std::cout<<"hash : "<<indexValue<<std::endl;
      b.next = index.positions[indexValue];
      std::cout<<"b.next : "<<b.next<<std::endl;
      Bucket oldRoot;
      if(b.next > -1)
        oldRoot = loadBucket(b.next);
      loadFreeList();
      if(freeList < 0)
        index.positions[indexValue] = appendBucket(b);
      else
        index.positions[indexValue] = insertInFreeList(b);
      updateIndex(index);
      if(b.next > -1){
        oldRoot.prev = index.positions[indexValue];
        updateBucket(b.next, oldRoot);
      }
    }  
//" appendBucket {{{1

long Hash::appendBucket(Bucket& b){
  std::fstream file;
  file.open(fileName, std::fstream::app | std::fstream::binary);
  file.seekp(0, std::ios::end);
  long pos = file.tellp();
  file.write((char*)&b, sizeof(b));
  file.close();
  std::cout<<"se apendeo el bucket en la posicion: "<<pos<<std::endl;
  return pos;
}
  
//" insertInFreeList(Bucket& b) {{{1 
long Hash::insertInFreeList(Bucket& b){
  loadFreeList();
  Bucket bu = loadBucket(freeList);
  updateBucket(freeList, b);
  long pos = freeList;
  updateFreeList(bu.freeNext);
  return pos;
} 
//" eliminateRegisterFromBucket(Bucket& b, int position) {{{1
void Hash::eliminateRegisterFromBucket(Bucket& b, int position){
  b.registros[position] = b.registros[--b.size];
}  
//"  rangeSearch(long begin, long end) {{{1 
   std::vector<Registro> Hash::rangeSearch(long begin , long end){
      std::vector<Registro> result;
      std::cout<<"comienzo del range search" <<std::endl;
      Index temp = loadIndex();
      for(int i = 0 ; i < (long)pow(2,D); i++){
        //traverso por todas las soluciones del hash que tengan asignados una posicion valida
        if(temp.positions[i] >= 0){
          std::cout<<"cargando bucket: "<<i<<" con posicion: "<<temp.positions[i]<<std::endl;
          // cargo el bucket cabeza y lo leo
          Bucket bTemp = loadBucket(temp.positions[i]);
          std::cout<<"hash cargado con exito "<<bTemp.size<<std::endl;

          for(int j = 0 ; j < bTemp.size; j++){
            // si el registro esta entre el begin y el end pues lo agrego al resultado
            std::cout<<"comprobando registro: "<<j <<" con key: "<<bTemp.registros[j].primary_key<<std::endl;
            if(bTemp.registros[j].primary_key >=  begin && bTemp.registros[j].primary_key <= end ){
              std::cout<<"registro aceptado"<<std::endl;
              result.push_back(bTemp.registros[j]);
            }
          }
          // si hay un bucket de overflow a leer la lista
          if(bTemp.next != -1){
            while(bTemp.next >= -1){
              //lo mismo de arriba
              std::cout<<"se cargará el bucket en posición: "<<bTemp.next<<std::endl;
              bTemp = loadBucket(bTemp.next);
              std::cout<<"El bucket se ve: "<<std::endl;
              printBucket(bTemp);
              for(int j = 0; j < bTemp.size; j++)
                if(bTemp.registros[j].primary_key >=  begin && bTemp.registros[j].primary_key <= end )
                  result.push_back(bTemp.registros[j]);
              if(bTemp.next  == -1 )
                break;
            }
          }
        }
        else{
          // si no tiene una posicion valida pues sigues con el siguiente
          continue;
        }
      }
      std::cout<<"casi finalizado"<<std::endl;
      // ordenas el vector de resultados
      std::sort(result.begin(), result.end(), compareKeys);
      return result;
    }


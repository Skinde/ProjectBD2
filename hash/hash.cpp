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
    // crea el archivo main
    if(freeList == -2){
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
  file.write((char*) &b, sizeof(b));
  file.close();
}
//" getHash(long& key) {{{1
long Hash::getHash(long& key){
  long hashed = key % (long) pow(2, D);
  Index index = loadIndex();
  int cuantosLeer=1;
  for(int i = 1; i <= D;i++){
    if(index.profundidad[readNBits(i,hashed)] > 0)
      cuantosLeer = i;
  }
  return readNBits(cuantosLeer,hashed);
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
      //si no tiene bucket
      if(positionOfBucket == -1){
        // lo creas e insertas el bucket
        Bucket nuevoBucket;
        nuevoBucket.registros[0] = nuevo;
        nuevoBucket.size++;
        insertNewBucket(nuevoBucket);
        
      }
      else{
        //si hay un bucket lo cargas
        Bucket current = loadBucket(positionOfBucket);
        int indexValue = getHash(current.registros[0].primary_key);
        Bucket nuevoBucket;

        //si el bucket esta lleno
        if(current.size == M){
          // si ya esta en la profundidad maxima solo queda hacer colisiones
          if(index.profundidad[indexValue] == D){ 
            nuevoBucket.registros[0] = nuevo;
            nuevoBucket.size++;
            insertNewBucket(nuevoBucket);
          }

          else{

            index.profundidad[indexValue]++;
            long pos = positionOfBucket;
            int nuevoIndexValue = indexValue + (int) pow(2,index.profundidad[indexValue]-1);
            index.profundidad[nuevoIndexValue] = index.profundidad[indexValue];
            
            updateIndex(index);

            partitionateBucket(pos,current, nuevoBucket, indexValue, nuevoIndexValue);

            if(nuevoBucket.size > 0){
              insertNewBucket(nuevoBucket);
              insertion(nuevo);
              updateBucket(pos, current);
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
          fillingBucket(positionOfBucket, current, nuevo);
        }        
      }
 }  
//" partitionateBucket {{{1
    void Hash::partitionateBucket(long& positionOfFull, Bucket& full, Bucket& empty, int indexFull, int indexEmpty){
      for(int i = 0; i < full.size;i++){
        while(getHash(full.registros[i].primary_key) == indexEmpty){
          if(empty.size == M){
          insertNewBucket(empty);
          Bucket temp;
          empty = temp;
          }
          empty.registros[empty.size++] =full.registros[i];
          eliminateRegisterFromBucket(full, i);
        }
      }
      if(full.size == 0){
        loadFreeList();
        full.freeNext = freeList;
        updateFreeList(positionOfFull);
        updateBucket(positionOfFull, full);

        if(full.prev == -1 && full.next == -1){
          Index index = loadIndex();
          index.profundidad[indexFull] = (indexFull == 0 || indexFull == 1)? 1:0;
          index.positions[indexFull] = -1;
          updateIndex(index);
        }
        else if(full.prev == -1){
          Index index = loadIndex();
          Bucket next = loadBucket(full.next);
          next.prev = -1;
          index.positions[indexFull] = full.next;
          updateIndex(index);
          updateBucket(full.next, next);
        }
        else if(full.next == -1){
          Bucket prev = loadBucket(full.prev);
          prev.next = -1;
          updateBucket(full.prev, prev);
        }
        else{
          Bucket prev = loadBucket(full.prev), next = loadBucket(full.next);
          next.prev = full.prev;
          prev.next = full.next;
          updateBucket(full.next, next);
          updateBucket(full.prev, prev);
        }
      }
      else if(full.size < M && full.size > 0){
        Index index = loadIndex();
        if(positionOfFull == index.positions[indexFull]){
          updateBucket(positionOfFull, full);
        }
        else{
          Bucket root = loadBucket(index.positions[indexFull]);
          for(int i = root.size-1; i > -1; i--){
            if(full.size < M){
              full.registros[full.size++] = root.registros[--root.size];
            }
          }
          if(full.size < M){
            if(root.next == positionOfFull){
              full.prev = -1;
              updateBucket(positionOfFull, full);
              loadFreeList();
              root.freeNext = freeList;
              updateFreeList(index.positions[indexFull]);
              index.positions[indexFull] = positionOfFull;
              updateIndex(index);
            }
            else{
              Bucket temp = loadBucket(root.next);
              temp.prev = -1;
              loadFreeList();
              root.freeNext = freeList;
              updateFreeList(index.positions[indexFull]);
              index.positions[indexFull] = root.next;
              updateIndex(index);
              for(int i = temp.size-1; i > -1 ; i--){
                if(full.size < M)
                  full.registros[full.size++] =temp.registros[--temp.size];
              }
              updateBucket(root.next, temp);
              updateBucket(freeList, root);
            }
          }
        }
      }
      if(full.next != -1){
        long pos;
        full = loadBucket(pos = full.next);
        partitionateBucket(pos, full, empty, indexFull, indexEmpty);
        
      }
    }

//" insertNewBucket {{{1
    void Hash::insertNewBucket(Bucket& b){
      Index index = loadIndex();
      int indexValue = getHash(b.registros[0].primary_key);
      b.next = index.positions[indexValue];
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
//" eliminate RegisterFromBucket(Bucket& b, int position) {{{1
void Hash::eliminateRegisterFromBucket(Bucket& b, int position){
  b.registros[position] = b.registros[--b.size];
}    
//"  rangeSearch(long begin, long end) {{{1o 
   std::vector<Registro> Hash::rangeSearch(long begin , long end){
      std::vector<Registro> result;
      Index temp = loadIndex();
      for(int i = 0 ; i < (long)pow(2,D); i++){
        //traverso por todas las soluciones del hash que tengan asignados una posicion valida
        if(temp.positions[i] >= 0){
          // cargo el bucket cabeza y lo leo
          Bucket bTemp = loadBucket(temp.positions[i]);

          for(int j = 0 ; j < bTemp.size; j++){
            // si el registro esta entre el begin y el end pues lo agrego al resultado
            if(bTemp.registros[j].primary_key >=  begin && bTemp.registros[j].primary_key <= end ){
              result.push_back(bTemp.registros[j]);
            }
          }
          // si hay un bucket de overflow a leer la lista
          if(bTemp.next != -1){
            while(bTemp.next >= -1){
              //lo mismo de arriba
              bTemp = loadBucket(bTemp.next);
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
      // ordenas el vector de resultados
      std::sort(result.begin(), result.end(), compareKeys);
      return result;
    }
//" fillingBucket(long& position, Bucket& bucket , Registro& reg){{{1
void Hash::fillingBucket(long& position, Bucket& bucket , Registro& reg){
  bucket.registros[bucket.size] = reg;
  bucket.size++;
  updateBucket(position, bucket);
}


//" SearchBucketInList(Bucket& bucket, long& key){{{1
Bucket Hash::SearchBucketInList(Bucket& bucket, long& key){
      while (bucket.next != -1)
      {
        Registro reg = linearSearchInBucket(bucket, key);
        if (reg.primary_key != -1)
        {
          return bucket;
        }
        bucket = loadBucket(bucket.next);
      }
      return bucket;
} 

//" SearchPositionInBucket(Bucket& bucket, long& key) {{{1
long SearchPositionInBucket(Bucket& bucket, long& key)
    {
      for (int i = 0; i < bucket.size; i++)
      {
        if (bucket.registros[i].primary_key == key)
        {
          return (i);
        }
      }
      return -1;
    }

// seekAndDestroyByMetallica(long& positionOfBucket, Bucket& bucket, long& key) {{{1
long Hash::seekAndDestroyByMetallica(long& positionOfBucket, Bucket& bucket, long& key){
  //std::cout<<"seek and destroy "<<positionOfBucket<<std::endl;
  for(int i = 0; i < bucket.size; i++){
    //std::cout<<bucket.registros[i].primary_key <<" = "<<key <<std::endl;
    if(bucket.registros[i].primary_key == key){
      eliminateRegisterFromBucket(bucket, i);
      return positionOfBucket;
    }
  }
  if(bucket.next == -1){
    return 0;
  }
  else{
    bucket = loadBucket(positionOfBucket = bucket.next);
    return seekAndDestroyByMetallica(positionOfBucket ,bucket, key);
  }
  
  return 0;
}

// del eteThis(long key) {{{1 
bool Hash::deleteThis(long key){
    int hashedKey = getHash(key);
    Index index = loadIndex();
    long positionOfBucket;
    Bucket current = loadBucket(positionOfBucket = index.positions[hashedKey]);

    if(seekAndDestroyByMetallica(positionOfBucket, current, key)){
      //std::cout<<"Encontrado y destruido"<<std::endl;
      /*if(current.size == 0){
        loadFreeList();
        current.freeNext = freeList;
        updateFreeList(positionOfBucket);
        updateBucket(positionOfBucket, current);

        if(current.next ==-1 && current.prev == -1){
          index.positions[hashedKey] = -1;
          index.profundidad[hashedKey] = 0;
          updateIndex(index);
          return true;
        }
        else if(current.next227552 == -1){
          Bucket prev = loadBucket(current.prev);
          prev.next = -1;
          updateBucket(current.prev, prev);
          return true;
        }
        else if(current.prev == -1){
          index.positions[hashedKey] = current.next;
          Bucket next = loadBucket(current.next);
          next.prev = -1;
          updateIndex(index);
          updateBucket(current.next, next);
          return true;
        }

      Bucket prev, next;
      prev = loadBucket(current.prev);
      next = loadBucket(current.next);
      prev.next = current.next;
      next.prev = current.prev;
      updateBucket(current.prev, prev);
      updateBucket(current.next, next);
      return true;
     } 
     else{*/
       if(index.positions[hashedKey] == positionOfBucket){
        if(current.size == 0){
          index.positions[hashedKey] = current.next;
          Bucket next = loadBucket(current.next);
          next.prev = -1;
          updateIndex(index);
          updateBucket(current.next, next);
          return true;
        }

        updateBucket(positionOfBucket, current);
        return true;
       }
       long rootPos;
        Bucket root = loadBucket(rootPos=index.positions[hashedKey]);
        current.registros[current.size++] = root.registros[--root.size];
        if(root.size == 0){
          index.positions[hashedKey] = root.next;
          Bucket rootNext = loadBucket(root.next);
          rootNext.prev = -1;
          updateIndex(index);
          updateBucket(root.next, rootNext);
        }
        updateBucket(rootPos, root);
        updateBucket(positionOfBucket, current);
        return true;
      //}
    }
    else
      return false;
    return false;
}

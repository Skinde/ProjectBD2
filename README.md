# Projecto 1 Base De Datos 2

## Introducción

Este proyecto tiene el objetivo de crear una base de datos que almacene información sobre juegos de Steam obtenidos de https://steamdb.info/. Se espera poder almacenar y obtener los datos de una forma eficiente en un lenguaje estructurado.

## Estructuras

En este proyecto utilizamos la estructura Hash el cual hemos modificado añadiendo un Freelist para un manejo de espacios vacios mas eficientes. 
A continucación nuestra estructura:

```cpp
struct Registro {
    long primary_key;
    long avg_players;
    long gain;
    long peak_players;
    int year;
    char URL [35]; 
    char game_name [43];
};
```
*dentro de lib.h*

La estructura Registro almacena la información sobre cada juego en steam. Los registros se encuentran dentro de la estructura bucket

```cpp
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
```
*dentro de hash.h*

Un Page es un bloque continuo de memoria virtual manejado por el sistema operativo. dado que el sistema operativo mueve la información en pages es mas eficiente mover la información en Pages para no tener que realizar busquedas inecesarias de información. Para esto esta la estructura Bucket que tiene un tamaño equivalente (en Bytes) al PAGE SIZE del sistema operativo. Hemos definido el PAGE SIZE como 4096 dado que los integrantes del grupo tenemos multiples sistemas operativos, pero todos comparten la propiedad de tener 4096 Bytes dedicados al page size.

Cada bucket tiene un next y prev que mantiene la posición fisica del siguiente y anterior bucket respectivanente que comparte el mismo producto del hash.

Si el bucket a sido eliminado el valor de freeNext mantendra la posición logica del siguiente bucket que a sido eliminado.

Los buckets son obtenidos por una función hash la cual se almacena en dos partes

```cpp
struct Index{
    bool created = false;
    int profundidad[(long) pow(2,D)] = {0};
    long positions[(long) pow(2,D)] = {0};
};
```

El index es la ultima estructura que es almacenada en el disco, se trata de dos matrices unidimensionales.

* Profunidad: Almacena la cantidad maxima de buckets que pueden estar vinculados por cada resultado de la función hash
* Position: Almacena la posición fisica de los buckets de la función hash

Por ultimo está la estructura hash la cual no se almacena en el disco.
```cpp
class Hash{

    public:
        void a(){
            long a = 3;
            updateFreeList(a);
        }
        Hash(std::string indexName, std::string fileName);
        Registro search(long key);
        std::vector<Registro> rangeSearch(long begin, long end);
        void insertion(Registro nuevo);
        void deleteThis(long key);
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
        long appendBucket(Bucket& b);
        void insertNewBucket(Bucket& b);
        void partitionateBucket(Index& index,Bucket& full, Bucket& empty, int indexFull, int indexEmpty);
        
        void eliminateRegisterFromBucket(Bucket& b, int position);

        long getHash(long& key);
        long readNBits(int N, long& number);
        Registro linearSearchInBucket(Bucket& bucket, long& key);
};
```

Se utiliza de manera exclusiva para almacenar los algoritmos y variables compartidas por estos.
## Algoritmos

A continuación una descripción breve de los algoritmos utilizados para el manejo de información:

### Inserción

```
Crea un nuevo bucket.
Si no hay ningun Bucket almacenando el resultado de la función hash
    Si existe algun espacio vacio (apuntado por la Free list)
        Sobrescribe dicho espacio con el nuevo bucket
    Si no
        Escribir el nuevo bucket al final del archivo
Si ya existe un Bucket almacenando el resultado de la función
    Si existe un espacio vacio (apuntado por la Free list)
        Sobrescribe dicho espacio con el nuevo bucket
        Apuntar el next del bucket anterior al nuevo bucket
    Si no
        Escribir el nuevo bucket al final del archivo
        Apuntar el next del bucket anterior al nuevo bucket
```
    




    
    




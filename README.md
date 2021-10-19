# Projecto 1 Base De Datos 2

## Introducción

Este proyecto tiene el objetivo de crear una base de datos que almacene información sobre juegos de Steam obtenidos de https://steamdb.info/. Se espera poder almacenar y obtener los datos de una forma eficiente en un lenguaje estructurado. Para esto utilizaremos dos estructuras: Hash y Sequential para guardar los datos.
Los datos son alojados en una URL temporal. Se persiste información útil para el negocio, como el número de jugadores activos o las ganancias acumuladas de un año específico. Para este proyecto, alojamos en un archivo binario estos datos, adicionando en el proceso un nuevo campo a modo de primary_key. Existen tres versiones de los datos, una con 1k de registros, otra con 5k y la más grande con casi 10k de registros.

## Estructuras

### Hash

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

### Sequential

El registro tien una estructura identica a la del hash los cambios son en dos estructuras nuevas

```cpp
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
```
Esta implementación de SequentialFile almacena los punteros next y nextDel como parte de su metadata, lo que significa que el usuario no tiene que molestarse en adicionar más campos de los ya establecidos en la base de datos original. Así mismo, el archivo original es el único que se modifica, ya que recordemos que no usamos indíces en esta estructura. El usuario recibe como salida el archivo ordenado en base al id, el cual es idéntico en estructura a la base de datos origina y a nuestro Hash.

```cpp
namespace SeqFile {
    // Lista de constantes
    const int RAW_RECORD_SIZE = 114;
    const long END_OF_LIST = -1;
    const long NO_DELETED = -2;
    const long ID_OFFSET = 2 * sizeof(long);
    const long NEXTDEL_OFFSET = sizeof(long);
    const long METADATA = 3 * sizeof(int) + sizeof(long);
    const long SIZE_OFFSET = sizeof(int);
    const long HEAP_OFFSET = 2 * sizeof(int);
    const long MIN_OFFSET = 3 * sizeof(int);

    stringstream fixedRead(ifstream& file, size_t size){
        char buffer [size];
        file.read((char*) &buffer, size);
        stringstream ss(buffer);
        return ss;
    }
```
Podemos ver las constantes utilizadas para la implementación. Dado que todas las variables necesarias para el mantemiento de la estructura de datos estan en memoria secundaria a modo de encabezado, debemos sumar este espacio cada vez que busquemos en el archivo (const long METADATA). Este encabezado tiene la siguiente estructura (en binario):
FREE_LIST_HEADER | SIZE | HEAP_SIZE | MIN_INDEX.
Size y heap size son los tamaños del main file y el heap respectivamente. Min index nos da la posición física del menor elemento, la cual es bastante importante para ciertos casos en las operaciones. También manejamos una free list cuyo encabezado esta en la posición cero.

## Algoritmos

A continuación una descripción breve de los algoritmos utilizados para el manejo de información:

### Hash

#### Inserción

```
Crea un nuevo bucket.
Si no hay ningun Bucket almacenando el resultado de la función hash
    Si existe algun espacio vacio (apuntado por la Free list)
        Actualizar el index con la posición fisica del nuevo bucket
        Actualizar el freelist con el freeNext del bucket que vas a sobrescribir
        Sobrescribe dicho espacio con el nuevo bucket
    Si no
        Escribir el nuevo bucket al final del archivo
        Actualizar el index
Si ya existe un Bucket almacenando el resultado de la función
    Si existe un espacio vacio (apuntado por la Free list)
        Actualizar el freelist con el freeNext del bucket que vas a sobreescribir
        Sobrescribe dicho espacio con el nuevo bucket
    Si no
        Escribir el nuevo bucket al final del archivo
    Si el tamaño de la cadena de buckets no es de tamaño D (3)
        Apuntar el next del bucket anterior al nuevo bucket
    Si el tamaño de la cadena de buckets es de tamaño D (3)
        Actualizar el index con la posición fisica del nuevo bucket (Crear una nueva cadena de buckets)
```

#### Eliminación
```
Busca el bucket que contiene el registro
elimina el registro
Mover el registro del primer bucket al bucket actual
Si el primer bucket queda vacio
    Si la cadena de buckets no tiene mas buckets
        Actualizar el index a -1
    Si no
        Actualizar el index a next
        Actualizar el prev del next
    Actualizar el freelist
```

#### Busqueda
```
Obtener el resultado de la función hash
Buscar la cadena de buckets con el mayor numero de bits
Buscar el registro dentro de cada bucket en la cadena
Si la cadena no está en el bucket seguir a la siguiente cadena
```

### Busqueda de rango
```
Tomar todos los elementos en el rango
realizar la busqueda de cada elemento
```

### Complejidad de los algoritmos del Hash

| Algoritmo | Complejidad |
|   :---:   |    :---:    |
| Inserción |     O(1)    |
|Eliminación|     O(D)    |
| Busqueda  |     O(D)    |
| Busqueda por rango | O(D * M) |


### Sequential


#### Inserción

```
Buscar el lugar correcto del nuevo registro
Añadir al aux
Si el aux esta lleno
    Reordenar
Cambiar el next del elemento anterior
Cambiar el next al siguiente elemento
```

#### Eliminación


#### Busqueda

```
Empieza en la mitad del registro
tomar un rango de busqueda "m" = mitad del tamaño del array
Si el registro actual es
Si la key del elemento es de menor tamaño
    Dividir la posicion del elemento entre m
Si la key del elemento es de mayor tamaño
    Multiplicar la posicion del elemento por m
Divide m/2
Si m = 1
    Realizar una busqueda lineal en el aux
```

### Complejidad de los algoritmos del Sequential

| Algoritmo |     Complejidad                                   | 
|   :---:   |        :---:                                      |
| Inserción |     O(Dlogn + k) ; si hay reordenamiento: O(MD)   |
|Eliminación|     O(MD)                                         |
| Busqueda  |     O(Dlogn + k)                                  |

# Compilación y uso

Hash: 
```
g++ main.cpp hash.cpp
```

Sequential:
```
g++ main.cpp 
```

# Resultados de tests:

Para el SequentialFile se implemento un set de pruebas que puede encontrar en SeqFile/test_seqfile.cpp Por favor, luego de correr los tests deberá reemplazar los archivos ubicados en la carpeta Data con los de la carpeta saves. Por ejemplo, si usted corre el test utilizando Data/5k.dat debera: ir a la carpeta Data, borrar el archivo 5k.dat, ir a la carpeta Data/saves, copiar el archivo 5k.dat y pegarlo en la carpeta Data. Esto es así ya que el tester realiza adds y deletes sobre el archivo, lo que podría llevar a falsos errores luego de la primera compilación.

![Resultados del test en Sequential](Resultados.png)

    




    
    




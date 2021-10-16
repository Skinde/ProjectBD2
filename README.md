# Projecto 1 Base De Datos 2

## Introducción

Este proyecto tiene el objetivo de crear una base de datos que almacene información sobre juegos de Steam obtenidos de https://steamdb.info/. Se espera poder almacenar y obtener los datos de una forma eficiente en un lenguaje estructurado.

## Técnicas

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




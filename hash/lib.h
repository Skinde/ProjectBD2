#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <bitset>
#include <algorithm>

struct Registro {
    long primary_key;
    long avg_players;
    long gain;
    long peak_players;
    int year;
    char URL [35]; //Expiran con el tiempo, asi que algunas no son validas xd
    char game_name [43];
};


#endif

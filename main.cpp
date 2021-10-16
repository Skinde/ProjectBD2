#include "hash.h"
#include <unistd.h> 
void printRegistro(Registro& r){
    std::cout<<r.primary_key<<" "<<r.avg_players<<" "<<r.gain<<" "<<r.peak_players<<" "<<r.year<<" "<<r.URL<<" "<<r.game_name<<std::endl;
}

int main(){
    Hash hashito("index.dat","file.dat");
    std::ifstream read("dbs/9_9k.dat", std::ios::binary | std::ios::in);
    Registro read_reg;
    int i {};
    while (read.read((char*) &read_reg, 114)) {
        std::cout << "(" << ++i << "): " << read_reg.primary_key << ", " << read_reg.avg_players << ", ";
        std::cout << read_reg.gain << ", " << read_reg.peak_players << ", ";
        std::cout << read_reg.year << ", " << read_reg.URL << ", " << read_reg.game_name << std::endl;
        hashito.insertion(read_reg);
    }
    read.close();
    hashito.printIndex();
    
    return 0;
}

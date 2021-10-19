#include "hash.h"
void printRegistro(Registro& r){
    std::cout<<r.primary_key<<" "<<r.avg_players<<" "<<r.gain<<" "<<r.peak_players<<" "<<r.year<<" "<<r.URL<<" "<<r.game_name<<std::endl;
}

int main(){
    Hash hashito("index.dat","file.dat");
    std::ifstream read("dbs/1k.dat", std::ios::binary | std::ios::in);
    Registro read_reg;
    //int i {};
    while (read.read((char*) &read_reg, 114)) hashito.insertion(read_reg);
        /*std::cout << "(" << ++i << "): " << read_reg.primary_key << ", " << read_reg.avg_players << ", ";
        std::cout << read_reg.gain << ", " << read_reg.peak_players << ", ";
        std::cout << read_reg.year << ", " << read_reg.URL << ", " << read_reg.game_name << std::endl;*/
        //hashito.insertion(read_reg);
    read.close();
    hashito.printIndex();
    std::vector<Registro> registros= hashito.rangeSearch(0,100000000);

    for(auto& i: registros)   printRegistro(i);
    std::cout<<"--------------------------------------------------------------"<<std::endl;
    for(long i = 1; i <=1012;i++ ) hashito.deleteThis(i);
    //hashito.showAllRootBuckets();
    registros= hashito.rangeSearch(0,100000000);
    //hashito.showBucket(8);
    for(auto& i: registros)   printRegistro(i);
    return 0;
}

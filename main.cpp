#include "SeqFile/test_seqfile.cpp"
using namespace SeqFile;

int main() {
    seqFileTest();
    /*
    ifstream read("Data/1k.dat", ios::binary | ios::in);
    Record read_reg;
    int i {};
    while (read.read((char*) &read_reg, 114)) {
        cout << "(" << i++ << "): " << read_reg.id << ", " << read_reg.avg_players << ", ";
        cout << read_reg.gain << ", " << read_reg.peak_players << ", ";
        cout << read_reg.year << ", " << read_reg.URL << ", " << read_reg.game_name << endl;
    }
    read.close();
    /*
    ifstream read("Data/1k.dat", ios::binary | ios::in);
    SeqRecord read_reg;
    int i {};
    read.seekg(METADATA);
    while (read.read((char*) &read_reg, 130)) {
        cout << "(" << i++ << "): " << read_reg.next << ", " << read_reg.nextDel << ", "; 
        cout << read_reg.id << ", " << read_reg.avg_players << ", ";
        cout << read_reg.gain << ", " << read_reg.peak_players << ", ";
        cout << read_reg.year << ", " << read_reg.URL << ", " << read_reg.game_name << endl;
        if (i==10) break;
    }
    read.close();*/
    return 0;
}
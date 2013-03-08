#ifndef TRAME_H
#define TRAME_H

#include <string>
#include "common.h"

using namespace std;

#define MAX_USERNAME_SIZE 100
#define MAX_DATA_SIZE 1000
//#define MAX_TRAME_SIZE 4*sizeof(int) + MAX_DATA_SIZE + MAX_USERNAME_SIZE
#define MAX_TRAME_SIZE 1132

class Trame {

public:
    #pragma GCC diagnostic ignored "-fpermissive"
    Trame(string fromName, TrameType type, int size=0, int numTrame=1, int nbTrame=1, string data="");
    ~Trame();

    string getFrom() const;
    TrameType getType() const;
    int getSize() const;
    int getNumTrame() const;
    int getNbTrame() const;
    string getData() const;

private:
    string fromName_;
    TrameType type_;
    int size_;
    int numTrame_;
    int nbTrame_;
    string data_;

};

#endif // TRAME_H

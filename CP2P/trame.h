#ifndef TRAME_H
#define TRAME_H

#include <string>
#include "common.h"

using namespace std;

typedef struct SerializableTrame {
    TrameType typeTrame;
    char nameSrc[MAX_USERNAME_SIZE];
    int numTrame;
    int nbTrames;
    int taille;
    char data[MAX_DATA_SIZE];
} SerializableTrame;

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
    SerializableTrame* getSerializableTrame();
    void extractNameIP(string& name, string& ip);

private:
    string fromName_;
    TrameType type_;
    int size_;
    int numTrame_;
    int nbTrame_;
    string data_;
    SerializableTrame* serializableTrame;

};

#endif // TRAME_H

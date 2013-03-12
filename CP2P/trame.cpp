#include "trame.h"
#include <iostream>
#include <stdexcept>
#include <string.h>

Trame::Trame(string fromName, TrameType type, int size, int numTrame, int nbTrame, string data) {
    fromName_ = fromName;
    type_ = type;
    size_ = size;
    numTrame_ = numTrame;
    nbTrame_ = nbTrame;
    data_ = data;
    serializableTrame = new SerializableTrame;
    for(int i = 0; i < size; i++) {
        serializableTrame->data[i] = data.data()[i];
    }

    serializableTrame->typeTrame = type;
    strcpy(serializableTrame->nameSrc,fromName.c_str());
    serializableTrame->numTrame = numTrame;
    serializableTrame->nbTrames = nbTrame;
    serializableTrame->taille = size;
}

Trame::~Trame() {
    delete serializableTrame;
}

string Trame::getFrom() const {
    return fromName_;
}

TrameType Trame::getType() const {
    return type_;
}

int Trame::getSize() const {
    return size_;
}

int Trame::getNumTrame() const {
    return numTrame_;
}

int Trame::getNbTrame() const {
    return nbTrame_;
}

string Trame::getData() const {
    return data_;
}

SerializableTrame* Trame::getSerializableTrame() {
    return serializableTrame;
}

void Trame::extractNameIP(string &name, string &ip) {
    size_t splitPos = data_.find(':');
    if(splitPos == string::npos) {
        throw runtime_error("Trame data does not contain ':'");
    }
    name = data_.substr(0,splitPos);
    ip = data_.substr(splitPos+1,data_.size());
}


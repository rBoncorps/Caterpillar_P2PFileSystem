#include "trame.h"

Trame::Trame(string fromName, TrameType type, int size, int numTrame, int nbTrame, string data) {
    fromName_ = fromName;
    type_ = type;
    size_ = size;
    numTrame_ = numTrame;
    nbTrame_ = nbTrame;
    data_ = data;
}

Trame::~Trame() {

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


#ifndef TPARAM_H
#define TPARAM_H

#include "savable.h"

#include <complex>

#include "Eigen/Dense"

using Type = std::complex<double>;

class Parameters : public Savable {
public:
    Parameters(Type m11, Type m12, Type m21, Type m22);
    Parameters(int num_ports);
    Parameters() : Parameters(2){}

    Eigen::MatrixXcd data;

    unsigned int ports() const { return data.cols();}

    // Access to elements is usually off-by-one (mostly 1-based indexing in literature but Eigen uses 0-based indexing)
    Type get(unsigned int row, unsigned int col) const {return data(row-1, col-1);}
    void set(unsigned int row, unsigned int col, Type t) { data(row-1, col-1) = t;}

    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
};

// forward declaration of parameter classes
class Sparam;
class Tparam;
class ABCDparam;

class Sparam : public Parameters {
public:
    using Parameters::Parameters;
    Sparam(const Tparam &t);
    Sparam(const ABCDparam &a, Type Z01, Type Z02);
    Sparam(const ABCDparam &a, Type Z0);
    Sparam operator+(const Sparam &r) const {
        Sparam p(ports());
        p.data = data+r.data;
        return p;
    }
    Sparam operator*(const Type &r) const {
        Sparam p(ports());
        p.data = data * r;
        return p;
    }
    void swapPorts(unsigned int p1, unsigned int p2);
};

class ABCDparam : public Parameters {
public:
    using Parameters::Parameters;
    ABCDparam() : Parameters(2){}
    ABCDparam(const Sparam &s, Type Z01, Type Z02);
    ABCDparam(const Sparam &s, Type Z0);
    ABCDparam operator*(const ABCDparam &r) {
        ABCDparam p;
        // ABCD parameters can be multiplied by matrix multiplication
        p.data = data * r.data;
        return p;
    }
    ABCDparam inverse() {
        ABCDparam i;
        // by hand, this is faster because the Eigen matrix is using dynamic size
        Type det = data(0,0)*data(1,1) - data(0,1)*data(2,1);
        i.data(0,0) = data(1,1) / det;
        i.data(0,1) = -data(0,1) / det;
        i.data(1,0) = -data(1,0) / det;
        i.data(1,1) = data(0,0) / det;
        return i;
    }
    ABCDparam operator*(const Type &r) const {
        ABCDparam p;
        p.data = data * r;
        return p;
    }
    ABCDparam root() {
        // calculate root of 2x2 matrix, according to https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix (choose positive roots)
        auto tau = data(0,0) + data(1,1);
        auto sigma = data(0,0)*data(1,1) - data(0,1)*data(1,0);
        auto s = sqrt(sigma);
        auto t = sqrt(tau + 2.0*s);
        ABCDparam r = *this;
        r.data(0,0) += s;
        r.data(1,1) += s;
        r = r * (1.0/t);
        return r;
    }
};

class Tparam : public Parameters {
public:
    using Parameters::Parameters;
    Tparam(const Sparam &s);
    Tparam operator*(const Tparam &r) {
        Tparam p;
        // T parameters can be multiplied by matrix multiplication
        p.data = data * r.data;
        return p;
    }
    Tparam operator+(const Tparam &r) {
        Tparam p;
        p.data = data + r.data;
        return p;
    }
    Tparam inverse() {
        Tparam i;
        Type det = data(0,0)*data(1,1) - data(0,1)*data(2,1);
        i.data(0,0) = data(1,1) / det;
        i.data(0,1) = -data(0,1) / det;
        i.data(1,0) = -data(1,0) / det;
        i.data(1,1) = data(0,0) / det;
        return i;
    }
    Tparam operator*(const Type &r) {
        Tparam p;
        p.data = data * r;
        return p;
    }
    Tparam operator*(const double &r) {
        Tparam p;
        p.data = data * r;
        return p;
    }
    Tparam root() {
        // calculate root of 2x2 matrix, according to https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix (choose positive roots)
        auto tau = data(0,0) + data(1,1);
        auto sigma = data(0,0)*data(1,1) - data(0,1)*data(1,0);
        auto s = sqrt(sigma);
        auto t = sqrt(tau + 2.0*s);
        Tparam r = *this;
        r.data(0,0) += s;
        r.data(1,1) += s;
        r = r * (1.0/t);
        return r;
    }
};

class Yparam : public Parameters {
public:
    using Parameters::Parameters;
    Yparam(const Sparam &s, Type Z01, Type Z02);
    Yparam(const Sparam &s, Type Z0);
};

#endif // TPARAM_H

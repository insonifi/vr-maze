#ifndef BVEC_HPP
#define BVEC_HPP

struct BVec {
    bool x;
    bool y;
    bool z;
    BVec(bool x, bool y, bool z): x(x), y(y), z(z) {}
    BVec(bool x): x(x), y(x), z(x) {}

    BVec operator &=(BVec a)
    {
        this->x &= a.x;
        this->y &= a.y;
        this->z &= a.z;

        return *this;
    }
    friend BVec operator & (BVec a, BVec b)
    {
        return a &= b;
    }
public:
    bool any() {
        return x || y || z;
    }
    bool all() {
        return x && y && z;
    }
};

#endif // BVEC_HPP

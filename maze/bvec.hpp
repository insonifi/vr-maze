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
        return BVec(
                    x & a.x
                    , y & a.y
                    , z & a.z
                    );
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

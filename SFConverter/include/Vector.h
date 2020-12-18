#pragma once

struct Point
{
    float x;
    float y;
    float z;

    Point toRightHandedYup() const
    {
        Point ret;
        ret.x = -this->x;
        ret.y = this->z;
        ret.z = this->y;
        return ret;
    }
};
struct Vector
{
    float x;
    float y;
    float z;

    Vector() : x(0), y(0), z(0) {}
    Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector(Point point) : x(point.x), y(point.y), z(point.z) {}

    float lenght(void) const { return sqrt(x*x + y * y + z * z); }
    float lenghtSQ(void) const { return (x*x + y * y + z * z); }
    void normalize(void) { float l = lenght(); x /= l; y /= l; z /= l; }


    Vector& operator += (const Vector& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector operator - (const Vector& v) const
    {
        Vector ret;
        ret.x = x - v.x;
        ret.y = y - v.y;
        ret.z = z - v.z;
        return ret;
    }

    Vector cross(const Vector& v) const
    {
        Vector ret;
        ret.x = y * v.z - z * v.y;
        ret.y = z * v.x - x * v.z;
        ret.z = x * v.y - y * v.x;
        return ret;
    }
};
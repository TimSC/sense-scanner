#ifndef VECTORS_H
#define VECTORS_H
#include <vector>
#include <iostream>

std::vector<float> SubVec(std::vector<float> &a, std::vector<float> &b)
{
    assert(a.size() == b.size());
    std::vector<float> out;
    for(unsigned i=0;i<a.size();i++)
        out.push_back(a[i] - b[i]);
    return out;
}

float MagVec(std::vector<float> &a)
{
    float total = 0.f;
    for(unsigned i=0;i<a.size();i++)
        total += a[i] * a[i];
    total = pow(total, 0.5f);
    return total;
}

std::vector<float> NormVec(std::vector<float> &a)
{
    float mag = MagVec(a);
    if(mag==0.f) mag = 1.f; //Prevent div by zero
    std::vector<float> out;
    for(unsigned i=0;i<a.size();i++)
        out.push_back(a[i] / mag);
    return out;
}

float DotVec(std::vector<float> &a, std::vector<float> &b)
{
    assert(a.size()==b.size());
    float total = 0.f;
    for(unsigned i=0;i<a.size();i++)
        total += a[i] * b[i];
    return total;
}

void PrintVec(std::vector<float> &a)
{
    for(unsigned i=0;i<a.size();i++)
    {
        std::cout << a[i];
        if(i<a.size()-1) std::cout << ",";
    }
    std::cout << std::endl;
}

#endif // VECTORS_H

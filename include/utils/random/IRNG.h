#pragma once

struct IRNG
{
    virtual ~IRNG() = default;
    virtual double uniform_real(double min, double max) = 0;
    virtual int uniform_int(int min, int max) = 0;
};

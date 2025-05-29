#pragma once
#include <iostream>
#include <vector>
#include <cmath>

typedef unsigned int count;

struct cross
{
    double time;
    count n;
    cross(double _time = -1, count _n = 1) : time(_time), n(_n) {};
};

struct flow
{
    std::vector<cross> crosses;
    count level;

    flow()
    {
        level = 0;
    }

    flow(const std::vector<double>& _crosses)
    {
        level = 0;
        append(_crosses);
    }

    flow(const std::vector<cross> _crosses, count _level = 0) : crosses(_crosses), level(_level) {};

    void append(const std::vector<double>& _crosses)
    {
        crosses.reserve(crosses.size() + _crosses.size());
        for (double time : _crosses)
        {
            crosses.emplace_back(time);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const flow& f)
    {
        if (f.level == 0)
        {
            os << "basic flow:\n";
            for (auto c : f.crosses)
            {
                os << c.time << ", ";
            }
        }
        else
        {
            os << "flow level " << f.level << '\n';
            for (auto c : f.crosses)
            {
                os << '(' << c.time << "; n = " << c.n << "), ";
            }
        }
        return os;
    }
};

#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>


#include "flow.H"

const static int max_pack = 1000;
const static double PI = 3.14159;
class flow_solver
{

    static double myErfInv2(double x) {
        double tt1, tt2, lnx, sgn;
        sgn = (x < 0) ? -1.0f : 1.0f;

        x = (1 - x) * (1 + x);        // x = 1 - x*x;
        lnx = logf(x);

        tt1 = 2 / (PI * 0.147) + 0.5f * lnx;
        tt2 = 1 / (0.147) * lnx;

        return(sgn * sqrtf(-tt1 + sqrtf(tt1 * tt1 - tt2)));
    }

    const double normal_cdf_inverse(double alpha) {
        // Check for valid input
        if (alpha < 0 || alpha > 1) {
            std::cerr << "Invalid input: alpha must be between 0 and 1" << std::endl;
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Calculate the value of x
        double x = sqrt(2) * myErfInv2(2 * alpha - 1);

        return x;
    }

    const std::vector<double> get_diff(const flow& f)
    {
        const std::vector<cross>& f_crosses = f.crosses;

        std::vector<double> diff(f_crosses.size());

        for (int i = 1; i < f_crosses.size(); ++i)
        {
            diff[i] = f_crosses[i].time - f_crosses[i - 1].time;
        }

        return diff;
    }

    const flow advanced_batches_compute_base(const flow& f, const double& C = 2)
    {
        flow result;

        std::vector<double> diff = get_diff(f);

        auto tmp_iterator = std::min_element(diff.begin() + 1, diff.end());

        const double h = C * (*tmp_iterator);

        std::cout << h << "\n\n";

        result = batches_compute_1(f, h);
        return result;
    }

public:

    flow batches_compute_1(const flow& f, const double h)
    {
        flow result;
        result.level = f.level + 1;

        std::vector<cross>& r_crosses = result.crosses;
        const std::vector<cross>& f_crosses = f.crosses;
        r_crosses.emplace_back(-1, f_crosses[0].n);

        for (int i = 1; i < f_crosses.size(); ++i)
        {
            if ((f_crosses[i].time - f_crosses[i - 1].time < h) && (r_crosses.back().n + f_crosses[i].n < max_pack))
            {
                r_crosses.back().n += f_crosses[i].n;
            }
            else
            {
                r_crosses.back().time = f_crosses[i - 1].time;
                r_crosses.emplace_back(-1, f_crosses[i].n);
            }
        }

        if (r_crosses.back().time == -1)
        {
            r_crosses.back().time = f_crosses.back().time;
        }

        return result;
    }

    const flow batches_compute_4(const flow& f, const double& C = 2)
    {
        flow result = advanced_batches_compute_base(f, C);
        result = advanced_batches_compute_base(result, C);
        return result;
    }

    const bool criterion_1(const flow& f, double alpha)
    {
        std::vector<double> diff = get_diff(f);
        std::vector<int> signs(f.crosses.size()); // 1 == +, 0 == 0, -1 == -
        std::vector<int> phases;
        int phases_count = 0;

        for (int i = 1; i < diff.size(); ++i)
        {
            double tmp = diff[i] - diff[i - 1];
            if (tmp > 0)
            {
                signs[i] = 1;
            }
            else if (tmp < 0)
            {
                signs[i] = -1;
            }
            else
            {
                signs[i] = 0;
            }
            if (signs[i] != signs[i - 1] && (signs[i] != 0))
            {
                phases_count++;
            }
        }

        phases_count -= 2;
        int n = diff.size();
        //double Z = std::abs((phases_count - (2 * n) / 3) * std::sqrt(90) / std::sqrt(16 * n - 29));
        double Z = (phases_count - (2 * n) / 3) * std::sqrt(90) / std::sqrt(16 * n - 29);

        double C = -normal_cdf_inverse(alpha);
        return Z < C;
    }

    const bool criterion_2(const flow& f, double alpha)
    {

        std::vector<double> diff = get_diff(f);

        std::vector<int> signs(f.crosses.size()); // 1 == +, 0 == 0, -1 == -
        std::vector<int> phases;
        int phases_count = 0;

        int longest_phase = 0;
        int current_phase = 0;

        for (int i = 1; i < diff.size(); ++i)
        {
            double tmp = diff[i] - diff[i - 1];
            if (tmp > 0)
            {
                signs[i] = 1;
            }
            else if (tmp < 0)
            {
                signs[i] = -1;
            }
            else
            {
                signs[i] = 0;
            }
            if (signs[i] != signs[i - 1] && (signs[i] != 0))
            {
                longest_phase = std::max(longest_phase, current_phase);
                current_phase = 0;
                phases_count++;
            }
            else
            {
                current_phase++;
            }
        }

        int n = diff.size();
        int C;
        if (n <= 26)    C = 5;
        else if (n <= 153)  C = 6;
        else if (n <= 1170) C = 7;
        else C = 8;

        return (phases_count > (2 * n - 1) / 3 - 1.96 * std::sqrt((16 * n - 29) / 90))
            && (longest_phase < C);
    }
};
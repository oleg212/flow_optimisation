
#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
// Отменяем макрос Complex перед включением Boost
#ifdef Complex
#undef Complex
#endif
#include <boost/math/distributions/normal.hpp> 

#include "flow.H"

const static int max_pack = 1000;
const static double PI = 3.14159;
class flow_solver
{
    const std::vector<double> get_diff(const flow& f)
    {
        const std::vector<cross>& f_crosses = f.crosses;

        std::vector<double> diff(f_crosses.size());

        for (size_t i = 1; i < f_crosses.size(); ++i)
        {
            diff[i] = f_crosses[i].time - f_crosses[i - 1].time;
        }

        return diff;
    }

    const flow advanced_batches_compute_base(const flow& f, const double& C = 1.5)
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
        r_crosses.emplace_back(f_crosses[0].time, f_crosses[0].n);

        for (size_t i = 1; i < f_crosses.size(); ++i)
        {
            if ((f_crosses[i].time - f_crosses[i - 1].time < h) && (r_crosses.back().n + f_crosses[i].n < max_pack))
            {
                r_crosses.back().n += f_crosses[i].n;
            }
            else
            {
                r_crosses.emplace_back(f_crosses[i].time, f_crosses[i].n);
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
        size_t n = f.size();
        std::vector<double> diff = get_diff(f);
        std::vector<int> signs(n-1);
        int phases_count = 1;

        for (size_t i = 0; i < n-1; ++i)
        {
            if (diff[i+1] > diff[i])
                signs[i] = 1;
            else
                signs[i] = -1;
        }

        for (size_t i = 0; i < n-1; ++i)
        {
            if (signs[i+1] != signs[i])
                ++phases_count;
        }

        phases_count-=2;
        double Z = (phases_count - (2*n-7)/3) * sqrt(90 / (16*n-29));
        boost::math::normal dist(0.0, 1.0);
        double C = boost::math::quantile(dist, 1.0 - alpha / 2.0);
        return !(Z > C);
    }

    const bool criterion_2(const flow& f, double alpha)
    {
        size_t n = f.size();
        std::vector<double> diff = get_diff(f);
        std::vector<int> signs(n-1);
        int phases_count = 0;

        int longest_phase = 1;
        int current_phase = 0;

        // Инициализация первого элемента
        signs[0] = (diff[1] - diff[0] > 0) ? 1 : ((diff[1] - diff[0] < 0) ? -1 : 0);

        for (size_t i = 1; i < diff.size(); ++i)
        {
            double tmp = diff[i] - diff[i - 1];
            if (tmp > 0)
            {
                signs[i] = 1;
            }
            else
            {
                signs[i] = -1;
            }
            if (signs[i] != signs[i - 1])
            {
                phases_count++;
            }
            current_phase++;
            if (signs[i] != signs[i - 1] && (signs[i] != 0))
            {
                longest_phase = std::max(longest_phase, current_phase);
                current_phase = 0;
            }
        }

        int C;
        if (n <= 26)    C = 5;
        else if (n <= 153)  C = 6;
        else if (n <= 1170) C = 7;
        else C = 8;

        return (phases_count > (2.0 * n - 1) / 3 - 1.96 * std::sqrt((16 * n - 29) / 90))
            && (longest_phase < C);
    }

    bool criterion_3(const flow& data, double alpha) 
    {
        // Проверяем, достаточно ли данных
        if (data.size() < 3) return true; // Недостаточно данных для анализа

        // Вычисляем межсобытийные интервалы (Δt_i = t_{i+1} - t_i)
        std::vector<double> intervals(data.size() - 1);
        for (size_t i = 0; i < data.size() - 1; ++i) {
            intervals[i] = data[i + 1] - data[i];
        }

        // Вычисляем разности между соседними интервалами (Δt_{i+1} - Δt_i)
        std::vector<double> diff(intervals.size() - 1);
        for (size_t i = 0; i < intervals.size() - 1; ++i) {
            diff[i] = intervals[i + 1] - intervals[i];
        }

        // Подсчитываем знаки разностей
        int n1 = 0, n2 = 0; // Количество положительных и отрицательных разностей
        for (double d : diff) {
            if (d > 0) n1++;
            else if (d < 0) n2++;
        }

        // Подсчитываем количество серий
        int runs = 1; // Начинаем с 1 (первая серия)
        for (size_t i = 1; i < diff.size(); ++i) {
            if ((diff[i] > 0 && diff[i - 1] <= 0) || (diff[i] < 0 && diff[i - 1] >= 0)) {
                runs++;
            }
        }

        // Ожидаемое значение и дисперсия
        double n = n1 + n2;
        if (n < 2) return true; // Слишком мало данных для теста
        double expected_runs = (2.0 * n1 * n2) / n + 1;
        double variance = (2.0 * n1 * n2 * (2.0 * n1 * n2 - n1 - n2)) / (n * n * (n - 1));
        double Z = std::abs((runs - expected_runs) / std::sqrt(variance));

        // Квантиль для нормального распределения
        boost::math::normal dist(0.0, 1.0);
        double C = boost::math::quantile(dist, 1.0 - alpha / 2.0);

        std::cout << "Z: " << Z << "; C: " << C << '\n';
        return Z <= C; // Если Z <= C, поток независим
    }

    bool criterion_4(const flow& data, double alpha) 
    {
        // Проверяем, достаточно ли данных
        if (data.size() < 3) return true; // Недостаточно данных для анализа

        // Вычисляем межсобытийные интервалы (Δt_i = t_{i+1} - t_i)
        std::vector<double> intervals(data.size() - 1);
        for (size_t i = 0; i < data.size() - 1; ++i) {
            intervals[i] = data[i + 1] - data[i];
        }

        // Подсчитываем количество инверсий
        long long inversions = 0;
        for (size_t i = 0; i < intervals.size(); ++i) {
            for (size_t j = i + 1; j < intervals.size(); ++j) {
                if (intervals[i] > intervals[j]) {
                    inversions++;
                }
            }
        }

        // Ожидаемое значение и дисперсия числа инверсий
        double n = intervals.size();
        double expected_inversions = (n * (n - 1)) / 4.0;
        double variance = (n * (n - 1) * (2 * n + 1)) / 72.0;
        if (variance == 0) return true; // Слишком мало данных

        // Нормализованная статистика
        double Z = std::abs((inversions - expected_inversions) / std::sqrt(variance));

        // Квантиль для нормального распределения
        boost::math::normal dist(0.0, 1.0);
        double C = boost::math::quantile(dist, 1.0 - alpha / 2.0);

        std::cout << "Z: " << Z << "; C: " << C << '\n';
        return Z <= C; // Если Z <= C, поток независим
    }
};
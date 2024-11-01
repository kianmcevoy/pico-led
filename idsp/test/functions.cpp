#include "idsp/functions.hpp"
#include "testers.hpp"

#include <random>

void test_rescale();
void test_sgn();
void test_min_max();
void test_clamp();
void test_tanh();
void test_is_between();
void test_wrap();
void test_interpolate();
void test_power();
void test_factorial();
void test_sin();
void test_scale();

int main(int argc, const char* argv[])
{
    test_rescale();
    test_sgn();
    test_min_max();
    test_clamp();
    test_tanh();
    test_is_between();
    test_wrap();
    test_interpolate();
    test_power();
    test_factorial();
    test_sin();
    test_scale();

    return 0;
}


void test_rescale()
{
    idsp::test_eq(idsp::rescale(2, 0, 10, 0, 20), 4, "Rescale int");
    idsp::test_eq(idsp::rescale(0.4, 0.3, 0.7, 0.0, 1.0), 0.25, "Rescale double");
    idsp::test_eq(idsp::rescale(0.25f, 0.0f, 1.0f, 1.0f, 0.0f), 0.75f, "Rescale invert float");
    idsp::test_eq(idsp::rescale(25, 0, 20, 10, 50), 60, "Rescale extra-range int");
}

void test_sgn()
{
    idsp::test_eq(idsp::sgn(5), 1, "Signum on positive int");
    idsp::test_eq(idsp::sgn(-0.3), -1, "Signum on negative double");
    idsp::test_eq(idsp::sgn(0), 0, "Signum on zero int");
}

void test_min_max()
{
    idsp::test_eq(idsp::min(4, 23), 4, "Min of positive int");
    idsp::test_eq(idsp::min(-9, 7), -9, "Min of bipolar int");
    idsp::test_eq(idsp::min(-3, -7), -7, "Min of negative int");
    idsp::test_eq(idsp::min(0.2, 0.35), 0.2, "Min of positive double");

    idsp::test_eq(idsp::max(4, 23), 23, "Max of positive int");
    idsp::test_eq(idsp::max(-9, 7), 7, "Max of bipolar int");
    idsp::test_eq(idsp::max(-3, -7), -3, "Max of negative int");
    idsp::test_eq(idsp::max(0.2, 0.35), 0.35, "Max of positive double");
}

void test_clamp()
{
    idsp::test_eq(idsp::clamp(-10, -5, 5), -5, "Clamp negative OOB int");
    idsp::test_eq(idsp::clamp(-3, -10, 10), -3, "Clamp negative IB int");
    idsp::test_eq(idsp::clamp(4, -10, 10), 4, "Clamp positive IB int");
    idsp::test_eq(idsp::clamp(14, -10, 10), 10, "Clamp positive OOB int");

    idsp::test_eq(idsp::clamp(-1.5, -1.0, 1.0), -1.0, "Clamp negative OOB double");
    idsp::test_eq(idsp::clamp(-0.7, -1.0, 1.0), -0.7, "Clamp negative IB double");
    idsp::test_eq(idsp::clamp(0.2, 0.0, 0.5), 0.2, "Clamp positive IB double");
    idsp::test_eq(idsp::clamp(2.5, 0.0, 1.0), 1.0, "Clamp positive OOB double");
}

void test_tanh()
{
    // Run tests on random numbers, comparing to std::tanh
    constexpr int num_tanh_tests {20};
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<double> dist(-5.0, 5.0);

    for (int i = 0; i < num_tanh_tests; i++)
    {
        const auto x = dist(eng);
        const auto control = std::tanh(x);
        const auto result = idsp::tanh_fast(x);
        // idsp::tanh is accurate to std::tanh within approx +/-0.025
        idsp::test_eq(control, result, "Tanh test " + std::to_string(i+1), 0.025);
    }
}

void test_is_between()
{
    // Run tests on random numbers, comparing to manually verified results
    constexpr int num_bounds_tests {20};
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<double> dist(-5.0, 5.0);

    for (int i = 0; i < num_bounds_tests; i++)
    {
        const auto x = dist(eng);
        const bool control = x >= -1.0 && x <= 1.0;
        const auto result = idsp::is_between(x, -1.0, 1.0);
        idsp::test(control == result, "Between test " + std::to_string(i+1) + ", value " + std::to_string(x));
    }

    idsp::test(idsp::is_between_safe(3.0, 4.0, 1.0), "Between safe positive IB int");
    idsp::test(!idsp::is_between_safe(1.0, 5.0, 4.0), "Between safe positive OOB int");
}

void test_wrap()
{
    constexpr int min {-6};
    constexpr int max {9};
    int expected = -5;
    for (int i = -50; i < 50; i++)
    {
        const auto x = idsp::wrap(i, min, max);
        idsp::test_eq(x, expected, "Wrap int " + std::to_string(i));
        expected++;
        if (expected >= max)
            expected = min;
    }

    idsp::test_eq(idsp::wrap(0.5), 0.5, "Wrap IB double");
    idsp::test_eq(idsp::wrap(1.5), 0.5, "Wrap positive OOB double");
    idsp::test_eq(idsp::wrap(2.5), 0.5, "Wrap positive VOOB double");
    idsp::test_eq(idsp::wrap(-0.5), 0.5, "Wrap negative OOB double");
    idsp::test_eq(idsp::wrap(-1.5), 0.5, "Wrap negative VOOB double");
}

void test_interpolate()
{
    idsp::test_eq(idsp::interpolate_2(0.5, 0, 10), 5, "Interpolate 2 int");
    idsp::test_eq(idsp::interpolate_2(0.5, 0, 10), 5, "Interpolate 2 int");
}

void test_power()
{

}

void test_factorial()
{

}

void test_sin()
{

}

void test_scale()
{

}


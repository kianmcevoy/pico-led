#include "idsp/lookup.hpp"
#include "testers.hpp"

static float inverter(float v)
{
    return -v;
}

static float chopper(float in, bool* chop_ptr)
{
    auto& chop = *chop_ptr;
    const float out = chop ? 0.f : in;
    chop = !chop;
    return out;
}

void test_inverter();

void test_chopper();

int main(int argc, const char* argv[])
{
    test_inverter();
    test_chopper();
    return 0;
}

void test_inverter()
{
    constexpr size_t table_size = 128;
    const idsp::LookupTable<float, table_size> table(inverter);

    idsp::test_eq(table.table().front(), -0.f, "First invert table value");
    idsp::test_eq(table.table().back(), -1.f, "Last invert table value");
    for (size_t i = 0; i < table_size; i++)
    {
        const float input = static_cast<float>(i) / static_cast<float>(table_size - 1);
        const float expected = inverter(input);
        idsp::test_eq(table[i], expected, "Invert table value " + std::to_string(i));
    }
}

void test_chopper()
{
    constexpr size_t table_size = 129;

    bool chop_switch = false;
    const idsp::LookupTable<float, table_size> table(chopper, &chop_switch);

    idsp::test_eq(table.table().front(), 0.f, "First chopped table value");
    idsp::test_eq(table.table().back(), 1.f, "Last chopped table value");
    chop_switch = false;
    for (size_t i = 0; i < table_size; i++)
    {
        const float input = static_cast<float>(i) / static_cast<float>(table_size - 1);
        const float expected = chopper(input, &chop_switch);
        idsp::test_eq(table[i], expected, "Chopped table value " + std::to_string(i));
    }
}


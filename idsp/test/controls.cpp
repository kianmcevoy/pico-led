#include "idsp/controls.hpp"
#include "testers.hpp"

void test_Flag();

void test_Parameter();

int main(int argc, const char* argv[])
{
    test_Flag();

    test_Parameter();

    return 0;
}


void test_Flag()
{
    idsp::Flag flag;
    idsp::test(
        !flag.is_high()
        && !flag.is_rising()
        && !flag.is_falling()
        && !flag.has_changed(),
        "Button has incorrect initial state"
    );
    flag.process(false);
    idsp::test(
        !flag.is_high()
        && !flag.is_rising()
        && !flag.is_falling()
        && !flag.has_changed(),
        "Button has incorrect state after initial false"
    );
    flag.process(true);
    idsp::test(
        flag.is_high()
        && flag.is_rising()
        && !flag.is_falling()
        && flag.has_changed(),
        "Button has incorrect state on rising edge"
    );
    flag.process(true);
    idsp::test(
        flag.is_high()
        && !flag.is_rising()
        && !flag.is_falling()
        && !flag.has_changed(),
        "Button has incorrect state on sustained true"
    );
    flag.process(false);
    idsp::test(
        !flag.is_high()
        && !flag.is_rising()
        && flag.is_falling()
        && flag.has_changed(),
        "Button has incorrect state on falling edge"
    );
    flag.process(false);
    idsp::test(
        !flag.is_high()
        && !flag.is_rising()
        && !flag.is_falling()
        && !flag.has_changed(),
        "Button has incorrect state on sustained false"
    );
    flag.process(true);
    idsp::test(
        flag.is_high()
        && flag.is_rising()
        && !flag.is_falling()
        && flag.has_changed(),
        "Button has incorrect state on rising edge (2)"
    );
    flag.process(false);
    idsp::test(
        !flag.is_high()
        && !flag.is_rising()
        && flag.is_falling()
        && flag.has_changed(),
        "Button has incorrect state on immediate falling edge"
    );
    flag.process(true);
    idsp::test(
        flag.is_high()
        && flag.is_rising()
        && !flag.is_falling()
        && flag.has_changed(),
        "Button has incorrect state on immediate rising edge"
    );
}

void test_Parameter()
{
    idsp::Parameter<int, uint16_t> bare_param;

    idsp::test(!bare_param.has_changed(), "Bare pot has changed after construction");
    idsp::test(bare_param.get_output() == 0, "Bare pot has non-zero initial value");
    bare_param.process(666);
    idsp::test(bare_param.has_changed(), "Bare pot has not changed after initial process");
    idsp::test(bare_param.get_output() == 666, "Bare pot has incorrect value after first process");
    bare_param.process(666);
    idsp::test(!bare_param.has_changed(), "Bare pot has changed after sustained process");
    idsp::test(bare_param.get_output() == 666, "Bare pot has incorrect value after sustained process");


    idsp::Parameter<int, uint16_t,
        idsp::paramproc::hysterisis::ChangeThreshold<int>
    > gated_param(
        idsp::paramproc::hysterisis::ChangeThreshold<int>(10)
    );

    idsp::test(!gated_param.has_changed(), "Gated pot has changed after construction");
    idsp::test(gated_param.get_output() == 0, "Gated pot has non-zero initial value");
    gated_param.process(50);
    idsp::test(gated_param.has_changed(), "Gated pot has not changed after initial process");
    idsp::test(gated_param.get_output() == 50, "Gated pot has incorrect value after first process");
    gated_param.process(55);
    idsp::test(!gated_param.has_changed(), "Gated pot has changed after process with insufficient difference (1)");
    idsp::test(gated_param.get_output() == 50, "Gated pot has incorrect value after second process");
    gated_param.process(42);
    idsp::test(!gated_param.has_changed(), "Gated pot has changed after process with insufficient difference (2)");
    idsp::test(gated_param.get_output() == 50, "Gated pot has incorrect value after third process");
    gated_param.process(75);
    idsp::test(gated_param.has_changed(), "Gated pot has not changed after process with sufficient difference");
    idsp::test(gated_param.get_output() == 75, "Gated pot has incorrect value after fourth process");
}

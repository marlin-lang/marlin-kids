#ifndef marlin_exec_standard_lib_hpp
#define marlin_exec_standard_lib_hpp

#include <string>

namespace marlin::exec {

inline const std::string standard_lib{
    R"std(

class ExternalInterrupt extends Error {
	constructor(message = "") {
        super(message);
        this.name = "ExternalInterrupt";
    }
}

function __interrupt__() { return false; }

let interrupt_step = 100;
class GlobalClock {
    constructor() {
        this.previous_time = Date.now() + interrupt_step;
    }
    check_termination() {
        let time = Date.now();
        if (time > this.previous_time) {
            if (__interrupt__()) {
                throw new ExternalInterrupt();
            } else {
                this.previous_time = time + interrupt_step;
            }
        }
    }
}
let global_clock = new GlobalClock();

function is_external_interrupt(exc) {
    return exc instanceof ExternalInterrupt;
}

// Arbitrary number of arguments
function print() {}

function* range(first, second = undefined, third = undefined) {
    var begin = 0, end, step = 1;
    if (second != undefined) {
        begin = first;
        end = second;
        if (third != undefined) {
            step = third;
        }
    } else {
        end = first;
    }
    for (var i = begin; step >= 0 ? i < end : i > end; i += step) {
        yield i;
    }
}

)std"};

};  // namespace marlin::exec

#endif  // marlin_exec_standard_lib_hpp
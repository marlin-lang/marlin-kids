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

var global_time = Date.now() + 100;
function check_termination() {
    var time = Date.now();
    if (time > global_time) {
        if (__interrupt__()) {
            throw new ExternalInterrupt();
        } else {
            global_time = time + 100;
        }
    }
}

function __interrupt__() {
    return new ExternalInterrupt();
}

function is_external_interrupt(exc) {
    return exc instanceof ExternalInterrupt;
}

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
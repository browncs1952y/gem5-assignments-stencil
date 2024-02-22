from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class ForwardingObject(SimObject):
    type = "ForwardingObject"
    cxx_header = "gem5-gearup/forwarding_object.hh"
    cxx_class = "gem5::ForwardingObject"

    icache_port = ResponsePort("")
    dcache_port = ResponsePort("")
    mem_port = RequestPort("")

class BuggyObject1(SimObject):
    type = "BuggyObject1"
    cxx_header = "gem5-gearup/buggy_object1.hh"
    cxx_class = "gem5::BuggyObject1"

    icache_port = ResponsePort("")
    dcache_port = ResponsePort("")
    mem_port = RequestPort("")

class BuggyObject2(SimObject):
    type = "BuggyObject2"
    cxx_header = "gem5-gearup/buggy_object2.hh"
    cxx_class = "gem5::BuggyObject2"

    icache_port = ResponsePort("")
    dcache_port = ResponsePort("")
    mem_port = RequestPort("")

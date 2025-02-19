# Author: YOUR NAME HERE!
# Copyright (c) 2023 Brown University
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, and permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUDE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILIT, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILTY OF SUCH DAMAGE.

from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject


class MicroCache(SimObject):
    type = "MicroCache"
    cxx_header = "mem/cache/micro-cache.hh"
    cxx_class = "gem5::MicroCache"

    cpu_side = ResponsePort("")
    mem_side = RequestPort("")

    latency = Param.Int(2, "Cache access latency")

    size = Param.Int(64, "Cache size in bytes")

    assoc = Param.Int(1, "Cache associativity")

    # UNUSED
    prefetcher = Param.BasePrefetcher(NULL, "No prefetching")

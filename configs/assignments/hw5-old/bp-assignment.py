# Copyright (c) 2015 Jason Power
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
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
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
This is the RISCV equivalent to `simple.py` (which is designed to run using the
X86 ISA). More detailed documentation can be found in `simple.py`.
"""

import m5
from m5.objects import *
from m5.objects.RiscvMMU import RiscvMMU
from m5.objects.RiscvCPU import RiscvCPU
from m5.objects.BaseO3CPU import BaseO3CPU
from m5.objects.BranchPredictor import *

m5.util.addToPath("../../")
from common import SimpleOpts

SimpleOpts.add_option("binary", type=str)
SimpleOpts.add_option("--branchpred", default="hybrid", choices=["hybrid", "tournament", "local", "bimode", "tage", "perceptron"])
SimpleOpts.add_option("--bp_entries", default=2048, type=int)
args = SimpleOpts.parse_args()

thispath = os.path.dirname(os.path.realpath(__file__))
bin_path = os.path.join(
    thispath,
    "../../..",
    args.binary
)

class O3CPUBP(BaseO3CPU):
    if args.branchpred == "hybrid":
        branchPred = Param.BranchPredictor(HybridBP(hybridPredictorEntries=args.bp_entries), "Hybrid BP")
    elif args.branchpred == "local":
        branchPred = Param.BranchPredictor(LocalBP(localPredictorSize=args.bp_entries), "Local BP")
    elif args.branchpred == "tournament":
        branchPred = Param.BranchPredictor(TournamentBP(localPredictorSize=args.bp_entries), "Tournament BP")
    elif args.branchpred == "tage":
        branchPred = Param.BranchPredictor(LTAGE(), "TAGE BP")
    elif args.branchpred == "bimode":
        branchPred = Param.BranchPredictor(BiModeBP(), "BiMode BP")
    elif args.branchpred == "perceptron":
        branchPred = Param.BranchPredictor(MultiperspectivePerceptron8KB(), "MultiperspectivePerceptron BP")

class RiscvO3CPUBP(O3CPUBP, RiscvCPU):
    mmu = RiscvMMU()


system = System()

system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

system.mem_mode = "timing"
system.mem_ranges = [AddrRange("512MB")]
system.cpu = RiscvO3CPUBP()

system.membus = SystemXBar()

system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

system.cpu.createInterruptController()

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

system.workload = SEWorkload.init_compatible(bin_path)

process = Process()
process.cmd = [bin_path]
system.cpu.workload = process
system.cpu.createThreads()

root = Root(full_system=False, system=system)
m5.instantiate()

print(f"Beginning simulation!")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")

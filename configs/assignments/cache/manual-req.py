from m5.objects import *
import m5
from m5.objects.DRAMInterface import *
from m5.objects.NVMInterface import *
from cache_configs import *

from common import SimpleOpts

# sample cmd: build/RISCV/gem5.debug --outdir=m5out1 configs/assignments/manual-req.py

SimpleOpts.add_option("--control", action="store_true")

# edit this to simulate next request!
# tuples are: (start addr, size (in bytes), r/w (True if read, False if write))
# address range (512 MB = 29 bits) = 0x00000000 - 0x1fffffff
reqs = [(0x0, 64, True), (0x0, 64, True)]

# 64 - (r_start % 64) > r_size
for r_start, r_size, r_read in reqs:
    print(r_start, r_size, r_read)
    if (64 - (r_start % 64) < r_size):
        print(f'Invalid request: ({r_start}, {r_size}, {r_read}). Crosses block boundaries. Exiting.')
        exit()

options = SimpleOpts.parse_args()

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "4GHz"
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'timing'

system.generator = PyTrafficGen()

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()

system.mem_ranges = [AddrRange('512MB')]
system.mem_ctrl.dram.range = system.mem_ranges[0]

if (options.control):
    system.l2cache = L2Cache(options)
else:
    system.l2cache = MicroL2Cache(options)
system.l2bus = L2XBar()

system.generator.port = system.l2bus.cpu_side_ports

system.l2cache.connectCPUSideBus(system.l2bus)

# Create a memory bus
system.membus = SystemXBar()

# Connect the L2 cache to the membus
system.l2cache.connectMemSideBus(system.membus)

system.mem_ctrl.port = system.membus.mem_side_ports

root = Root(full_system=False, system=system)

m5.instantiate()

# <duration (ticks)> <start addr> <end addr> <access size (bytes)>
# <min period (ticks)> <max period (ticks)> <percent reads> <data limit (bytes)>
DUR = 6000
PER = int(DUR / 2)

def trace():
    for r_start, r_size, r_read in reqs:
        yield system.generator.createLinear(DUR, r_start, r_start - (r_start % 64) + 63, r_size, PER, PER, 100 if r_read else 0, 0)
    yield system.generator.createExit(0)

system.generator.start(trace())

exit_event = m5.simulate()

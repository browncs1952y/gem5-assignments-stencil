from m5.objects import *
import m5
from m5.objects.DRAMInterface import *
from m5.objects.NVMInterface import *
from cache_configs import *

from common import SimpleOpts

# sample cmd: build/RISCV/gem5.debug --outdir=m5out1 configs/assignments/traffgen.py linear 100 --traffic_request_size=32
SimpleOpts.add_option("--traffic_max_addr", default="512MB")
SimpleOpts.add_option("--traffic_request_size", default=64, type=int)
SimpleOpts.add_option("--traffic_num_reqs", default = 10000, type=int)

SimpleOpts.add_option("--control", action="store_true")

# required args
SimpleOpts.add_option(
    "traffic_mode",
    type = str,
    help = "pattern of generated addresses, linear or random."
)
SimpleOpts.add_option(
    "rd_prct",
    type=int,
    help="Read Percentage, the rest will be writes, ex: 70",
)

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

if (args.control):
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

def createRandomTraffic(tgen):
    yield tgen.createRandom(1000000000,                    # duration
                            0,                                   # min_addr
                            AddrRange(options.traffic_max_addr).end, # max_adr
                            options.traffic_request_size,                                  # block_size
                            1000,                  # min_period
                            1000,                  # max_period
                            options.rd_prct,                     # rd_perc
                            options.traffic_num_reqs * options.traffic_request_size)                                   # data_limit
    yield tgen.createExit(0)

def createLinearTraffic(tgen):
    yield tgen.createLinear(1000000000,                    # duration
                            0,                                   # min_addr
                            AddrRange(options.traffic_max_addr).end, # max_adr
                            options.traffic_request_size,                                  # block_size
                            1000,                  # min_period
                            1000,                  # max_period
                            options.rd_prct,                     # rd_perc
                            options.traffic_num_reqs * options.traffic_request_size)                                   # data_limit
    yield tgen.createExit(0)


# def createStridedTraffic(tgen):
#     yield tgen.createStrided(1000000000,                    # duration
#                             0,                                   # min_addr
#                             AddrRange(options.traffic_max_addr).end, # max_adr
#                             options.traffic_request_size,                                  # block_size
#                             32, # stride
#                             0, # gen_id
#                             1000,                  # min_period
#                             1000,                  # max_period
#                             options.rd_prct,                     # rd_perc
#                             options.traffic_num_reqs * options.traffic_request_size)                                   # data_limit
#     yield tgen.createExit(0)

root = Root(full_system=False, system=system)

m5.instantiate()

if options.traffic_mode == 'linear':
    system.generator.start(createLinearTraffic(system.generator))
elif options.traffic_mode == 'random':
    system.generator.start(createRandomTraffic(system.generator))
#elif options.traffic_mode == 'strided':
#    system.generator.start(createStridedTraffic(system.generator))
else:
    print('Wrong traffic type! Exiting!')
    exit()

exit_event = m5.simulate()
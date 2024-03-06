TARGET_ISA=riscv

GEM5_HOME=$(realpath ./../../..)
$(info   GEM5_HOME is $(GEM5_HOME))

CXX=/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc

CFLAGS=-I$(GEM5_HOME)/include -static

LDFLAGS=-L$(GEM5_HOME)/util/m5/build/$(TARGET_ISA)/out -lm5

OBJECTS= leaky-prog

all: leaky-prog

leaky-prog:
	$(CXX) -o $(OBJECTS) leaky-prog.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

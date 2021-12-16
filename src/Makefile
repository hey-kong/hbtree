CXX = g++ -O2
LDFLAGS = -pthread -lpmemobj -lpmem -lrt -lm -mpopcnt -mlzcnt -mbmi
DEFS = -DSINGLE_BTREE -DNO_VALUE
MULT_DEFS = -DNO_VALUE
WFLAGS = -Wall -Wno-format -Wno-unused-variable
CFLAGS = -ggdb

SING_CXXFLAGS += $(DEFS) $(IFLAGS) $(WFLAGS) $(CFLAGS)
MULT_CXXFLAGS += $(MULT_DEFS) $(IFLAGS) $(WFLAGS)

.PHONY: clean
default:  all

all: hbtree alex

clean:
	rm -f /mnt/pmem/data/*
	rm -f /mnt/pmem/log/*
	rm -f alex
	rm -f hbtree

hbtree: alex_base.cc alex_fanout_tree.cc btree.cc nvm_allocator.cc inner_node.cc hbtree.cc test.cc
	$(CXX) $(SING_CXXFLAGS) alex_base.cc alex_fanout_tree.cc btree.cc nvm_allocator.cc inner_node.cc hbtree.cc test.cc -o hbtree $(LDFLAGS)
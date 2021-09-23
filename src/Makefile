CXX = g++
LDFLAGS = -pthread -lpmemobj -lpmem -lrt -lm
DEFS = -DSINGLE_BTREE -DNO_VALUE
MULT_DEFS = -DNO_VALUE
WFLAGS = -Wall -Wno-format -Wno-unused-variable
CFLAGS = -ggdb

SING_CXXFLAGS += $(DEFS) $(IFLAGS) $(WFLAGS) $(CFLAGS)
MULT_CXXFLAGS += $(MULT_DEFS) $(IFLAGS) $(WFLAGS)

.PHONY: clean
default:  all

all: clean hbtree

cfile:
	rm -f /pmem0/data/*
	rm -f /pmem0/log/*

clean:
	rm -f /pmem0/data/*
	rm -f /pmem0/log/*
	rm -f hbtree

hbtree:
	$(CXX) $(SING_CXXFLAGS) test.cc -o hbtree $(LDFLAGS)

base:= sample realloc

CFLAGS += -gdwarf-2 -g3 

all:$(base)

.PHONY: clean

clean:
	-rm $(base)

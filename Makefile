
BIN = subsystem libsubsystem.so cat

subsystem_SRC = subsystem.c
libsubsystem_SRC = libsubsystem.c
cat_SRC = cat.c

.PHONY: all clean start stop

all: $(BIN)

clean: stop
	$(RM) $(BIN)

subsystem: $(subsystem_SRC)
	$(CC) -o $@ $^

libsubsystem.so: $(libsubsystem_SRC)
	$(CC) -shared -ldl -o $@ -fPIC $^

cat: $(cat_SRC)
	$(CC) -o $@ -L. -I. -lsubsystem -Wl,-R -Wl,. $^

# provide targets to start / stop the subsystem process
start:
	[ -S subsystem.socket ] || ./subsystem &

stop:
	[ ! -S subsystem.socket ] || killall subsystem
	$(RM) subsystem.socket

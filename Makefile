
CFLAGS?= -O2 -fomit-frame-pointer -std=gnu99 -march=native -fPIE -pie -fno-plt \
-ffast-math \
-Wall -Wextra -Wredundant-decls \
-Wno-unused-result -Wno-strict-aliasing -Wno-unused-function

ifdef debug
CFLAGS+= -O0 -g -Wno-format -fno-omit-frame-pointer
CFLAGS+= -DZEQ_DEBUG
endif

################################################################################
# Object files
################################################################################
_OBJECTS=               \
 ack                    \
 ack_mgr                \
 crc                    \
 hash                   \
 load_scheduler         \
 main                   \
 net_recv               \
 packet_alloc           \
 packet_alloc_legacy    \
 pfs                    \
 resource_thread        \
 syncbuf                \
 wld                    \
 work_queue             \
 zeq_atomic             \
 zeq_bit                \
 zeq_clock              \
 zeq_file               \
 zeq_printf             \
 zeq_semaphore          \
 zeq_string             \
 zeq_thread             \
 zone_load              \
 zone_load_wld

OBJECTS= $(patsubst %,build/%.o,$(_OBJECTS))

################################################################################
# Linker flags
################################################################################
CLIBS= -lpthread -lz

################################################################################
# Util
################################################################################
Q= @
E= @echo -e
RM= rm -f

################################################################################
# Build rules
################################################################################
.PHONY: default install all clean

default all: bin/zeq

amalg:
	$(Q)mkdir -p bin
	$(E) "Building and linking amalgamated source file"
	$(Q)$(CC) -o bin/zeq src/amalg.c $(CFLAGS) $(CLIBS)

bin/zeq: $(OBJECTS)
	$(Q)mkdir -p bin
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(CLIBS)

build/%.o: src/%.c $($(CC) -M src/%.c)
	$(Q)mkdir -p build
	$(E) "\e[0;32mCC    $@\e(B\e[m"
	$(Q)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(Q)$(RM) -r build/*
	$(Q)$(RM) bin/zeq
	$(E) "Cleaned build directory"


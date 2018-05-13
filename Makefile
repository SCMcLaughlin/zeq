
CFLAGS?= -O2 -fomit-frame-pointer -std=gnu89 \
-Wall -Wextra -Wredundant-decls \
-Wno-unused-result -Wno-strict-aliasing

ifdef debug
CFLAGS+= -O0 -Wno-format -fno-omit-frame-pointer
CFLAGS+= -DZEQ_DEBUG
endif

################################################################################
# Object files
################################################################################
_OBJECTS=               \
 main                   \
 packet_alloc           \
 packet_alloc_legacy    \
 zeq_atomic             \
 zeq_bit

OBJECTS= $(patsubst %,build/%.o,$(_OBJECTS))

################################################################################
# Linker flags
################################################################################
CLIBS= 

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

bin/zeq: $(OBJECTS)
	$(Q)mkdir -p bin
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(CLIBS)

build/%.o: src/%.c $($(CC) -M src/%.c)
	$(Q)mkdir -p build
	$(E) "\e[0;32mCC    $@\e(B\e[m"
	$(Q)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(Q)$(RM) build/*.o
	$(Q)$(RM) bin/zeq
	$(E) "Cleaned build directory"


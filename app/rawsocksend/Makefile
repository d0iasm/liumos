TARGET=rawsocksend.bin
OBJS=rawsocksend.o syscall.o

default: $(TARGET)

include ../../common.mk

%.o : %.c Makefile
	$(LLVM_CC) -target x86_64-unknown-none-elf -static -nostdlib -fPIE -I$(CLANG_SYSTEM_INC_PATH) -I../../src/third_party_root/include/ -B/usr/local/opt/llvm/bin/ -c -o $@ $*.c

%.o : %.S Makefile
	$(LLVM_CC) -target x86_64-unknown-none-elf -static -nostdlib -fPIE -B/usr/local/opt/llvm/bin/ -c -o $@ $*.S

$(TARGET) : $(OBJS) Makefile
	$(LLVM_LD_LLD) -static --no-rosegment -e main -o $@ $(OBJS)

run_on_linux: $(TARGET)
	sudo strace ./$(TARGET)

clean :
	-rm *.o
	-rm *.bin

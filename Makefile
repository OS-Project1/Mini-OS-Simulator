CC      := cc

# Bellek modülü — örnek: make MM_FRAME_COUNT=8 MM_PAGE_SIZE=512
MM_PAGE_SIZE    ?= 256
MM_FRAME_COUNT  ?= 4
MM_MAX_PAGES    ?= 16
MM_MAX_PID      ?= 32
MM_LOAD_TICKS   ?= 3

MM_CPPFLAGS := \
	-DMM_PAGE_SIZE=$(MM_PAGE_SIZE) \
	-DMM_FRAME_COUNT=$(MM_FRAME_COUNT) \
	-DMM_MAX_PAGES=$(MM_MAX_PAGES) \
	-DMM_MAX_PID=$(MM_MAX_PID) \
	-DMM_LOAD_TICKS=$(MM_LOAD_TICKS)

CFLAGS  := -Wall -Wextra -std=c11 $(MM_CPPFLAGS)
LDFLAGS :=

TARGET  := mini-os
SRCS    := main.c process.c scheduler.c memory.c concurrency.c file_system.c logging.c
OBJS    := $(SRCS:.c=.o)
OUTFILE := cikti.txt

.PHONY: all clean run txt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

txt: $(TARGET)
	./$(TARGET) > $(OUTFILE)
	@echo "Çıktı $(OUTFILE) dosyasına yazıldı."

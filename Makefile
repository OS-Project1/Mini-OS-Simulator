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

INC_DIR   := include
SRC_DIR   := src
BUILD_DIR := build

CFLAGS  := -Wall -Wextra -std=c11 $(MM_CPPFLAGS) -I$(INC_DIR)
LDFLAGS :=

TARGET  := mini-os
SRCS    := $(SRC_DIR)/main.c \
	$(SRC_DIR)/process.c \
	$(SRC_DIR)/scheduler.c \
	$(SRC_DIR)/memory.c \
	$(SRC_DIR)/concurrency.c \
	$(SRC_DIR)/file_system.c \
	$(SRC_DIR)/logging.c
OBJS    := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
OUTFILE := cikti.txt

.PHONY: all clean run txt failure1 txt-failure1 conc sched mem log log-pre log-post

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f *.o mini-os

run: $(TARGET)
	./$(TARGET)

txt: $(TARGET)
	./$(TARGET) > $(OUTFILE)
	@echo "Çıktı $(OUTFILE) dosyasına yazıldı."

# --- Modül çıktıları (mini-os tam çalışır; stdout süzülür) ---

# Zamanlayıcı + bellek + I/O: ilk [Scheduler] satırından ilk [Concurrency] öncesi (scheduler.c + memory.c, round_robin)
sched: $(TARGET)
	./$(TARGET) | awk '/^\[Scheduler\]/ && !started { started=1 } started && /\[Concurrency\]/ { exit } started { print }'

# Sadece memory.c satırları
mem: $(TARGET)
	./$(TARGET) | grep '^\[Memory\]'

# Tüm dosya işlem günlükleri (logging.c)
log: $(TARGET)
	./$(TARGET) | grep '^\[LOG'

# RR öncesi: CREATE / WRITE logları (ilk [LOG] … ilk [Scheduler] öncesi)
log-pre: $(TARGET)
	./$(TARGET) | awk '/^\[LOG / && !started { started=1 } started && /^\[Scheduler\]/ { exit } started { print }'

# concurrency_demo_run sonrası: READ / DELETE logları
log-post: $(TARGET)
	./$(TARGET) | awk '/^\[Result\] Enhanced expected=/ { seen=1; next } seen && /^\[LOG / { post=1 } post { print }'

# Sadece concurrency.c çıktısının tamamı: ilk [Concurrency] satırından önce boş satır; ilk [LOG satırına kadar (grep kaçırmaz)
conc: $(TARGET)
	./$(TARGET) | awk '/\[Concurrency\]/ && !started { started=1; print "" } started && /^\[LOG / { exit } started { print }'

# Senaryo 1 (FAILURE_SCENARIO.md): fizik çerçeve baskısı — az çerçeve + daha geniş sanal alan + yükleme gecikmesi
# Kullanım: make failure1   veya   make txt-failure1
failure1:
	$(MAKE) clean
	$(MAKE) MM_FRAME_COUNT=2 MM_MAX_PAGES=24 MM_LOAD_TICKS=4 all
	./$(TARGET)

txt-failure1:
	$(MAKE) clean
	$(MAKE) MM_FRAME_COUNT=2 MM_MAX_PAGES=24 MM_LOAD_TICKS=4 all
	./$(TARGET) > cikti_senaryo1_bellek.txt
	@echo "Senaryo 1 çıktısı cikti_senaryo1_bellek.txt dosyasına yazıldı."

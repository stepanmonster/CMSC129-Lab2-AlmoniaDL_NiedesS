CC     = gcc
CFLAGS = -g -Wall -O0 -std=c99 -Iinclude

# ─── Sources ─────────────────────────────────────────────────────────
SOURCES = src/main.c     \
          src/fcfs.c     \
          src/sjf.c      \
          src/stcf.c     \
          src/rr.c       \
          src/metrics.c  \
          src/gantt.c    \
          src/process.c  \
          src/utils.c    \
		  src/mlfq.c     \

OBJECTS = $(SOURCES:.c=.o)
TARGET  = schedsim
HEADERS = include/process.h   \
          include/scheduler.h \
          include/gantt.h     \
          include/metrics.h

# ─── Build ───────────────────────────────────────────────────────────
all: $(TARGET)
	@echo "Build complete. Cleaning object files..."
	$(MAKE) clean-objects

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# ─── Cleanup ─────────────────────────────────────────────────────────
.PHONY: all clean clean-objects test

clean-objects:
	rm -f $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
	@bash tests/test_suite.sh
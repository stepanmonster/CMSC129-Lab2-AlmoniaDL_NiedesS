CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
SRCS    = src/main.c src/fcfs.c src/sjf.c src/stcf.c src/rr.c \
          src/mlfq.c src/process.c src/gantt.c src/metrics.c src/utils.c
TARGET  = schedsim

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
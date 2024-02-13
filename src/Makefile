CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ 
AR=ar
CPU=2
PARAM = ./input/p1.txt ./input/p2.txt ./input/p3.txt ./input/p4.txt ./input/p5.txt ./input/p6.txt
TARGET = sched_sim

OBJS=linked_list.o\
     fake_process.o\
     fake_os.o

HEADERS=linked_list.h  fake_process.h

BINS=fake_process_test sched_sim

#disastros_test

.phony: clean all


all:	$(BINS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

fake_process_test:	fake_process_test.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^

sched_sim:	sched_sim.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^

run: $(TARGET)
	./$(TARGET) $(CPU) $(PARAM) 

clean:
	rm -rf *.o *~ $(OBJS) $(BINS)

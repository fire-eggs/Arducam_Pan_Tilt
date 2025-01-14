NAME=RunServoDemo
CC=gcc

# -Wall : produce all warnings
# -pthread : add multithreading support via pthreads library
# -lm : link against the m (math) library
# -g  : produce debugging info
CFLAGS=-Wall -pthread -lm -g -O0

ODIR=obj
SDIR=src

_DEPS=bcm283x_board_driver.h sccb_bus.h bcm283x_board_driver.h PCA9685_servo_driver.h
DEPS=$(patsubst %,$(SDIR)/%,$(_DEPS))

_OBJ=RunServoDemo.o bcm283x_board_driver.o sccb_bus.o PCA9685_servo_driver.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: src/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:	Run

Run:	$(NAME)
	sudo ./$(NAME)

.PHONY:	clean
clean:	
	rm $(NAME) $(ODIR)/*.o

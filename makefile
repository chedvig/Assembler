CC          = gcc
CFLAGS      = -ansi -pedantic -Wall
PROG_NAME   = assembler

ifdef DEBUG
CFLAGS += -g 
endif

$(PROG_NAME): main.o compile.o first_iteration.o second_iteration.o symbol_table.o linked_list.o pre_processor.o syntax.o 
	$(CC) $(CFLAGS)  $^ -o $@


main.o: main.c compile.h symbol_table.h linked_list.h define.h syntax.h

compile.o: compile.c compile.h symbol_table.h linked_list.h define.h \
 syntax.h

first_iteration.o: first_iteration.c compile.h symbol_table.h \
 linked_list.h define.h syntax.h

second_iteration.o: second_iteration.c compile.h symbol_table.h \
 linked_list.h define.h syntax.h

symbol_table.o: symbol_table.c symbol_table.h linked_list.h define.h

linked_list.o: linked_list.c linked_list.h define.h

pre_processor.o: pre_processor.c pre_processor.h

syntax.o: syntax.c syntax.h define.h


%.o:
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f $(PROG_NAME) *.o

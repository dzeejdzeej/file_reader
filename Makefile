RM := rm -rf
COMPILER ?= gcc

C_STD := -std=c99
C_OPT := -O3
C_WARNS :=

ifeq ($(COMPILER), clang)
	C_WARNS += -Weverything -Wno-padded
else ifneq (, $(filter $(COMPILER), cc gcc))
	C_WARNS += -Wall -Wextra -pedantic -Wcast-align \
				-Winit-self -Wlogical-op -Wmissing-include-dirs \
			 	-Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef  \
				-Wwrite-strings -Wpointer-arith -Wmissing-declarations \
				-Wuninitialized -Wold-style-definition -Wstrict-prototypes \
				-Wmissing-prototypes -Wswitch-default -Wbad-function-cast \
				-Wnested-externs -Wconversion -Wunreachable-code
endif

C_FLAGS := $(C_STD) $(C_OPT) $(C_WARNS)

.PHONY:all
all:
	@$(MAKE) app --no-print-directory

.PHONY:app
app:
	$(COMPILER) $(C_FLAGS) src/*.c app/*.c -I./inc -o main.out

.PHONY:test
test:
	$(COMPILER) $(C_FLAGS) -g src/*.c test/*.c -I./inc -o test.out

.PHONY:memcheck
memcheck: test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./test.out

.PHONY:clean
clean:
	@$(RM) main.out
	@$(RM) test.out


src = $(wildcard ./src/*.c)
obj = $(patsubst ./src/%.c, ./obj/%.o, $(src))
args = -g -Wall -pthread

ALL : main.out
main.out : $(obj)
	gcc $^ -o $@ $(args)

$(obj) : ./obj/%.o : ./src/%.c
	gcc -c $< -o $@ -I ./head $(args)

.PHONY : clean
clean :
	rm -rf $(obj) main.out

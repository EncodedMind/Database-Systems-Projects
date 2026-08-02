bf:
	@echo " Compile bf_main ...";
	mkdir -p ./build
	rm -f ./build/bf_main
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/*.c -lbf -o ./build/bf_main -O2;

hp:
	@echo " Compile hp_main ..."
	@mkdir -p ./build
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/*.c -lbf -o ./build/hp_main -O2

run-bf: bf
	@echo " Running bf_main ..."
	rm -f *.db
	./build/bf_main

run-hp: hp
	@echo " Running hp_main ..."
	rm -f *.db
	./build/hp_main

run-hp-valgrind: hp
	@echo " Running hp_main with valgrind ..."
	@mkdir -p ./build
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/hp_main.c ./src/*.c -lbf -o ./build/hp_main -O2 -g3
	rm -f *.db
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/hp_main

run-bf-valgrind: bf
	@echo " Running bf_main with valgrind ..."
	@mkdir -p ./build
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c ./src/*.c -lbf -o ./build/bf_main -O2 -g3
	rm -f *.db
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/bf_main






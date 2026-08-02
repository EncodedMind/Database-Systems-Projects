sort:
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2 -g3

run-sort: sort
	@echo " Running sort_main ...";
	./build/sort_main

run-valgrind: sort 
	@echo " Running sort_main with valgrind ...";
	valgrind ./build/sort_main

clean: 
	rm *.db
	rm ./build/sort_main

2way:
	@echo " Compile sort_main_2way ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main_2way.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main_2way -O2 -g3
	@echo " Running sort_main_2way ...";
	./build/sort_main_2way
	rm *.db
	rm ./build/sort_main_2way

10way:
	@echo " Compile sort_main_10way ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main_10way.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main_10way -O2 -g3
	@echo " Running sort_main_10way ...";
	./build/sort_main_10way
	rm *.db
	rm ./build/sort_main_10way
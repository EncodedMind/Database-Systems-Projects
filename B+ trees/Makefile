bplus_main_compile:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bplus_main.c ./src/*.c -lbf -o ./build/bp_main -O2 -g3;

bplus_tests_compile:
	@echo " Compile bf_tests ...";
	rm -rf ./build
	mkdir build
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bplus_tests.c ./src/*.c -lbf -o ./build/bp_tests -O2 -g3;

bplus_main_run: bplus_main_compile
	@echo " Running bf_main ..."
	rm -f *.db
	./build/bp_main

bplus_tests_run: bplus_tests_compile
	@echo " Running bf_tests ..."
	rm -f *.db
	./build/bp_tests

bplus_main_run_valgrind: bplus_main_compile
	@echo " Running bf_main with valgrind..."
	rm -f *.db
	valgrind --track-origins=yes ./build/bp_main

bplus_tests_run_valgrind: bplus_tests_compile
	@echo " Running bf_tests with valgrind ..."
	rm -f *.db
	valgrind --track-origins=yes ./build/bp_tests

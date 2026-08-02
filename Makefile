.PHONY: all heap-bf heap-hp bplus sort run-heap-bf run-heap-hp run-bplus run-sort clean

all: heap-bf heap-hp bplus sort

heap-bf:
	$(MAKE) -C Heap bf

heap-hp:
	$(MAKE) -C Heap hp

run-heap-bf:
	$(MAKE) -C Heap run-bf

run-heap-hp:
	$(MAKE) -C Heap run-hp

bplus:
	$(MAKE) -C "B+ trees" bplus_main_compile

run-bplus:
	$(MAKE) -C "B+ trees" bplus_main_run

sort:
	$(MAKE) -C Chunk sort

run-sort:
	$(MAKE) -C Chunk run-sort

clean:
	$(MAKE) -C Heap clean
	$(MAKE) -C Chunk clean
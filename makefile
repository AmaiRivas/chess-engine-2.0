all:
	gcc -oFast chengine.c -o chengine

debug:
	gcc chengine.c -o chengine
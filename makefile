debug: src/main.c
	gcc -Iinclude -g src/main.c -o multigrep
release: main.c
	gcc -Iinclude src/main.c multigrep
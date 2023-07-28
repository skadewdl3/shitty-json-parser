all: clean build run
run:
	./json-parser.sh
build:
	gcc ./json-parser.c -o ./json-parser.sh -lm
clean:
	rm -rf ./json-parser.sh
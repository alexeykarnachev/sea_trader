all:
	g++ \
	-Wall -pedantic -std=c++2a \
	-I./deps/include \
	-o ./build/linux/sea_trader \
	./src/*.cpp \
	-L./deps/lib/linux -lraylib -lGL -lpthread -ldl

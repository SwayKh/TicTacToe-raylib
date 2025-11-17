BUILD_DIR = build
$(shell mkdir -p $(BUILD_DIR))

default:
	gcc main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Llib -Iinclude -o game

build:
	gcc main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Llib -Iinclude -o $(BUILD_DIR)/tictactoe

run:
	gcc main.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Llib -Iinclude -o $(BUILD_DIR)/tictactoe && $(BUILD_DIR)/tictactoe

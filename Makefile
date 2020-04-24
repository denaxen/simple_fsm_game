path = ../../3rdparty/SFML-2.5.1

player_states:
	g++ ./player_states.cpp -std=c++11 -o player_states -I $(path)/include -L $(path)/lib/ -lsfml-graphics -lsfml-window -lsfml-system 

clear:
	rm ./*.exe
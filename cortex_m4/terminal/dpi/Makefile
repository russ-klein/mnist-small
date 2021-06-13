
all: ../bin/terminal_emulator ../questa/terminal.so

../questa/terminal.so: terminal.c 
	gcc -DVSIM -fPIC -shared -o ../questa/terminal.so terminal.c -I $(MODELTECH_HOME)/include -I ../questa 

../veloce/terminal.so: terminal.c 
	g++ -fPIC -shared -o ../veloce/terminal.so terminal.c -I $(TBX_HOME)/include -I ../veloce 

../bin/terminal_emulator: terminal_emulator.c
	gcc -o ../bin/terminal_emulator terminal_emulator.c -lpthread

clean:
	rm -f ../questa/terminal.so ../veloce/terminal.so ../bin/terminal_emulator

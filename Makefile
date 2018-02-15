CC = gcc
CCFLAGS = -W -Wall

BINARY = Example

all:
	$(CC) $(CCFLAGS) -I. Example.c MQTT.c -o $(BINARY)

clean:
	rm -r $(BINARY)

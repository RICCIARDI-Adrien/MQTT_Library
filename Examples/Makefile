CC = gcc
CCFLAGS = -W -Wall

all:
	$(CC) $(CCFLAGS) -I.. Publish.c ../MQTT.c -o Publish
	$(CC) $(CCFLAGS) -I.. Subscribe.c ../MQTT.c -o Subscribe

clean:
	rm -r Publish Subscribe

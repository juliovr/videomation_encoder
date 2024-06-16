CC = gcc
CFLAGS = -Og -g -lGL -lm -lpthread -ldl -lrt -lX11

all: youtube_backup

youtube_backup: src/youtube_backup.cpp
	$(CC) $(CFLAGS) src/youtube_backup.cpp lib/libraylib.a -o bin/youtube_backup 

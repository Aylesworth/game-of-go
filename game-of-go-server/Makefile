CC = g++
TARGET = server
SOURCE = server.cpp
LDFLAGS = -lmysqlcppconn -lssl -lcrypto

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET)
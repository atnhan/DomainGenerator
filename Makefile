CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		DomainGenerator.o

LIBS =

TARGET =	DomainGenerator

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

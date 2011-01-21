all: ebootsign
ebootsign: ebootsign.o crypto.o kirk_engine.o

install: ebootsign
	install $^ $(shell psp-config --pspdev-path)/bin

clean:
	$(RM) ebootsign
	$(RM) *.o
	$(RM) *~


CC = gcc
FLAGS  = -std=c11 -O3 -pedantic
FLAGS += -Wall -Wextra -Wpedantic -Werror
FLAGS += -lX11 -D_GNU_SOURCE

PARTICLE = bin/particle
DAMIXER  = bin/damixer
CRONOS   = bin/cronos

all: clear $(PARTICLE) $(DAMIXER) $(CRONOS)

$(CRONOS): src/cronos.c
	echo $@
	mkdir -p $(@D)
	$(CC) $(FLAGS) $< -o $@

$(PARTICLE): src/particle.c
	echo $@
	mkdir -p $(@D)
	$(CC) $(FLAGS) $< -o $@


$(DAMIXER): src/damixer.c
	echo $@
	mkdir -p $(@D)
	$(CC)   $(FLAGS)  -lasound $< -o $@


clean:
	rm -rf bin


clear:
	printf "\E[H\E[3J"
	clear


install: all
	cp -f bin/* ~/.local/bin


.PHONY: clear clean all install
.SILENT: clear clean all $(PARTICLE) $(DAMIXER) $(CRONOS)


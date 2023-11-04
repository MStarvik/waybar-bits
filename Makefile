CC = gcc
CFLAGS = -std=gnu17 -Os -s -DNDEBUG
# CFLAGS = -std=gnu17 -O0 -g

CFLAGS += $(shell pkg-config --cflags json-c)
JSON_C_LIBS = $(shell pkg-config --libs json-c)

all: keyboard-layout network-traffic

keyboard-layout: src/keyboard_layout.o
	$(CC) $(CFLAGS) $(JSON_C_LIBS) -o $@ $^

network-traffic: src/network_traffic.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: install
install: keyboard-layout network-traffic
	install -m 755 keyboard-layout $(HOME)/.config/waybar/scripts/keyboard-layout
	install -m 755 network-traffic $(HOME)/.config/waybar/scripts/network-traffic

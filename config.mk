CC=gcc
NAME=prayer-notify
DEBUGGER=gdb
SRC_DIR=src
VERSION:=$(shell git describe --tags --always --dirty 2>/dev/null || echo "1.0.0")
BUILD_DIR=build
LIBS=-lm -lcjson
SRC_FILES=$(shell find $(SRC_DIR) -name '*.c')
MAIN=$(BUILD_DIR)/${NAME}
INC_DIR=-Iinclude `pkg-config --libs --cflags libnotify`
CFLAGS=-Wall -Wextra -no-pie -Wpedantic -DAPP_VERSION=\"$(VERSION)\" $(INC_DIR) -Wno-unused-variable
RELEASE_FLAGS=-s -O3
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
SERVICE=$(NAME).service
SERVICEDIR=$(HOME)/.config/systemd/user

.PHONY: all clean run debug test install uninstall release

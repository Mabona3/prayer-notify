CC=gcc
NAME=prayer-notify
DEBUGGER=gdb

SRC_DIR=src
BUILD_DIR=build

VERSION:=$(shell git describe --tags --always --dirty 2>/dev/null || echo "1.0.0")

PKG_DEPS=libnotify
PKG_FLAGS:=$(shell pkg-config --cflags $(PKG_DEPS))
PKG_LIBS:=$(shell pkg-config --libs $(PKG_DEPS))

INC_FLAGS=-Iinclude $(PKG_FLAGS)
LIBS=-lm -lcjson $(PKG_LIBS)

CFLAGS=-Wall -Wextra -Wpedantic -DAPP_VERSION=\"$(VERSION)\" -Wno-unused-variable $(PKG_FLAGS) -DDAEMON_SYSTEMD -std=c11

SRC_FILES=$(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN=$(BUILD_DIR)/${NAME}

PREFIX=/usr
BINDIR=$(PREFIX)/bin
DESTDIR=

SERVICE=$(NAME).service
SERVICEDIR=$(PREFIX)/lib/systemd/user

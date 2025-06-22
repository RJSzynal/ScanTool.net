## Base layer for shared dependencies and tools installation.
# This image contains the basic build tools needed to compile the project.
FROM debian:sid-slim AS base

# ENV CPATH=:/usr/include
ENV CPATH=:/usr/share/mingw-w64/include
ENV LD_LIBRARY_PATH=/usr/share/mingw-w64/lib
ENV CC=i686-w64-mingw32-gcc
ENV WINDRES_BIN=i686-w64-mingw32-windres
ENV MINGDIR=/usr/share/mingw-w64

RUN apt-get update && apt-get install -y \
        build-essential \
        gcc-mingw-w64-i686-win32 \
        gcc-multilib \
        mingw-w64-tools \
    && rm -rf /var/lib/apt/lists/*

ADD build_tools/allegro-mingw-unix-4.2.3.tar.gz /usr/share/mingw-w64/
ADD build_tools/dx70_mgw_unix.tar.gz /usr/share/mingw-w64/
RUN cp /usr/share/mingw-w64/lib/liballeg.{a,so} \
     && cp /usr/share/mingw-w64/lib/liballeg_s.{a,so}

ENTRYPOINT ["bash"]


## Development environment for dev container use
FROM base AS dev

RUN apt-get update && apt-get install -y \
        git \
        wget \
    && rm -rf /var/lib/apt/lists/*

ENV DEBUGMODE=1

# Add scantool user
RUN useradd -m scantool

# Run as non privileged user
USER scantool



## Build of the application
FROM base AS build

WORKDIR /app
COPY . /app

# Compile the program
RUN make NOWERROR=1

FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    perl \
    git \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
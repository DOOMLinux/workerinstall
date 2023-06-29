FROM ubuntu:focal

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install --no-install-recommends -y libxml2-dev libcurl4-openssl-dev pkg-config g++ gcc git make

WORKDIR /program

COPY . .

RUN make -j "$(nproc)"
RUN make install
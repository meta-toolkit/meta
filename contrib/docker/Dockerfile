FROM debian:jessie

MAINTAINER Maciej Szymkiewicz "matthew.szymkiewicz@gmail.com"

ENV METADIR /opt/meta
RUN mkdir -p $METADIR

RUN apt-get update && apt-get -y install cmake libicu-dev git g++ && apt-get clean

WORKDIR $METADIR
RUN git clone --depth 1 https://github.com/meta-toolkit/meta.git .
RUN git submodule update --init --recursive
RUN mkdir $METADIR/build

WORKDIR $METADIR/build
RUN cp  $METADIR/config.toml .
RUN cmake $METADIR -DCMAKE_BUILD_TYPE=Release && make
RUN ctest --output-on-failure

RUN apt-get -y purge git && apt-get -y autoremove 

# docker build --build-arg http_proxy=http://192.168.4.22:3128 -t ddt --rm -f src/docker/Dockerfile .
# docker run -ti --rm ddt

FROM ubuntu:16.04

MAINTAINER Mathieu Brédif "mathieu.bredif@ign.fr"

RUN	apt-get update &&  \
	apt-get install -y cmake libboost-all-dev build-essential libcgal-dev libcgal-qt5-dev libeigen3-dev --assume-yes

ADD CMakeLists.txt /ddt/CMakeLists.txt
ADD src /ddt/src
ADD test /ddt/test

RUN cd /ddt && rm -rf build && mkdir -p build && cd build && cmake .. && make

CMD /ddt/build/ddt


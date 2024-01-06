FROM debian:11

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    autoconf \
    libtool \
    pkg-config \
    git \
    && apt-get clean

ENV GRPC_INSTALL_DIR /usr/local
ENV GRPC_RELEASE_TAG v1.60.0

RUN mkdir -p ${GRPC_INSTALL_DIR}
RUN export PATH="$PATH:${GRPC_INSTALL_DIR}/bin"
RUN git clone --recurse-submodules -b ${GRPC_RELEASE_TAG} --depth 1 --shallow-submodules https://github.com/grpc/grpc
RUN cd grpc && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake -DgRPC_INSTALL=ON \
        -DgRPC_BUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=${GRPC_INSTALL_DIR} \
        ../.. && \
    make -j $(nproc) && \
    make install && \
    cd ../..

WORKDIR /
CMD ["/bin/bash"]
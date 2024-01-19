FROM grpc-cpp-image:latest AS build

COPY . /w

WORKDIR /w/tests

RUN mkdir build

WORKDIR build

RUN cmake ..

RUN make

FROM debian:12
COPY --from=build /w/tests/build/test_client /r/

CMD [ "/r/test_client" ]





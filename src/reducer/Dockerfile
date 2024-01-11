FROM grpc-cpp-image:latest AS build

COPY . /w

WORKDIR /w/reducer

RUN mkdir build

WORKDIR build

RUN cmake ..

RUN make

FROM debian:11

COPY --from=build /w/reducer/build/main /r/

EXPOSE 50051

CMD [ "/r/main", "2>", "/fs/err.txt" ]




FROM grpc-cpp-image:latest AS build

COPY . /w

WORKDIR /w/master

RUN mkdir build

WORKDIR build

RUN cmake ..

RUN make

FROM debian:11
COPY --from=build /w/master/build/main /r/
COPY --from=build /w/fs /fs

EXPOSE 50051

CMD [ "/r/main" ]




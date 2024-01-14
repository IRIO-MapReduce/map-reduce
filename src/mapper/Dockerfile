FROM grpc-cpp-image:latest AS build

COPY . /w

WORKDIR /w/mapper

RUN mkdir build

WORKDIR build

RUN cmake ..

RUN make

FROM debian:12
COPY --from=build /w/mapper/build/main /r/
COPY --from=build /w/fs /fs

EXPOSE 50054

CMD [ "/r/main" ]




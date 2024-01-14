FROM grpc-cpp-image:latest AS build

COPY . /w

WORKDIR /w/reducer

RUN mkdir build

WORKDIR build

RUN cmake ..

RUN make

FROM debian:12
COPY --from=build /w/reducer/build/main /r/
COPY --from=build /w/fs /fs

EXPOSE 50055

CMD [ "/r/main" ]




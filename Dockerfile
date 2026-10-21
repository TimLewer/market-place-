FROM openapitools/openapi-generator-cli:v7.10.0 AS codegen

WORKDIR /work

COPY openapi/ ./openapi/
COPY templates/ ./templates/

RUN java -jar \
    /opt/openapi-generator/modules/openapi-generator-cli/target/openapi-generator-cli.jar \
    generate \
    -i /work/openapi/openapi.yaml \
    -g cpp-pistache-server \
    -t /work/templates \
    -o /work/generated

FROM ubuntu:24.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        g++ \
        cmake \
        make \
        pkg-config \
        libpistache-dev \
        nlohmann-json3-dev \
        libpqxx-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt ./
COPY src/ ./src/

COPY --from=codegen /work/generated/api/ ./generated/api/
COPY --from=codegen /work/generated/model/ ./generated/model/

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build -j 2

EXPOSE 8080

CMD ["/app/build/marketplace"]

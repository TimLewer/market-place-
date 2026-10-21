#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

docker run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$PROJECT_DIR:/work" \
  openapitools/openapi-generator-cli:v7.10.0 \
  generate \
  -i /work/openapi/openapi.yaml \
  -g cpp-pistache-server \
  -o /work/generated

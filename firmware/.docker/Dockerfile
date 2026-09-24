FROM gcc:14
RUN apt-get update -qq \
 && apt-get install -y -qq libssl-dev >/dev/null 2>&1 \
 && rm -rf /var/lib/apt/lists/*

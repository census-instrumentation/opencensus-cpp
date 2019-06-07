FROM ubuntu:cosmic

RUN apt update && \
    apt install -y clang-format golang git python-pip && \
    go get -v github.com/bazelbuild/buildtools/buildifier && \
    pip install 'cmake_format>=0.5.2'

CMD ["/bin/bash"]

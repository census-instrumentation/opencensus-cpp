FROM ubuntu:cosmic

RUN perl -pi -e 's@//archive@//old-releases@g' /etc/apt/sources.list
RUN perl -pi -e 's@^(.*security.*)$@#$1@g' /etc/apt/sources.list
RUN cat /etc/apt/sources.list

RUN apt update && \
    apt install -y clang-format golang git python-pip && \
    go get -v github.com/bazelbuild/buildtools/buildifier && \
    pip install 'cmake_format>=0.5.2'

CMD ["/bin/bash"]

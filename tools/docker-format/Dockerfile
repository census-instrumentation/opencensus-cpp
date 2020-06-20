FROM ubuntu:cosmic

# repositories are obsolete. the image need to be changed.
RUN perl -pi -e 's@//archive@//old-releases@g' /etc/apt/sources.list
# disable security updates altogether
RUN perl -pi -e 's@^(.*security.*)$@#$1@g' /etc/apt/sources.list

RUN apt update && \
    apt install -y clang-format git python-pip && \
    pip install 'cmake_format>=0.5.2'

# brings golang to 1.10 by default.bazelbuilds wants 1.12 minimum....
#RUN apt install -y golang

RUN apt install -y curl
RUN curl -sL https://dl.google.com/go/go1.13.3.linux-amd64.tar.gz -o go1.13.3.linux-amd64.tar.gz
RUN tar -xf go1.13.3.linux-amd64.tar.gz
RUN mv go /usr/local/go-1.13
RUN rm go1.13.3.linux-amd64.tar.gz

ENV GOROOT=/usr/local/go-1.13
ENV GOPATH=/root/go
RUN mkdir ${GOPATH}

RUN ${GOROOT}/bin/go get github.com/bazelbuild/buildtools/buildifier
ENV PATH=${GOPATH}/bin:${GOROOT}/bin:${PATH}

CMD ["/bin/bash"]

# Usage:
# -----
#
#   - Build container:
#        docker build -t prisci-build-env .
#
#   - To compile the project inside the container:
#        docker run --rm -it -v "$(pwd)":/home/rust/src prisci-build-env:latest

FROM opensuse/leap:15.2

RUN zypper -n install curl gcc-c++
RUN zypper --non-interactive install -t pattern devel_basis

WORKDIR /home/user/src

CMD make

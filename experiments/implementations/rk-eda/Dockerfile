FROM opensuse/leap:15.2

RUN zypper -n install curl
RUN curl https://sh.rustup.rs -sSf | sh -s -- -y
RUN zypper --non-interactive install -t pattern devel_basis

WORKDIR /home/rust/src

CMD source $HOME/.cargo/env && cargo build --release

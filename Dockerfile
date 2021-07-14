FROM node:8.9.0-slim

# set some ENV vars
ENV HOME /root
# ENV TERM dumb
ENV TERM=xterm-256color
ENV PROJECT_ROOT /opt/app

# install dependencies:
RUN apt-get update \
  && apt-get install -qqy build-essential pkg-config libc6-dev m4 g++-multilib \
  && autoconf libtool ncurses-dev unzip git python \
  && zlib1g-dev wget bsdmainutils automake
  && rm -rf /var/lib/apt/lists/*

# install some apt packages (man)
RUN apt-get update \
  && apt-get install -qqy netcat net-tools s3cmd yarn \
  && rm -rf /var/lib/apt/lists/*

# Change TimeZone
# ENV TZ=America/Los_Angeles
# RUN ln -sf /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
# RUN echo "America/Los_Angeles" > /etc/timezone
# RUN date

# From here we load our application's code in, therefore the previous docker
# "layer" thats been cached will be used if possible
WORKDIR $PROJECT_ROOT
ADD . $PROJECT_ROOT

# expose app port
# EXPOSE 3000
# EXPOSE 3333
# EXPOSE 9333

# Build
RUN ./btcputil/build.sh -j$(nproc)
# Fetch Zcash ceremony keys
RUN ./btcputil/fetch-params.sh
# Run
CMD ["./src/btcpd"]

RELEASE=8.1.6
docker build -t oxend:${RELEASE} -f Dockerfile.oxend --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) .

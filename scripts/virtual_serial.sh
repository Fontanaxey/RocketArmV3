#!/bin/bash

# /tmp/ttyV0 for C tests
# /tmp/ttyV1 will be used by the py test script
socat -d -d PTY,link=/tmp/ttyV0,raw,echo=0 PTY,link=/tmp/ttyV1,raw,echo=0
#!/bin/bash

LIGHT_CYAN='\033[1;36m'
NC='\033[0m'

# Build twmailer-client
echo -e "${LIGHT_CYAN}Build twmailer-client${NC}"
(cd ./client && make clean && make)

echo ''

# Build twmailer-server
echo -e "${LIGHT_CYAN}Build twmailer-server${NC}"
(cd ./server && make clean && make)
#!/bin/bash

LIGHT_PURPLE='\033[1;35m'
NC='\033[0m'

# Clean twmailer-client
echo -e "${LIGHT_PURPLE}Clean twmailer-client${NC}"
(cd ./client && make clean)

echo ''

# Clean twmailer-server
echo -e "${LIGHT_PURPLE}Clean twmailer-server${NC}"
(cd ./server && make clean)
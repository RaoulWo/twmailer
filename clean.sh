#!/bin/bash

# Clean twmailer-client
echo 'Clean twmailer-client'
(cd ./client && make clean)

echo ''

# Clean twmailer-server
echo 'Clean twmailer-server'
(cd ./server && make clean)
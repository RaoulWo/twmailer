#!/bin/bash

# Build twmailer-client
echo 'Build twmailer-client'
(cd ./client && make)
echo ''

# Build twmailer-server
echo 'Build twmailer-server'
(cd ./server && make)
echo ''
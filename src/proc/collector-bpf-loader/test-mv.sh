#!/bin/bash

echo "$(date)" > ./test.log.a
mv ./test.log.a ./test.log.b
rm ./test.log.b

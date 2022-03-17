#!/usr/bin/bash

rm -f Pixie16Msg.log

./testing/example $1 --config=testing/example_config.json 

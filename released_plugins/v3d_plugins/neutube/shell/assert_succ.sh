#!/bin/bash
 
function assert_succ {
    if [ $? -ne 0 ]
    then
	echo ':( Mission failed.'
	echo 'a)<'
	exit 1;
    fi
}
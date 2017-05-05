#!/usr/bin/env zsh

launch_test(){
	echo "cleaning ..."
	make clean
	echo "build ...."
	make
	GST_DEBUG_DUMP_DOT_DIR=./dots/ GST_DEBUG=4 ./test /home/niaba/Documents 0

}

dot2svg(){
    dot -Tsvg $1 > pipeline.svg
}

cleandots(){
    rm ./dots/*.dot
}
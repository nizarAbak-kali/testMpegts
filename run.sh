#!/usr/bin/env zsh

LOCATION=~/Documents/test/testMpegts/


launch_test(){
	echo "cleaning ..."
	make clean
	cleandots
	echo "build ...."
	make
	GST_DEBUG_DUMP_DOT_DIR=./dots/ GST_DEBUG=2,tsdemux:6 ./test /home/niaba/Documents 2
    echo "dots 2 svg ..."
    alldots2svg
}

dot2svg(){
    IMAGE_NAME=pipeline.svg
    dot -Tsvg $1 > $IMAGE_NAME
    eog $IMAGE_NAME &

}

alldots2svg(){
    LOCATION=~/Documents/test/testMpegts/
    python $LOCATION/creates_graphs.py
}

cleandots(){
    LOCATION=~/Documents/test/testMpegts/dots/
    rm $LOCATION/*.dot
}
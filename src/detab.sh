#!/bin/sh

TMPFILE=foo42

if [ -f $TMPFILE ] ; then
    echo "Weird...the temp file $TMPFILE already exists.  Get rid of it."
    exit 0
fi

while [ $# -ne 0 ] ; do
    expand -t8 < $1 > $TMPFILE
    if [ $? -eq 0 ] ; then
        mv $TMPFILE $1
        echo Successfully removed the tabs from $1.
    fi
    shift
done

rm -f $TMPFILE


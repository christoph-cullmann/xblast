#!/bin/sh
autoheader
aclocal -I m4
autoconf
automake --gnu --add-missing --copy 
if [ -s config.cache ] ; then 
    rm config.cache;  
fi


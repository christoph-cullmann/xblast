#!/bin/sh
autoheader
aclocal
autoconf
automake --gnu --add-missing --copy 
if [ -s config.cache ] ; then 
    rm config.cache;  
fi


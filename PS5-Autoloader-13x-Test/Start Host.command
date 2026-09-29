#!/bin/zsh
cd -- "${0:A:h}" || exit 1
print 'Experimental PS5 Autoloader 13.x host'
print 'Keep this window open while testing. Press Control-C to stop.'
print 'Enter your Mac login password if prompted (typing stays hidden).'
sudo /Library/Frameworks/Python.framework/Versions/3.12/bin/python3 ./webkit-autoloader-host_v0.4.0-relapse-13x.py --no-update-check --http-port 8080
print '
Host stopped. Press Return to close.'
read

# cadir (build supports debian 9/10)
If you suffer from many builds requesting the same vendor libraries
than cadir can help! It works e.g. with composer or npm and all
other package manager that have a file to identiy the current vendor 
package versions (like composer.lock or package-lock.json). cadir will 
also run the command for installing vendors. 

## Installation
Checkout repository

    git clone https://github.com/dirkwinkhaus/cadir.git

Build

    cd cadir
    make
    
Move or link the executable to somewhere the system finds it.

## Usage
Lets say you have a build server. On this each branch is built many times 
and the vendors will change not often. Here an example for a project with 
npm. To see which vendors in which version are needed the identity file is
"package-lock.json". On your build server you create the directory "vendorCache" 
in "/tmp". So here is the example:

    cadir --cache-source="vendor" --identity-file="composer.lock" --cache-destination="/tmp/vendorCache" --command-working-directory=`pwd` --setup="composer install --ignore-platform-reqs --no-cache --no-interaction" --finalize="composer dump-autoload --no-interaction" --verbose
   
cadir will check if a directory in the cache folder exists which has a name 
equal to the checksum of "package-lock.json". If not, it will the command in 
the working dorectory for the command which installs the npm dependencies and 
copy it to the cache folder. If cadir finds a cached copy with a fitting name
than it will copy or link it to the projekt folder.

## Return values
    0 = Successfully executed
    1 = Wrong usage of arguments
    2 = Identity file error (not found/no rights)
    3 = Setup command failed
    4 = Finalize command failed
    5 = Cannot copy to cache directoy
    6 = Cannot copy from cache directoy
    7 = Cannot create link from cache
    8 = Removing existing cache folder failed
    9 = Cannot create cache directories

## Compiling without cmake
Run

    c++ -Wall main.cpp -o cadir -lcrypto -lssl -std=c++17 -lstdc++fs
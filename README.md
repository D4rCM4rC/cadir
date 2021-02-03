# cadir
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
in "/tmp". So here is the example. The help of cadir which is displayed when less 
than 5 arguments are used shows you this:

    cadir [DIRECTORY_TO_BE_CACHED] [FILE_FOR_IDENTIFY_CACHE] [DIRECTORY_FOR_CACHES] [COMMAND_WORKING_DIRECTORY] [COMMAND]

So in your case this call will enable the directory cache

    cadir /home/my-project/current/node_modules /home/my-project/current/package-lock.json /tmp/vendorCache /home/my-project/current "npm ci" 
   
cadir will check if a directory in the cache folder exists which has a name 
equal to the checksum of "package-lock.json". If not, it will the command in 
the working dorectory for the command which installs the npm dependencies and 
copy it to the cache folder. If cadir finds a cached copy with a fitting name
than it will copy or *link it to the projekt folder.

## *Still to come
Some features are not included at present. This will follow:

- optional linking files from cache
- optional verbose output 
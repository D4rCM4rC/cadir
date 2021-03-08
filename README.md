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

    cadir cadir --cache-source="test/source/vendor" --identity-file="test/source/composer.lock" --cache-destination="test/cache" --command-working-directory="test/source/" --command="composer install --ignore-platform-reqs" --finalize-command="composer dump-autoload" --verbose"
   
cadir will check if a directory in the cache folder exists which has a name 
equal to the checksum of "package-lock.json". If not, it will the command in 
the working dorectory for the command which installs the npm dependencies and 
copy it to the cache folder. If cadir finds a cached copy with a fitting name
than it will copy or link it to the projekt folder.
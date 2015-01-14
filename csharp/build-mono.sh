#!/usr/bin/env bash
###################


# install required packages (well, for APT systems)
sudo apt-get -y install mono-complete nunit-console

# Use snowball to generate .cs files
./generate.sh

# Build the .NET solution
xbuild /p:Configuration=Release Snowball.sln

# Run automatic tests (needs nunit to be installed)
nunit-console Unit\ Tests/bin/Release/Unit\ Tests.dll

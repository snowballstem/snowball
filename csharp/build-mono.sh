# install required packages
sudo apt-get -y install mono-complete nunit-console

# Use snowball to generate .cs files
sh generate.sh

# Build the .NET solution
xbuild /p:Configuration=Release Snowball.sln

# Run automatic tests (needs nunit installed)
# apt-get install nunit-console
nunit-console Unit\ Tests/bin/Release/Unit\ Tests.dll

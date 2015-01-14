# install required packages
apt-get install mono-complete nunit-console

# Use snowball to generate .cs files
./generate

# Build the .NET solution
xbuild /p:Configuration=Release Snowball.sln

# Run automatic tests (needs nunit installed)
# apt-get install nunit-console
nunit-console Unit\ Tests/bin/Release/Unit\ Tests.dll

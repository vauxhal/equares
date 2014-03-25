call prep-vars.bat
cmd /k mongod.exe --dbpath data --setParameter textSearchEnabled=true

# Equares

## About

Equares is a tool for numeric analysis of ordinary differential equations.
The tool is available online at [equares.ctmech.ru](http://equares.ctmech.ru).

## Motivation

We have formulated our requirements as follows:
* Speed matters. Equares performs faster than
  [MATLAB](http://www.mathworks.com/products/matlab/),
  [SciLab](http://www.scilab.org/), and the like.
* Freedom matters. Equares is free software. Thus, everyone can extend its functionality.
* Ease of use matters.
  * User should be able to use it easily without having to do advanced training.
  * User shouldn't have to install a ton of software to get it all work.

Fast basically means implementation of your custom equations in a native language, e.g. C++. So our system needs a compiler to compile your equations.
But we don't want to force user to install a compiler! That's why Equares is an online system: we install everything on the server, you just use it.


## Architecture

Equares is built of the following components
* Core
  * ```equares_core``` (C++): library providing the computational functionality
  * ```equares_con``` (C++): console front-end to ```equares_core```
* Web interface
  * ```equares_http``` (node.js): HTTP-server providing the Web interface to Equares
* Utilities
  * ```captchagen``` (C++): captcha generator
  * ```imgresize``` (C++): image resizer

## Technology
Technologies involved are as follows
* C++ code is using [Qt](http://qt-project.org/)
* Server code is written in JavaScript and is run by [node.js](http://nodejs.org/). There are a number of modules that are required to run the server app
  *  ```line```
  *  ```express```
  *  ```jade```
  *  ```mongodb```
  *  ```mongoose```
  *  ```mongoose```
  *  ```connect```
  *  ```passport```
  *  ```passport```
  *  ```connect```
  *  ```marked```
  *  ```nodemailer```
  *  ```activator```
* Besides, running server app requires the [mongoDB](http://www.mongodb.org/) database engine

## Building
To build the Equares core, you will need QtCreator (see [qt-project.org](http://qt-project.org/).

Project file to open with QtCreator is ```equares.pro```.
Optionally, you can build project using the command line:
<pre>
mkdir build-equares-release
cd build-equares-release
qmake CONFIG+=release ../equares/equares.pro
make
<pre>

Nothing more needs to be built

## Running
To run equares http server, you will need to install
* [node.js](http://nodejs.org/)
* [npm](https://www.npmjs.org/)
* [mongoDB](http://www.mongodb.org/)
* node modules mentioned above; this is done with the command ```npm install``` issued from the ```equares_http``` subdirectory.

The EQUARES_BIN environment variable must be set to point to the directory containing Equares core binaries, e.g. (on Linux, prvided
the build root is ```$HOME/build-equares-release```),
<pre>
export EQUARES_BIN=$HOME/build-equares-release/bin
</pre>
On Linux, you will likely need to set the ```LD_LIBRARY_PATH``` environment variable to the same directory:
<pre>
export LD_LIBRARY_PATH=$EQUARES_BIN
</pre>

Before running the server, you should also do these things.
* Make sure mongodb server is running on localhost.
* Provide the file ```equares_http/email-settings.json``` (consider example file ```equares_http/email-settings-example.json```).

The server is started with the command
<pre>
node app.js
</pre>
issued in the ```equares_http``` subdirectory.

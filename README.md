# Equares

## About

Equares is a tool for numeric analysis of ordinary differential equations

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
TODO

## Running
TODO


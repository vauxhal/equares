# Equares

## About

Equares is a tool for numeric analysis of ordinary differential equations

## Motivation
TODO

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

